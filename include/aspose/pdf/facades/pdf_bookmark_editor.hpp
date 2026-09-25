#pragma once

// =============================================================================
// Aspose::Pdf::Facades::PdfBookmarkEditor — outline (bookmark) editor
// for the bound PDF (create / extract / delete / modify / import /
// export). Mirrors canonical Aspose.Pdf.Facades.PdfBookmarkEditor;
// extends SaveableFacade.
//
// Create/Extract/Delete/Modify are real (staged /Outlines write at
// Save + readback via foundation::outlines). XML export/import
// round-trip through a small line-oriented format with entity
// escaping; HTML export writes a flat list.
//
// Phased drops:
//   * CreateBookmarks(Color, bool, bool) — System.Drawing.Color cascade.
//   * Export/Import *(Stream) overloads — Stream cascade.
// =============================================================================

#include <string>
#include <vector>

#include <aspose/pdf/document.hpp>
#include <aspose/pdf/facades/bookmark.hpp>
#include <aspose/pdf/facades/saveable_facade.hpp>

namespace Aspose::Pdf::Facades {

class PdfBookmarkEditor : public SaveableFacade {
public:
    PdfBookmarkEditor() noexcept = default;
    explicit PdfBookmarkEditor(Aspose::Pdf::Document& document);

    // ---- Create ----

    void CreateBookmarks();
    void CreateBookmarks(const Bookmark& bookmark);
    void CreateBookmarkOfPage(const std::string& title, int pageNumber);
    void CreateBookmarkOfPage(const std::vector<std::string>& title,
                              const std::vector<int>& pageNumber);

    // ---- Delete / Modify ----

    void DeleteBookmarks();
    void DeleteBookmarks(const std::string& title);
    void ModifyBookmarks(const std::string& oldTitle,
                         const std::string& newTitle);

    // ---- Extract ----

    Bookmarks ExtractBookmarks();
    Bookmarks ExtractBookmarks(bool keepLevels);
    Bookmarks ExtractBookmarks(const std::string& title);
    Bookmarks ExtractBookmarks(const char* title);
    Bookmarks ExtractBookmarks(const Bookmark& parent);

    // ---- Import / Export ----

    // dataDir is accepted for API parity (Aspose's input-data-location
    // convention — callers pass the bound PDF's path/directory); it is
    // NOT used as the output directory. The HTML file is always written
    // to outputFile.
    void ExtractBookmarksToHTML(const std::string& dataDir,
                                const std::string& outputFile);
    void ExportBookmarksToHtml(const std::string& dataDir,
                               const std::string& outputFile);
    void ExportBookmarksToXML(const std::string& outputFile);
    void ImportBookmarksWithXML(const std::string& xmlFile);

private:
    // Push the accumulated bookmarks onto the bound document's staged
    // /Outlines tree (flushed at Save).
    void Restage();
    static Aspose::Pdf::Document::OutlineNode ToNode(const Bookmark& bm);

    std::vector<Bookmark> staged_;
};

}  // namespace Aspose::Pdf::Facades
