
# Librcsc

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)

librcsc is a basic library to develop a simulated soccer team and related tools for the RoboCup Soccer Simulation.
All programs can work with rcssserver-18.

- The RoboCup Soccer Simulator: https://rcsoccersim.github.io/
- RoboCup Official Homepage: https://www.robocup.org/

## Quick Start

The latest librcsc depends on the following libraries:
 - C++17
 - Boost 1.41 or later https://www.boost.org/
 - (optional) Doxygen
 - (optional) Graphviz

In the case of Ubuntu 20.04 or later, execute the following commands for installing a basic development environment:
```
sudo apt update
sudo apt install build-essential libboost-all-dev autoconf automake libtool
```

To build the library, execute commands from the root of source directory:
```
./bootstrap
./configure --disable-unit-test
make
```

Once successfully built, you can install the library file and header files to the default installation directory (``/usr/local``):
```
sudo make install
```
It is recommended to change the installation directory to the under of your home directory.
See the next section in detail.

You can generate documetation files using Doxygen and Graphviz.
```
sudo apt install doxygen graphviz
make doc
```


## Configuring

If you do not have an administration privilege, you may need to install the library under your home directory and configure some environment variables.
You can change the installation directory by passing ``--prefix`` option to the configure script:
```
./configure --prefix=/path/to/installation
```
For example, if you'd like to install under the ``~/local``, type the following:
```
./configure --prefix=$HOME/local
```

See `./configure --help` for other options.


### Configuring Environment Variables

``~/local`` is supported by the configure script of helios-base, soccerwindow2, fedit2 by default.
However, if you change the installation directory, you need to modify the followng two environment variables:

- ``LD_LIBRARY_PATH`` (in ``~/.bashrc``)
- ``PATH`` (in ``~/.profile``)

The following instructions assume that librcsc was installed under ``~/local``.

Add the following lines at the end of ``~/.bashrc``:
```
LD_LIBRARY_PATH=$HOME/local/lib:$LD_LIBRARY_PATH
export EDITOR RCSSMONITOR LD_LIBRARY_PATH
```
This setting is enabled by opening a new terminal or executing ``source ~/.bashrc``.

You can find the existing ``PATH`` variable at the end of ``~/.profile``.
Add the following at the end of the file, then log out and log in again.
```
PATH="$HOME/local/bin:$PATH"
```

## Uninstall

librcsc can also be easily removed by entering the distribution directory and running:
```
make uninstall
```
This will remove all the files that where installed, but not any directories that were created during the installation process.


## References

- Hidehisa Akiyama, Tomoharu Nakashima, HELIOS Base: An Open Source Package for the RoboCup Soccer 2D Simulation, In Sven Behnke, Manuela Veloso, Arnoud Visser, and Rong Xiong editors, RoboCup2013: Robot World XVII, Lecture Notes in Artificial Intelligence, Springer Verlag, Berlin, 2014. http://dx.doi.org/10.1007/978-3-662-44468-9_46
- Hidehisa Akiyama, Itsuki Noda, Multi-Agent Positioning Mechanism in the Dynamic Environment, In Ubbo Visser, Fernando Ribeiro, Takeshi Ohashi, and Frank Dellaert, editors, RoboCup 2007: Robot Soccer World Cup XI Lecture Notes in Artificial Intelligence, vol. 5001, Springer, pp.377-384, July 2008. https://doi.org/10.1007/978-3-540-68847-1_38
