// Release-package smoke test: exercises the installed library through
// the single-entry header only — author a page, save, reopen, extract
// text, and rasterise (which pulls in the embedded Standard-14 fonts).
#include <aspose.pdf.foss.hpp>

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

int main() {
    namespace pdf = Aspose::Pdf;
    namespace txt = Aspose::Pdf::Text;

    const std::string marker = "Aspose.PDF FOSS package test";
    const auto path =
        (std::filesystem::temp_directory_path() / "aspose_pdf_foss_package_test.pdf").string();

    try {
        {
            pdf::Document doc;
            pdf::Page page = doc.Pages().Add();
            page.SetPageSize(595.0, 842.0);

            txt::TextFragment frag(marker);
            frag.TextState().Font(txt::FontRepository::FindFont("Helvetica"));
            frag.TextState().FontSize(14.0f);
            frag.Position(txt::Position(60.0, 760.0));
            txt::TextBuilder{page}.AppendText(frag);

            doc.Save(path);
        }

        pdf::Document doc(path);
        if (doc.Pages().Count() != 1) {
            std::cerr << "expected 1 page, got " << doc.Pages().Count() << "\n";
            return 1;
        }

        txt::TextAbsorber absorber;
        absorber.Visit(doc);
        if (absorber.Text().find(marker) == std::string::npos) {
            std::cerr << "extracted text does not contain the marker:\n"
                      << absorber.Text() << "\n";
            return 1;
        }

        std::ostringstream png;
        pdf::Devices::PngDevice device(pdf::Devices::Resolution(72));
        device.Process(doc.Pages()[1], png);
        const std::string bytes = png.str();
        if (bytes.size() < 8 || bytes.compare(1, 3, "PNG") != 0) {
            std::cerr << "PNG rendering produced no valid image\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "exception: " << e.what() << "\n";
        return 1;
    }

    std::remove(path.c_str());
    std::cout << "package test OK\n";
    return 0;
}
