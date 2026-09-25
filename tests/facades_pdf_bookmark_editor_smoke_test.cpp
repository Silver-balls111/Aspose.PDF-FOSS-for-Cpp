// =============================================================================
// facades_pdf_bookmark_editor_smoke_test — beat Fa6 of the Facades
// cluster. PdfBookmarkEditor + the Bookmark / Bookmarks value types.
// Bookmark/Bookmarks are real value types; CreateBookmarks/Extract are
// REAL (parity gap 6) — staged /Outlines write at Save + readback
// via foundation::outlines. XML export/import round-trip with entity
// escaping; HTML export writes a flat list.
// =============================================================================

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <aspose/pdf/document.hpp>
#include <aspose/pdf/facades/bookmark.hpp>
#include <aspose/pdf/facades/bookmarks.hpp>
#include <aspose/pdf/facades/pdf_bookmark_editor.hpp>

#include <gtest/gtest.h>

namespace {

using namespace Aspose::Pdf;
using namespace Aspose::Pdf::Facades;

std::filesystem::path FixtureRoot() {
    if (const char* env = std::getenv("SMOKE_FIXTURE_DIR"); env != nullptr) {
        return std::filesystem::path(env);
    }
    return std::filesystem::path(__FILE__).parent_path().parent_path() / "pdfs";
}

std::string HelloWorldPdf() {
    return (FixtureRoot() / "hello_world.pdf").string();
}

std::string TwoPagesPdf() {
    return (std::filesystem::path(__FILE__).parent_path() / "fixtures" /
            "text_extractor" / "two_pages.pdf").string();
}

std::string BmTmp(const std::string& n) {
    return (std::filesystem::temp_directory_path() /
            ("aspose_bookmark_" + n)).string();
}

}  // namespace

TEST(FacadesBookmarkSmoke, BookmarkPodDefaultsAndRoundtrip) {
    Bookmark bm;
    EXPECT_TRUE(bm.Title().empty());
    EXPECT_EQ(bm.PageNumber(), 1);
    EXPECT_EQ(bm.Level(), 1);
    EXPECT_FALSE(bm.BoldFlag());
    EXPECT_FALSE(bm.ItalicFlag());
    EXPECT_FALSE(bm.Open());

    bm.Title("Chapter 1");
    bm.PageNumber(5);
    bm.Level(2);
    bm.BoldFlag(true);
    bm.ItalicFlag(true);
    bm.Open(true);
    bm.Action("GoTo");
    bm.Destination("dest1");
    bm.PageDisplay("FitH");
    bm.PageDisplay_Left(10);
    bm.PageDisplay_Top(700);
    bm.PageDisplay_Zoom(100);
    bm.RemoteFile("other.pdf");

    EXPECT_EQ(bm.Title(), "Chapter 1");
    EXPECT_EQ(bm.PageNumber(), 5);
    EXPECT_EQ(bm.Level(), 2);
    EXPECT_TRUE(bm.BoldFlag());
    EXPECT_TRUE(bm.ItalicFlag());
    EXPECT_TRUE(bm.Open());
    EXPECT_EQ(bm.Action(), "GoTo");
    EXPECT_EQ(bm.Destination(), "dest1");
    EXPECT_EQ(bm.PageDisplay(), "FitH");
    EXPECT_EQ(bm.PageDisplay_Left(), 10);
    EXPECT_EQ(bm.PageDisplay_Top(), 700);
    EXPECT_EQ(bm.PageDisplay_Zoom(), 100);
    EXPECT_EQ(bm.RemoteFile(), "other.pdf");
}

TEST(FacadesBookmarkSmoke, BookmarksCollectionIsAVector) {
    Bookmarks list;
    EXPECT_TRUE(list.empty());

    Bookmark a;
    a.Title("A");
    Bookmark b;
    b.Title("B");
    list.push_back(a);
    list.push_back(b);

    ASSERT_EQ(list.size(), 2u);
    EXPECT_EQ(list[0].Title(), "A");
    EXPECT_EQ(list[1].Title(), "B");
}

