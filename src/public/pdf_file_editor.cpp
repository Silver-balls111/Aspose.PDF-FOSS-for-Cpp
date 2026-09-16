#include <aspose/pdf/facades/pdf_file_editor.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <aspose/pdf/document.hpp>
#include <aspose/pdf/page.hpp>
#include <aspose/pdf/page_collection.hpp>

namespace Aspose::Pdf::Facades {

namespace {

// Exception policy shared by the throwing / Try* facade forms: the
// throwing form rethrows only when AllowConcatenateExceptions is set;
// the Try* form always swallows and returns false.
template <class F>
bool RunEditor(F&& op, bool isTry, bool allowExceptions) {
    try {
        op();
        return true;
    } catch (const std::exception&) {
        if (!isTry && allowExceptions) throw;
        return false;
    }
}

std::vector<int> AllPages(const Aspose::Pdf::Document& doc) {
    std::vector<int> v;
    const int n = static_cast<int>(
        const_cast<Aspose::Pdf::Document&>(doc).Pages().Count());
    for (int i = 1; i <= n; ++i) v.push_back(i);
    return v;
}

// Clamp a [start..end] 1-based inclusive range to [1..count]; end<=0
// means "to last".
std::vector<int> PageRange(int count, int start, int end) {
    if (start < 1) start = 1;
    if (end <= 0 || end > count) end = count;
    std::vector<int> v;
    for (int i = start; i <= end; ++i) v.push_back(i);
    return v;
}

// Pages of [1..count] that are NOT in `keep`.
std::vector<int> Complement(int count, const std::vector<int>& keep) {
    std::set<int> k(keep.begin(), keep.end());
    std::vector<int> v;
    for (int i = 1; i <= count; ++i)
        if (k.find(i) == k.end()) v.push_back(i);
    return v;
}

std::string SplitName(const std::string& input, const std::string& tag) {
    std::filesystem::path p(input);
    std::filesystem::path out =
        p.parent_path() / (p.stem().string() + "_" + tag +
                           p.extension().string());
    return out.string();
}

}  // namespace

// ===== PageBreak =============================================================

PdfFileEditor::PageBreak::PageBreak(int pageNumber,
                                      double position) noexcept
    : page_number_(pageNumber), position_(position) {}

int PdfFileEditor::PageBreak::PageNumber() const noexcept {
    return page_number_;
}
void PdfFileEditor::PageBreak::PageNumber(int v) noexcept {
    page_number_ = v;
}
double PdfFileEditor::PageBreak::Position() const noexcept {
    return position_;
}
void PdfFileEditor::PageBreak::Position(double v) noexcept {
    position_ = v;
}

// ===== CorruptedItem =========================================================

PdfFileEditor::CorruptedItem::CorruptedItem(int index) noexcept
    : index_(index) {}

int PdfFileEditor::CorruptedItem::Index() const noexcept {
    return index_;
}

// ===== ContentsResizeValue ===================================================

PdfFileEditor::ContentsResizeValue
PdfFileEditor::ContentsResizeValue::Percents(double value) noexcept {
    ContentsResizeValue v;
    v.value_ = value;
    v.is_percent_ = true;
    v.is_auto_ = false;
    return v;
}

PdfFileEditor::ContentsResizeValue
PdfFileEditor::ContentsResizeValue::Units(double value) noexcept {
    ContentsResizeValue v;
    v.value_ = value;
    v.is_percent_ = false;
    v.is_auto_ = false;
    return v;
}

PdfFileEditor::ContentsResizeValue
PdfFileEditor::ContentsResizeValue::Auto() noexcept {
    ContentsResizeValue v;
    v.value_ = 0.0;
    v.is_auto_ = true;
    return v;
}

void PdfFileEditor::ContentsResizeValue::PercentValue(
        double v) noexcept {
    value_ = v;
    is_percent_ = true;
    is_auto_ = false;
}
void PdfFileEditor::ContentsResizeValue::UnitValue(
        double v) noexcept {
    value_ = v;
    is_percent_ = false;
    is_auto_ = false;
}

double PdfFileEditor::ContentsResizeValue::Value() const noexcept {
    return value_;
}
bool PdfFileEditor::ContentsResizeValue::IsPercent() const noexcept {
    return is_percent_;
}

// ===== ContentsResizeParameters ==============================================

#define CRP_ACCESSOR(NAME, FIELD)                                  \
    PdfFileEditor::ContentsResizeValue                              \
    PdfFileEditor::ContentsResizeParameters::NAME() const noexcept { \
        return FIELD;                                              \
    }                                                              \
    void PdfFileEditor::ContentsResizeParameters::NAME(            \
            ContentsResizeValue v) noexcept {                      \
        FIELD = std::move(v);                                      \
    }

CRP_ACCESSOR(NewPageWidth,   new_page_width_)
CRP_ACCESSOR(NewPageHeight,  new_page_height_)
CRP_ACCESSOR(LeftMargin,     left_margin_)
CRP_ACCESSOR(RightMargin,    right_margin_)
CRP_ACCESSOR(TopMargin,      top_margin_)
CRP_ACCESSOR(BottomMargin,   bottom_margin_)
CRP_ACCESSOR(ContentsWidth,  contents_width_)
CRP_ACCESSOR(ContentsHeight, contents_height_)

#undef CRP_ACCESSOR

// ===== PdfFileEditor — real file operations ==================================
// Concatenate / Append / Insert deep-import the source pages' object
// graph into the destination (Document::ImportPagesFrom); Extract /
// Delete / Split reduce to PageCollection page removal. Each writes the
// result via incremental update. Layout ops (booklet / n-up / resize /
// margins / page breaks) compose the same primitives below.

namespace {

bool DoDelete(const std::string& inputFile, const std::vector<int>& pages,
              const std::string& out, bool isTry, bool allowEx) {
    return RunEditor([&] {
        Aspose::Pdf::Document d(inputFile);
        d.Pages().Delete(pages);
        d.Save(out);
    }, isTry, allowEx);
}

bool DoExtractKeep(const std::string& inputFile,
                   const std::vector<int>& keep, const std::string& out,
                   bool isTry, bool allowEx) {
    return RunEditor([&] {
        Aspose::Pdf::Document d(inputFile);
        const int count = static_cast<int>(d.Pages().Count());
        const std::vector<int> del = Complement(count, keep);
        if (!del.empty()) d.Pages().Delete(del);
        d.Save(out);
    }, isTry, allowEx);
}

}  // namespace

bool PdfFileEditor::ConcatenateTwo(const std::string& a, const std::string& b,
                                   const std::string& out, bool isTry) {
    return RunEditor([&] {
        Aspose::Pdf::Document d1(a);
        Aspose::Pdf::Document d2(b);
        d1.ImportPagesFrom(d2, AllPages(d2), 0);
        d1.Save(out);
    }, isTry, allow_concatenate_exceptions_);
}
bool PdfFileEditor::ConcatenateMany(const std::vector<std::string>& files,
                                    const std::string& out, bool isTry) {
    return RunEditor([&] {
        if (files.empty())
            throw std::runtime_error("PdfFileEditor: no input files");
        Aspose::Pdf::Document d(files.front());
        for (std::size_t i = 1; i < files.size(); ++i) {
            Aspose::Pdf::Document s(files[i]);
            d.ImportPagesFrom(s, AllPages(s), 0);
        }
        d.Save(out);
    }, isTry, allow_concatenate_exceptions_);
}
bool PdfFileEditor::AppendImpl(const std::string& portFile,
                               const std::vector<std::string>& addFiles,
                               int startPage, int endPage,
                               const std::string& out, bool isTry) {
    return RunEditor([&] {
        Aspose::Pdf::Document d(portFile);
        for (const auto& add : addFiles) {
            Aspose::Pdf::Document s(add);
            const int count = static_cast<int>(s.Pages().Count());
            d.ImportPagesFrom(s, PageRange(count, startPage, endPage), 0);
        }
        d.Save(out);
    }, isTry, allow_concatenate_exceptions_);
}
bool PdfFileEditor::InsertImpl(const std::string& inputFile,
                               int insertLocation,
                               const std::string& portFile, int startPage,
                               int endPage, const std::string& out,
                               bool isTry) {
    return RunEditor([&] {
        Aspose::Pdf::Document d(inputFile);
        Aspose::Pdf::Document s(portFile);
        const int count = static_cast<int>(s.Pages().Count());
        d.ImportPagesFrom(s, PageRange(count, startPage, endPage),
                          insertLocation);
        d.Save(out);
    }, isTry, allow_concatenate_exceptions_);
}

bool PdfFileEditor::Concatenate(const std::string& a, const std::string& b,
                                const std::string& out) {
    return ConcatenateTwo(a, b, out, false);
}
bool PdfFileEditor::Concatenate(const std::vector<std::string>& files,
                                const std::string& out) {
    return ConcatenateMany(files, out, false);
}
bool PdfFileEditor::Concatenate(
        const std::vector<Aspose::Pdf::Document*>& src,
        Aspose::Pdf::Document& dest) {
    return RunEditor([&] {
        for (auto* s : src) {
            if (s != nullptr)
                dest.ImportPagesFrom(*s, AllPages(*s), 0);
        }
    }, false, allow_concatenate_exceptions_);
}

bool PdfFileEditor::TryConcatenate(const std::string& a, const std::string& b,
                                   const std::string& out) {
    return ConcatenateTwo(a, b, out, true);
}
bool PdfFileEditor::TryConcatenate(const std::vector<std::string>& files,
                                   const std::string& out) {
    return ConcatenateMany(files, out, true);
}

bool PdfFileEditor::Append(const std::string& portFile,
                           const std::vector<std::string>& addFiles,
                           int startPage, int endPage,
                           const std::string& out) {
    return AppendImpl(portFile, addFiles, startPage, endPage, out, false);
}
bool PdfFileEditor::TryAppend(const std::string& portFile,
                              const std::vector<std::string>& addFiles,
                              int startPage, int endPage,
                              const std::string& out) {
    return AppendImpl(portFile, addFiles, startPage, endPage, out, true);
}

bool PdfFileEditor::Insert(const std::string& inputFile, int insertLocation,
                           const std::string& portFile, int startPage,
                           int endPage, const std::string& out) {
    return InsertImpl(inputFile, insertLocation, portFile, startPage,
                      endPage, out, false);
}
bool PdfFileEditor::TryInsert(const std::string& inputFile,
                              int insertLocation, const std::string& portFile,
                              int startPage, int endPage,
                              const std::string& out) {
    return InsertImpl(inputFile, insertLocation, portFile, startPage,
                      endPage, out, true);
}

bool PdfFileEditor::Delete(const std::string& inputFile,
                           const std::vector<int>& pageNumber,
                           const std::string& out) {
    return DoDelete(inputFile, pageNumber, out, false,
                    allow_concatenate_exceptions_);
}
bool PdfFileEditor::TryDelete(const std::string& inputFile,
                              const std::vector<int>& pageNumber,
                              const std::string& out) {
    return DoDelete(inputFile, pageNumber, out, true,
                    allow_concatenate_exceptions_);
}

bool PdfFileEditor::Extract(const std::string& inputFile, int startPage,
                            int endPage, const std::string& out) {
    return RunEditor([&] {
        Aspose::Pdf::Document d(inputFile);
        const int count = static_cast<int>(d.Pages().Count());
        const std::vector<int> keep = PageRange(count, startPage, endPage);
        const std::vector<int> del = Complement(count, keep);
        if (!del.empty()) d.Pages().Delete(del);
        d.Save(out);
    }, false, allow_concatenate_exceptions_);
}
bool PdfFileEditor::Extract(const std::string& inputFile,
                            const std::vector<int>& pageNumber,
                            const std::string& out) {
    return DoExtractKeep(inputFile, pageNumber, out, false,
                         allow_concatenate_exceptions_);
}
bool PdfFileEditor::TryExtract(const std::string& inputFile, int startPage,
                               int endPage, const std::string& out) {
    return RunEditor([&] {
        Aspose::Pdf::Document d(inputFile);
        const int count = static_cast<int>(d.Pages().Count());
        const std::vector<int> keep = PageRange(count, startPage, endPage);
        const std::vector<int> del = Complement(count, keep);
        if (!del.empty()) d.Pages().Delete(del);
        d.Save(out);
    }, true, allow_concatenate_exceptions_);
}
bool PdfFileEditor::TryExtract(const std::string& inputFile,
                               const std::vector<int>& pageNumber,
                               const std::string& out) {
    return DoExtractKeep(inputFile, pageNumber, out, true,
                         allow_concatenate_exceptions_);
}

bool PdfFileEditor::SplitFromFirst(const std::string& inputFile, int location,
                                   const std::string& out) {
    return RunEditor([&] {
        Aspose::Pdf::Document d(inputFile);
        const int count = static_cast<int>(d.Pages().Count());
        const std::vector<int> del = Complement(count,
                                                 PageRange(count, 1, location));
        if (!del.empty()) d.Pages().Delete(del);
        d.Save(out);
    }, false, allow_concatenate_exceptions_);
}
bool PdfFileEditor::SplitToEnd(const std::string& inputFile, int location,
                               const std::string& out) {
    return RunEditor([&] {
        Aspose::Pdf::Document d(inputFile);
        const int count = static_cast<int>(d.Pages().Count());
        const std::vector<int> del =
            Complement(count, PageRange(count, location, count));
        if (!del.empty()) d.Pages().Delete(del);
        d.Save(out);
    }, false, allow_concatenate_exceptions_);
}
std::vector<std::string> PdfFileEditor::SplitToPages(
        const std::string& inputFile) {
    std::vector<std::string> out_files;
    Aspose::Pdf::Document probe(inputFile);
    const int count = static_cast<int>(probe.Pages().Count());
    for (int i = 1; i <= count; ++i) {
        Aspose::Pdf::Document d(inputFile);
        const std::vector<int> del = Complement(count, {i});
        if (!del.empty()) d.Pages().Delete(del);
        const std::string name = SplitName(inputFile, std::to_string(i));
        d.Save(name);
        out_files.push_back(name);
    }
    return out_files;
}
std::vector<std::string> PdfFileEditor::SplitToBulks(
        const std::string& inputFile,
        const std::vector<std::vector<int>>& numberOfPage) {
    std::vector<std::string> out_files;
    const int count =
        static_cast<int>(Aspose::Pdf::Document(inputFile).Pages().Count());
    for (std::size_t g = 0; g < numberOfPage.size(); ++g) {
        Aspose::Pdf::Document d(inputFile);
        const std::vector<int> del = Complement(count, numberOfPage[g]);
        if (!del.empty()) d.Pages().Delete(del);
        const std::string name =
            SplitName(inputFile, "bulk" + std::to_string(g + 1));
        d.Save(name);
        out_files.push_back(name);
    }
    return out_files;
}
bool PdfFileEditor::TrySplitFromFirst(const std::string& inputFile,
                                      int location, const std::string& out) {
    return RunEditor([&] {
        Aspose::Pdf::Document d(inputFile);
        const int count = static_cast<int>(d.Pages().Count());
        const std::vector<int> del = Complement(count,
                                                 PageRange(count, 1, location));
        if (!del.empty()) d.Pages().Delete(del);
        d.Save(out);
    }, true, allow_concatenate_exceptions_);
}
bool PdfFileEditor::TrySplitToEnd(const std::string& inputFile, int location,
                                  const std::string& out) {
    return RunEditor([&] {
        Aspose::Pdf::Document d(inputFile);
        const int count = static_cast<int>(d.Pages().Count());
        const std::vector<int> del =
            Complement(count, PageRange(count, location, count));
        if (!del.empty()) d.Pages().Delete(del);
        d.Save(out);
    }, true, allow_concatenate_exceptions_);
}

bool PdfFileEditor::MakeBookletImpl(const std::string& inputFile,
                                    const std::string& outputFile,
                                    const Aspose::Pdf::PageSize* pageSize,
                                    bool isTry) {
    return RunEditor([&] {
        Aspose::Pdf::Document src(inputFile);
        const int count = static_cast<int>(src.Pages().Count());
        if (count == 0) return;
        int total = ((count + 3) / 4) * 4;
        std::vector<int> booklet_order;
        int l = 1, r = total;
        while (l < r) {
            booklet_order.push_back(r);
            booklet_order.push_back(l);
            booklet_order.push_back(l + 1);
            booklet_order.push_back(r - 1);
            l += 2;
            r -= 2;
        }
        Aspose::Pdf::Document dest;
        for (int p : booklet_order) {
            if (p <= count) {
                dest.ImportPagesFrom(src, {p}, 0);
            } else {
                dest.Pages().Add();
            }
        }
        if (pageSize != nullptr) {
            for (std::size_t i = 1; i <= dest.Pages().Count(); ++i) {
                dest.Pages()[static_cast<int>(i)].SetPageSize(pageSize->Width(), pageSize->Height());
            }
        }
        dest.Save(outputFile);
    }, isTry, allow_concatenate_exceptions_);
}

bool PdfFileEditor::MakeBooklet(const std::string& inputFile,
                                const std::string& outputFile) {
    return MakeBookletImpl(inputFile, outputFile, nullptr, false);
}

bool PdfFileEditor::MakeBooklet(const std::string& inputFile,
                                const std::string& outputFile,
                                Aspose::Pdf::PageSize pageSize) {
    return MakeBookletImpl(inputFile, outputFile, &pageSize, false);
}

bool PdfFileEditor::TryMakeBooklet(const std::string& inputFile,
                                   const std::string& outputFile) {
    return MakeBookletImpl(inputFile, outputFile, nullptr, true);
}

// ===== N-up imposition (real) ================================================
// Each output sheet carries two source pages. Sheets are composed by
// importing each source page as a Form XObject (Document::ImportPageAsForm)
// and drawing it at its cell origin (Document::DrawFormOnPage), so the
// source content, fonts and images survive as real PDF objects.

bool PdfFileEditor::MakeNUpImpl(const std::vector<std::string>& inputFiles,
                                const std::string& outputFile,
                                bool pairwise, bool isSidewise,
                                bool isTry) {
    return RunEditor(
        [&] {
            if (inputFiles.empty())
                throw std::runtime_error(
                    "Aspose::Pdf::PdfFileEditor::MakeNUp: no input files");
            if (pairwise && inputFiles.size() != 2)
                throw std::runtime_error(
                    "Aspose::Pdf::PdfFileEditor::MakeNUp: two input files "
                    "required");

            std::vector<Aspose::Pdf::Document> docs;
            docs.reserve(inputFiles.size());
            for (const auto& f : inputFiles) docs.emplace_back(f);

            Aspose::Pdf::Document dest;

            // Import a source page as a Form XObject; report its footprint.
            auto import_cell = [&](Aspose::Pdf::Document& src, int page,
                                   double& w, double& h)
                -> std::pair<std::uint32_t, std::string> {
                const std::uint32_t formId =
                    dest.ImportPageAsForm(src, page, w, h);
                return {formId, "Frm" + std::to_string(formId)};
            };
            auto draw_cell = [&](const std::pair<std::uint32_t,
                                                 std::string>& form,
                                 double dx, double dy) {
                const std::size_t leaf =
                    static_cast<std::size_t>(dest.Pages().Count()) - 1;
                dest.DrawFormOnPage(leaf, form.first, form.second, 1.0, 1.0,
                                    dx, dy);
            };

            if (pairwise) {
                // Output page i joins page i of each input file side by
                // side; the shorter input is padded with blank sheets.
                const int n =
                    std::max(static_cast<int>(docs[0].Pages().Count()),
                             static_cast<int>(docs[1].Pages().Count()));
                for (int i = 1; i <= n; ++i) {
                    const bool has0 = i <= static_cast<int>(docs[0].Pages().Count());
                    const bool has1 = i <= static_cast<int>(docs[1].Pages().Count());
                    double w0 = 0.0, h0 = 0.0, w1 = 0.0, h1 = 0.0;
                    const auto f0 = has0
                        ? import_cell(docs[0], i, w0, h0)
                        : std::pair<std::uint32_t, std::string>{};
                    const auto f1 = has1
                        ? import_cell(docs[1], i, w1, h1)
                        : std::pair<std::uint32_t, std::string>{};
                    const double sw = w0 + w1;
                    const double sh = std::max(h0, h1);
                    dest.AddPageInternal(0, false, 0, sw, sh);
                    if (has0) draw_cell(f0, 0.0, 0.0);
                    if (has1) draw_cell(f1, w0, 0.0);
                }
            } else {
                // Sequential: source pages consumed in order, two per
                // sheet. isSidewise stacks the cells vertically instead of
                // placing them side by side.
                struct Cell {
                    std::size_t doc;
                    int page;
                };
                std::vector<Cell> cells;
                for (std::size_t d = 0; d < docs.size(); ++d)
                    for (int p = 1;
                         p <= static_cast<int>(docs[d].Pages().Count()); ++p)
                        cells.push_back({d, p});
                for (std::size_t i = 0; i < cells.size(); i += 2) {
                    double wa = 0.0, ha = 0.0, wb = 0.0, hb = 0.0;
                    const auto fa = import_cell(docs[cells[i].doc],
                                                cells[i].page, wa, ha);
                    const bool hasB = i + 1 < cells.size();
                    const auto fb =
                        hasB ? import_cell(docs[cells[i + 1].doc],
                                           cells[i + 1].page, wb, hb)
                             : std::pair<std::uint32_t, std::string>{};
                    const double sw = isSidewise ? std::max(wa, wb) : wa + wb;
                    const double sh = isSidewise ? ha + hb : std::max(ha, hb);
                    dest.AddPageInternal(0, false, 0, sw, sh);
                    draw_cell(fa, 0.0, 0.0);
                    if (hasB)
                        draw_cell(fb, isSidewise ? 0.0 : wa,
                                  isSidewise ? ha : 0.0);
                }
            }
            dest.Save(outputFile);
        },
        isTry, allow_concatenate_exceptions_);
}

bool PdfFileEditor::MakeNUp(const std::string& firstInputFile,
                            const std::string& secondInputFile,
                            const std::string& outputFile) {
    return MakeNUpImpl({firstInputFile, secondInputFile}, outputFile,
                       /*pairwise=*/true, /*isSidewise=*/false,
                       /*isTry=*/false);
}

bool PdfFileEditor::MakeNUp(const std::vector<std::string>& inputFiles,
                            const std::string& outputFile,
                            bool isSidewise) {
    return MakeNUpImpl(inputFiles, outputFile, /*pairwise=*/false, isSidewise,
                       /*isTry=*/false);
}

bool PdfFileEditor::TryMakeNUp(const std::string& firstInputFile,
                               const std::string& secondInputFile,
                               const std::string& outputFile) {
    return MakeNUpImpl({firstInputFile, secondInputFile}, outputFile,
                       /*pairwise=*/true, /*isSidewise=*/false,
                       /*isTry=*/true);
}

// ===== Contents resize / margins (real) ======================================
// The page content is scaled with a `q <matrix> cm ... Q` wrapper
// (Document::TransformPageContent): ResizeContents shrinks the content into
// the new page box minus its margins, ResizeContentsPct shrinks the content
// and centres it while the page box itself is left untouched, and
// AddMargins grows the page box while translating the content to keep it
// anchored at the new bottom-left origin.

bool PdfFileEditor::ResizeContentsImpl(const std::string& inputFile,
                                       const std::string& outputFile,
                                       const std::vector<int>& pages,
                                       const ContentsResizeParameters& parameters,
                                       bool isTry) {
    return RunEditor([&] {
        Aspose::Pdf::Document doc(inputFile);
        const int count = static_cast<int>(doc.Pages().Count());
        const auto target_pages = pages.empty() ? AllPages(doc) : pages;
        for (int p : target_pages) {
            if (p < 1 || p > count) continue;
            auto page = doc.Pages()[p];
            auto rect = page.Rect();
            const double oldW = rect.Width();
            const double oldH = rect.Height();
            if (oldW <= 0.0 || oldH <= 0.0) continue;
            double newW = oldW;
            double newH = oldH;
            if (parameters.NewPageWidth().Value() > 0) {
                newW = parameters.NewPageWidth().IsPercent()
                    ? (oldW * parameters.NewPageWidth().Value() / 100.0)
                    : parameters.NewPageWidth().Value();
            }
            if (parameters.NewPageHeight().Value() > 0) {
                newH = parameters.NewPageHeight().IsPercent()
                    ? (oldH * parameters.NewPageHeight().Value() / 100.0)
                    : parameters.NewPageHeight().Value();
            }
            auto margin = [](const ContentsResizeValue& v, double base) {
                if (v.Value() <= 0.0) return 0.0;  // Auto / unset
                return v.IsPercent() ? base * v.Value() / 100.0
                                     : v.Value();
            };
            const double lm = margin(parameters.LeftMargin(), newW);
            const double rm = margin(parameters.RightMargin(), newW);
            const double bm = margin(parameters.BottomMargin(), newH);
            const double tm = margin(parameters.TopMargin(), newH);
            const double cw = newW - lm - rm;
            const double ch = newH - bm - tm;
            page.SetPageSize(newW, newH);
            if (cw > 0.0 && ch > 0.0) {
                doc.TransformPageContent(
                    static_cast<std::size_t>(p - 1), cw / oldW, ch / oldH,
                    lm, bm);
            }
        }
        doc.Save(outputFile);
    }, isTry, allow_concatenate_exceptions_);
}

bool PdfFileEditor::ResizeContents(const std::string& inputFile,
                                   const std::string& outputFile,
                                   const std::vector<int>& pages,
                                   ContentsResizeParameters parameters) {
    return ResizeContentsImpl(inputFile, outputFile, pages, parameters, false);
}

bool PdfFileEditor::ResizeContentsPct(const std::string& inputFile,
                                      const std::string& outputFile,
                                      double leftRightPct,
                                      double topBottomPct) {
    return RunEditor([&] {
        Aspose::Pdf::Document doc(inputFile);
        const int count = static_cast<int>(doc.Pages().Count());
        for (int i = 1; i <= count; ++i) {
            auto page = doc.Pages()[i];
            auto rect = page.Rect();
            const double w = rect.Width();
            const double h = rect.Height();
            if (w <= 0.0 || h <= 0.0) continue;
            const double sx = leftRightPct / 100.0;
            const double sy = topBottomPct / 100.0;
            // The page box stays the same; the content shrinks to the
            // requested percentage of the page and is centred.
            doc.TransformPageContent(
                static_cast<std::size_t>(i - 1), sx, sy,
                w * (1.0 - sx) / 2.0, h * (1.0 - sy) / 2.0);
        }
        doc.Save(outputFile);
    }, false, allow_concatenate_exceptions_);
}

bool PdfFileEditor::TryResizeContents(const std::string& inputFile,
                                      const std::string& outputFile,
                                      const std::vector<int>& pages,
                                      ContentsResizeParameters parameters) {
    return ResizeContentsImpl(inputFile, outputFile, pages, parameters, true);
}

bool PdfFileEditor::AddMargins(const std::string& inputFile,
                               const std::string& outputFile,
                               const std::vector<int>& pages,
                               double leftMargin, double rightMargin,
                               double topMargin, double bottomMargin) {
    return RunEditor([&] {
        Aspose::Pdf::Document doc(inputFile);
        const int count = static_cast<int>(doc.Pages().Count());
        const auto target_pages = pages.empty() ? AllPages(doc) : pages;
        for (int p : target_pages) {
            if (p < 1 || p > count) continue;
            auto page = doc.Pages()[p];
            auto rect = page.Rect();
            const double w = rect.Width();
            const double h = rect.Height();
            page.SetPageSize(w + leftMargin + rightMargin,
                             h + topMargin + bottomMargin);
            // Content moves with the enlarged bottom-left origin so the
            // added left/bottom margins stay blank.
            doc.TransformPageContent(static_cast<std::size_t>(p - 1), 1.0,
                                     1.0, leftMargin, bottomMargin);
        }
        doc.Save(outputFile);
    }, false, allow_concatenate_exceptions_);
}

bool PdfFileEditor::AddMarginsPct(const std::string& inputFile,
                                  const std::string& outputFile,
                                  const std::vector<int>& pages,
                                  double leftMargin, double rightMargin,
                                  double topMargin, double bottomMargin) {
    return RunEditor([&] {
        Aspose::Pdf::Document doc(inputFile);
        const int count = static_cast<int>(doc.Pages().Count());
        const auto target_pages = pages.empty() ? AllPages(doc) : pages;
        for (int p : target_pages) {
            if (p < 1 || p > count) continue;
            auto page = doc.Pages()[p];
            auto rect = page.Rect();
            const double w = rect.Width();
            const double h = rect.Height();
            const double lm = w * leftMargin / 100.0;
            const double bm = h * bottomMargin / 100.0;
            page.SetPageSize(w * (1.0 + (leftMargin + rightMargin) / 100.0),
                             h * (1.0 + (topMargin + bottomMargin) / 100.0));
            doc.TransformPageContent(static_cast<std::size_t>(p - 1), 1.0,
                                     1.0, lm, bm);
        }
        doc.Save(outputFile);
    }, false, allow_concatenate_exceptions_);
}

// ===== Page breaks (real — clip-based split) =================================
// A page with break positions is copied once per y-band (ImportPagesFrom)
// and each copy's content is wrapped in `q <band> re W n … Q`
// (TransformPageContent with a clip), so every band page shows only its
// region while the full content streams — and their extractable text —
// remain intact. v1 notes: annotations / outlines on the split pages are
// not carried over, and each band page still carries the page's full text
// in its content stream (clipping is visual).

bool PdfFileEditor::AddPageBreak(const std::string& inputFile,
                                 const std::string& outputFile,
                                 const std::vector<PageBreak>& pageBreaks) {
    return RunEditor([&] {
        Aspose::Pdf::Document doc(inputFile);
        if (pageBreaks.empty()) {
            doc.Save(outputFile);
            return;
        }
        std::map<int, std::vector<double>> cuts_by_page;
        for (const auto& pb : pageBreaks)
            if (pb.PageNumber() >= 1)
                cuts_by_page[pb.PageNumber()].push_back(pb.Position());

        Aspose::Pdf::Document dest;
        const int count = static_cast<int>(doc.Pages().Count());
        for (int p = 1; p <= count; ++p) {
            auto it = cuts_by_page.find(p);
            if (it == cuts_by_page.end()) {
                dest.ImportPagesFrom(doc, {p}, 0);
                continue;
            }

            const auto rect = doc.Pages()[p].Rect();
            const double w = rect.Width();
            const double h = rect.Height();
            std::vector<double> cuts = it->second;
            for (double& c : cuts) c = std::clamp(c, 0.0, h);
            std::sort(cuts.begin(), cuts.end());
            cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());
            cuts.erase(std::remove_if(cuts.begin(), cuts.end(),
                                      [h](double c) {
                                          return c <= 0.0 || c >= h;
                                      }),
                       cuts.end());
            if (cuts.empty()) {
                dest.ImportPagesFrom(doc, {p}, 0);
                continue;
            }

            // Bands from the top: [cut_k, cut_{k-1}], …, [0, cut_last].
            // Pages are appended in order, so the leaf index of every
            // previously staged clip stays valid.
            double bottom = h;
            for (auto b = cuts.rbegin(); b != cuts.rend(); ++b) {
                dest.ImportPagesFrom(doc, {p}, 0);
                const std::size_t leaf =
                    static_cast<std::size_t>(dest.Pages().Count()) - 1;
                const Aspose::Pdf::Rectangle clip(0.0, *b, w, bottom, false);
                dest.TransformPageContent(leaf, 1.0, 1.0, 0.0, 0.0, &clip);
                bottom = *b;
            }
            dest.ImportPagesFrom(doc, {p}, 0);
            {
                const std::size_t leaf =
                    static_cast<std::size_t>(dest.Pages().Count()) - 1;
                const Aspose::Pdf::Rectangle clip(0.0, 0.0, w, bottom, false);
                dest.TransformPageContent(leaf, 1.0, 1.0, 0.0, 0.0, &clip);
            }
        }
        dest.Save(outputFile);
    }, false, allow_concatenate_exceptions_);
}

