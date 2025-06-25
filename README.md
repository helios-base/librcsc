# Librcsc

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)

**librcsc** is a comprehensive C++ library for developing simulated soccer teams and related tools for the RoboCup Soccer Simulation 2D. This library provides essential components for creating intelligent soccer agents that can compete in the RoboCup Soccer Simulation environment.

## About This Fork

This repository is a fork of the original librcsc library, specifically maintained for the **Espadana Simulation Team** which participated in the RoboCup Soccer Simulation 2D competition at RoboCup 2005 Osaka, Japan. The team utilized this library as the foundation for their soccer simulation agents, demonstrating the library's effectiveness in competitive RoboCup environments.

## Features

- **Complete Soccer Agent Framework**: Comprehensive classes for player behavior, decision making, and action execution
- **Formation Management**: Flexible formation systems with multiple parser formats (CSV, JSON, Static)
- **Geometric Utilities**: Advanced 2D geometry library for soccer field calculations
- **Communication Protocols**: Support for coach and trainer communication
- **Replay Analysis**: Tools for analyzing game logs and generating statistics
- **Cross-platform**: Works on Linux, macOS, and Windows

## Quick Start

### Prerequisites

The latest librcsc depends on the following libraries:
- **C++17** compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- **Boost 1.41 or later** - https://www.boost.org/
- **Autotools** (autoconf, automake, libtool)
- *(Optional)* **Doxygen** - for documentation generation
- *(Optional)* **Graphviz** - for documentation diagrams

### Installation on Ubuntu/Debian

For Ubuntu 20.04 or later, install the basic development environment:

```bash
sudo apt update
sudo apt install build-essential libboost-all-dev autoconf automake libtool
```

### Building from Source

1. **Clone and bootstrap**:
```bash
git clone <repository-url>
cd librcsc
./bootstrap
```

2. **Configure and build**:
```bash
./configure --disable-unit-test
make
```

3. **Install** (optional):
```bash
sudo make install
```

### Documentation

Generate comprehensive documentation using Doxygen:

```bash
sudo apt install doxygen graphviz
make doc
```

## Configuration

### Custom Installation Directory

If you don't have administrative privileges, install under your home directory:

```bash
./configure --prefix=$HOME/local
make
make install
```

### Environment Variables

For custom installations, add these to your shell configuration:

**For `~/.bashrc`**:
```bash
export LD_LIBRARY_PATH=$HOME/local/lib:$LD_LIBRARY_PATH
export EDITOR RCSSMONITOR LD_LIBRARY_PATH
```

**For `~/.profile`**:
```bash
export PATH="$HOME/local/bin:$PATH"
```

## Project Structure

```
librcsc/
├── rcsc/                    # Core library components
│   ├── action/             # Player action implementations
│   ├── coach/              # Coach agent functionality
│   ├── formation/          # Formation management
│   ├── geom/               # Geometric utilities
│   ├── player/             # Player object models
│   └── ...
├── src/                    # Command-line tools
├── example/                # Usage examples
└── test/                   # Test files
```

## Usage Examples

### Basic Player Agent

```cpp
#include <rcsc/common/soccer_agent.h>
#include <rcsc/action/basic_actions.h>

class MyPlayer : public rcsc::SoccerAgent {
public:
    MyPlayer() : SoccerAgent() {}
    
    bool initImpl(rcsc::CmdLineParser& cmd_parser) override {
        // Initialize your agent
        return true;
    }
    
    void actionImpl() override {
        // Implement your player logic here
        if (world().self().isKickable()) {
            doKick();
        } else {
            doMove();
        }
    }
};
```

## Compatibility

This library is compatible with:
- **rcssserver-18** and later versions
- **RoboCup Soccer Simulation 2D** environment
- **C++17** standard and later

## Uninstallation

To remove librcsc from your system:

```bash
make uninstall
```

This removes all installed files but preserves created directories.

## Contributing

We welcome contributions! Please ensure your code follows the existing style and includes appropriate tests.

## License

This project is licensed under the LGPL v3 License - see the [LICENSE](COPYING.LESSER) file for details.

## References

- **HELIOS Base**: Hidehisa Akiyama, Tomoharu Nakashima, "HELIOS Base: An Open Source Package for the RoboCup Soccer 2D Simulation", In Sven Behnke, Manuela Veloso, Arnoud Visser, and Rong Xiong editors, RoboCup2013: Robot World XVII, Lecture Notes in Artificial Intelligence, Springer Verlag, Berlin, 2014. http://dx.doi.org/10.1007/978-3-662-44468-9_46

- **Multi-Agent Positioning**: Hidehisa Akiyama, Itsuki Noda, "Multi-Agent Positioning Mechanism in the Dynamic Environment", In Ubbo Visser, Fernando Ribeiro, Takeshi Ohashi, and Frank Dellaert, editors, RoboCup 2007: Robot Soccer World Cup XI Lecture Notes in Artificial Intelligence, vol. 5001, Springer, pp.377-384, July 2008. https://doi.org/10.1007/978-3-540-68847-1_38

## Related Links

- [The RoboCup Soccer Simulator](https://rcsoccersim.github.io/)
- [RoboCup Official Homepage](https://www.robocup.org/)
- [Boost C++ Libraries](https://www.boost.org/)
