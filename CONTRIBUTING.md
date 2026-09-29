# Contributing to Aspose.PDF FOSS for C++

Thanks for your interest in contributing! This document covers how to
build, test, and submit changes.

## Building

Requirements: a C++20 compiler (clang ≥ 16, gcc ≥ 13, or MSVC 2022 ≥
17.5), CMake ≥ 3.22, and Python 3 (used by a build step that embeds the
bundled font outlines).

```bash
cmake -S . -B build
cmake --build build -j
```

The static library lands at `build/libaspose_pdf_foss.a` (`.lib` on
MSVC). See [`README.md`](README.md) for the full requirements and a
quick-start tour of the API.

## Running the tests

```bash
cmake --build build
cd build && ctest --output-on-failure
```

The test suite uses [GoogleTest](https://github.com/google/googletest),
which CMake fetches automatically at configure time (this needs network
access on the first configure).

## Repository layout

| Path | Contents |
|------|----------|
| `include/aspose/pdf/` | Public, consumer-facing API headers |
| `include/internal/`   | Internal foundation-primitive headers |
| `src/public/`         | Public-API implementation |
| `src/internal/`       | Foundation primitives (codecs, parsers, rasteriser, …) |
| `tests/`              | GoogleTest unit + smoke tests |
| `examples/`           | Runnable demo programs |
| `pdfs/`               | Small sample PDFs used by the examples |

Public-API code under `src/public/` calls into the foundation primitives
under `src/internal/`; the primitives never reach back into the public
API.

## Coding conventions

- Match the style of the surrounding code (indentation, naming, comment
  density). The codebase is C++20 and dependency-free at runtime — please
  keep it that way; do not introduce third-party runtime dependencies.
- Keep public headers under `include/aspose/pdf/` clean: the public
  surface mirrors the canonical Aspose.PDF API names and shapes. Public
  headers must never include anything from `include/internal/`, because
  only `include/aspose/` is installed. When you add a public header, also
  add it to the single-entry header `include/aspose.pdf.foss.hpp`.
- Add or update tests under `tests/` for any behavioural change, and make
  sure `ctest` is green before opening a pull request.

## Submitting a pull request

1. Fork the repository and create a topic branch.
2. Make your change with accompanying tests.
3. Ensure `cmake --build build` and `ctest` both pass.
4. Open a pull request describing the change and the motivation.

By submitting a contribution you agree that it is licensed under the
project's [MIT License](LICENSE).

## Releasing

Releases are built by [`.github/workflows/release.yml`](.github/workflows/release.yml)
when a `vX.Y.Z` tag is pushed (`vX.Y.Z-rc.1` and similar produce a
pre-release). The version follows [Semantic Versioning](https://semver.org).

1. **Bump the version** in `project(Aspose_PDF_FOSS VERSION X.Y.Z ...)` in
   `CMakeLists.txt`. The workflow fails if the tag and this version
   differ. If the major version changes, also update the version in
   `find_package(aspose_pdf_foss X.Y ...)` in
   `.github/package-test/CMakeLists.txt`.
2. **Update `CHANGELOG.md`.** Move the `[Unreleased]` entries into a new
   `## [X.Y.Z] - YYYY-MM-DD` section and update the compare links at the
   bottom.
3. **Write release notes** (optional). The GitHub release body is taken from
   `.github/release-notes/vX.Y.Z.md` when that file exists, and otherwise
   from the `## [X.Y.Z]` section of `CHANGELOG.md`.
4. **Dry-run the packaging** (optional). Run the *Release* workflow by hand
   on your branch (Actions → Release → Run workflow). It builds and tests
   the archives and uploads them as workflow artifacts, without creating a
   release.
5. **Commit, tag, and push:**
   ```bash
   git commit -am "Release X.Y.Z"
   git tag -a vX.Y.Z -m "Aspose.PDF FOSS for C++ X.Y.Z"
   git push origin main vX.Y.Z
   ```
6. **Publish.** The workflow builds and tests Linux x64 (GCC 13) and
   Windows x64 (MSVC Release + Debug), packages each `cmake --install`
   tree, compiles and runs `.github/package-test` against every package,
   and creates a **draft** GitHub release with the archives and
   `SHA256SUMS.txt` attached. Review the draft on GitHub and press
   *Publish release*.

Re-running the workflow for an existing tag replaces the release's assets
and leaves its notes unchanged.
