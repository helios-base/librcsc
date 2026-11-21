# Release Notes

librcsc does not publish versioned releases.
This file summarizes new features and major changes as they are merged.

## Unreleased

- Added this release notes file to track new features and major changes.
- Boost:
  - Boost is now used as header-only; the Boost.System library is no longer required.
    librcsc can now be built with Boost 1.89 or later, in which the Boost.System library was removed.
  - The minimum required Boost version is now 1.70.
  - The CMake build finds Boost with the `BoostConfig.cmake` provided by Boost (`find_package(Boost 1.70 CONFIG)`),
    instead of the FindBoost module of CMake.
  - The installed header files no longer include the Boost headers, so the programs using librcsc do not need Boost unless they use it themselves.
    A program that used the headers included through the librcsc headers (e.g. `boost/scoped_ptr.hpp`, `boost/array.hpp`,
    or the standard headers included by them) must include them directly.
  - `librcsc.spec.in`: the devel package no longer requires `boost-devel`.
