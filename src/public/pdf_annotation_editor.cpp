#include <aspose/pdf/facades/pdf_annotation_editor.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>

#include <aspose/pdf/annotations/annotation.hpp>
#include <aspose/pdf/annotations/annotation_collection.hpp>
#include <aspose/pdf/annotations/circle_annotation.hpp>
#include <aspose/pdf/annotations/highlight_annotation.hpp>
#include <aspose/pdf/annotations/markup_annotation.hpp>
#include <aspose/pdf/annotations/square_annotation.hpp>
#include <aspose/pdf/annotations/text_annotation.hpp>
#include <aspose/pdf/document.hpp>
#include <aspose/pdf/page.hpp>
#include <aspose/pdf/page_collection.hpp>

#include <aspose/pdf/annotations/free_text_annotation.hpp>
#include <aspose/pdf/annotations/line_annotation.hpp>
#include <aspose/pdf/annotations/strike_out_annotation.hpp>
#include <aspose/pdf/annotations/underline_annotation.hpp>
#include <optional>

namespace Aspose::Pdf::Facades {

using namespace Aspose::Pdf::Annotations;

PdfAnnotationEditor::PdfAnnotationEditor(Aspose::Pdf::Document& document) {
    BindPdf(document);
}

namespace {

std::string DecodeXmlEntities(const std::string& str) {
    std::string out;
    out.reserve(str.size());
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '&') {
            auto semi = str.find(';', i);
            if (semi != std::string::npos && semi - i < 10) {
                std::string ent = str.substr(i + 1, semi - i - 1);
                if (ent == "amp") { out += '&'; i = semi; continue; }
                if (ent == "lt") { out += '<'; i = semi; continue; }
                if (ent == "gt") { out += '>'; i = semi; continue; }
                if (ent == "quot") { out += '"'; i = semi; continue; }
                if (ent == "apos") { out += '\''; i = semi; continue; }
                if (!ent.empty() && ent[0] == '#') {
                    int code = 0;
                    if (ent.size() > 1 && (ent[1] == 'x' || ent[1] == 'X')) {
                        try { code = std::stoi(ent.substr(2), nullptr, 16); } catch (...) {}
                    } else {
                        try { code = std::stoi(ent.substr(1)); } catch (...) {}
                    }
                    if (code > 0 && code < 128) {
                        out += static_cast<char>(code);
                        i = semi;
                        continue;
                    }
                }
            }
        }
        out += str[i];
    }
    return out;
}

std::optional<Rectangle> ParseRect(const std::string& str) {
    if (str.empty()) return std::nullopt;
    std::stringstream ss(str);
    std::string item;
    std::vector<double> vals;
    while (std::getline(ss, item, ',')) {
        try {
            vals.push_back(std::stod(item));
        } catch (...) {
            return std::nullopt;
        }
    }
    if (vals.size() >= 4) {
        return Rectangle(vals[0], vals[1], vals[2], vals[3], true);
    }
    return std::nullopt;
}

std::string GetAttr(const std::string& tag, const std::string& attrName) {
    auto pos = tag.find(attrName + "=\"");
    if (pos == std::string::npos) {
        pos = tag.find(attrName + "='");
        if (pos == std::string::npos) return "";
        pos += attrName.size() + 2;
        auto end = tag.find("'", pos);
        if (end == std::string::npos) return "";
        return DecodeXmlEntities(tag.substr(pos, end - pos));
    }
    pos += attrName.size() + 2;
    auto end = tag.find("\"", pos);
    if (end == std::string::npos) return "";
    return DecodeXmlEntities(tag.substr(pos, end - pos));
}

}  // namespace

// ===== Import (XFDF) =========================================================

void PdfAnnotationEditor::ImportAnnotationsFromXfdf(const std::string& xfdfFile) {
    ImportAnnotationFromXfdf(xfdfFile, {});
}

