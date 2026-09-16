// =============================================================================
// facades_pdf_file_editor_ops_smoke_test — PdfFileEditor real file ops
// (parity gap 3). Concatenate / Append / Insert deep-import the source
// pages' object graph (Document::ImportPagesFrom); Extract / Delete /
// Split reduce to PageCollection page removal. Closes pdflib's
// PdfFileEditor concatenate / extract / delete / split parity gap.
// =============================================================================

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <vector>

#include <aspose/pdf/document.hpp>
#include <aspose/pdf/facades/pdf_file_editor.hpp>
#include <aspose/pdf/page.hpp>
#include <aspose/pdf/page_collection.hpp>
#include <aspose/pdf/text_absorber.hpp>

#include "flate.hpp"
#include "objects.hpp"
#include "pages_tree.hpp"
#include "trailer.hpp"

#include <gtest/gtest.h>

namespace {

using namespace Aspose::Pdf;
using namespace Aspose::Pdf::Facades;

std::string HelloWorldPdf() {
    return (std::filesystem::path(__FILE__).parent_path().parent_path() /
            "pdfs" / "hello_world.pdf").string();
}
std::string TwoPagesPdf() {
    return (std::filesystem::path(__FILE__).parent_path() / "fixtures" /
            "text_extractor" / "two_pages.pdf").string();
}
std::string Tmp(const std::string& n) {
    return (std::filesystem::temp_directory_path() /
            ("aspose_fileeditor_" + n)).string();
}
std::string DocText(const std::string& path) {
    Document d{path};
    Aspose::Pdf::Text::TextAbsorber abs;
    abs.Visit(d);
    return abs.Text();
}

std::vector<std::byte> ReadAll(const std::string& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    const auto end = in.tellg();
    std::vector<std::byte> bytes(static_cast<std::size_t>(end));
    in.seekg(0, std::ios::beg);
    in.read(reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    return bytes;
}

// Decoded bodies of the page's /Contents streams, in order.
std::vector<std::string> PageContentStreams(const std::string& path,
                                            int page1Based) {
    const auto bytes = ReadAll(path);
    std::span<const std::byte> sp(bytes.data(), bytes.size());
    const auto tree = foundation::pages_tree::Parse(sp);
    const auto dump = foundation::objects::Parse(sp);
    std::vector<std::string> out;
    if (page1Based < 1 ||
        static_cast<std::size_t>(page1Based) > tree.leaves.size())
        return out;
    const std::uint32_t pid = tree.leaves[page1Based - 1].id;
    const foundation::objects::IndirectObject* page_obj = nullptr;
    for (const auto& o : dump.objects)
        if (o.id == pid) page_obj = &o;
    if (page_obj == nullptr) return out;
    const auto* pd =
        std::get_if<foundation::objects::Dict>(&page_obj->value.v);
    if (pd == nullptr) return out;
    std::vector<std::uint32_t> ids;
    for (const auto& kv : pd->entries) {
        if (kv.first != "Contents") continue;
        if (const auto* r = std::get_if<foundation::objects::Ref>(&kv.second.v))
            ids.push_back(r->id);
        else if (const auto* a =
                     std::get_if<foundation::objects::Array>(&kv.second.v))
            for (const auto& it : a->items)
                if (const auto* rr =
                        std::get_if<foundation::objects::Ref>(&it.v))
                    ids.push_back(rr->id);
    }
    for (std::uint32_t sid : ids) {
        for (const auto& o : dump.objects) {
            if (o.id != sid) continue;
            const auto* st =
                std::get_if<foundation::objects::Stream>(&o.value.v);
            if (st == nullptr) break;
            std::vector<std::byte> body(st->body.begin(), st->body.end());
            bool flate = false;
            for (const auto& kv : st->header.entries) {
                if (kv.first == "Filter")
                    if (const auto* n = std::get_if<std::string>(&kv.second.v))
                        flate = *n == "FlateDecode";
            }
            if (flate) {
                try {
                    body = foundation::flate::Decode(st->body);
                } catch (const std::exception&) {
                }
            }
            out.emplace_back(reinterpret_cast<const char*>(body.data()),
                             body.size());
            break;
        }
    }
    return out;
}

// True when some /Subtype /Form stream in the file contains `needle`.
bool FormBodyContains(const std::string& path, const std::string& needle) {
    const auto bytes = ReadAll(path);
    const auto dump = foundation::objects::Parse(
        std::span<const std::byte>(bytes.data(), bytes.size()));
    for (const auto& o : dump.objects) {
        const auto* st = std::get_if<foundation::objects::Stream>(&o.value.v);
        if (st == nullptr) continue;
        bool is_form = false;
        for (const auto& kv : st->header.entries) {
            if (kv.first == "Subtype")
                if (const auto* n = std::get_if<std::string>(&kv.second.v))
                    is_form = *n == "Form";
        }
        if (!is_form) continue;
        const std::string body(reinterpret_cast<const char*>(st->body.data()),
                               st->body.size());
        if (body.find(needle) != std::string::npos) return true;
    }
    return false;
}

}  // namespace

TEST(PdfFileEditorOpsSmoke, ConcatenateTwoFiles) {
    const std::string out = Tmp("concat.pdf");
    PdfFileEditor ed;
    ASSERT_TRUE(ed.Concatenate(HelloWorldPdf(), TwoPagesPdf(), out));
    Document re{out};
    EXPECT_EQ(re.Pages().Count(), 3u);  // 1 + 2
    const std::string text = DocText(out);
    EXPECT_NE(text.find("Hello World"), std::string::npos);
    EXPECT_NE(text.find("Page one"), std::string::npos);
    EXPECT_NE(text.find("Page two"), std::string::npos);
    std::filesystem::remove(out);
}

TEST(PdfFileEditorOpsSmoke, ConcatenateManyFiles) {
    const std::string out = Tmp("concat_many.pdf");
    PdfFileEditor ed;
    ASSERT_TRUE(ed.Concatenate(
        std::vector<std::string>{HelloWorldPdf(), TwoPagesPdf(),
                                 HelloWorldPdf()},
        out));
    Document re{out};
    EXPECT_EQ(re.Pages().Count(), 4u);  // 1 + 2 + 1
    std::filesystem::remove(out);
}

TEST(PdfFileEditorOpsSmoke, ExtractPageRange) {
    const std::string out = Tmp("extract.pdf");
    PdfFileEditor ed;
    // Keep only page 2 of two_pages ("Page two").
    ASSERT_TRUE(ed.Extract(TwoPagesPdf(), 2, 2, out));
    Document re{out};
    EXPECT_EQ(re.Pages().Count(), 1u);
    EXPECT_NE(DocText(out).find("Page two"), std::string::npos);
    EXPECT_EQ(DocText(out).find("Page one"), std::string::npos);
    std::filesystem::remove(out);
}

TEST(PdfFileEditorOpsSmoke, DeletePages) {
    const std::string out = Tmp("delete.pdf");
    PdfFileEditor ed;
    ASSERT_TRUE(ed.Delete(TwoPagesPdf(), std::vector<int>{1}, out));
    Document re{out};
    EXPECT_EQ(re.Pages().Count(), 1u);
    EXPECT_NE(DocText(out).find("Page two"), std::string::npos);
    std::filesystem::remove(out);
}

TEST(PdfFileEditorOpsSmoke, AppendRange) {
    const std::string out = Tmp("append.pdf");
    PdfFileEditor ed;
    // Append page 1 ("Page one") of two_pages onto hello_world.
    ASSERT_TRUE(ed.Append(HelloWorldPdf(),
                          std::vector<std::string>{TwoPagesPdf()}, 1, 1,
                          out));
    Document re{out};
    EXPECT_EQ(re.Pages().Count(), 2u);  // 1 + 1
    const std::string text = DocText(out);
    EXPECT_NE(text.find("Hello World"), std::string::npos);
    EXPECT_NE(text.find("Page one"), std::string::npos);
    std::filesystem::remove(out);
}

TEST(PdfFileEditorOpsSmoke, SplitToPages) {
    // Copy two_pages to a temp so split files land in temp dir.
    const std::string src = Tmp("split_src.pdf");
    std::filesystem::copy_file(
        TwoPagesPdf(), src,
        std::filesystem::copy_options::overwrite_existing);
    PdfFileEditor ed;
    std::vector<std::string> files = ed.SplitToPages(src);
    ASSERT_EQ(files.size(), 2u);
    for (const auto& f : files) {
        EXPECT_TRUE(std::filesystem::exists(f));
        Document d{f};
        EXPECT_EQ(d.Pages().Count(), 1u);
        std::filesystem::remove(f);
    }
    std::filesystem::remove(src);
}

TEST(PdfFileEditorOpsSmoke, MakeBooklet) {
    const std::string out = Tmp("booklet.pdf");
    PdfFileEditor ed;
    ASSERT_TRUE(ed.MakeBooklet(TwoPagesPdf(), out));
    Document re{out};
    EXPECT_EQ(re.Pages().Count(), 4u);  // 2 pages rounded up to 4 for booklet
    std::filesystem::remove(out);
}

TEST(PdfFileEditorOpsSmoke, MakeNUp) {
    const std::string out = Tmp("nup.pdf");
    PdfFileEditor ed;
    ASSERT_TRUE(ed.MakeNUp(HelloWorldPdf(), TwoPagesPdf(), out));
    Document re{out};
    // Real 2-up imposition: the two files' pages pair onto sheets —
    // 1 + 2 source pages → 2 output sheets, not a 3-page concatenation.
    EXPECT_EQ(re.Pages().Count(), 2u);
    // Sheet 1 joins hello_world (612x792) with two_pages p1 (612x792)
    // side by side.
    const auto s1 = re.Pages()[1];
    EXPECT_FLOAT_EQ(s1.Rect().Width(), 1224.0);
    EXPECT_FLOAT_EQ(s1.Rect().Height(), 792.0);
    // Sheet 2 carries only the second file's page 2.
    const auto s2 = re.Pages()[2];
    EXPECT_FLOAT_EQ(s2.Rect().Width(), 612.0);
    EXPECT_FLOAT_EQ(s2.Rect().Height(), 792.0);
    // Each source page's content moved into a Form XObject.
    EXPECT_TRUE(FormBodyContains(out, "Hello World"));
    EXPECT_TRUE(FormBodyContains(out, "Page one"));
    EXPECT_TRUE(FormBodyContains(out, "Page two"));
    std::filesystem::remove(out);
}

TEST(PdfFileEditorOpsSmoke, ResizeContents) {
    // Parameters variant: new page size, content scaled into the box.
    const std::string out = Tmp("resize_params.pdf");
    PdfFileEditor ed;
    PdfFileEditor::ContentsResizeParameters params;
    params.NewPageWidth(PdfFileEditor::ContentsResizeValue::Units(400.0));
    params.NewPageHeight(PdfFileEditor::ContentsResizeValue::Units(300.0));
    ASSERT_TRUE(ed.ResizeContents(HelloWorldPdf(), out, {1}, params));
    {
        Document re{out};
        const auto page = re.Pages()[1];
        EXPECT_FLOAT_EQ(page.Rect().Width(), 400.0);
        EXPECT_FLOAT_EQ(page.Rect().Height(), 300.0);
    }
    std::filesystem::remove(out);

    // Percent variant: the page box stays untouched, the content shrinks
    // to the requested percentage and is centred (a q .. cm / Q wrapper
    // appears around the content streams).
    const std::string out2 = Tmp("resize_pct.pdf");
    ASSERT_TRUE(ed.ResizeContentsPct(HelloWorldPdf(), out2, 50.0, 50.0));
    {
        Document re{out2};
        const auto page = re.Pages()[1];
        EXPECT_FLOAT_EQ(page.Rect().Width(), 612.0);
        EXPECT_FLOAT_EQ(page.Rect().Height(), 792.0);
    }
    const auto streams = PageContentStreams(out2, 1);
    ASSERT_GE(streams.size(), 3u);  // wrapper + original + wrapper
    EXPECT_NE(streams.front().find("cm"), std::string::npos);
    EXPECT_EQ(streams.front().find("q "), 0u);
    EXPECT_EQ(streams.back(), "Q\n");
    std::filesystem::remove(out2);
}

TEST(PdfFileEditorOpsSmoke, AddMargins) {
    const std::string out = Tmp("margins.pdf");
    PdfFileEditor ed;
    ASSERT_TRUE(ed.AddMargins(HelloWorldPdf(), out, {1}, 10, 10, 20, 20));
    Document re{out};
    EXPECT_EQ(re.Pages().Count(), 1u);
    const auto page = re.Pages()[1];
    // The page grows by the margins...
    EXPECT_FLOAT_EQ(page.Rect().Width(), 632.0);
    EXPECT_FLOAT_EQ(page.Rect().Height(), 832.0);
    // ...and the content is translated to the new bottom-left origin
    // (left = 10, bottom = 20 stay blank).
    const auto streams = PageContentStreams(out, 1);
    ASSERT_GE(streams.size(), 3u);
    EXPECT_NE(streams.front().find("q 1 0 0 1 10 20 cm"), std::string::npos);
    std::filesystem::remove(out);
}

TEST(PdfFileEditorOpsSmoke, TryMakeBookletFailureReturnsFalse) {
    const std::string out = Tmp("try_booklet_fail.pdf");
    PdfFileEditor ed;
    EXPECT_FALSE(ed.TryMakeBooklet("non_existent_file.pdf", out));
    EXPECT_FALSE(std::filesystem::exists(out));
}

TEST(PdfFileEditorOpsSmoke, TryResizeContentsFailureReturnsFalse) {
    const std::string out = Tmp("try_resize_fail.pdf");
    PdfFileEditor ed;
    PdfFileEditor::ContentsResizeParameters params;
    EXPECT_FALSE(ed.TryResizeContents("non_existent_file.pdf", out, {1}, params));
    EXPECT_FALSE(std::filesystem::exists(out));
}

TEST(PdfFileEditorOpsSmoke, AddPageBreak) {
    const std::string out = Tmp("pagebreak.pdf");
    PdfFileEditor ed;
    // Empty breaks is a safe no-op round-trip.
    EXPECT_TRUE(ed.AddPageBreak(HelloWorldPdf(), out, {}));
    EXPECT_TRUE(std::filesystem::exists(out));
    std::filesystem::remove(out);

    // A break at y=100 on page 1 splits it into two same-size pages.
    PdfFileEditor::PageBreak pb{1, 100.0};
    ASSERT_TRUE(ed.AddPageBreak(HelloWorldPdf(), out, {pb}));
    Document re{out};
    ASSERT_EQ(re.Pages().Count(), 2u);
    EXPECT_FLOAT_EQ(re.Pages()[1].Rect().Width(), 612.0);
    EXPECT_FLOAT_EQ(re.Pages()[1].Rect().Height(), 792.0);
    EXPECT_FLOAT_EQ(re.Pages()[2].Rect().Width(), 612.0);
    EXPECT_FLOAT_EQ(re.Pages()[2].Rect().Height(), 792.0);
    // Clip-based split: both band pages carry the original text.
    EXPECT_NE(DocText(out).find("Hello World"), std::string::npos);
    // Each band's content is wrapped in a `re W n` clip.
    for (int p = 1; p <= 2; ++p) {
        const auto streams = PageContentStreams(out, p);
        ASSERT_GE(streams.size(), 3u);
        EXPECT_NE(streams.front().find("re W n"), std::string::npos);
        EXPECT_EQ(streams.back(), "Q\n");
    }
    std::filesystem::remove(out);
}

