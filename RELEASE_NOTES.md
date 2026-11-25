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
    Remove `~/local/lib` from `LD_LIBRARY_PATH` if it is set, because `LD_LIBRARY_PATH` is searched before the library path recorded in the programs.
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
- (2026-11-21) CI: more checks.
  - The CMake jobs run the tests by `cmake-helper.sh` (the unit tests and the tests of the installation),
    and check that the uninstall removes all the installed files.
  - The Autotools jobs check that each installed header file can be compiled alone (`cmake-helper.sh check-headers`),
    and check the source tarball (`make distcheck`, with the unit tests).
  - A job compares the files installed by CMake and Autotools (except `lib/cmake/librcsc/` of CMake and `.la` of Autotools).
- (2026-11-21) Documents: `README.md` describes the quick start with `cmake-helper.sh` on Ubuntu 24.04 and 26.04
  (the default installation directory is `~/.local`). The details are moved to `docs/build.md`:
  the commands of `cmake-helper.sh`, the options of CMake, the installed files and the uninstall,
  the migration from `~/local`, the tests, the Autotools build, using librcsc from other programs
  (CMake, pkg-config, `librcsc-config`, `LD_LIBRARY_PATH`), and the API documents.
- (2026-11-21) CMake build: the generated `Doxyfile` enables the graphs (`HAVE_DOT`) if Graphviz (`dot`) is found, as in the Autotools build.
- (2026-11-21) Added the check of the installed tools: the `installed_tools` test of CMake, `./cmake-helper.sh check-tools PREFIX`,
  and the CI of Autotools check that the installed tools run without `LD_LIBRARY_PATH` and use librcsc in the same installation.
  `docs/build.md` describes that `sudo ldconfig` is needed after installing librcsc into a system directory such as `/usr/local`.
- (2026-11-24) Support the new server parameters of rcssserver 20 for the TCP and WebSocket connections:
  `tcp_port`, `tcp_coach_port`, `tcp_olcoach_port`, `ws_port` and `ws_allowed_origins`.
  librcsc only reads them; the clients of librcsc still connect with UDP.
  - rcssserver does not send these parameters to players and coaches,
    but writes them to the game log (rcg) and sends them to monitors.
  - `rcsc::ServerParam`: new accessors `tcpPlayerPort()`, `tcpTrainerPort()`, `tcpOnlineCoachPort()`, `wsPort()` and `wsAllowedOrigins()`.
    0 means that TCP or WebSocket is disabled.
    If a TCP port parameter is -1, the accessor returns the matching UDP port number (`playerPort()`, `trainerPort()` or `onlineCoachPort()`), as rcssserver does.
    `ServerParam::toServerString()` also prints these parameters.
  - `rcsc::rcg::ServerParamT`: new members `tcp_port_`, `tcp_coach_port_`, `tcp_olcoach_port_`, `ws_port_` and `ws_allowed_origins_`,
    which are converted by `ServerParam::convertFrom()` and `convertTo()`.
    The rcg files of rcssserver 20 are read without the warnings `Unsupported parameter.` for these parameters.
  - The sizes of `rcsc::ServerParam` and `rcsc::rcg::ServerParamT` are changed.
    Rebuild the programs built with librcsc of the develop branch before this change (the library version 20 is not released yet).
- (2026-11-24) Tests: added the unit test `run_test_server_param` (`rcsc/common`, CMake and Autotools).
  The unit tests of the CMake build are run with `LD_LIBRARY_PATH` set to the library directory in the build tree,
  so they test the built librcsc even if `LD_LIBRARY_PATH` contains an installed librcsc.
- (2026-11-24) Fixed the values of the noise rate parameters of the player types written by librcsc
  (`rcsc::rcg::PlayerTypeT::toServerString()` and `toJSON()`).
  `focus_dist_noise_rate`, `land_dist_noise_rate` and `land_focus_dist_noise_rate` were written with the value of `dist_noise_rate`.
  The rcg files written by librcsc (e.g. by `rcgrenameteam`, `rcgreverse`, `rcgverconv` and soccerwindow2) had the wrong values;
  with the default values of rcssserver, `land_dist_noise_rate` and `land_focus_dist_noise_rate` became 0.0125 instead of 0.00125.
  The rcg files written by rcssserver are not affected.