TEST(FacadesBookmarkSmoke, ChildItemsRoundtrip) {
    Bookmark parent;
    parent.Title("Parent");

    Bookmarks children;
    Bookmark child;
    child.Title("Child");
    children.push_back(child);

    parent.ChildItems(children);
    Bookmarks back = parent.ChildItems();
    ASSERT_EQ(back.size(), 1u);
    EXPECT_EQ(back[0].Title(), "Child");

    // ChildItem is a canonical alias over the same backing collection.
    EXPECT_EQ(parent.ChildItem().size(), 1u);
}

// Full editor sweep: every call runs against a bound document, file
// outputs land in the temp dir.
TEST(FacadesBookmarkSmoke, EditorCallsDoNotThrow) {
    Document doc{HelloWorldPdf()};
    PdfBookmarkEditor editor{doc};

    Bookmark bm;
    bm.Title("Intro");
    bm.PageNumber(1);

    editor.CreateBookmarks();
    editor.CreateBookmarks(bm);
    editor.CreateBookmarkOfPage("Intro", 1);
    editor.CreateBookmarkOfPage(std::vector<std::string>{"A", "B"},
                                std::vector<int>{1, 2});
    editor.ModifyBookmarks("old", "new");
    editor.DeleteBookmarks("Intro");
    editor.DeleteBookmarks();

    const std::string xml = BmTmp("sweep.xml");
    const std::string html = BmTmp("sweep.html");
    editor.ExportBookmarksToXML(xml);
    editor.ImportBookmarksWithXML(BmTmp("missing_in.xml"));  // absent: no-op
    // dataDir is parity-only (Aspose's input-data-location convention);
    // the HTML always goes to the outputFile argument.
    editor.ExportBookmarksToHtml("dir", html);
    editor.ExtractBookmarksToHTML("dir", html);
    EXPECT_TRUE(std::filesystem::exists(xml));
    EXPECT_TRUE(std::filesystem::exists(html));
    std::filesystem::remove(xml);
    std::filesystem::remove(html);
}

// Titles containing XML metacharacters must escape on export and
// survive the round-trip through ImportBookmarksWithXML intact
// (regression for probe B1 — the old exporter wrote them raw).
TEST(FacadesBookmarkSmoke, XmlEscapingRoundtrip) {
    const std::string xml = BmTmp("escape.xml");
    const std::string out = BmTmp("escape_out.pdf");
    const std::string title = "Tom & Jerry \"quoted\" <tag>";
    {
        Document doc{TwoPagesPdf()};
        PdfBookmarkEditor ed{doc};
        ed.CreateBookmarkOfPage(title, 1);
        ed.ExportBookmarksToXML(xml);
    }
    {
        std::ifstream in(xml);
        std::string content((std::istreambuf_iterator<char>(in)),
                            std::istreambuf_iterator<char>());
        EXPECT_NE(content.find(
                      "Tom &amp; Jerry &quot;quoted&quot; &lt;tag&gt;"),
                  std::string::npos)
            << "exporter must escape the Title attribute";
    }
    {
        Document doc{TwoPagesPdf()};
        PdfBookmarkEditor ed{doc};
        ed.ImportBookmarksWithXML(xml);
        ed.Save(out);
    }
    Document re{out};
    PdfBookmarkEditor ed3{re};
    Bookmarks bms = ed3.ExtractBookmarks();
    ASSERT_EQ(bms.size(), 1u);
    EXPECT_EQ(bms[0].Title(), title);

    std::filesystem::remove(xml);
    std::filesystem::remove(out);
}

