#pragma once

// =============================================================================
// Aspose::Pdf::Text::Encoding — charset codec, the cpp substitute for
// BCL System.Text.Encoding. Five fixed charsets in v1: utf-8,
// utf-16-le, utf-16-be, latin-1, windows-1252. See the project spec
// for the full BCL contract.
//
// Replacement-char policy: replace mode (no throw). Encode-side
// produces 0x3F ('?') for unmappable codepoints; decode-side
// produces U+FFFD for malformed byte sequences.
//
// Public because TextDevice takes an Encoding; library internals
// reach it through the foundation::encoding alias in
// include/internal/encoding.hpp.
// =============================================================================

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Aspose::Pdf::Text {

class Encoding {
 public:
    static const Encoding& UTF8() noexcept;
    static const Encoding& Unicode() noexcept;          // UTF-16-LE
    static const Encoding& BigEndianUnicode() noexcept; // UTF-16-BE
    static const Encoding& Latin1() noexcept;
    static const Encoding& Windows1252() noexcept;

    static const Encoding& GetEncoding(std::string_view name);

    std::vector<std::byte> GetBytes(std::string_view text) const;
    std::string GetString(std::span<const std::byte> bytes) const;
    int GetByteCount(std::string_view text) const;
    int GetCharCount(std::span<const std::byte> bytes) const;

    std::string_view WebName() const noexcept;
    int CodePage() const noexcept;
    std::span<const std::byte> Preamble() const noexcept;

    friend bool operator==(const Encoding& a, const Encoding& b)
        noexcept { return &a == &b; }
    friend bool operator!=(const Encoding& a, const Encoding& b)
        noexcept { return &a != &b; }

    Encoding(const Encoding&)            = delete;
    Encoding(Encoding&&)                 = delete;
    Encoding& operator=(const Encoding&) = delete;
    Encoding& operator=(Encoding&&)      = delete;

 private:
    enum class Kind {
        UTF8, UTF16LE, UTF16BE, LATIN1, WINDOWS1252,
    };
    constexpr explicit Encoding(Kind k) noexcept : kind_(k) {}
    Kind kind_;
};

}  // namespace Aspose::Pdf::Text
