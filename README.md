# Librcsc

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)

librcsc is a basic library to develop a simulated soccer team and related tools for the RoboCup Soccer Simulation.
All programs can work with rcssserver-18.

- The RoboCup Soccer Simulator: https://rcsoccersim.github.io/
- RoboCup Official Homepage: https://www.robocup.org/

## Requirements

- A C++17 compiler (GCC or Clang)
- CMake 3.16 or later
- Boost 1.70 or later (only the headers are used) https://www.boost.org/
- zlib (optional, needed to read and write the gzipped files)
- CppUnit (optional, needed only for the unit tests)

## Quick Start

The following commands are for Ubuntu 24.04 and 26.04.
See [docs/build.md](docs/build.md) for the details (the other options, Autotools, tests, documents, etc.).

Install the packages:

```
sudo apt update
sudo apt install build-essential cmake pkg-config libboost-dev zlib1g-dev libcppunit-dev
```

Build and install librcsc with `cmake-helper.sh`:

```
./cmake-helper.sh build
./cmake-helper.sh install
```

`cmake-helper.sh` prints the cmake commands that it runs. The commands above are equivalent to:

```
cmake -S . -B build
cmake --build build --parallel
cmake --install build
```

librcsc is installed into `~/.local` by default: the library in `~/.local/lib`, the header files in `~/.local/include/rcsc`,
and the tools (e.g. `rcg2txt`) in `~/.local/bin`.
soccerwindow2 and helios-base find librcsc in `~/.local` without any setting.

To run the tests, run `./cmake-helper.sh test`.
To uninstall librcsc, run `./cmake-helper.sh uninstall`.
Run `./cmake-helper.sh help` for the other commands.

The default installation directory was `~/local` before.
If you have a build directory configured before, it keeps `~/local`.
Run `./cmake-helper.sh uninstall` (to remove the files in `~/local`) and `./cmake-helper.sh clean` first
(see [docs/build.md](docs/build.md#installing-and-uninstalling)).

## Using librcsc

On Ubuntu, `~/.local/bin` is added to `PATH` at the login if it exists, so log in again after the first installation to use the tools.
The tools record the location of librcsc, so `LD_LIBRARY_PATH` is not needed to run them.

A CMake project can use librcsc as follows:

```
find_package(librcsc CONFIG REQUIRED)
target_link_libraries(your_program PRIVATE librcsc::rcsc)
```

See [docs/build.md](docs/build.md#using-librcsc-in-other-programs) for pkg-config, `librcsc-config`, and `LD_LIBRARY_PATH`.

## References

- Hidehisa Akiyama, Tomoharu Nakashima, HELIOS Base: An Open Source Package for the RoboCup Soccer 2D Simulation, In Sven Behnke, Manuela Veloso, Arnoud Visser, and Rong Xiong editors, RoboCup2013: Robot World XVII, Lecture Notes in Artificial Intelligence, Springer Verlag, Berlin, 2014. http://dx.doi.org/10.1007/978-3-662-44468-9_46
- Hidehisa Akiyama, Itsuki Noda, Multi-Agent Positioning Mechanism in the Dynamic Environment, In Ubbo Visser, Fernando Ribeiro, Takeshi Ohashi, and Frank Dellaert, editors, RoboCup 2007: Robot Soccer World Cup XI Lecture Notes in Artificial Intelligence, vol. 5001, Springer, pp.377-384, July 2008. https://doi.org/10.1007/978-3-540-68847-1_38
