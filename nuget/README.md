# Aspose.PDF FOSS for C++

A free, open-source (MIT) C++20 library for working with PDF documents. It has no third-party
runtime dependencies: every codec and the page rasteriser are implemented from scratch. Its
public API is a strict subset of Aspose.PDF for .NET.

This package is the native static library for **Visual Studio C++ projects** (MSBuild), built
for x64, x86 and ARM64 in both Release and Debug.

## What it does

- Open, edit and save PDF documents, including password-protected files.
- Extract text from a whole document, a single page, or as positioned fragments.
- Render pages to PNG, JPEG, BMP and TIFF, including multi-page and palettised TIFF.
- Create documents with text, tables, vector graphics, images and watermarks.
- Add annotations and AcroForm fields, and flatten forms.
- Encrypt and decrypt with RC4-40/128 and AES-128/256, and add detached PKCS#7 signatures.
- Work with bookmarks, named destinations, page labels, embedded files and metadata.
- Use the classic Facades API (`PdfConverter`, `PdfExtractor`, `PdfFileEditor`, and others).

## Usage

Install the package into a C++ project (**Manage NuGet Packages** → `Aspose.PDF.Cpp.FOSS`).
The package does the rest:

- adds the include directory;
- links the library that matches your platform and configuration;
- raises the project's language standard to C++20 if it is set lower.

```cpp
#include <aspose.pdf.foss.hpp>
#include <fstream>
#include <iostream>

int main() {
    Aspose::Pdf::Document doc("input.pdf");
    std::cout << "Pages: " << doc.Pages().Count() << "\n";

    Aspose::Pdf::Text::TextAbsorber absorber;
    absorber.Visit(doc);
    std::cout << absorber.Text() << "\n";

    Aspose::Pdf::Devices::PngDevice png(Aspose::Pdf::Devices::Resolution(150));
    std::ofstream out("page1.png", std::ios::binary);
    png.Process(doc.Pages()[1], out);
}
```

## Requirements

- Visual Studio 2022 (MSVC toolset v143) or newer.
- Platform `x64`, `Win32` or `ARM64`.
- The DLL C runtime: `/MD` for Release or `/MDd` for Debug. This is the Visual Studio default,
  and the package warns if the project uses the static runtime (`/MT`, `/MTd`).
- Debug or Release is picked from `UseDebugLibraries`, which Visual Studio sets for Debug
  configurations.

To stop the package from changing the language standard, or from linking the library
automatically, set these properties in your project or in a `Directory.Build.props`:

```xml
<AsposePdfFossSetLanguageStandard>false</AsposePdfFossSetLanguageStandard>
<AsposePdfFossLinkLibrary>false</AsposePdfFossLinkLibrary>
```

CMake users, and Linux users, can use the `find_package`-ready archives attached to each
[GitHub release](https://github.com/aspose-pdf-foss/Aspose.PDF-FOSS-for-Cpp/releases).

## Links

- [Source code and examples](https://github.com/aspose-pdf-foss/Aspose.PDF-FOSS-for-Cpp)
- [Getting started guide](https://docs.aspose.org/pdf/cpp/)
- [API reference](https://reference.aspose.org/pdf/cpp/)
- [Changelog](https://github.com/aspose-pdf-foss/Aspose.PDF-FOSS-for-Cpp/blob/main/CHANGELOG.md)
- [Issues](https://github.com/aspose-pdf-foss/Aspose.PDF-FOSS-for-Cpp/issues)

## License

The library is licensed under the MIT License. It compiles in the Liberation fonts, which are
licensed under the SIL Open Font License 1.1, so the package license is `MIT AND OFL-1.1`. The
full license texts are in the package's `licenses/` folder.
