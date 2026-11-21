# Release Notes

librcsc does not publish versioned releases.
This file summarizes new features and major changes as they are merged.
Each item has the date when the change was made.

## Unreleased

- (2026-11-21) Added this release notes file to track new features and major changes.
- (2026-11-21) Boost:
  - Boost is now used as header-only; the Boost.System library is no longer required.
    librcsc can now be built with Boost 1.89 or later, in which the Boost.System library was removed.
  - The minimum required Boost version is now 1.70.
  - The CMake build finds Boost with the `BoostConfig.cmake` provided by Boost (`find_package(Boost 1.70 CONFIG)`),
    instead of the FindBoost module of CMake.
  - The installed header files no longer include the Boost headers, so the programs using librcsc do not need Boost unless they use it themselves.
    A program that used the headers included through the librcsc headers (e.g. `boost/scoped_ptr.hpp`, `boost/array.hpp`,
    or the standard headers included by them) must include them directly.
  - `librcsc.spec.in`: the devel package no longer requires `boost-devel`.
- (2026-11-21) CMake build: the build is fixed, and the same files as the Autotools build are installed.
  - Fixed the build error caused by the missing check of `netinet/in.h`.
  - The missing header files `rcsc/player/world_state.h` and `rcsc/time/time_stamp.h` are installed.
    The programs including `rcsc/player/world_model.h` or `rcsc/time/timer.h` could not be compiled with the librcsc installed by CMake.
  - `rcgvalidator` is built and installed, and `rcg2csv` is installed.
  - `rclmtableprinter` is built from `tableprinter.cpp`. It was built from the source of `rcgresultprinter` by mistake.
  - `librcsc.so` is linked with zlib, so the programs using librcsc do not need to link zlib themselves.
  - The development tool `object_table_printer` (not installed) is also built, as in the Autotools build.
- (2026-11-21) Autotools build: `librcsc.so` is linked with the C++ linker, so `libstdc++` is recorded as its dependency.
- (2026-11-21) The package version of the Autotools build (`AC_INIT`) is now 2026, the same as the CMake build.
  It is shown by `librcsc-config --version` and pkg-config (`pkg-config --modversion librcsc`).
- (2026-11-21) `librcsc-config --libs` prints `-lrcsc -lz -lm`, the same as pkg-config.
  It printed the libraries that no longer exist (e.g. `-lrcsc_agent`).
- (2026-11-21) Moved `rcsc/action`, which has not been built since 2022, to `obsolete/action`.
- (2026-11-21) Unit tests (Autotools):
  - CppUnit is now found by pkg-config (`cppunit.pc`), so `./configure --enable-unit-test` works with CppUnit 1.13 or later,
    which no longer provides `cppunit-config`. pkg-config is now needed to run `./bootstrap`.
  - Fixed the unit tests that could not be compiled (`rcsc/geom/test_rundom_convex_hull.cpp` and `rcsc/clang/test_clang_parser.cpp`).
  - Run the tests with `./configure --enable-unit-test` and `make check` (CppUnit is needed, e.g. `libcppunit-dev` on Ubuntu).
- (2026-11-21) Fixed the build errors with GCC 15 (e.g. Ubuntu 26.04):
  - Added `#include <algorithm>` to the files that use the algorithms, which are no longer included through the other standard headers.
  - Patched the bundled simdjson 3.8.0 with the upstream fix for GCC 15 (the calls of the member functions of the derived class in `base_formatter`).
- (2026-11-21) Added the CI with GitHub Actions (`.github/workflows/build.yml`).
  It builds and installs librcsc with CMake and Autotools on Ubuntu 24.04 and 26.04, and runs the unit tests with Autotools.
