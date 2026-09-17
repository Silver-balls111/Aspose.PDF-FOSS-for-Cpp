#pragma once

// =============================================================================
// Aspose::Pdf::Facades::PdfContentEditor — content / link / stamp / image /
// attachment / text editor. Mirrors canonical
// Aspose.Pdf.Facades.PdfContentEditor; extends SaveableFacade.
//
// v1 baseline: text replace routes through Document::ReplaceTextInContent
// (real content-stream rewrite), attachments use EmbeddedFiles, and
// ReplaceImage / DeleteImage edit the page /Resources /XObject images.
// Still unsupported in v1 (no-ops / defaults): document additional
// actions, viewer preferences, and the stamp-management methods —
// stamps are one-shot content-stream writes with no persistent
// identity for a by-id registry to target.
//
// The bulk of the canonical surface drops in cpp: every Create*/Draw*
// overload routes through System.Drawing.Rectangle / System.Drawing.Color
// (geometry/colour cascades), and the text-edit/replace options +
// Stamp/LineInfo/StampInfo types aren't in the cpp catalog. v1 ships the
// primitive-typed editor surface (attachments / viewer-preference /
// image-replace / text-replace(string) / stamp visibility + move).
// =============================================================================

#include <string>
#include <vector>

#include <aspose/pdf/facades/saveable_facade.hpp>

namespace Aspose::Pdf {
class Document;
}

namespace Aspose::Pdf::Facades {

class PdfContentEditor : public SaveableFacade {
public:
    // ---- Document-action trigger names (canonical) ----
    inline static const std::string DocumentOpen = "DO";
    inline static const std::string DocumentClose = "WC";
    inline static const std::string DocumentWillSave = "WS";
    inline static const std::string DocumentSaved = "DS";
    inline static const std::string DocumentWillPrint = "WP";
    inline static const std::string DocumentPrinted = "DP";

    PdfContentEditor() noexcept = default;
    explicit PdfContentEditor(Aspose::Pdf::Document& document);

    // ---- Attachments (real) ----
    void AddDocumentAttachment(const std::string& path,
                               const std::string& description);
    void DeleteAttachments();

    // ---- Document actions (v1 stubs) ----
    void AddDocumentAdditionalAction(const std::string& actionName,
                                     const std::string& javaScript);
    void RemoveDocumentOpenAction();

    // ---- Viewer preference (v1 stubs) ----
    void ChangeViewerPreference(int pageLayout);
    int GetViewerPreference();

    // ---- Image (DeleteImage & ReplaceImage real) ----
    void ReplaceImage(int pageNum, int imageNum, const std::string& fileName);
    void DeleteImage(int pageNum, const std::vector<int>& imageNum);
    void DeleteImage();

    // ---- Text replace (real — Document::ReplaceTextInContent) ----
    bool ReplaceText(const std::string& srcText, const std::string& destText);
    bool ReplaceText(const std::string& srcText, int pageNum,
                     const std::string& destText);
    bool ReplaceText(const std::string& srcText, const std::string& destText,
                     int pageNum);

    // ---- Stamps (v1 stubs) ----
    void DeleteStamp(int pageNum, const std::vector<int>& stampId);
    void DeleteStampByIds(const std::vector<int>& stampIds);
    void DeleteStampByIds(int pageNum, const std::vector<int>& stampIds);
    void DeleteStampById(int pageNum, int stampId);
    void DeleteStampById(int stampId);
    void HideStampById(int pageNum, int stampId);
    void ShowStampById(int pageNum, int stampId);
    void MoveStampById(int pageNum, int stampId, double offsetX,
                       double offsetY);
    void MoveStamp(int pageNum, int stampId, double offsetX, double offsetY);
};

}  // namespace Aspose::Pdf::Facades
