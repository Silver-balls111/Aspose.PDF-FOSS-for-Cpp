# Changelog

<<<<<<< HEAD
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
  - Implemented real half-fold booklet imposition in `MakeBooklet` / `TryMakeBooklet`: two source pages per output sheet via Form XObjects in booklet order (4 pages → 2 sheets). The `PageSize` overload selects the booklet half-page size; source pages are scaled to fit and centred in each half.
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

### Fixed
- XFDF import: attribute lookup is now boundary-checked, so a `page="…"` lookup no longer matches inside an unrelated attribute such as `subpage="1"` (annotations landed on the wrong page).
- XFDF/bookmark import: numeric character references beyond ASCII (`&#233;`, `&#xE9;`) decode to UTF-8 instead of staying literal.
- Bookmark XML/HTML export: titles are escaped (`&`, `<`, `>`, `"`), so a title like `Tom & Jerry "quoted" <tag>` produces well-formed output and survives the round-trip through `ImportBookmarksWithXML` (previously truncated at the first inner quote).
- Bookmark XML import: parses the whole buffer instead of one line per tag, so externally produced or pretty-printed XML (tags spanning lines, attributes in any order) imports too.
- Memory-safety fixes surfaced by the UBSan/ASan audit: flate Lz77 heap-buffer-overflow when a match reached the end of the final block, a use-after-free in `objects_tolerance_smoke_test` reading a Stream body backed by a temporary, and zero-length `memcpy` calls in the digest padding and tiff device (UBSan).
- The bookmark editor smoke tests no longer write `out.html` / `out.xml` into the working directory (they used the temp dir paths everywhere and the stray files were removed from the repo).

### Still unsupported in v1 (deliberate stubs)
- `FormEditor`: `SetSubmitFlag`, `SetSubmitUrl`, `SetFieldScript`, `AddFieldScript`, `RemoveFieldAction` — field action / submit plumbing needs action storage on fields, which v1 does not carry (`AddSubmitBtn` also ignores its URL parameter).
- `PdfContentEditor`: document additional actions (`AddDocumentAdditionalAction`, `RemoveDocumentOpenAction`), viewer preferences (`ChangeViewerPreference`, `GetViewerPreference`), and all stamp-management methods (`DeleteStamp*`, `HideStampById`, `ShowStampById`, `MoveStamp*`) — stamps are one-shot content-stream writes with no persistent identity for a by-id registry to target.
- `PdfFileSecurity`: the passwordless `SetPrivilege(DocumentPrivilege)` overload (use the user/owner-password overload instead).
- `PdfAnnotationEditor`: `ImportAnnotationsFromFdf` — FDF uses PDF COS syntax rather than XML/XFDF, and FDF parsing is not supported in v1 (use the XFDF import instead).
- `PdfFileSignature`: `RemoveUsageRights`, `ContainsUsageRights`, `IsLtvEnabled` — usage-rights and LTV infrastructure is not present in the FOSS codebase.
- `PdfFileInfo`: `HasCollection` (the `/Collection` catalog entry is not surfaced).

### Tests
- Added full test coverage for all newly implemented facade methods across smoke test suites (987 tests run, 965 passing, 22 skipped — the skips are the upstream parity tests that compare against the closed-source Aspose library).
- The facade layout/flattening tests assert the real semantics: imposition page counts and sheet geometry, page-box vs content scaling, clip wrappers on split pages, and burned-in flatten appearance verified at the COS level.
- New regressions for the fixed bugs: XFDF `subpage`/`page` attribute shadowing, non-ASCII numeric entities, bookmark title escaping round-trip, pretty-printed external bookmark XML, and booklet sheet geometry/ordering/content.

### Notes
- The `PdfFileInfo` password probes both report `IsEncrypted()` — the underlying `Document` exposes no user-password vs owner-password distinction in v1.
=======
All notable changes to Aspose.PDF FOSS for C++ are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html). The public API is the set of
headers installed under `include/aspose/` plus `include/aspose.pdf.foss.hpp`; anything under
`include/internal/` is an implementation detail and may change in any release.

## [Unreleased]

## [1.0.1] - 2026-09-29

Packaging-only release; the library code and public API are unchanged from 1.0.0.

### Added

- NuGet package `Aspose.PDF.Cpp.FOSS` for Visual Studio C++ projects: static libraries for x64,
  x86, and ARM64 (Release `/MD` and Debug `/MDd`). Its MSBuild `.targets` adds the include path,
  links the matching library, and raises the project to C++20; it warns when a project uses the
  static CRT. The release workflow builds the package, verifies it with a Visual Studio consumer
  project, attaches it to the GitHub release, and publishes it to nuget.org after manual approval.

### Fixed

- The Debug library in the Windows release archive now embeds its debug information (`/Z7`), so
  linking it no longer produces LNK4099 "PDB not found" warnings.

## [1.0.0] - 2026-09-29

First public release: a free, open-source, dependency-free C++20 library for reading, rendering,
editing, and creating PDF documents, with a public API that is a strict subset of Aspose.PDF for
.NET.

### Added

#### Documents and pages

- `Document` — open existing PDFs (`Document(path)`, `Document(path, password)` for encrypted files,
  `Document(path, LoadOptions)`), create documents from scratch (`Document()`), and save them with
  `Save(path)`; byte-verbatim round-trips and an incremental `/Info` update that preserves the
  original bytes.
- SVG import: `Document(path, SvgLoadOptions)` converts an SVG file to a single-page PDF (v1
  conversion subset).
- `PageCollection` with a 1-based indexer, `Add`, `Insert`, `Delete`, and `Count`; `Page` with page
  size, paragraphs, annotations, artifacts, and `AddImage`.