void PdfAnnotationEditor::ImportAnnotationsFromFdf(const std::string& /*fdfFile*/) {
    // FDF uses PDF COS syntax, not XML/XFDF. FDF parsing is not supported in v1 FOSS.
}

void PdfAnnotationEditor::ImportAnnotationFromXfdf(const std::string& xfdfFile) {
    ImportAnnotationFromXfdf(xfdfFile, {});
}

void PdfAnnotationEditor::ImportAnnotationFromXfdf(
    const std::string& xfdfFile,
    const std::vector<Aspose::Pdf::Annotations::AnnotationType>& annotTypes) {
    if (document_ == nullptr) return;
    std::ifstream in(xfdfFile);
    if (!in.is_open()) return;

    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

    auto& pages = document_->Pages();
    const int pageCount = static_cast<int>(pages.Count());

    auto isTypeAllowed = [&](AnnotationType type) {
        if (annotTypes.empty()) return true;
        return std::find(annotTypes.begin(), annotTypes.end(), type) != annotTypes.end();
    };

    auto processTags = [&](const std::string& tagType, AnnotationType mappedType) {
        if (!isTypeAllowed(mappedType)) return;
        size_t pos = 0;
        std::string openTag = "<" + tagType;
        while ((pos = content.find(openTag, pos)) != std::string::npos) {
            // Ensure tag delimiter match (e.g. not matching <textsomething)
            if (pos + openTag.size() < content.size()) {
                char nextChar = content[pos + openTag.size()];
                if (nextChar != ' ' && nextChar != '>' && nextChar != '/' && nextChar != '\t' && nextChar != '\n' && nextChar != '\r') {
                    pos += openTag.size();
                    continue;
                }
            }
            auto closePos = content.find(">", pos);
            if (closePos == std::string::npos) break;
            std::string tagHeader = content.substr(pos, closePos - pos + 1);

            int page = 0;
            std::string pageStr = GetAttr(tagHeader, "page");
            if (!pageStr.empty()) {
                try { page = std::stoi(pageStr); } catch (...) {}
            }
            int pageNum = page + 1; // XFDF is 0-based, Aspose is 1-based
            if (pageNum < 1 || pageNum > pageCount) pageNum = 1;

            auto rectOpt = ParseRect(GetAttr(tagHeader, "rect"));
            if (!rectOpt.has_value()) {
                pos = closePos + 1;
                continue;
            }
            Rectangle rect = *rectOpt;
            std::string title = GetAttr(tagHeader, "title");
            std::string contents = GetAttr(tagHeader, "contents");

            bool isSelfClosing = (closePos > 0 && content[closePos - 1] == '/');
            if (!isSelfClosing) {
                std::string endTag = "</" + tagType + ">";
                auto endPos = content.find(endTag, closePos);
                if (endPos != std::string::npos) {
                    std::string body = content.substr(closePos + 1, endPos - closePos - 1);
                    auto cstart = body.find("<contents>");
                    if (cstart != std::string::npos) {
                        cstart += 10;
                        auto cend = body.find("</contents>", cstart);
                        if (cend != std::string::npos) {
                            contents = DecodeXmlEntities(body.substr(cstart, cend - cstart));
                        }
                    } else {
                        auto rstart = body.find("<contents-richtext>");
                        if (rstart != std::string::npos) {
                            rstart += 19;
                            auto rend = body.find("</contents-richtext>", rstart);
                            if (rend != std::string::npos) {
                                contents = DecodeXmlEntities(body.substr(rstart, rend - rstart));
                            }
                        }
                    }
                    pos = endPos + endTag.size();
                } else {
                    pos = closePos + 1;
                }
            } else {
                pos = closePos + 1;
            }

            Page pg = pages[pageNum];
            std::unique_ptr<Annotation> annot;
            if (tagType == "square") {
                annot = std::make_unique<SquareAnnotation>(pg, rect);
            } else if (tagType == "circle") {
                annot = std::make_unique<CircleAnnotation>(pg, rect);
            } else if (tagType == "text") {
                annot = std::make_unique<TextAnnotation>(pg, rect);
            } else if (tagType == "highlight") {
                annot = std::make_unique<HighlightAnnotation>(pg, rect);
            } else if (tagType == "freetext") {
                annot = std::make_unique<FreeTextAnnotation>(pg, rect, DefaultAppearance{});
            } else if (tagType == "underline") {
                annot = std::make_unique<UnderlineAnnotation>(pg, rect);
            } else if (tagType == "strikeout") {
                annot = std::make_unique<StrikeOutAnnotation>(pg, rect);
            } else if (tagType == "line") {
                annot = std::make_unique<LineAnnotation>(
                    pg, rect, Point(rect.LLX(), rect.LLY()), Point(rect.URX(), rect.URY()));
            }

            if (annot != nullptr) {
                if (!contents.empty()) annot->Contents(contents);
                if (!title.empty()) {
                    if (auto* ma = dynamic_cast<MarkupAnnotation*>(annot.get())) {
                        ma->Title(title);
                    }
                }
                pg.Annotations().Add(std::move(annot));
            }
        }
    };

    processTags("square", AnnotationType::Square);
    processTags("circle", AnnotationType::Circle);
    processTags("text", AnnotationType::Text);
    processTags("highlight", AnnotationType::Highlight);
    processTags("freetext", AnnotationType::FreeText);
    processTags("underline", AnnotationType::Underline);
    processTags("strikeout", AnnotationType::StrikeOut);
    processTags("line", AnnotationType::Line);
}

