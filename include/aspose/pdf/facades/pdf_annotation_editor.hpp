#pragma once

// =============================================================================
// Aspose::Pdf::Facades::PdfAnnotationEditor — annotation-level editor
// for the bound PDF (import / export / extract / modify / delete /
// flatten / redact). Mirrors canonical
// Aspose.Pdf.Facades.PdfAnnotationEditor; extends SaveableFacade.
//
// v1 capabilities:
//   * Deletion is REAL — DeleteAnnotations() clears every page's
//     annotation collection; DeleteAnnotation(name) removes the
//     uniquely-named annotation; DeleteAnnotations(type) deletes by
//     canonical type name.
//   * Import (XFDF) is REAL — imports supported annotation subtypes
//     from XFDF files with XML entity decoding and type filtering.
//   * Modify is REAL — ModifyAnnotations and ModifyAnnotationsAuthor.
//   * Flattening is REAL — burns annotation appearances into static
//     page content streams.
//   * Import (FDF) is a deliberate stub (FDF uses PDF COS syntax).
//
// Phased drops:
//   * Every Stream-based overload (Import/Export with a Stream or
//     Stream[] parameter) — Stream cascade.
//   * ModifyAnnotations(int, int, System.Enum, Annotation) — the
//     untyped System.Enum parameter is non-idiomatic; the
//     (int, int, Annotation) overload is the idiomatic surface.
//   * ExtractAnnotations ×2 — return IList<Annotation> (abstract
//     element); the C++ idiom iterates the page collections
//     directly (drops.yaml).
//   * RedactArea — System.Drawing.Color cascade.
// =============================================================================

#include <memory>
#include <string>
#include <vector>

#include <aspose/pdf/annotations/annotation_type.hpp>
#include <aspose/pdf/facades/saveable_facade.hpp>
#include <aspose/pdf/forms/form.hpp>

namespace Aspose::Pdf {
class Document;
namespace Annotations { class Annotation; }
}

// MSVC C4250: BindPdf/Close are inherited from Facade by dominance over
// the virtual IFacade base also reached via ISaveableFacade — intended.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

namespace Aspose::Pdf::Facades {

class PdfAnnotationEditor : public SaveableFacade {
public:
    PdfAnnotationEditor() noexcept = default;
    explicit PdfAnnotationEditor(Aspose::Pdf::Document& document);

    // ---- Import ----

    void ImportAnnotationsFromXfdf(const std::string& xfdfFile);
    void ImportAnnotationsFromFdf(const std::string& fdfFile);
    void ImportAnnotationFromXfdf(const std::string& xfdfFile);
    void ImportAnnotationFromXfdf(
        const std::string& xfdfFile,
        const std::vector<Aspose::Pdf::Annotations::AnnotationType>& annotType);
    void ImportAnnotations(
        const std::vector<std::string>& annotFile,
        const std::vector<Aspose::Pdf::Annotations::AnnotationType>& annotType);
    void ImportAnnotations(const std::vector<std::string>& annotFile);

    // ---- Modify ----

    void ModifyAnnotations(int start, int end,
                           Aspose::Pdf::Annotations::Annotation& annotation);
    void ModifyAnnotationsAuthor(int start, int end,
                                 const std::string& srcAuthor,
                                 const std::string& desAuthor);

    // ---- Flatten ----

    void FlatteningAnnotations();
    void FlatteningAnnotations(
        const Aspose::Pdf::Forms::Form::FlattenSettings& flattenSettings);
    void FlatteningAnnotations(
        int start, int end,
        const std::vector<Aspose::Pdf::Annotations::AnnotationType>& annotType);

    // ---- Delete ----

    // Clear every page's annotation collection (real).
    void DeleteAnnotations();
    // Delete annotations of the given subtype name (real).
    void DeleteAnnotations(const std::string& annotType);
    // Delete the uniquely-named annotation across all pages (real).
    void DeleteAnnotation(const std::string& annotName);
};

}  // namespace Aspose::Pdf::Facades

#ifdef _MSC_VER
#pragma warning(pop)
#endif
