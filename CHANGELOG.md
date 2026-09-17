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
  - Implemented booklet page ordering (`MakeBooklet`, `TryMakeBooklet`).
  - Implemented real 2-up imposition in `MakeNUp` / `TryMakeNUp`: two source pages are placed per output sheet (side by side, or stacked vertically when `isSidewise` is set) by importing each source page as a Form XObject and drawing it at its cell origin. The two-file overload pairs page *i* of each input; the shorter input is padded with blank sheets.
  - Implemented real content resize: `ResizeContents` / `TryResizeContents` scale the page content into the new page box minus its margins; `ResizeContentsPct` shrinks the content to the requested percentage of the page and centres it while the page box stays untouched.
  - Implemented real margins: `AddMargins` / `AddMarginsPct` grow the page box and translate the content so the added left/bottom margins stay blank.
  - Implemented `AddPageBreak` for non-empty break lists: pages are split at the requested y positions into same-size band pages. The split is clip-based — every band page carries the original content streams (text stays extractable) visually clipped to its band; annotations and outlines are not carried over to the split pages.
- **`Aspose::Pdf::Facades::PdfAnnotationEditor`**:
  - Implemented robust XFDF annotation import (`ImportAnnotationsFromXfdf`) supporting child `<contents>` tags, XML entity decoding, type filtering, and `Square`, `Circle`, `Text`, `Highlight`, `FreeText`, `Underline`, `StrikeOut`, and `Line` annotations.
  - Implemented annotation modification (`ModifyAnnotations`, `ModifyAnnotationsAuthor`).
  - Implemented real annotation flattening (`FlatteningAnnotations` overloads): each annotation's appearance is burned into the page content stream as static drawing operations before the annotation is removed — annotations become permanent page content instead of disappearing. Loaded annotations are burned through their existing `/AP` appearance streams (§12.5.5 BBox→Rect mapping), annotations created in memory are burned through freshly generated appearance forms; hidden annotations are dropped without burning. The `FlattenSettings` overload currently ignores its toggles.
- **`Aspose::Pdf::Facades::PdfBookmarkEditor`**:
  - Implemented XML bookmark export and import (`ExportBookmarksToXML`, `ImportBookmarksWithXML`).
  - Implemented HTML bookmark extraction (`ExportBookmarksToHtml`, `ExtractBookmarksToHTML`).
- **`Aspose::Pdf::Facades::PdfContentEditor`**:
  - Implemented document attachment manipulation (`AddDocumentAttachment`, `DeleteAttachments`).
  - Implemented page image replacement (`ReplaceImage`).
- **`Aspose::Pdf::Facades::PdfFileInfo` & `Aspose::Pdf::Facades::PdfPageEditor`**:
  - Implemented page dimension and rotation queries (`GetPageWidth`, `GetPageHeight`, `GetPageRotation`, `GetPageSize`).
  - Implemented page rotation/resizing staged writes, `ApplyChanges()`, and `MovePosition` (content translation via the same content-stream wrapper).
  - Implemented PDF header version detection (`GetPdfVersion`) and encryption detection (`HasOpenPassword`, `HasEditPassword`).
- **`Aspose::Pdf::Facades::PdfFileSignature`**:
  - Implemented digital signature removal (`RemoveSignature` with `keepFieldEmpty` support, `RemoveSignatures`).
- **`Aspose::Pdf::Facades::PdfExtractor`**:
  - Implemented embedded file attachment extraction (`ExtractAttachment`, `GetAttachment`, `GetAttachNames`, `GetAttachmentInfo`).
  - Updated `FileSpecification` constructor to eagerly load file data from disk.
- **`Aspose::Pdf::Facades::PdfFileStamp`**:
  - `PageHeight` / `PageWidth` now report the first page's real geometry instead of hardcoded Letter defaults.
- **New `Document` internals backing the facade work**: `TransformPageContent` (content-stream `q … cm` / `Q` wrapper with optional clip), `FlattenPageAnnotations` (appearance burn + `/Annots` rewrite), `ImportPageAsForm` / `DrawFormOnPage` (page → Form XObject imposition primitives), and their save-time staged writers.

### Still unsupported in v1 (deliberate stubs)
- `FormEditor`: `SetSubmitFlag`, `SetSubmitUrl`, `SetFieldScript`, `AddFieldScript`, `RemoveFieldAction` — field action / submit plumbing needs action storage on fields, which v1 does not carry (`AddSubmitBtn` also ignores its URL parameter).
- `PdfContentEditor`: document additional actions (`AddDocumentAdditionalAction`, `RemoveDocumentOpenAction`), viewer preferences (`ChangeViewerPreference`, `GetViewerPreference`), and all stamp-management methods (`DeleteStamp*`, `HideStampById`, `ShowStampById`, `MoveStamp*`) — stamps are one-shot content-stream writes with no persistent identity for a by-id registry to target.
- `PdfFileSecurity`: the passwordless `SetPrivilege(DocumentPrivilege)` overload (use the user/owner-password overload instead).
- `PdfFileSignature`: `RemoveUsageRights`, `ContainsUsageRights`, `IsLtvEnabled` — usage-rights and LTV infrastructure is not present in the FOSS codebase.
- `PdfFileInfo`: `HasCollection` (the `/Collection` catalog entry is not surfaced).

### Tests
- Added full test coverage for all newly implemented facade methods across smoke test suites (980 tests run, 958 passing, 22 skipped — the skips are the upstream parity tests that compare against the closed-source Aspose library).
- The facade layout/flattening tests assert the real semantics: imposition page counts and sheet geometry, page-box vs content scaling, clip wrappers on split pages, and burned-in flatten appearance verified at the COS level.

### Notes
- The `PdfFileInfo` password probes both report `IsEncrypted()` — the underlying `Document` exposes no user-password vs owner-password distinction in v1.