void PdfAnnotationEditor::ImportAnnotations(
    const std::vector<std::string>& annotFiles,
    const std::vector<Aspose::Pdf::Annotations::AnnotationType>& annotTypes) {
    for (const auto& f : annotFiles) ImportAnnotationFromXfdf(f, annotTypes);
}
void PdfAnnotationEditor::ImportAnnotations(
    const std::vector<std::string>& annotFiles) {
    for (const auto& f : annotFiles) ImportAnnotationFromXfdf(f, {});
}

// ===== Modify ================================================================

void PdfAnnotationEditor::ModifyAnnotations(
    int start, int end, Aspose::Pdf::Annotations::Annotation& annotation) {
    if (document_ == nullptr) return;
    auto& pages = document_->Pages();
    const int count = static_cast<int>(pages.Count());
    int s = std::max(1, start);
    int e = (end <= 0 || end > count) ? count : end;
    for (int p = s; p <= e; ++p) {
        auto& annots = pages[p].Annotations();
        for (int i = 0; i < annots.Count(); ++i) {
            if (!annotation.Contents().empty()) {
                annots[i].Contents(annotation.Contents());
            }
            if (annotation.Color().A() > 0) {
                annots[i].Color(annotation.Color());
            }
        }
    }
}

void PdfAnnotationEditor::ModifyAnnotationsAuthor(
    int start, int end, const std::string& srcAuthor, const std::string& desAuthor) {
    if (document_ == nullptr) return;
    auto& pages = document_->Pages();
    const int count = static_cast<int>(pages.Count());
    int s = std::max(1, start);
    int e = (end <= 0 || end > count) ? count : end;
    for (int p = s; p <= e; ++p) {
        auto& annots = pages[p].Annotations();
        for (int i = 0; i < annots.Count(); ++i) {
            auto& a = annots[i];
            if (auto* ma = dynamic_cast<Aspose::Pdf::Annotations::MarkupAnnotation*>(&a)) {
                if (srcAuthor.empty() || ma->Title() == srcAuthor) {
                    ma->Title(desAuthor);
                }
            }
        }
    }
}

// ===== Flatten ===============================================================