- (2026-11-24) Tests: added the unit test `run_test_rcg_types` (`rcsc/rcg`, CMake and Autotools).
  It checks that each parameter of `PlayerTypeT` is written with its own value.
- (2026-11-24) Tests and tools for the development of the rcg parser (see `docs/build.md`):
  - The unit test `run_test_rcg_parser` (`rcsc/rcg`, CMake and Autotools) reads the first records of real game logs
    (text v5 and v6, and JSON) in `rcsc/rcg/test_data`.
  - `rcg_parser_bench` (built in `src/`, not installed) measures the time to load game logs.
  - `test/compare_rcg_output.sh` checks that two builds of librcsc read the same data from game logs, by the outputs of `rcgverconv`.
- (2026-11-24) The bundled simdjson, used by the JSON rcg parser, is updated from 3.8.0 to 5.1.1.
  The 9 warnings of GCC 15 in `simdjson.cpp` (`-Wtemplate-id-cdtor`) are gone,
  and the fix for GCC 15 applied to 3.8.0 is included in 5.1.1.
  The header files of simdjson are not installed, so the programs using librcsc are not affected.
  Compiling `simdjson.cpp` takes longer (about 9 seconds instead of 3 with `-O3`).
- (2026-11-24) Moved the old JSON rcg parser `rcsc/rcg/parser_json.{h,cpp}`, which has never been built, to `obsolete/rcg`.
  Deleted the bundled nlohmann/json (`rcsc/rcg/nlohmann/json.hpp`), which only this parser used.
- (2026-11-25) New function `rcsc::gzread_file()` (`rcsc/gz/gzread_file.h`, also included by `rcsc/gz.h`)
  reads a whole file into memory: a gzip file is decompressed with zlib, and any other file is read as it is.
  It is faster than reading the file through `rcsc::gzifstream`.
  A gzip file that ends in the middle is read up to there, with a warning.
- (2026-11-25) The rcg parser parses the game log in memory, which is faster:
  - New functions `rcsc::rcg::Parser::create( const char * data, std::size_t size )` and
    `Parser::parse( const char * data, std::size_t size, Handler & handler )` for the whole game log in memory
    (e.g. read by `rcsc::gzread_file()`).
  - `Parser::parse( const std::string & filepath, Handler & )` reads the file by `rcsc::gzread_file()`, so it can read a gzip file.
    It can be called on the derived classes (e.g. `ParserV4`), which hid it before.
  - `ParserV4::parse( std::istream &, Handler & )` and `ParserSimdJSON::parse( std::istream &, Handler & )`
    read the stream into memory and parse it there, so the programs using them (e.g. soccerwindow2) also get faster.
  - A JSON game log of one record per line, as rcssserver writes it, is parsed line by line.
    A JSON game log whose last `]` is missing (rcssserver stopped abnormally) can be read.
    The other JSON game logs (e.g. in one line, or pretty-printed) are parsed as one JSON document as before.
  - The JSON parser stops if `Handler::handleLogVersion()` returns false, as the text parser does.
    A handler that returns false for the version -1 (JSON) no longer reads JSON game logs.
  - The virtual functions of `Parser` are changed. Rebuild the programs built with librcsc of the develop branch before this change.
- (2026-11-25) The tools (`rcg2csv`, `rcg2txt`, `rcgrenameteam`, `rcgresultprinter`, `rcgreverse`, `rcgvalidator` and `rcgverconv`)
  read the game log by `rcsc::gzread_file()` and parse it in memory.
  - `rcgresultprinter` no longer writes the decompressed gzip file to the temporary file `temp.rcg`.
  - `rcg2csv` and `rcgvalidator` accept JSON game logs explicitly.
    They returned false from `handleLogVersion()` for JSON (`rcgvalidator` printed `Unsupported RCG version -1`),
    and read JSON game logs only because the JSON parser ignored it.
  - Fixed the dates printed by `rcgresultprinter`, which could be one hour off,
    because `tm_isdst` was not initialized before `mktime()`.