// ===== Properties — real storage =============================================

bool PdfFileEditor::AllowConcatenateExceptions() const noexcept {
    return allow_concatenate_exceptions_;
}
void PdfFileEditor::AllowConcatenateExceptions(bool v) noexcept {
    allow_concatenate_exceptions_ = v;
}

bool PdfFileEditor::CloseConcatenatedStreams() const noexcept {
    return close_concatenated_streams_;
}
void PdfFileEditor::CloseConcatenatedStreams(bool v) noexcept {
    close_concatenated_streams_ = v;
}

int PdfFileEditor::ConcatenationPacketSize() const noexcept {
    return concatenation_packet_size_;
}
void PdfFileEditor::ConcatenationPacketSize(int v) noexcept {
    concatenation_packet_size_ = v;
}

const std::string& PdfFileEditor::ConversionLog() const noexcept {
    return conversion_log_;
}

void PdfFileEditor::ConvertTo(Aspose::Pdf::PdfFormat v) noexcept {
    convert_to_ = v;
}

bool PdfFileEditor::CopyLogicalStructure() const noexcept {
    return copy_logical_structure_;
}
void PdfFileEditor::CopyLogicalStructure(bool v) noexcept {
    copy_logical_structure_ = v;
}

bool PdfFileEditor::CopyOutlines() const noexcept {
    return copy_outlines_;
}
void PdfFileEditor::CopyOutlines(bool v) noexcept {
    copy_outlines_ = v;
}