- `DocumentInfo` typed metadata accessors (title, author, subject, keywords, creator, producer,
  dates) plus `Add`/`Remove`/`ClearCustomData`; read access to XMP metadata via `Metadata` and
  `XmpValue`.
- Page labels (`PageLabel`, `PageLabelCollection`), embedded files (`Document::EmbeddedFiles()`,
  `FileSpecification`), outlines (`OutlineCollection`, `OutlineItemCollection`) and named
  destinations (`NamedDestinationCollection`, explicit destination subtypes).

#### Text

- Text extraction with `Text::TextAbsorber` (whole document or single page) and
  `Text::TextFragmentAbsorber` (positioned fragments with font, size, and colour).
- Text authoring with `Text::TextBuilder`, `TextFragment`, `TextParagraph`, `TextState`, and
  `FontRepository::FindFont`.
- `Text::Encoding` — a `System.Text.Encoding`-style charset codec (UTF-8, UTF-16LE, UTF-16BE,
  Latin-1, Windows-1252) used by `TextDevice`.
- Standard-14 fonts without an embedded `/FontFile` render via system fonts, falling back to
  bundled Liberation substitutes (SIL OFL 1.1) for Helvetica, Times-Roman, and Courier.

#### Rendering

- Anti-aliased page rasteriser with no third-party dependency, exposed through `PngDevice`,
  `JpegDevice`, `BmpDevice`, and `TiffDevice` (single page, or a multi-page document range in one
  call).
- `TiffDevice` 1/4/8/24-bpp output with median-cut palette quantisation, or a caller-supplied
  `IIndexBitmapConverter`; `TiffSettings`, `Resolution`, and `RenderingOptions`.
- Built-in codecs implemented from scratch: Flate, LZW, RunLength, ASCII85, ASCIIHex, CCITT, DCT
  (JPEG), JBIG2, JPX (JPEG 2000), PNG, TIFF, and BMP; inline images (`BI`/`ID`/`EI`).

#### Content creation

- Tables (`Table`, `Row`, `Cell`) with column widths, borders, background colours, and column
  spans.
- Vector graphics via `Drawing::Graph` with `Line`, `Rectangle`, `Circle`, and `Ellipse`.
- Rotated, semi-transparent overlays via `WatermarkArtifact`; `FloatingBox`, `Hyperlink`.

#### Annotations and forms

- Annotations with pre-generated `/AP` appearance streams: `Highlight`, `Underline`, `Squiggly`,
  `StrikeOut`, `Square`, `Circle`, `Line`, `Ink`, `Text`, `FreeText`, `Stamp`, `Link` (with
  `GoToAction`/`GoToURIAction`), and `FileAttachment`.
- AcroForm fields — `TextBoxField`, `CheckboxField`, `RadioButtonField`, `ComboBoxField`,
  `ListBoxField`, `ButtonField` — through `Document::Form()`, with `Form::Flatten()`.

#### Security and signing

- `Document::Encrypt` / `Decrypt` with RC4-40, RC4-128, AES-128, and AES-256 (PDF 2.0, R=6),
  governed by the `Permissions` flags enum.
- Detached PKCS#7 (`adbe.pkcs7.detached`) signatures with a byte-exact `/ByteRange` via
  `Facades::PdfFileSignature`.

#### Facades

- `Aspose::Pdf::Facades` API: `PdfConverter`, `PdfExtractor`, `PdfFileSecurity`,
  `PdfFileSignature`, `PdfBookmarkEditor`, `PdfFileEditor` (concatenate and split), `PdfFileStamp`
  (header, footer, page numbers), `PdfFileInfo`, `PdfPageEditor`, `PdfContentEditor`,
  `PdfAnnotationEditor`, `PdfXmpMetadata`, and `FormEditor`.

#### Packaging and build

- Single-entry header `aspose.pdf.foss.hpp`, which includes the whole public API.
- CMake package export: `find_package(aspose_pdf_foss CONFIG)` provides the
  `aspose_pdf_foss::aspose_pdf_foss` target (also available as an alias for `add_subdirectory`
  consumers). The target requires C++20 and passes that requirement on to consumers.
- `cmake --install` installs only the public headers, the static library, the CMake package files,
  and the license texts (MIT, OFL 1.1 for the bundled fonts, third-party notices).
- Prebuilt release archives for Linux x64 (GCC 13) and Windows x64 (MSVC, Release and Debug), each
  smoke-tested as a `find_package` consumer before publishing.
- MSVC warning C4250 (inheritance via dominance in the facade hierarchy) is suppressed inside the
  facade headers, so consumers building at `/W4` see no warnings from the library.
- 12 runnable examples under `examples/` and a GoogleTest suite; CI on Linux (GCC 13, Clang 16)
  and Windows (MSVC).

### Known limitations

- The `XmpValue` helpers `IsDateTime`, `IsField`, `IsNamedValue`, `IsRaw`, `IsNamedValues`, and
  `IsStructure` are stubs that always return `false`.
- Standard-14 fallback fonts cover Helvetica, Times-Roman, and Courier (four styles each); Symbol
  and ZapfDingbats have no bundled fallback.
- Setting metadata on a from-scratch document and saving throws: the incremental writer can only
  patch an existing `/Info` object.
- Encryption permissions are enforced by PDF viewers, not by this library.

[Unreleased]: https://github.com/aspose-pdf-foss/Aspose.PDF-FOSS-for-Cpp/compare/v1.0.1...HEAD
[1.0.1]: https://github.com/aspose-pdf-foss/Aspose.PDF-FOSS-for-Cpp/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/aspose-pdf-foss/Aspose.PDF-FOSS-for-Cpp/releases/tag/v1.0.0
>>>>>>> upstream/main