- (2026-11-25) Tests: added the unit test `run_test_gzread_file` (`rcsc/gz`, CMake and Autotools).
- (2026-11-25) The rcg parser parses the lines of a game log in parallel, which makes loading a game log about twice as fast:
  - The text game logs (v4 to v6) and the JSON game logs of one record per line are parsed by several threads,
    if they have many lines (at least 1024 lines for a thread).
  - The handler is always called in the thread that calls `parse()`, in the order of the lines,
    so the handlers do not need to be thread-safe.
  - New functions `rcsc::rcg::Parser::setMaxThreads( int n )` and `Parser::maxThreads()`:
    the upper limit of the number of the threads. The default is the smaller of the number of the hardware threads and 8.
    `setMaxThreads( 1 )` disables the parallel parsing, e.g. for a program that processes many game logs in parallel processes.
  - The return values of the handler's functions no longer stop the parsing (the JSON parser stopped before).
    An exception thrown by the handler goes out of `parse()`.
  - A line that cannot be parsed is skipped with a message, and the parsing goes on.
    Before, the parsing stopped there and `parse()` returned false.
  - `rcsc::rcg::Handler::handleTimestamp()` is now virtual. Rebuild the programs built with librcsc of the develop branch before this change.
  - librcsc is linked with the thread library (`Threads::Threads` of CMake, `-pthread` of Autotools).
- (2026-11-25) Tests: added the unit test `run_test_rcg_line_parser` (`rcsc/rcg`, CMake and Autotools).
- (2026-11-25) libdeflate (optional): if it is found, `rcsc::gzread_file()` decompresses gzip files with libdeflate,
  which is about twice as fast as zlib (e.g. loading a game log by the tools takes about 35% less time).
  - It is used if it is found. `-DENABLE_LIBDEFLATE=OFF` (CMake) or `--without-libdeflate` (Autotools) disables it,
    and `--with-libdeflate` makes it an error of `configure` if it is not found. Install it by `sudo apt install libdeflate-dev` on Ubuntu.
  - zlib is still used to read a broken gzip file (e.g. one that ends in the middle) up to where it is broken,
    to write gzip files, and for the gzip streams (`rcsc::gzifstream`, `rcsc::gzofstream`).
  - `librcsc.so` is linked with libdeflate if it is used. The programs using librcsc do not need it themselves.
  - CI: the CMake jobs build librcsc with libdeflate, and the Autotools jobs without it.
- (2026-11-25) Fixed the problems of the rcg parsers in reading broken or new game logs:
  - Text (v4 to v6): a record name longer than 31 characters overflowed a buffer.
    The player number 0 in a show wrote outside the array of the players.
    A show with a broken time or ball was not rejected. A broken header (e.g. `ULGx`) threw an exception.
    An empty line was an error. An unknown record now is skipped and reported once in a process.
    The strings of a team graphic were read only up to 15 characters. The team name `null` in a team line is now empty, as in a show line.
  - JSON: the server version and the timestamp are passed to the handler (`handleServerVersion()`, `handleTimestamp()`).
    The view quality of a player was stored as 1 or 0 instead of `'h'` or `'l'`,
    so a JSON game log rewritten by the tools (e.g. `rcgverconv`) had `"vq":"l"` for all the players,
    and a text game log converted from JSON had a control character there.
    The players of a show are placed by their side and number, not by the order in the array;
    the fields that the server may omit keep their default values, and illegal players are skipped.
    An unknown record or key is skipped and reported once in a process (it stopped the whole parsing before).
    `ParserSimdJSON::parseData()` (for a live connection) returns false for a broken message instead of throwing an exception.
- (2026-11-25) `rcgreverse`, `rcgrenameteam` and `rcgverconv` write the server version and the timestamp of a JSON game log
  to the output, instead of `"version":"unknown"`: the beginning of the output is written before the first record,
  after the version and the timestamp are read.
- (2026-11-25) `rcgversion` prints the version 6 (it printed `json`), and `unknown` for an unknown version.