PdfFileEditor::ConcatenateCorruptedFileAction
PdfFileEditor::CorruptedFileAction() const noexcept {
    return corrupted_file_action_;
}
void PdfFileEditor::CorruptedFileAction(
        ConcatenateCorruptedFileAction v) noexcept {
    corrupted_file_action_ = v;
}

const std::vector<PdfFileEditor::CorruptedItem>&
PdfFileEditor::CorruptedItems() const noexcept {
    return corrupted_items_;
}

bool PdfFileEditor::IncrementalUpdates() const noexcept {
    return incremental_updates_;
}
void PdfFileEditor::IncrementalUpdates(bool v) noexcept {
    incremental_updates_ = v;
}

bool PdfFileEditor::KeepActions() const noexcept { return keep_actions_; }
void PdfFileEditor::KeepActions(bool v) noexcept { keep_actions_ = v; }

bool PdfFileEditor::KeepFieldsUnique() const noexcept {
    return keep_fields_unique_;
}
void PdfFileEditor::KeepFieldsUnique(bool v) noexcept {
    keep_fields_unique_ = v;
}

void PdfFileEditor::LastException(const std::string& v) {
    (void)v;
    // v1 stub — System.Exception drops via translations; the
    // setter accepts the call but doesn't surface the exception.
}

