# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added / Implemented
- **`Aspose::Pdf::Facades::FormEditor`**:
  - Implemented field attribute modifications (`SetFieldAttribute` for `ReadOnly`, `Required`, `NoExport`).
  - Implemented field appearance flags (`SetFieldAppearance` / `GetFieldAppearance`).
  - Implemented field constraints and text properties (`SetFieldLimit`, `SetFieldCombNumber`, `Single2Multiple`).
  - Implemented field positioning and text alignment (`MoveField`, `SetFieldAlignment`, `SetFieldAlignmentV`, `RenameField`).
  - Implemented choice field option management (`AddListItem`, `DelListItem`).
  - Implemented field decoration and copying (`DecorateField`, `CopyInnerField`, `CopyOuterField`, `AddSubmitBtn`).
- **`Aspose::Pdf::Facades::PdfFileEditor`**:
  - Implemented booklet layout creation (`MakeBooklet`, `TryMakeBooklet`).
  - Implemented multi-document sheet merging (`MakeNUp`, `TryMakeNUp`).
  - Implemented content resizing and margins (`ResizeContents`, `ResizeContentsPct`, `TryResizeContents`, `AddMargins`, `AddMarginsPct`, `AddPageBreak`).
- **`Aspose::Pdf::Facades::PdfAnnotationEditor`**:
  - Implemented XFDF annotation import (`ImportAnnotationsFromXfdf`) supporting `Square`, `Circle`, `Text`, and `Highlight` annotations.
  - Implemented annotation modification (`ModifyAnnotations`, `ModifyAnnotationsAuthor`) and flattening (`FlatteningAnnotations`).
- **`Aspose::Pdf::Facades::PdfBookmarkEditor`**:
  - Implemented XML bookmark export and import (`ExportBookmarksToXML`, `ImportBookmarksWithXML`).
  - Implemented HTML bookmark extraction (`ExportBookmarksToHtml`, `ExtractBookmarksToHTML`).
- **`Aspose::Pdf::Facades::PdfContentEditor`**:
  - Implemented document attachment manipulation (`AddDocumentAttachment`, `DeleteAttachments`).
  - Implemented page image replacement (`ReplaceImage`).
- **`Aspose::Pdf::Facades::PdfFileInfo` & `Aspose::Pdf::Facades::PdfPageEditor`**:
  - Implemented page dimension and rotation queries (`GetPageWidth`, `GetPageHeight`, `GetPageRotation`, `GetPageSize`).
  - Implemented page rotation/resizing staged writes and `ApplyChanges()`.
  - Implemented PDF header version detection (`GetPdfVersion`) and encryption detection (`HasOpenPassword`, `HasEditPassword`).
- **`Aspose::Pdf::Facades::PdfFileSignature`**:
  - Implemented digital signature removal (`RemoveSignature` with `keepFieldEmpty` support, `RemoveSignatures`, `RemoveUsageRights`).
- **`Aspose::Pdf::Facades::PdfExtractor`**:
  - Implemented embedded file attachment extraction (`ExtractAttachment`, `GetAttachment`, `GetAttachNames`, `GetAttachmentInfo`).
  - Updated `FileSpecification` constructor to eagerly load file data from disk.
- **Tests**:
  - Added full test coverage for all newly implemented facade methods across smoke test suites (950 tests passing).
