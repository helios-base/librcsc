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
- (2026-11-21) CMake build: modernized.
  - The minimum required CMake version is now 3.16.
  - The default installation directory is changed from `~/local` to `~/.local`.
    A build directory configured before keeps `~/local` in the cache.
    To move to `~/.local`, remove the old build directory (or give `-DCMAKE_INSTALL_PREFIX=$HOME/.local`),
    and remove the files installed in `~/local` if they are no longer needed.
  - The CMake package configuration files are installed in `PREFIX/lib/cmake/librcsc` (CMake build only, not Autotools).
    The other CMake projects can use librcsc by `find_package(librcsc CONFIG REQUIRED)` and
    `target_link_libraries(<target> PRIVATE librcsc::rcsc)`, which adds the include directory and requires C++17.
    A version can be requested, e.g. `find_package(librcsc 2024 CONFIG)` accepts librcsc 2026 (a newer version is accepted).
    CMake also searches `PREFIX` when `PREFIX/bin` is in `PATH` (e.g. `~/.local/bin` on Ubuntu);
    otherwise give the installation directory by `-DCMAKE_PREFIX_PATH=PREFIX`.
  - The installed programs record the relative path to librcsc (RPATH `$ORIGIN/../lib`),
    so `LD_LIBRARY_PATH` is not needed to run them, as in the Autotools build.
  - Added the `uninstall` target (`cmake --build build --target uninstall`), which removes the files listed in `install_manifest.txt`
    and the directories `PREFIX/include/rcsc` and `PREFIX/lib/cmake/librcsc` if they become empty.
  - `CMAKE_CXX_FLAGS` given by the user is no longer overwritten. The warning options (`-W -Wall`) are added to the targets of librcsc.
  - All the installation directories follow `GNUInstallDirs` (e.g. `CMAKE_INSTALL_LIBDIR` for `librcsc.pc`).
- (2026-11-21) Tests of the CMake build (run them by `./cmake-helper.sh test` or `cd build && ctest --output-on-failure`):
  - The unit tests (CppUnit) in `rcsc/geom` and `rcsc/clang` are built and run by CTest, the same programs as `make check` of the Autotools build.
    CppUnit is found by pkg-config. Without CppUnit, the unit tests are not built and the other tests are run.
    `-DBUILD_TESTING=OFF` disables all the tests.
  - New tests of the installation: `install` (installs librcsc into `test-install` in the build directory;
    `install_manifest.txt` of the user's installation is kept), `installed_headers` (each installed header file can be compiled alone,
    e.g. no header file is missing in the installation), and `find_package` (a CMake project can use the installed librcsc by
    `find_package(librcsc CONFIG)` and `librcsc::rcsc`).
- (2026-11-21) Added `cmake-helper.sh`, shortcuts for the frequently used CMake commands
  (`configure`, `build`, `test`, `install [--prefix DIR]`, `uninstall`, `check-headers PREFIX`, `clean`).
  The commands are printed before they are run. Run `./cmake-helper.sh help` for the usage.
  `./cmake-helper.sh check-headers PREFIX` checks the header files installed in `PREFIX`, also for librcsc installed by Autotools.
- (2026-11-21) Fixed the header files that could not be compiled alone: `rcsc/ann/bpn1.h` (missing `<iterator>`)
  and `rcsc/geom/polygon_2d.h` (it now includes `rcsc/geom/vector_2d.h`).
- (2026-11-21) CMake build: `config.h` is searched in the build directory before the source directory,
  so that the CMake build is not affected by `config.h` of the Autotools build configured in the source directory.
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
