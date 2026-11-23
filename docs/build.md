# Building librcsc

This document describes how to build, test, and install librcsc in detail, and how to use it from other programs.
For a quick start, see [README.md](../README.md).

- [Requirements](#requirements)
- [Installing the dependencies on Ubuntu](#installing-the-dependencies-on-ubuntu)
- [Building with CMake (recommended)](#building-with-cmake-recommended)
- [Running the tests](#running-the-tests)
- [Building with Autotools](#building-with-autotools)
- [Using librcsc in other programs](#using-librcsc-in-other-programs)
- [Generating the API documents](#generating-the-api-documents)
- [Notes](#notes)

## Requirements

| Software | Version | Note |
|---|---|---|
| C++ compiler | C++17 | GCC or Clang |
| CMake | 3.16 or later | for the CMake build (recommended) |
| Autoconf / Automake / Libtool | | for the Autotools build |
| Boost | 1.70 or later | only the headers are used (the Boost.System library is not needed) |
| zlib | | optional; needed to read and write the gzipped files (e.g. `.rcg.gz`) |
| pkg-config | | to find CppUnit; also needed to run `./bootstrap` of the Autotools build |
| CppUnit | 1.12 or later | optional; for the unit tests |
| Doxygen / Graphviz | | optional; for the API documents |

The installed header files of librcsc do not include the Boost headers,
so the programs using librcsc do not need Boost unless they use it themselves.

librcsc is tested by GitHub Actions on Ubuntu 24.04 (GCC 13.3, CMake 3.31, Boost 1.83)
and Ubuntu 26.04 (GCC 15.2, CMake 4.5, Boost 1.90) with both CMake and Autotools.

## Installing the dependencies on Ubuntu

The following commands are for Ubuntu 24.04 and 26.04.

```
sudo apt update
sudo apt install build-essential cmake pkg-config libboost-dev zlib1g-dev
sudo apt install libcppunit-dev                 # for the unit tests
sudo apt install autoconf automake libtool      # for the Autotools build
sudo apt install doxygen graphviz               # for the API documents
```

## Building with CMake (recommended)

### cmake-helper.sh

`cmake-helper.sh` in the top directory runs the frequently used CMake commands.
It prints each command before running it, so you can also learn the actual cmake commands.
It can be run from any directory; it always works in the top directory of librcsc.

```
./cmake-helper.sh build        # configure (if needed) and build
./cmake-helper.sh test         # build and run the tests
./cmake-helper.sh install      # build and install (into ~/.local by default)
./cmake-helper.sh uninstall    # remove the installed files
./cmake-helper.sh help         # show the usage
```

All the commands:

| Command | What it does | The main cmake command |
|---|---|---|
| `configure` | (re)configure the build directory | `cmake -S . -B build` |
| `build` | configure (if needed) and build | `cmake --build build --parallel JOBS` |
| `test` | build and run the tests | `cd build && ctest --output-on-failure` |
| `install [--prefix DIR]` | build and install | `cmake --install build [--prefix DIR]` |
| `uninstall` | remove the files installed from the build directory | `cmake --build build --target uninstall` |
| `check-headers PREFIX` | check that each header file installed in `PREFIX` can be compiled alone (also for librcsc installed by Autotools) | `cmake -S test/installed_headers -B build/check-headers -DLIBRCSC_PREFIX=PREFIX` and `cmake --build build/check-headers` |
| `check-tools PREFIX` | check that the tools installed in `PREFIX` run without `LD_LIBRARY_PATH` (also for librcsc installed by Autotools) | `cmake -DPREFIX=PREFIX -P test/CheckInstalledTools.cmake` |
| `clean` | remove the build directory | `rm -rf build` |
| `help` | show the usage | |

`install` without `--prefix` shows the installation directory saved in the build directory before installing.

Environment variables:

- `BUILD_DIR`: the build directory (default: `build`).
- `CMAKE_ARGS`: additional options for configure. Configure is run again when it is set. For example:
  ```
  CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Debug" ./cmake-helper.sh build
  ```
- `JOBS`: the number of parallel build jobs (default: the number of CPUs).

`clean` removes the build directory only if it contains `CMakeCache.txt`.

### Using cmake directly

```
cmake -S . -B build
cmake --build build --parallel
cmake --install build
```

The library is created as `build/rcsc/librcsc.so`, and the tools in `build/src`.

Configure options (`cmake -S . -B build -D<option>=<value>`):

| Option | Default | Description |
|---|---|---|
| `CMAKE_INSTALL_PREFIX` | `~/.local` | the installation directory |
| `CMAKE_BUILD_TYPE` | `Release` | the build type (`Release`, `Debug`, `RelWithDebInfo`, ...) |
| `BUILD_TESTING` | `ON` | build the tests (see [Running the tests](#running-the-tests)) |
| `CMAKE_PREFIX_PATH` | | the directories where the dependencies (e.g. Boost) are searched, if they are not in the standard locations |

The configure output shows the main settings:

```
-- Build settings:
--   BUILD_TYPE=Release
--   INSTALL_PREFIX=/home/user/.local
--   BUILD_TESTING=ON (unit tests: ON)
```

The flags given by `CMAKE_CXX_FLAGS` are used in addition to the warning options (`-W -Wall`).

### Installing and uninstalling

`cmake --install build` (or `./cmake-helper.sh install`) installs the following files into `PREFIX` (`~/.local` by default):

- `PREFIX/lib/librcsc.so` and `librcsc.so.20*`: the library
- `PREFIX/include/rcsc/`: the header files
- `PREFIX/bin/`: the tools (`rcg2csv`, `rcg2txt`, `rcgrenameteam`, `rcgresultprinter`, `rcgreverse`, `rcgvalidator`, `rcgverconv`, `rcgversion`,
  `rclmscheduler`, `rclmtableprinter`) and the scripts `librcsc-config` and `librcscenv`
  (see [Using librcsc in other programs](#using-librcsc-in-other-programs))
- `PREFIX/lib/pkgconfig/librcsc.pc`: the pkg-config file
- `PREFIX/lib/cmake/librcsc/`: the CMake package configuration files (CMake build only)

`lib` is `CMAKE_INSTALL_LIBDIR` of GNUInstallDirs (e.g. `lib/x86_64-linux-gnu` for the prefix `/usr` on Debian and Ubuntu).
The installed tools record the relative path to the library (RPATH `$ORIGIN/../lib`),
so `LD_LIBRARY_PATH` is not needed to run them, even if the prefix is changed by `--prefix` at the installation.

If `PREFIX` is a system directory (e.g. `/usr/local`), run `sudo ldconfig` after the first installation,
so that the other programs find librcsc without `LD_LIBRARY_PATH` (the tools of librcsc do not need it).

The default installation directory was `~/local` before. A build directory configured before keeps it in the cache.
To move to `~/.local`, uninstall the files with that build directory, and create the build directory again:

```
./cmake-helper.sh uninstall    # remove the files installed into ~/local
./cmake-helper.sh clean
./cmake-helper.sh install      # install into ~/.local
```

Alternatively, give the installation directory explicitly by `-DCMAKE_INSTALL_PREFIX=$HOME/.local`.
If librcsc was installed into `~/local` by Autotools, remove it by `make uninstall` in that source tree, or remove the files directly.
Note that the Autotools build of soccerwindow2 searches `~/local` before `~/.local`.
Also remove `~/local/lib` from `LD_LIBRARY_PATH` (e.g. in `~/.bashrc`) if it is set:
`LD_LIBRARY_PATH` is searched before the library path recorded in the programs (RUNPATH),
so an old `librcsc.so` left in `~/local/lib` would be used instead of the new one in `~/.local/lib`.

To uninstall, run the `uninstall` target (or `./cmake-helper.sh uninstall`):

```
cmake --build build --target uninstall
```

It removes the files listed in `install_manifest.txt`, which is created in the build directory by the install,
and the directories `PREFIX/include/rcsc` and `PREFIX/lib/cmake/librcsc` if they become empty.
The other directories (e.g. `PREFIX/bin` and `PREFIX/lib`) are not removed.
Run it with the same build directory that was used for the install.

## Running the tests

```
./cmake-helper.sh test                     # CMake
cd build && ctest --output-on-failure      # CMake (ctest --test-dir build ... with CMake 3.20 or later)
```

The tests of the CMake build are:

| Test | What it checks |
|---|---|
| `run_test_vector_2d`, `run_test_matrix_2d`, `run_test_segment_2d`, `run_test_triangle_2d`, `run_test_rect_2d`, `run_test_polygon_2d`, `run_test_voronoi_diagram`, `run_test_convex_hull`, `rundom_convex_hull` | the unit tests of `rcsc/geom` (CppUnit) |
| `test_clang_parser` | the unit test of `rcsc/clang` (CppUnit) |
| `run_test_rcg_types` | the unit test of the data types of `rcsc/rcg` (e.g. the output of `rcsc::rcg::PlayerTypeT`) (CppUnit) |
| `run_test_server_param` | the unit test of `rcsc::ServerParam` and `rcsc::rcg::ServerParamT` in `rcsc/common` (CppUnit) |
| `install` | installs librcsc into `test-install` in the build directory, for the following tests |
| `installed_headers` | each installed header file can be compiled alone (e.g. no header file that it includes is missing in the installation) |
| `find_package` | a CMake project can use the installed librcsc by `find_package(librcsc CONFIG)` and `librcsc::rcsc` |
| `installed_tools` | the installed tools run without `LD_LIBRARY_PATH` and use librcsc in the same installation (the recorded library path) |

- The unit tests need CppUnit, which is found by pkg-config.
  Without CppUnit, they are not built (the configure output shows `unit tests: OFF`), and the other tests are run.
- The unit tests are run with `LD_LIBRARY_PATH` set to the library directory in the build tree,
  so they test the built librcsc even if `LD_LIBRARY_PATH` contains an installed librcsc (e.g. `~/.local/lib`).
- The `install` test does not change `install_manifest.txt` of your installation, so the `uninstall` target still works for it.
- `-DBUILD_TESTING=OFF` disables all the tests.

To check librcsc installed anywhere (e.g. by Autotools), run:

```
./cmake-helper.sh check-headers ~/.local    # each header file can be compiled alone
./cmake-helper.sh check-tools ~/.local      # the tools run without LD_LIBRARY_PATH
```

The Autotools build runs the same unit tests by `make check`, if it is configured with `--enable-unit-test`
(see [Building with Autotools](#building-with-autotools)).

## Building with Autotools

```
./bootstrap
./configure --prefix=$HOME/.local
make
make install
```

- `./bootstrap` needs Autoconf, Automake, Libtool, and pkg-config.
- The default installation directory of `configure` is `/usr/local` (`sudo make install` is needed).
  Use `--prefix=$HOME/.local` to install librcsc into the same directory as the CMake build.
- The installed files are the same as the CMake build, except that the CMake package configuration files (`PREFIX/lib/cmake/librcsc/`)
  are not installed and the libtool library file (`PREFIX/lib/librcsc.la`) is installed.
  Therefore, a CMake project cannot find librcsc installed by Autotools with `find_package(librcsc CONFIG)`.
- The default compiler options are `-g -O2` (the CMake build uses `-O3 -DNDEBUG` in the `Release` build type).
- libtool records the absolute path of the library directory (`PREFIX/lib`) in the installed tools (RPATH),
  so `LD_LIBRARY_PATH` is not needed to run them (unlike the relative path of the CMake build, the tools do not work if the installation is moved).
  However, libtool does not record it for a system directory (e.g. `/usr/local/lib` and `/usr/lib`).
  After installing librcsc into such a directory (e.g. `sudo make install` with the default prefix), run `sudo ldconfig`.

Configure options:

| Option | Description |
|---|---|
| `--prefix=DIR` | the installation directory (default: `/usr/local`) |
| `--enable-unit-test` | build the unit tests for `make check` (CppUnit is needed) |
| `--enable-debug` | define `DEBUG` in the preprocessor |
| `--enable-example` | build the example programs in `example/` (they cannot be built at the moment) |

Other targets:

- `make check`: run the unit tests (with `--enable-unit-test`)
- `make uninstall`: uninstall
- `make doc`: generate the API documents (see [Generating the API documents](#generating-the-api-documents))
- `make dist` / `make distcheck`: create / check the source tarball (`librcsc-2026.tar.gz`).
  The tarball does not contain the files of the CMake build.

## Using librcsc in other programs

### CMake

The CMake build installs the CMake package configuration files, with which a CMake project can use librcsc:

```
find_package(librcsc CONFIG REQUIRED)
target_link_libraries(your_program PRIVATE librcsc::rcsc)
```

- The imported target `librcsc::rcsc` adds the include directory of librcsc and requires C++17.
- CMake searches `PREFIX` when `PREFIX/bin` is in `PATH` (e.g. `~/.local/bin` on Ubuntu).
  Otherwise, give the installation directory by `-DCMAKE_PREFIX_PATH=PREFIX` (e.g. `-DCMAKE_PREFIX_PATH=$HOME/.local`).
- A version can be requested, e.g. `find_package(librcsc 2024 CONFIG)`. A newer version of librcsc is accepted
  (the version is the year, e.g. 2026).
- The variables `librcsc_FOUND`, `librcsc_VERSION`, and `librcsc_DIR` are set.

### pkg-config

```
PKG_CONFIG_PATH=$HOME/.local/lib/pkgconfig pkg-config --cflags --libs librcsc
```

It prints `-I$HOME/.local/include -L$HOME/.local/lib -lrcsc -lz -lm`.

### librcsc-config

`librcsc-config` (installed in `PREFIX/bin`) prints the same options:

```
librcsc-config --cflags --libs
librcsc-config --version
```

### LD_LIBRARY_PATH

A program linked with librcsc has to find `librcsc.so` when it runs.
If the program does not record the library path (RPATH) and librcsc is not in a system directory, set `LD_LIBRARY_PATH`, e.g. in `~/.bashrc`:

```
export LD_LIBRARY_PATH=$HOME/.local/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
```

`librcscenv` runs a program with the library directory of librcsc added to `LD_LIBRARY_PATH`:

```
librcscenv ./your_program
```

The tools of librcsc and soccerwindow2 record the library path, so `LD_LIBRARY_PATH` is not needed for them.
Note that `LD_LIBRARY_PATH` is searched before the recorded library path (RUNPATH).
Do not leave the directory of another (e.g. old) librcsc in `LD_LIBRARY_PATH`.

## Generating the API documents

The API documents are generated by Doxygen (and Graphviz for the graphs, if `dot` is found).

```
doxygen build/Doxyfile        # CMake: generated in build/doc
make doc                      # Autotools: generated in doc
```

## Notes

- librcsc does not use the Boost.System library, which was removed in Boost 1.89.
  The CMake build finds Boost by the `BoostConfig.cmake` provided by Boost 1.70 or later.
- The version history and the major changes are summarized in [RELEASE_NOTES.md](../RELEASE_NOTES.md).