// Numeric character references beyond ASCII decode to UTF-8 on import
// (regression for probe P10 — &#233; used to stay literal).
TEST(FacadesBookmarkSmoke, XmlEntityDecodeOnImport) {
    const std::string xml = BmTmp("entities.xml");
    const std::string out = BmTmp("entities_out.pdf");
    {
        std::ofstream outXml(xml);
        outXml << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
               << "<Bookmarks>\n"
               << "  <Bookmark Title=\"Caf&#233; M&#xE9;tro\" Page=\"1\" "
               << "Level=\"1\" />\n"
               << "</Bookmarks>\n";
    }
    {
        Document doc{TwoPagesPdf()};
        PdfBookmarkEditor ed{doc};
        ed.ImportBookmarksWithXML(xml);
        ed.Save(out);
    }
    Document re{out};
    PdfBookmarkEditor ed3{re};
    Bookmarks bms = ed3.ExtractBookmarks();
    ASSERT_EQ(bms.size(), 1u);
    EXPECT_EQ(bms[0].Title(), "Caf\xC3\xA9 M\xC3\xA9tro");

    std::filesystem::remove(xml);
    std::filesystem::remove(out);
}

// Externally produced, pretty-printed XML (attributes in any order, a
// tag spanning lines) imports too — the importer is not limited to its
// own one-line-per-bookmark output format.
TEST(FacadesBookmarkSmoke, ExternalPrettyPrintedXmlImports) {
    const std::string xml = BmTmp("external.xml");
    const std::string out = BmTmp("external_out.pdf");
    {
        std::ofstream outXml(xml);
        outXml << "<?xml version=\"1.0\"?>\n"
               << "<Bookmarks>\n"
               << "  <Bookmark\n"
               << "        Page=\"2\"\n"
               << "        Level=\"1\"\n"
               << "        Title=\"Second Page &amp; More\" />\n"
               << "</Bookmarks>\n";
    }
    {
        Document doc{TwoPagesPdf()};
        PdfBookmarkEditor ed{doc};
        ed.ImportBookmarksWithXML(xml);
        ed.Save(out);
    }
    Document re{out};
    PdfBookmarkEditor ed3{re};
    Bookmarks bms = ed3.ExtractBookmarks();
    ASSERT_EQ(bms.size(), 1u);
    EXPECT_EQ(bms[0].Title(), "Second Page & More");
    // PageNumber is deliberately not asserted: ExtractBookmarks reads
    // only the outline titles/levels back, never destinations.

    std::filesystem::remove(xml);
    std::filesystem::remove(out);
}

TEST(FacadesBookmarkSmoke, ExtractReturnsEmptyBookmarks) {
    Document doc{HelloWorldPdf()};
    PdfBookmarkEditor editor{doc};
    EXPECT_TRUE(editor.ExtractBookmarks().empty());
    EXPECT_TRUE(editor.ExtractBookmarks(true).empty());
    EXPECT_TRUE(editor.ExtractBookmarks("Intro").empty());

    Bookmark parent;
    EXPECT_TRUE(editor.ExtractBookmarks(parent).empty());
}


TEST(PdfBookmarkEditorRealSmoke, CreateSaveReloadExtract) {
    const std::string out = BmTmp("create.pdf");
    {
        Document doc{TwoPagesPdf()};
        PdfBookmarkEditor ed{doc};
        ed.CreateBookmarkOfPage("Chapter 1", 1);
        ed.CreateBookmarkOfPage("Chapter 2", 2);
        ed.Save(out);
    }
    Document re{out};
    PdfBookmarkEditor ed2{re};
    Bookmarks bms = ed2.ExtractBookmarks();
    ASSERT_GE(bms.size(), 2u);
    EXPECT_EQ(bms[0].Title(), "Chapter 1");
    EXPECT_EQ(bms[1].Title(), "Chapter 2");
    std::filesystem::remove(out);
}

TEST(PdfBookmarkEditorRealSmoke, NestedBookmarksRoundTrip) {
    const std::string out = BmTmp("nested.pdf");
    {
        Document doc{TwoPagesPdf()};
        PdfBookmarkEditor ed{doc};
        Bookmark parent;
        parent.Title("Part I");
        parent.PageNumber(1);
        Bookmarks kids;
        Bookmark child;
        child.Title("Section 1.1");
        child.PageNumber(2);
        kids.push_back(child);
        parent.ChildItems(kids);
        ed.CreateBookmarks(parent);
        ed.Save(out);
    }
    Document re{out};
    PdfBookmarkEditor ed2{re};
    Bookmarks bms = ed2.ExtractBookmarks();
    ASSERT_GE(bms.size(), 2u);
    EXPECT_EQ(bms[0].Title(), "Part I");
    EXPECT_EQ(bms[1].Title(), "Section 1.1");
    EXPECT_GT(bms[1].Level(), bms[0].Level());
    std::filesystem::remove(out);
}

