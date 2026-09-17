// =============================================================================
// facades_pdf_page_editor_smoke_test — beat Fa11 of the Facades cluster.
// PdfPageEditor stages page-level edits (move/rotate/zoom/align +
// transitions). GetPages / GetPageSize / GetPageRotation query the bound
// document, and ApplyChanges applies the staged rotation, page size, and
// MovePosition content translation. 16 page-transition constants are
// pinned to canonical values.
// =============================================================================

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <vector>

#include <aspose/pdf/document.hpp>
#include <aspose/pdf/facades/alignment_type.hpp>
#include <aspose/pdf/facades/pdf_page_editor.hpp>
#include <aspose/pdf/facades/vertical_alignment_type.hpp>
#include <aspose/pdf/horizontal_alignment.hpp>
#include <aspose/pdf/page_collection.hpp>
#include <aspose/pdf/page_size.hpp>
#include <aspose/pdf/vertical_alignment.hpp>

#include "objects.hpp"
#include "pages_tree.hpp"
#include "trailer.hpp"

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

}  // namespace

TEST(FacadesPdfPageEditorSmoke, TransitionConstants) {
    EXPECT_EQ(PdfPageEditor::SPLITVOUT, 1);
    EXPECT_EQ(PdfPageEditor::SPLITHIN, 4);
    EXPECT_EQ(PdfPageEditor::INBOX, 7);
    EXPECT_EQ(PdfPageEditor::DISSOLVE, 13);
    EXPECT_EQ(PdfPageEditor::DGLITTER, 16);
}

TEST(FacadesPdfPageEditorSmoke, GetPagesReal) {
    Document doc{HelloWorldPdf()};
    PdfPageEditor editor{doc};
    EXPECT_EQ(editor.GetPages(), static_cast<int>(doc.Pages().Count()));
    EXPECT_GT(editor.GetPages(), 0);
}

TEST(FacadesPdfPageEditorSmoke, GeometryAndApplyChanges) {
    Document doc{HelloWorldPdf()};
    PdfPageEditor editor{doc};
    auto sz = editor.GetPageSize(1);
    EXPECT_FLOAT_EQ(sz.Width(), 612.0f);
    EXPECT_FLOAT_EQ(sz.Height(), 792.0f);
    EXPECT_EQ(editor.GetPageRotation(1), 0);

    // Stage rotation and page size, then apply
    editor.Rotation(90);
    editor.PageSize(PageSize::A4());
    editor.ApplyChanges();

    EXPECT_EQ(editor.GetPageRotation(1), 90);
    auto newSz = editor.GetPageSize(1);
    EXPECT_NEAR(newSz.Width(), PageSize::A4().Width(), 1.0f);
    EXPECT_NEAR(newSz.Height(), PageSize::A4().Height(), 1.0f);
}

TEST(FacadesPdfPageEditorSmoke, UnboundGetPagesZero) {
    PdfPageEditor editor;
    EXPECT_EQ(editor.GetPages(), 0);
}

TEST(FacadesPdfPageEditorSmoke, MovePositionTranslatesContentOnApply) {
    Document doc{HelloWorldPdf()};
    PdfPageEditor editor{doc};
    editor.MovePosition(15.0f, 25.0f);
    editor.ApplyChanges();

    const std::string out =
        (std::filesystem::temp_directory_path() / "pageeditor_move.pdf")
            .string();
    doc.Save(out);

    // The content streams gain a `q 1 0 0 1 15 25 cm` wrapper + `Q`.
    std::ifstream in(out, std::ios::binary | std::ios::ate);
    const auto end = in.tellg();
    std::vector<std::byte> bytes(static_cast<std::size_t>(end));
    in.seekg(0, std::ios::beg);
    in.read(reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    in.close();
    std::span<const std::byte> sp(bytes.data(), bytes.size());
    const auto tree = foundation::pages_tree::Parse(sp);
    const auto dump = foundation::objects::Parse(sp);
    ASSERT_FALSE(tree.leaves.empty());
    bool found_wrapper = false;
    std::vector<std::uint32_t> stream_ids;
    for (const auto& o : dump.objects) {
        if (o.id != tree.leaves[0].id) continue;
        const auto* pd = std::get_if<foundation::objects::Dict>(&o.value.v);
        ASSERT_NE(pd, nullptr);
        for (const auto& kv : pd->entries) {
            if (kv.first != "Contents") continue;
            if (const auto* r =
                    std::get_if<foundation::objects::Ref>(&kv.second.v))
                stream_ids.push_back(r->id);
            else if (const auto* a = std::get_if<foundation::objects::Array>(
                         &kv.second.v))
                for (const auto& it : a->items)
                    if (const auto* rr =
                            std::get_if<foundation::objects::Ref>(&it.v))
                        stream_ids.push_back(rr->id);
        }
    }
    ASSERT_GE(stream_ids.size(), 3u);  // wrapper + original + wrapper
    for (std::uint32_t sid : stream_ids) {
        for (const auto& o : dump.objects) {
            if (o.id != sid) continue;
            const auto* st =
                std::get_if<foundation::objects::Stream>(&o.value.v);
            if (st == nullptr) break;
            const std::string body(
                reinterpret_cast<const char*>(st->body.data()),
                st->body.size());
            if (body.find("q 1 0 0 1 15 25 cm") != std::string::npos)
                found_wrapper = true;
            break;
        }
    }
    EXPECT_TRUE(found_wrapper);
    EXPECT_EQ(std::filesystem::remove(out), true);
}

TEST(FacadesPdfPageEditorSmoke, PropertyRoundtrip) {
    PdfPageEditor editor;
    editor.TransitionDuration(5);
    editor.TransitionType(PdfPageEditor::BLINDV);
    editor.DisplayDuration(3);
    editor.ProcessPages(std::vector<int>{1, 3, 5});
    editor.Rotation(90);
    editor.Zoom(1.5f);
    editor.PageSize(PageSize::A4());
    editor.Alignment(AlignmentType::Center());
    editor.HorizontalAlignment(HorizontalAlignment::Left);
    editor.VerticalAlignment(VerticalAlignmentType::Bottom());
    editor.VerticalAlignmentType(VerticalAlignment::Top);

    EXPECT_EQ(editor.TransitionDuration(), 5);
    EXPECT_EQ(editor.TransitionType(), PdfPageEditor::BLINDV);
    EXPECT_EQ(editor.DisplayDuration(), 3);
    ASSERT_EQ(editor.ProcessPages().size(), 3u);
    EXPECT_EQ(editor.ProcessPages()[1], 3);
    EXPECT_EQ(editor.Rotation(), 90);
    EXPECT_FLOAT_EQ(editor.Zoom(), 1.5f);
    EXPECT_FLOAT_EQ(editor.PageSize().Width(), PageSize::A4().Width());
    EXPECT_EQ(editor.Alignment(), AlignmentType::Center());
    EXPECT_EQ(editor.HorizontalAlignment(), HorizontalAlignment::Left);
    EXPECT_EQ(editor.VerticalAlignment(), VerticalAlignmentType::Bottom());
    EXPECT_EQ(editor.VerticalAlignmentType(), VerticalAlignment::Top);
}
