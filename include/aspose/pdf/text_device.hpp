#pragma once

// =============================================================================
// Aspose::Pdf::Devices::TextDevice — sealed concrete PageDevice that
// emits extracted text from a page into the output stream.
//
// v1 surface (per capabilities/text_device.yaml after the cpp Stream
// cascade in translations.yaml drops Process(Page, Stream)):
//   * default ctor + Encoding-taking ctor
//   * Encoding property (read-write)
//   * Process(Page, ostream&) hand-authored convenience (kept for
//     library users who need direct std::ostream output)
//
// Encoding-taking ctor + Encoding property activate the
// Aspose::Pdf::Text::Encoding charset codec — see
// the project spec for the BCL contract. Default is
// UTF-8 (matches existing csharp lib state per minimum-change rule).
// =============================================================================

#include <ostream>

#include "page_device.hpp"
#include <aspose/pdf/encoding.hpp>

namespace Aspose::Pdf {
class Page;
}

namespace Aspose::Pdf::Devices {

class TextDevice final : public PageDevice {
public:
    TextDevice();
    explicit TextDevice(const ::Aspose::Pdf::Text::Encoding& encoding);

    const ::Aspose::Pdf::Text::Encoding& GetEncoding() const noexcept;
    void SetEncoding(const ::Aspose::Pdf::Text::Encoding& encoding) noexcept;

    void Process(const ::Aspose::Pdf::Page& page,
                 std::ostream& output) override;

private:
    const ::Aspose::Pdf::Text::Encoding* encoding_;
};

}