bool PdfFileEditor::MergeDuplicateLayers() const noexcept {
    return merge_duplicate_layers_;
}
void PdfFileEditor::MergeDuplicateLayers(bool v) noexcept {
    merge_duplicate_layers_ = v;
}

bool PdfFileEditor::MergeDuplicateOutlines() const noexcept {
    return merge_duplicate_outlines_;
}
void PdfFileEditor::MergeDuplicateOutlines(bool v) noexcept {
    merge_duplicate_outlines_ = v;
}

bool PdfFileEditor::OptimizeSize() const noexcept {
    return optimize_size_;
}
void PdfFileEditor::OptimizeSize(bool v) noexcept { optimize_size_ = v; }

const std::string& PdfFileEditor::OwnerPassword() const noexcept {
    return owner_password_;
}
void PdfFileEditor::OwnerPassword(std::string v) {
    owner_password_ = std::move(v);
}

bool PdfFileEditor::PreserveUserRights() const noexcept {
    return preserve_user_rights_;
}
void PdfFileEditor::PreserveUserRights(bool v) noexcept {
    preserve_user_rights_ = v;
}

bool PdfFileEditor::RemoveSignatures() const noexcept {
    return remove_signatures_;
}
void PdfFileEditor::RemoveSignatures(bool v) noexcept {
    remove_signatures_ = v;
}

const std::string& PdfFileEditor::UniqueSuffix() const noexcept {
    return unique_suffix_;
}
void PdfFileEditor::UniqueSuffix(std::string v) {
    unique_suffix_ = std::move(v);
}

bool PdfFileEditor::UseDiskBuffer() const noexcept {
    return use_disk_buffer_;
}
void PdfFileEditor::UseDiskBuffer(bool v) noexcept {
    use_disk_buffer_ = v;
}

}  // namespace Aspose::Pdf::Facades