void PdfAnnotationEditor::FlatteningAnnotations() {
    DeleteAnnotations();
}
void PdfAnnotationEditor::FlatteningAnnotations(
    const Aspose::Pdf::Forms::Form::FlattenSettings&) {
    DeleteAnnotations();
}
void PdfAnnotationEditor::FlatteningAnnotations(
    int start, int end,
    const std::vector<Aspose::Pdf::Annotations::AnnotationType>& annotType) {
    if (document_ == nullptr) return;
    auto& pages = document_->Pages();
    const int count = static_cast<int>(pages.Count());
    int s = std::max(1, start);
    int e = (end <= 0 || end > count) ? count : end;
    for (int p = s; p <= e; ++p) {
        auto& annots = pages[p].Annotations();
        for (int i = annots.Count() - 1; i >= 0; --i) {
            auto t = annots[i].AnnotationType();
            for (auto target : annotType) {
                if (t == target) {
                    annots.Delete(i);
                    break;
                }
            }
        }
    }
}

// ===== Delete ================================================================

void PdfAnnotationEditor::DeleteAnnotations() {
    if (document_ == nullptr) {
        return;
    }
    Aspose::Pdf::PageCollection& pages = document_->Pages();
    const int count = static_cast<int>(pages.Count());
    for (int i = 1; i <= count; ++i) {
        pages[i].Annotations().Delete();
    }
}

namespace {

// Canonical annotation-type name (AnnotationType.ToString()) for the
// delete-by-type filter.
const char* AnnotTypeName(Aspose::Pdf::Annotations::AnnotationType t) {
    using AT = Aspose::Pdf::Annotations::AnnotationType;
    switch (t) {
        case AT::Text:           return "Text";
        case AT::Circle:         return "Circle";
        case AT::Polygon:        return "Polygon";
        case AT::PolyLine:       return "PolyLine";
        case AT::Line:           return "Line";
        case AT::Square:         return "Square";
        case AT::FreeText:       return "FreeText";
        case AT::Highlight:      return "Highlight";
        case AT::Underline:      return "Underline";
        case AT::StrikeOut:      return "StrikeOut";
        case AT::Squiggly:       return "Squiggly";
        case AT::Link:           return "Link";
        case AT::Ink:            return "Ink";
        case AT::Stamp:          return "Stamp";
        case AT::Caret:          return "Caret";
        case AT::Redaction:      return "Redaction";
        case AT::Watermark:      return "Watermark";
        case AT::FileAttachment: return "FileAttachment";
        case AT::Popup:          return "Popup";
        case AT::Widget:         return "Widget";
        default:                 return "";
    }
}

}  // namespace

void PdfAnnotationEditor::DeleteAnnotations(const std::string& annotType) {
    if (document_ == nullptr) {
        return;
    }
    Aspose::Pdf::PageCollection& pages = document_->Pages();
    const int count = static_cast<int>(pages.Count());
    for (int p = 1; p <= count; ++p) {
        auto& annots = pages[p].Annotations();
        for (int i = annots.Count() - 1; i >= 0; --i) {
            if (annotType == AnnotTypeName(annots[i].AnnotationType())) {
                annots.Delete(i);
            }
        }
    }
}

void PdfAnnotationEditor::DeleteAnnotation(const std::string& annotName) {
    if (document_ == nullptr) {
        return;
    }
    Aspose::Pdf::PageCollection& pages = document_->Pages();
    const int pageCount = static_cast<int>(pages.Count());
    for (int p = 1; p <= pageCount; ++p) {
        Aspose::Pdf::Page page = pages[p];
        Aspose::Pdf::Annotations::AnnotationCollection& annots =
            page.Annotations();
        const int annotCount = annots.Count();
        // AnnotationCollection is 0-based (cf. PageCollection's
        // 1-based indexing).
        for (int a = 0; a < annotCount; ++a) {
            if (annots[a].Name() == annotName) {
                annots.Delete(a);
                return;
            }
        }
    }
}

}  // namespace Aspose::Pdf::Facades
