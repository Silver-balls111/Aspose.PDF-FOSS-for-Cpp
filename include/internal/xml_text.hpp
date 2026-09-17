#pragma once

#include <string>

namespace foundation::xml_text {

// Escape/decode helpers shared by the lightweight XML readers and
// writers in the facades (XFDF annotation import, bookmark XML/HTML
// export + import). These facades deliberately handle small,
// well-formed fragments with string scanning — a full XML parser is
// out of scope for v1 — so the escaping rules live here instead of
// being re-implemented per facade.

// Escape for element text / HTML body content.
inline std::string EscapeXmlText(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            default: out += c; break;
        }
    }
    return out;
}

// Escape for double-quoted attribute values (adds &quot; over text).
inline std::string EscapeXmlAttr(const std::string& text) {
    if (text.find('"') == std::string::npos) return EscapeXmlText(text);
    std::string out = EscapeXmlText(text);
    std::string replaced;
    replaced.reserve(out.size());
    for (char c : out) {
        if (c == '"') replaced += "&quot;";
        else replaced += c;
    }
    return replaced;
}

// Append the UTF-8 encoding of a scalar codepoint.
inline void AppendUtf8(unsigned int code, std::string& out) {
    if (code < 0x80) {
        out += static_cast<char>(code);
    } else if (code < 0x800) {
        out += static_cast<char>(0xC0 | (code >> 6));
        out += static_cast<char>(0x80 | (code & 0x3F));
    } else if (code < 0x10000) {
        out += static_cast<char>(0xE0 | (code >> 12));
        out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (code & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (code >> 18));
        out += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (code & 0x3F));
    }
}

// Decode the five predefined XML entities plus decimal/hex numeric
// character references. Codepoints >= 128 are emitted as UTF-8;
// malformed, zero, surrogate, or beyond-Unicode references are left
// literal in the output.
inline std::string DecodeXmlEntities(const std::string& str) {
    std::string out;
    out.reserve(str.size());
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (str[i] != '&') {
            out += str[i];
            continue;
        }
        const std::size_t semi = str.find(';', i);
        if (semi == std::string::npos || semi - i >= 10) {
            out += str[i];
            continue;
        }
        const std::string ent = str.substr(i + 1, semi - i - 1);
        if (ent == "amp") { out += '&'; i = semi; continue; }
        if (ent == "lt") { out += '<'; i = semi; continue; }
        if (ent == "gt") { out += '>'; i = semi; continue; }
        if (ent == "quot") { out += '"'; i = semi; continue; }
        if (ent == "apos") { out += '\''; i = semi; continue; }
        if (ent.size() > 1 && ent[0] == '#') {
            const bool hex = ent[1] == 'x' || ent[1] == 'X';
            std::size_t consumed = 0;
            unsigned long code = 0;
            try {
                code = std::stoul(ent.substr(hex ? 2u : 1u), &consumed,
                                  hex ? 16 : 10);
            } catch (...) {
                out += str[i];
                continue;
            }
            const bool digitsConsumed = consumed == ent.size() - (hex ? 2u : 1u);
            const bool inRange = code > 0 && code <= 0x10FFFF &&
                                 (code < 0xD800 || code > 0xDFFF);
            if (digitsConsumed && inRange) {
                AppendUtf8(static_cast<unsigned int>(code), out);
                i = semi;
                continue;
            }
        }
        out += str[i];
    }
    return out;
}

// Value of `attrName` inside a start-tag header such as
// `<square page="0" rect="..." />`. A match only counts when the name
// is preceded by an attribute boundary (whitespace or the tag-opening
// '<'), so looking up "page" does not match inside `subpage="1"`.
// Double- then single-quoted forms are accepted; the returned value is
// entity-decoded. Returns "" when the attribute is absent.
inline std::string FindAttrValue(const std::string& tag,
                                 const std::string& attrName) {
    const std::string needleD = attrName + "=\"";
    const std::string needleS = attrName + "='";
    std::size_t from = 0;
    while (true) {
        const std::size_t posD = tag.find(needleD, from);
        const std::size_t posS = tag.find(needleS, from);
        if (posD == std::string::npos && posS == std::string::npos) return "";

        std::size_t pos;
        char quote;
        const std::string* needle;
        if (posS == std::string::npos ||
            (posD != std::string::npos && posD < posS)) {
            pos = posD;
            quote = '"';
            needle = &needleD;
        } else {
            pos = posS;
            quote = '\'';
            needle = &needleS;
        }

        const bool boundary =
            pos == 0 || tag[pos - 1] == ' ' || tag[pos - 1] == '\t' ||
            tag[pos - 1] == '\r' || tag[pos - 1] == '\n' ||
            tag[pos - 1] == '<';
        if (boundary) {
            const std::size_t valueStart = pos + attrName.size() + 2;
            const std::size_t end = tag.find(quote, valueStart);
            if (end != std::string::npos) {
                return DecodeXmlEntities(
                    tag.substr(valueStart, end - valueStart));
            }
        }
        from = pos + 1;
    }
}

}  // namespace foundation::xml_text