TEST(PdfBookmarkEditorRealSmoke, XmlAndHtmlExportImport) {
    const std::string xmlFile = BmTmp("export.xml");
    const std::string htmlFile = BmTmp("export.html");
    const std::string pdfOut = BmTmp("imported_bms.pdf");

    // Create bookmarks and export to XML & HTML
    {
        Document doc{TwoPagesPdf()};
        PdfBookmarkEditor ed{doc};
        ed.CreateBookmarkOfPage("Chapter 1: Start", 1);
        ed.CreateBookmarkOfPage("Chapter 2: Finish", 2);
        ed.ExportBookmarksToXML(xmlFile);
        ed.ExportBookmarksToHtml(TwoPagesPdf(), htmlFile);
    }

    EXPECT_TRUE(std::filesystem::exists(xmlFile));
    EXPECT_TRUE(std::filesystem::exists(htmlFile));

    // Import from XML into a new document
    {
        Document doc2{TwoPagesPdf()};
        PdfBookmarkEditor ed2{doc2};
        ed2.ImportBookmarksWithXML(xmlFile);
        ed2.Save(pdfOut);
    }

    Document re{pdfOut};
    PdfBookmarkEditor ed3{re};
    Bookmarks bms = ed3.ExtractBookmarks();
    ASSERT_EQ(bms.size(), 2u);
    EXPECT_EQ(bms[0].Title(), "Chapter 1: Start");
    EXPECT_EQ(bms[1].Title(), "Chapter 2: Finish");

    std::filesystem::remove(xmlFile);
    std::filesystem::remove(htmlFile);
    std::filesystem::remove(pdfOut);
}

TEST(PdfBookmarkEditorRealSmoke, ExtractBookmarksFiltered) {
    const std::string out = BmTmp("filtered_bms.pdf");
    {
        Document doc{TwoPagesPdf()};
        PdfBookmarkEditor ed{doc};

        Bookmark parent1;
        parent1.Title("Chapter 1");
        parent1.PageNumber(1);

        Bookmark child1;
        child1.Title("Section 1.1");
        child1.PageNumber(1);

        Bookmarks kids;
        kids.push_back(child1);
        parent1.ChildItems(kids);

        Bookmark parent2;
        parent2.Title("Chapter 2");
        parent2.PageNumber(2);

        ed.CreateBookmarks(parent1);
        ed.CreateBookmarks(parent2);
        ed.Save(out);
    }

    Document re{out};
    PdfBookmarkEditor ed2{re};

    // Filter by title
    Bookmarks byTitle = ed2.ExtractBookmarks("Chapter 1");
    ASSERT_EQ(byTitle.size(), 1u);
    EXPECT_EQ(byTitle[0].Title(), "Chapter 1");

    // Filter by keepLevels = false (only top-level bookmarks)
    Bookmarks topOnly = ed2.ExtractBookmarks(false);
    ASSERT_EQ(topOnly.size(), 2u);
    EXPECT_EQ(topOnly[0].Title(), "Chapter 1");
    EXPECT_EQ(topOnly[1].Title(), "Chapter 2");
    EXPECT_EQ(topOnly[0].Level(), 1);
    EXPECT_EQ(topOnly[1].Level(), 1);

    // Filter by parent bookmark
    Bookmark p1Query;
    p1Query.Title("Chapter 1");
    p1Query.Level(1);
    Bookmarks children = ed2.ExtractBookmarks(p1Query);
    ASSERT_EQ(children.size(), 1u);
    EXPECT_EQ(children[0].Title(), "Section 1.1");

    std::filesystem::remove(out);
}

