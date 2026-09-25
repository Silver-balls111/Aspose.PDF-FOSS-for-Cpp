#include <aspose/pdf/facades/pdf_bookmark_editor.hpp>

#include <fstream>
#include <sstream>

#include <aspose/pdf/document.hpp>

#include "xml_text.hpp"

namespace Aspose::Pdf::Facades {

PdfBookmarkEditor::PdfBookmarkEditor(Aspose::Pdf::Document& document) {
    BindPdf(document);
}

// Outline create/extract are REAL — create stages an /Outlines tree on
// the bound document (flushed at Save); extract parses the existing
// /Outlines via foundation::outlines. XML export/import round-trips
// through a small line-oriented format with entity escaping; HTML
// export writes a flat list.

Aspose::Pdf::Document::OutlineNode PdfBookmarkEditor::ToNode(
        const Bookmark& bm) {
    Aspose::Pdf::Document::OutlineNode n;
    n.title = bm.Title();
    n.page = bm.PageNumber();
    Bookmarks kids = bm.ChildItems();
    for (const auto& child : kids) n.children.push_back(ToNode(child));
    return n;
}

void PdfBookmarkEditor::Restage() {
    if (document_ == nullptr) return;
    std::vector<Aspose::Pdf::Document::OutlineNode> nodes;
    nodes.reserve(staged_.size());
    for (const auto& b : staged_) nodes.push_back(ToNode(b));
    document_->SetStagedOutlines(std::move(nodes));
}

void PdfBookmarkEditor::CreateBookmarks() { Restage(); }
void PdfBookmarkEditor::CreateBookmarks(const Bookmark& bookmark) {
    staged_.push_back(bookmark);
    Restage();
}
void PdfBookmarkEditor::CreateBookmarkOfPage(const std::string& title,
                                             int pageNumber) {
    Bookmark b;
    b.Title(title);
    b.PageNumber(pageNumber);
    staged_.push_back(std::move(b));
    Restage();
}
void PdfBookmarkEditor::CreateBookmarkOfPage(
        const std::vector<std::string>& title,
        const std::vector<int>& pageNumber) {
    for (std::size_t i = 0; i < title.size(); ++i) {
        Bookmark b;
        b.Title(title[i]);
        b.PageNumber(i < pageNumber.size() ? pageNumber[i] : 1);
        staged_.push_back(std::move(b));
    }
    Restage();
}

void PdfBookmarkEditor::DeleteBookmarks() {
    staged_.clear();
    if (document_ != nullptr) document_->ClearStagedOutlines();
}
void PdfBookmarkEditor::DeleteBookmarks(const std::string& title) {
    for (auto it = staged_.begin(); it != staged_.end();) {
        if (it->Title() == title)
            it = staged_.erase(it);
        else
            ++it;
    }
    Restage();
}
void PdfBookmarkEditor::ModifyBookmarks(const std::string& oldTitle,
                                        const std::string& newTitle) {
    for (auto& b : staged_)
        if (b.Title() == oldTitle) b.Title(newTitle);
    Restage();
}

Bookmarks PdfBookmarkEditor::ExtractBookmarks() {
    Bookmarks result;
    if (document_ == nullptr) return result;
    for (const auto& [depth, title] : document_->ParseOutlineItems()) {
        Bookmark b;
        b.Title(title);
        b.Level(depth + 1);
        result.push_back(std::move(b));
    }
    return result;
}
Bookmarks PdfBookmarkEditor::ExtractBookmarks(bool keepLevels) {
    if (keepLevels) return ExtractBookmarks();
    Bookmarks all = ExtractBookmarks();
    Bookmarks result;
    for (auto& b : all) {
        if (b.Level() == 1) {
            result.push_back(std::move(b));
        }
    }
    return result;
}

Bookmarks PdfBookmarkEditor::ExtractBookmarks(const std::string& title) {
    Bookmarks all = ExtractBookmarks();
    Bookmarks result;
    for (auto& b : all) {
        if (b.Title() == title) {
            result.push_back(std::move(b));
        }
    }
    return result;
}

Bookmarks PdfBookmarkEditor::ExtractBookmarks(const char* title) {
    return ExtractBookmarks(title != nullptr ? std::string(title) : std::string{});
}


Bookmarks PdfBookmarkEditor::ExtractBookmarks(const Bookmark& parent) {
    Bookmarks all = ExtractBookmarks();
    Bookmarks result;
    if (parent.Title().empty()) return result;
    bool collecting = false;
    int parentLevel = parent.Level();
    for (const auto& b : all) {
        if (!collecting) {
            if (b.Title() == parent.Title()) {
                collecting = true;
                parentLevel = b.Level();
            }
        } else {
            if (b.Level() > parentLevel) {
                if (b.Level() == parentLevel + 1) {
                    result.push_back(b);
                }
            } else {
                break;
            }
        }
    }
    return result;
}

void PdfBookmarkEditor::ExtractBookmarksToHTML(const std::string& dataDir,
                                               const std::string& outputFile) {
    ExportBookmarksToHtml(dataDir, outputFile);
}

void PdfBookmarkEditor::ExportBookmarksToHtml(const std::string& /*dataDir*/,
                                              const std::string& outputFile) {
    Bookmarks bms = ExtractBookmarks();
    if (bms.empty() && !staged_.empty()) {
        bms.assign(staged_.begin(), staged_.end());
    }
    std::ofstream out(outputFile);
    out << "<!DOCTYPE html>\n<html>\n<head><title>Bookmarks</title></head>\n<body>\n<ul>\n";
    for (const auto& bm : bms) {
        out << "  <li><a href=\"#page=" << bm.PageNumber() << "\">"
            << foundation::xml_text::EscapeXmlText(bm.Title()) << "</a></li>\n";
    }
    out << "</ul>\n</body>\n</html>\n";
}

void PdfBookmarkEditor::ExportBookmarksToXML(const std::string& outputFile) {
    Bookmarks bms = ExtractBookmarks();
    if (bms.empty() && !staged_.empty()) {
        bms.assign(staged_.begin(), staged_.end());
    }
    std::ofstream out(outputFile);
    out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    out << "<Bookmarks>\n";
    for (const auto& bm : bms) {
        out << "  <Bookmark Title=\""
            << foundation::xml_text::EscapeXmlAttr(bm.Title()) << "\" Page=\""
            << bm.PageNumber() << "\" Level=\"" << bm.Level() << "\" />\n";
    }
    out << "</Bookmarks>\n";
}

void PdfBookmarkEditor::ImportBookmarksWithXML(const std::string& xmlFile) {
    std::ifstream in(xmlFile);
    if (!in.is_open()) return;
    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

    // Scan the whole buffer rather than lines, so externally produced or
    // pretty-printed XML (where the <Bookmark> tag spans lines or carries
    // attributes in any order) parses too. Values arrive entity-decoded.
    std::size_t pos = 0;
    while ((pos = content.find("<Bookmark", pos)) != std::string::npos) {
        // Reject longer names that merely start with "Bookmark" (e.g. <Bookmarks>).
        const char next = pos + 9 < content.size() ? content[pos + 9] : '\0';
        if (next != ' ' && next != '>' && next != '/' && next != '\t' &&
            next != '\n' && next != '\r') {
            pos += 9;
            continue;
        }
        const auto closePos = content.find('>', pos);
        if (closePos == std::string::npos) break;
        const std::string tagHeader = content.substr(pos, closePos - pos + 1);

        const std::string title =
            foundation::xml_text::FindAttrValue(tagHeader, "Title");
        if (!title.empty()) {
            int pageNum = 1;
            const std::string pageStr =
                foundation::xml_text::FindAttrValue(tagHeader, "Page");
            if (!pageStr.empty()) {
                try {
                    pageNum = std::stoi(pageStr);
                } catch (...) {}
            }
            CreateBookmarkOfPage(title, pageNum);
        }
        pos = closePos + 1;
    }
}

}  // namespace Aspose::Pdf::Facades
