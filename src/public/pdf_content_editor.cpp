#include <aspose/pdf/facades/pdf_content_editor.hpp>

#include <fstream>
#include <vector>

#include <aspose/pdf/document.hpp>
#include <aspose/pdf/embedded_file_collection.hpp>
#include <aspose/pdf/file_specification.hpp>
#include <aspose/pdf/page.hpp>
#include <aspose/pdf/page_collection.hpp>
#include <aspose/pdf/resources.hpp>
#include <aspose/pdf/x_image_collection.hpp>

namespace Aspose::Pdf::Facades {

PdfContentEditor::PdfContentEditor(Aspose::Pdf::Document& document) {
    BindPdf(document);
}

void PdfContentEditor::AddDocumentAttachment(const std::string& path,
                                             const std::string& description) {
    if (document_ == nullptr) return;
    document_->EmbeddedFiles().Add(Aspose::Pdf::FileSpecification(path, description));
}

void PdfContentEditor::DeleteAttachments() {
    if (document_ == nullptr) return;
    document_->EmbeddedFiles().Delete();
}

// ===== Document Actions (v1 stubs) =========================================

void PdfContentEditor::AddDocumentAdditionalAction(const std::string&,
                                                   const std::string&) {}
void PdfContentEditor::RemoveDocumentOpenAction() {}

// ===== Viewer Preferences (v1 stubs) =======================================

void PdfContentEditor::ChangeViewerPreference(int) {}
int PdfContentEditor::GetViewerPreference() { return 0; }

// ===== Image Operations (real) =============================================

void PdfContentEditor::ReplaceImage(int pageNum, int imageNum, const std::string& fileName) {
    if (document_ == nullptr) return;
    if (pageNum < 1 || pageNum > static_cast<int>(document_->Pages().Count())) return;
    std::ifstream f(fileName, std::ios::binary);
    if (!f.is_open()) return;
    f.seekg(0, std::ios::end);
    auto size = f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<std::byte> data(size);
    f.read(reinterpret_cast<char*>(data.data()), size);

    auto page = document_->Pages()[pageNum];
    page.Resources().Images().Replace(imageNum, data);
}
void PdfContentEditor::DeleteImage(int pageNum,
                                   const std::vector<int>& imageNum) {
    if (document_ == nullptr) return;
    document_->DeleteImagesFromPage(pageNum, imageNum);
}
void PdfContentEditor::DeleteImage() {
    if (document_ == nullptr) return;
    document_->DeleteImagesFromPage(0, {});  // every image on every page
}

// ===== Text Operations (real) ==============================================

bool PdfContentEditor::ReplaceText(const std::string& srcText,
                                   const std::string& destText) {
    if (document_ == nullptr) return false;
    return document_->ReplaceTextInContent(srcText, destText, 0) > 0;
}
bool PdfContentEditor::ReplaceText(const std::string& srcText, int pageNum,
                                   const std::string& destText) {
    if (document_ == nullptr) return false;
    return document_->ReplaceTextInContent(srcText, destText, pageNum) > 0;
}
bool PdfContentEditor::ReplaceText(const std::string& srcText,
                                   const std::string& destText, int pageNum) {
    if (document_ == nullptr) return false;
    return document_->ReplaceTextInContent(srcText, destText, pageNum) > 0;
}

// ===== Stamp Operations (v1 stubs) =========================================

void PdfContentEditor::DeleteStamp(int, const std::vector<int>&) {}
void PdfContentEditor::DeleteStampByIds(const std::vector<int>&) {}
void PdfContentEditor::DeleteStampByIds(int, const std::vector<int>&) {}
void PdfContentEditor::DeleteStampById(int, int) {}
void PdfContentEditor::DeleteStampById(int) {}
void PdfContentEditor::HideStampById(int, int) {}
void PdfContentEditor::ShowStampById(int, int) {}
void PdfContentEditor::MoveStampById(int, int, double, double) {}
void PdfContentEditor::MoveStamp(int, int, double, double) {}

}  // namespace Aspose::Pdf::Facades
