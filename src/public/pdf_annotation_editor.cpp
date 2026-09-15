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

namespace Aspose::Pdf::Facades {

using namespace Aspose::Pdf::Annotations;

PdfAnnotationEditor::PdfAnnotationEditor(Aspose::Pdf::Document& document) {
    BindPdf(document);
}

namespace {

Rectangle ParseRect(const std::string& str) {
    std::stringstream ss(str);
    std::string item;
    std::vector<double> vals;
    while (std::getline(ss, item, ',')) {
        try {
            vals.push_back(std::stod(item));
        } catch (...) {}
    }
    if (vals.size() >= 4) {
        return Rectangle(vals[0], vals[1], vals[2], vals[3], true);
    }
    return Rectangle(0, 0, 100, 100, true);
}

std::string GetAttr(const std::string& tag, const std::string& attrName) {
    auto pos = tag.find(attrName + "=\"");
    if (pos == std::string::npos) return "";
    pos += attrName.size() + 2;
    auto end = tag.find("\"", pos);
    if (end == std::string::npos) return "";
    return tag.substr(pos, end - pos);
}

}  // namespace

// ===== Import (XFDF) =========================================================

void PdfAnnotationEditor::ImportAnnotationsFromXfdf(const std::string& xfdfFile) {
    if (document_ == nullptr) return;
    std::ifstream in(xfdfFile);
    if (!in.is_open()) return;

    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

    auto& pages = document_->Pages();
    const int pageCount = static_cast<int>(pages.Count());

    auto processTags = [&](const std::string& tagType) {
        size_t pos = 0;
        std::string openTag = "<" + tagType;
        while ((pos = content.find(openTag, pos)) != std::string::npos) {
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

            Rectangle rect = ParseRect(GetAttr(tagHeader, "rect"));
            std::string title = GetAttr(tagHeader, "title");
            std::string contents = GetAttr(tagHeader, "contents");

            Page pg = pages[pageNum];
            if (tagType == "square") {
                auto sq = std::make_unique<SquareAnnotation>(pg, rect);
                if (!contents.empty()) sq->Contents(contents);
                if (!title.empty()) sq->Title(title);
                pg.Annotations().Add(*sq);
                owned_annotations_.push_back(std::move(sq));
            } else if (tagType == "circle") {
                auto ci = std::make_unique<CircleAnnotation>(pg, rect);
                if (!contents.empty()) ci->Contents(contents);
                if (!title.empty()) ci->Title(title);
                pg.Annotations().Add(*ci);
                owned_annotations_.push_back(std::move(ci));
            } else if (tagType == "text") {
                auto txt = std::make_unique<TextAnnotation>(pg, rect);
                if (!contents.empty()) txt->Contents(contents);
                if (!title.empty()) txt->Title(title);
                pg.Annotations().Add(*txt);
                owned_annotations_.push_back(std::move(txt));
            } else if (tagType == "highlight") {
                auto hl = std::make_unique<HighlightAnnotation>(pg, rect);
                if (!contents.empty()) hl->Contents(contents);
                if (!title.empty()) hl->Title(title);
                pg.Annotations().Add(*hl);
                owned_annotations_.push_back(std::move(hl));
            }
            pos = closePos + 1;
        }
    };

    processTags("square");
    processTags("circle");
    processTags("text");
    processTags("highlight");
}

void PdfAnnotationEditor::ImportAnnotationsFromFdf(const std::string& fdfFile) {
    ImportAnnotationsFromXfdf(fdfFile);
}
void PdfAnnotationEditor::ImportAnnotationFromXfdf(const std::string& xfdfFile) {
    ImportAnnotationsFromXfdf(xfdfFile);
}
void PdfAnnotationEditor::ImportAnnotationFromXfdf(
    const std::string& xfdfFile,
    const std::vector<Aspose::Pdf::Annotations::AnnotationType>&) {
    ImportAnnotationsFromXfdf(xfdfFile);
}
void PdfAnnotationEditor::ImportAnnotations(
    const std::vector<std::string>& annotFiles,
    const std::vector<Aspose::Pdf::Annotations::AnnotationType>&) {
    for (const auto& f : annotFiles) ImportAnnotationsFromXfdf(f);
}
void PdfAnnotationEditor::ImportAnnotations(
    const std::vector<std::string>& annotFiles) {
    for (const auto& f : annotFiles) ImportAnnotationsFromXfdf(f);
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
