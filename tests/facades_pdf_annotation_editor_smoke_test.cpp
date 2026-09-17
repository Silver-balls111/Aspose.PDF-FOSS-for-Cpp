// =============================================================================
// facades_pdf_annotation_editor_smoke_test — beat Fa4 of the Facades
// cluster. PdfAnnotationEditor extends SaveableFacade and edits the
// bound document's annotations: deletion and XFDF import are real, and
// FlatteningAnnotations burns each annotation's appearance into the page
// content (via Document::FlattenPageAnnotations) before removing it.
// =============================================================================

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include <aspose/pdf/annotations/annotation_collection.hpp>
#include <aspose/pdf/annotations/annotation_type.hpp>
#include <aspose/pdf/annotations/circle_annotation.hpp>
#include <aspose/pdf/annotations/text_annotation.hpp>
#include <aspose/pdf/document.hpp>
#include <aspose/pdf/facades/pdf_annotation_editor.hpp>
#include <aspose/pdf/page.hpp>
#include <aspose/pdf/page_collection.hpp>
#include <aspose/pdf/rectangle.hpp>

#include "objects.hpp"
#include "pages_tree.hpp"
#include "trailer.hpp"

#include <fstream>
#include <gtest/gtest.h>

namespace {

using namespace Aspose::Pdf;
using namespace Aspose::Pdf::Annotations;
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

TEST(FacadesPdfAnnotationEditorSmoke, DeleteAllAnnotationsClearsEveryPage) {
    Document doc{HelloWorldPdf()};
    TextAnnotation a{doc};
    CircleAnnotation b{doc};
    doc.Pages()[1].Annotations().Add(a);
    doc.Pages()[1].Annotations().Add(b);
    ASSERT_EQ(doc.Pages()[1].Annotations().Count(), 2);

    PdfAnnotationEditor editor{doc};
    editor.DeleteAnnotations();
    EXPECT_EQ(doc.Pages()[1].Annotations().Count(), 0);
}

TEST(FacadesPdfAnnotationEditorSmoke, DeleteAnnotationByNameRemovesOne) {
    Document doc{HelloWorldPdf()};
    TextAnnotation a{doc};
    TextAnnotation b{doc};
    a.Name("first");
    b.Name("second");
    doc.Pages()[1].Annotations().Add(a);
    doc.Pages()[1].Annotations().Add(b);
    ASSERT_EQ(doc.Pages()[1].Annotations().Count(), 2);

    PdfAnnotationEditor editor{doc};
    editor.DeleteAnnotation("first");

    auto& annots = doc.Pages()[1].Annotations();
    ASSERT_EQ(annots.Count(), 1);
    EXPECT_EQ(annots[0].Name(), "second");
}

TEST(FacadesPdfAnnotationEditorSmoke, DeleteAnnotationUnknownNameNoOp) {
    Document doc{HelloWorldPdf()};
    TextAnnotation a{doc};
    a.Name("only");
    doc.Pages()[1].Annotations().Add(a);

    PdfAnnotationEditor editor{doc};
    editor.DeleteAnnotation("missing");
    EXPECT_EQ(doc.Pages()[1].Annotations().Count(), 1);
}

TEST(FacadesPdfAnnotationEditorSmoke, StubsDoNotThrow) {
    Document doc{HelloWorldPdf()};
    PdfAnnotationEditor editor{doc};

    const std::vector<AnnotationType> types{AnnotationType::Text};
    const std::vector<std::string> files{"a.xfdf"};

    // Import / modify / flatten / delete-by-subtype are v1 stubs:
    // they must bind + run without throwing on a bound document.
    editor.ImportAnnotationsFromXfdf("a.xfdf");
    editor.ImportAnnotationsFromFdf("a.fdf");
    editor.ImportAnnotationFromXfdf("a.xfdf");
    editor.ImportAnnotationFromXfdf("a.xfdf", types);
    editor.ImportAnnotations(files, types);
    editor.ImportAnnotations(files);
    editor.ModifyAnnotationsAuthor(1, 1, "old", "new");
    editor.FlatteningAnnotations();
    editor.FlatteningAnnotations(1, 1, types);
    editor.DeleteAnnotations("Text");  // delete-by-subtype stub

    // ModifyAnnotations(start, end, Annotation&) stub.
    TextAnnotation tmpl{doc};
    editor.ModifyAnnotations(1, 1, tmpl);

    SUCCEED();
}

TEST(FacadesPdfAnnotationEditorSmoke, ImportFromXfdf) {
    std::string xfdf = (std::filesystem::temp_directory_path() / "test_import.xfdf").string();
    {
        std::ofstream out(xfdf);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<xfdf xmlns=\"http://ns.adobe.com/xfdf/\">\n"
            << "<annotations>\n"
            << "  <square page=\"0\" rect=\"10,20,110,34\" title=\"Author1\" contents=\"Square Note\"/>\n"
            << "  <text page=\"0\" rect=\"10,50,110,70\" title=\"Author2\" contents=\"Sticky Note\"/>\n"
            << "</annotations>\n"
            << "</xfdf>\n";
    }

    Document doc{HelloWorldPdf()};
    PdfAnnotationEditor editor{doc};
    editor.ImportAnnotationsFromXfdf(xfdf);

    ASSERT_EQ(doc.Pages()[1].Annotations().Count(), 2);
    EXPECT_EQ(doc.Pages()[1].Annotations()[0].Contents(), "Square Note");
    EXPECT_EQ(doc.Pages()[1].Annotations()[1].Contents(), "Sticky Note");

    // Test ModifyAnnotationsAuthor
    editor.ModifyAnnotationsAuthor(1, 1, "Author1", "NewAuthor");
    auto* ma = dynamic_cast<MarkupAnnotation*>(&doc.Pages()[1].Annotations()[0]);
    ASSERT_NE(ma, nullptr);
    EXPECT_EQ(ma->Title(), "NewAuthor");

    // Test Flattening by type
    editor.FlatteningAnnotations(1, 1, {AnnotationType::Square});
    ASSERT_EQ(doc.Pages()[1].Annotations().Count(), 1);
    EXPECT_EQ(doc.Pages()[1].Annotations()[0].AnnotationType(), AnnotationType::Text);

    std::filesystem::remove(xfdf);
}

TEST(FacadesPdfAnnotationEditorSmoke, XfdfChildContentsEntitiesAndMalformedRects) {
    std::string xfdf = (std::filesystem::temp_directory_path() / "test_advanced.xfdf").string();
    {
        std::ofstream out(xfdf);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<xfdf xmlns=\"http://ns.adobe.com/xfdf/\">\n"
            << "<annotations>\n"
            << "  <square page=\"0\" rect=\"10,20,110,34\" title=\"Fish &amp; Chips\">\n"
            << "    <contents>Note with &lt;special&gt; &amp; &quot;quoted&quot; text</contents>\n"
            << "  </square>\n"
            << "  <circle page=\"0\" rect=\"malformed,rect\" title=\"Bad\">\n"
            << "    <contents>Should be skipped</contents>\n"
            << "  </circle>\n"
            << "  <text page=\"0\" rect=\"10,50,110,70\" title=\"Author &apos;Bob&apos;\" contents=\"Attribute &amp; Note\"/>\n"
            << "</annotations>\n"
            << "</xfdf>\n";
    }

    Document doc{HelloWorldPdf()};
    {
        PdfAnnotationEditor editor{doc};
        editor.ImportAnnotationsFromXfdf(xfdf);
    }
    // Verifying no UAF: editor was destroyed, doc and page annotations remain valid!
    ASSERT_EQ(doc.Pages()[1].Annotations().Count(), 2);  // Malformed circle skipped
    EXPECT_EQ(doc.Pages()[1].Annotations()[0].Contents(),
              "Note with <special> & \"quoted\" text");
    auto* ma0 = dynamic_cast<MarkupAnnotation*>(&doc.Pages()[1].Annotations()[0]);
    ASSERT_NE(ma0, nullptr);
    EXPECT_EQ(ma0->Title(), "Fish & Chips");

    EXPECT_EQ(doc.Pages()[1].Annotations()[1].Contents(), "Attribute & Note");
    auto* ma1 = dynamic_cast<MarkupAnnotation*>(&doc.Pages()[1].Annotations()[1]);
    ASSERT_NE(ma1, nullptr);
    EXPECT_EQ(ma1->Title(), "Author 'Bob'");

    std::string out = (std::filesystem::temp_directory_path() / "test_uaf_saved.pdf").string();
    EXPECT_NO_THROW(doc.Save(out));
    EXPECT_TRUE(std::filesystem::exists(out));
    std::filesystem::remove(out);
    std::filesystem::remove(xfdf);
}

TEST(FacadesPdfAnnotationEditorSmoke, XfdfTypeFiltering) {
    std::string xfdf = (std::filesystem::temp_directory_path() / "test_filter.xfdf").string();
    {
        std::ofstream out(xfdf);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<xfdf xmlns=\"http://ns.adobe.com/xfdf/\">\n"
            << "<annotations>\n"
            << "  <square page=\"0\" rect=\"10,20,110,34\" contents=\"Square Note\"/>\n"
            << "  <text page=\"0\" rect=\"10,50,110,70\" contents=\"Text Note\"/>\n"
            << "</annotations>\n"
            << "</xfdf>\n";
    }

    Document doc{HelloWorldPdf()};
    PdfAnnotationEditor editor{doc};
    // Filter to only import Text annotations
    editor.ImportAnnotationFromXfdf(xfdf, {AnnotationType::Text});

    ASSERT_EQ(doc.Pages()[1].Annotations().Count(), 1);
    EXPECT_EQ(doc.Pages()[1].Annotations()[0].AnnotationType(), AnnotationType::Text);
    EXPECT_EQ(doc.Pages()[1].Annotations()[0].Contents(), "Text Note");

    std::filesystem::remove(xfdf);
}

TEST(FacadesPdfAnnotationEditorSmoke, UnboundDeleteIsSafe) {
    PdfAnnotationEditor editor;  // nothing bound
    editor.DeleteAnnotations();
    editor.DeleteAnnotation("anything");
    SUCCEED();
}

namespace {

std::vector<std::byte> ReadPdfBytes(const std::string& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    const auto end = in.tellg();
    std::vector<std::byte> bytes(static_cast<std::size_t>(end));
    in.seekg(0, std::ios::beg);
    in.read(reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    return bytes;
}

const foundation::objects::IndirectObject* FindObj(
        const foundation::objects::Dump& dump, std::uint32_t id) {
    for (const auto& o : dump.objects)
        if (o.id == id) return &o;
    return nullptr;
}

const foundation::objects::Value* DictVal(
        const foundation::objects::Dict& d, const std::string& key) {
    for (const auto& kv : d.entries)
        if (kv.first == key) return &kv.second;
    return nullptr;
}

}  // namespace

TEST(FacadesPdfAnnotationEditorSmoke, FlattenBurnsAppearanceIntoPageContent) {
    const std::string out =
        (std::filesystem::temp_directory_path() /
         "annot_flatten_burn.pdf").string();

    Document doc{HelloWorldPdf()};
    // A circle (generated appearance) + a sticky note (no appearance).
    CircleAnnotation c{doc};
    c.Rect(Rectangle{40.0, 500.0, 240.0, 600.0, false});
    TextAnnotation t{doc};
    t.Rect(Rectangle{40.0, 700.0, 140.0, 720.0, false});
    auto& annots = doc.Pages()[1].Annotations();
    annots.Add(c);
    annots.Add(t);

    PdfAnnotationEditor editor{doc};
    editor.FlatteningAnnotations();
    // Both annotations are removed from the live collection…
    EXPECT_EQ(annots.Count(), 0);

    doc.Save(out);

    // …and after save the circle's appearance is static page content:
    // the page gains a FlaF* Form XObject drawn through a burn stream,
    // while /Annots stays absent (the sticky note had no appearance to
    // burn and was never persisted).
    const auto bytes = ReadPdfBytes(out);
    std::span<const std::byte> sp(bytes.data(), bytes.size());
    const auto tree = foundation::pages_tree::Parse(sp);
    const auto dump = foundation::objects::Parse(sp);
    ASSERT_FALSE(tree.leaves.empty());
    const auto* page_obj = FindObj(dump, tree.leaves[0].id);
    ASSERT_NE(page_obj, nullptr);
    const auto* page_dict =
        std::get_if<foundation::objects::Dict>(&page_obj->value.v);
    ASSERT_NE(page_dict, nullptr);

    // /Annots absent (or empty).
    if (const auto* annots_v = DictVal(*page_dict, "Annots")) {
        if (const auto* arr =
                std::get_if<foundation::objects::Array>(&annots_v->v))
            EXPECT_TRUE(arr->items.empty());
    }

    // /Resources /XObject carries the burn form.
    const auto* res_v = DictVal(*page_dict, "Resources");
    ASSERT_NE(res_v, nullptr);
    const foundation::objects::Dict* res_dict = nullptr;
    if (const auto* d = std::get_if<foundation::objects::Dict>(&res_v->v))
        res_dict = d;
    else if (const auto* r = std::get_if<foundation::objects::Ref>(&res_v->v))
        if (const auto* o = FindObj(dump, r->id))
            res_dict = std::get_if<foundation::objects::Dict>(&o->value.v);
    ASSERT_NE(res_dict, nullptr);
    const auto* xo_v = DictVal(*res_dict, "XObject");
    ASSERT_NE(xo_v, nullptr);
    const auto* xo_dict =
        std::get_if<foundation::objects::Dict>(&xo_v->v);
    ASSERT_NE(xo_dict, nullptr);
    std::uint32_t burn_form_id = 0;
    for (const auto& kv : xo_dict->entries) {
        if (kv.first.rfind("FlaF", 0) != 0) continue;
        const auto* r = std::get_if<foundation::objects::Ref>(&kv.second.v);
        ASSERT_NE(r, nullptr);
        burn_form_id = r->id;
    }
    ASSERT_NE(burn_form_id, 0u);
    const auto* form_obj = FindObj(dump, burn_form_id);
    ASSERT_NE(form_obj, nullptr);
    const auto* form_stream =
        std::get_if<foundation::objects::Stream>(&form_obj->value.v);
    ASSERT_NE(form_stream, nullptr);
    const std::string form_body(
        reinterpret_cast<const char*>(form_stream->body.data()),
        form_stream->body.size());
    // The circle recipe: dashed stroke + bezier ellipse + stroke op.
    EXPECT_NE(form_body.find("[3 3] 0 d"), std::string::npos);
    EXPECT_NE(form_body.find(" c "), std::string::npos);

    // The burn stream on the page invokes the form.
    const auto* contents_v = DictVal(*page_dict, "Contents");
    ASSERT_NE(contents_v, nullptr);
    const auto* contents_arr =
        std::get_if<foundation::objects::Array>(&contents_v->v);
    ASSERT_NE(contents_arr, nullptr);
    ASSERT_FALSE(contents_arr->items.empty());
    const auto* last_ref =
        std::get_if<foundation::objects::Ref>(&contents_arr->items.back().v);
    ASSERT_NE(last_ref, nullptr);
    const auto* burn_obj = FindObj(dump, last_ref->id);
    ASSERT_NE(burn_obj, nullptr);
    const auto* burn_stream =
        std::get_if<foundation::objects::Stream>(&burn_obj->value.v);
    ASSERT_NE(burn_stream, nullptr);
    const std::string burn_body(
        reinterpret_cast<const char*>(burn_stream->body.data()),
        burn_stream->body.size());
    EXPECT_NE(burn_body.find("/FlaF"), std::string::npos);
    EXPECT_NE(burn_body.find(" Do"), std::string::npos);

    std::filesystem::remove(out);
}
