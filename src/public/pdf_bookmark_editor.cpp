#include <aspose/pdf/facades/pdf_bookmark_editor.hpp>

#include <fstream>
#include <sstream>

#include <aspose/pdf/document.hpp>

namespace Aspose::Pdf::Facades {

PdfBookmarkEditor::PdfBookmarkEditor(Aspose::Pdf::Document& document) {
    BindPdf(document);
}

// Outline create/extract are REAL — create stages an /Outlines tree on
// the bound document (flushed at Save); extract parses the existing
// /Outlines via foundation::outlines. HTML/XML import/export remain
// stubs.

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
Bookmarks PdfBookmarkEditor::ExtractBookmarks(bool) {
    return ExtractBookmarks();
}
Bookmarks PdfBookmarkEditor::ExtractBookmarks(const std::string&) {
    return ExtractBookmarks();
}
Bookmarks PdfBookmarkEditor::ExtractBookmarks(const Bookmark&) {
    return ExtractBookmarks();
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
            << bm.Title() << "</a></li>\n";
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
        out << "  <Bookmark Title=\"" << bm.Title() << "\" Page=\""
            << bm.PageNumber() << "\" Level=\"" << bm.Level() << "\" />\n";
    }
    out << "</Bookmarks>\n";
}

void PdfBookmarkEditor::ImportBookmarksWithXML(const std::string& xmlFile) {
    std::ifstream in(xmlFile);
    if (!in.is_open()) return;
    std::string line;
    while (std::getline(in, line)) {
        auto tagPos = line.find("<Bookmark");
        if (tagPos == std::string::npos) continue;
        auto titlePos = line.find("Title=\"", tagPos);
        if (titlePos == std::string::npos) continue;
        titlePos += 7;
        auto titleEnd = line.find("\"", titlePos);
        if (titleEnd == std::string::npos) continue;
        std::string title = line.substr(titlePos, titleEnd - titlePos);

        int pageNum = 1;
        auto pagePos = line.find("Page=\"", tagPos);
        if (pagePos != std::string::npos) {
            pagePos += 6;
            auto pageEnd = line.find("\"", pagePos);
            if (pageEnd != std::string::npos) {
                try {
                    pageNum = std::stoi(line.substr(pagePos, pageEnd - pagePos));
                } catch (...) {}
            }
        }
        CreateBookmarkOfPage(title, pageNum);
    }
}

}  // namespace Aspose::Pdf::Facades
