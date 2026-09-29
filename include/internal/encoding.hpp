#pragma once

// Charset codec primitive — the class itself is public API
// (Aspose::Pdf::Text::Encoding, <aspose/pdf/encoding.hpp>) because
// TextDevice exposes it. This header keeps the foundation::encoding
// spelling working for library internals and tests.

#include <aspose/pdf/encoding.hpp>

namespace foundation::encoding {

using Encoding = ::Aspose::Pdf::Text::Encoding;

}  // namespace foundation::encoding
