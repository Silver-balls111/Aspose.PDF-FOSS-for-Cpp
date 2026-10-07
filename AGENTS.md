# AGENTS.md

Guidance for AI coding agents working in this repository.

## What this is

Aspose.PDF FOSS for C++ — a dependency-free C++20 library for opening, editing,
creating, rendering, encrypting, and signing PDF documents. The public API is a
deliberate subset of the commercial Aspose.PDF for .NET API: class names, method
names, and shapes mirror it, so match the canonical .NET names when adding API.
Everything (TIFF/JPEG/PNG codecs, rasteriser, crypto, TrueType parser) is
implemented from scratch on top of the C++ standard library.

**This checkout is a fork.** `origin` is `Silver-balls111/Aspose.PDF-FOSS-for-Cpp`;
`upstream` is `aspose-pdf-foss/Aspose.PDF-FOSS-for-Cpp`, which is merged in
periodically. Consequences:

- Record fork-specific work in **`CHANGELOG.fork.md`** (Keep a Changelog format).
  `CHANGELOG.md` belongs to upstream and is overwritten by upstream merges — do
  not put fork work there.
- When resolving merge conflicts after an upstream merge, upstream's
  `CHANGELOG.md` / `README.md` / `CONTRIBUTING.md` generally win; fork changes
  live in the fork changelog and in code.

## Build and test

```bash
cmake -S . -B build          # defaults to Release when no build type given
cmake --build build -j
ctest --test-dir build --output-on-failure
```

- Toolchain: C++20 compiler (gcc ≥ 13, clang ≥ 16, or MSVC 2022 ≥ 17.5),
  CMake ≥ 3.22, and Python 3 on PATH (used at build time only, to embed the
  bundled Liberation font outlines into a generated `.inc`).
- First configure needs network access: GoogleTest is fetched via
  `FetchContent` and is the only dependency (test-only).
- The library lands at `build/libaspose_pdf_foss.a` (`.lib` on MSVC).
- Keep the default Release build; the rasteriser/TrueType code is ~10× slower
  unoptimised. Pass `-DCMAKE_BUILD_TYPE=Debug` only when you need a debugger.
- CMake presets: `default` (Ninja, Release), `default-debug`, and
  `windows-msvc` on Windows (`cmake --preset default` etc.).

### Test suite rules

- One GoogleTest binary, `aspose_pdf_foss_tests`, built from `tests/`.
  CMake globs **only** `tests/*_smoke_test.cpp` and `tests/*.cc`
  (`CONFIGURE_DEPENDS`). Files named `*_test.cpp` in `tests/` are legacy
  standalone `int main()` programs that are **not compiled** — name new test
  files `*_smoke_test.cpp` or they will silently never run.
- The whole binary registers as a single ctest test. To run individual cases:
  `./build/aspose_pdf_foss_tests --gtest_filter=SuiteName.CaseName`.
- Add or update tests for any behavioural change, and get `ctest` green before
  finishing.
- Test targets may include `include/internal/` headers directly (private
  include path); production code may not — see below.

## Architecture and layering

| Path | Role |
|------|------|
| `include/aspose/pdf/` | Public, consumer-facing API headers |
| `include/aspose.pdf.foss.hpp` | Single-entry header; register new public headers here |
| `include/internal/` | Internal foundation-primitive headers |
| `src/public/` | Public-API implementation |
| `src/internal/` | Foundation primitives (codecs, parsers, rasteriser, crypto, …) |
| `tests/` | GoogleTest suite (`*_smoke_test.cpp`) |
| `examples/` | Standalone demo programs (own `CMakeLists.txt`), with sample PDFs in `pdfs/` |

Hard rules:

- `src/public/` calls into `src/internal/`; foundation primitives never reach
  back into the public API.
- Public headers must never include anything from `include/internal/` — only
  `include/aspose/` is installed/shipped.
- The library source list in `CMakeLists.txt` is an **explicit list, not a
  glob**. Adding a `.cpp` file requires adding it there. Foundation primitives
  are only compiled once their public-API consumer lands — unreferenced
  primitives that exist on disk stay out of the build on purpose (they'd pull
  system dependencies for no caller).

## Coding conventions

- Match the surrounding code's style, naming, and comment density. Sources are
  UTF-8 (em dashes, ≥, → appear in comments and string literals; MSVC builds
  with `/utf-8`).
- Keep the runtime dependency-free: no third-party runtime libraries, no
  package-manager additions. If a feature needs a codec or primitive, it is
  implemented from scratch under `src/internal/`.
- Do not commit build output or local state: `build/`, `.cache/`, `.zcode/`,
  `*.o`, lock/scratch files are gitignored.

## Releasing (fork maintainer)

Versions follow SemVer. Bump `project(Aspose_PDF_FOSS VERSION X.Y.Z)` in
`CMakeLists.txt` (the release workflow fails if the pushed tag differs), then
tag `vX.Y.Z`. Upstream's full checklist lives in `CONTRIBUTING.md`; the same
version-consistency rule applies to this fork's releases.
