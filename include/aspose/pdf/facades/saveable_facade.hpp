#pragma once

// =============================================================================
// Aspose::Pdf::Facades::SaveableFacade — Facade subclass that adds
// Save methods so callers can serialise the bound document back
// to disk after the facade mutates it. Mirrors canonical
// Aspose.Pdf.Facades.SaveableFacade.
//
// Phased drops:
//   * Save(Stream) — Stream cascade
// =============================================================================

#include <string>

#include <aspose/pdf/facades/facade.hpp>
#include <aspose/pdf/facades/i_saveable_facade.hpp>

// MSVC C4250: BindPdf/Close are inherited from Facade by dominance over
// the virtual IFacade base also reached via ISaveableFacade — intended.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

namespace Aspose::Pdf::Facades {

class SaveableFacade : public Facade, public virtual ISaveableFacade {
public:
    SaveableFacade() noexcept = default;
    ~SaveableFacade() override = default;

    void Save(const std::string& destFile) override;
};

}  // namespace Aspose::Pdf::Facades

#ifdef _MSC_VER
#pragma warning(pop)
#endif
