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
- (2026-11-21) CMake build: fixed the build error caused by the missing check of `netinet/in.h`.
- (2026-11-21) Unit tests (Autotools):
  - CppUnit is now found by pkg-config (`cppunit.pc`), so `./configure --enable-unit-test` works with CppUnit 1.13 or later,
    which no longer provides `cppunit-config`. pkg-config is now needed to run `./bootstrap`.
  - Fixed the unit tests that could not be compiled (`rcsc/geom/test_rundom_convex_hull.cpp` and `rcsc/clang/test_clang_parser.cpp`).
  - Run the tests with `./configure --enable-unit-test` and `make check` (CppUnit is needed, e.g. `libcppunit-dev` on Ubuntu).
- (2026-11-21) Added the CI with GitHub Actions (`.github/workflows/build.yml`).
  It builds and installs librcsc with CMake and Autotools on Ubuntu 24.04 and 26.04, and runs the unit tests with Autotools.
