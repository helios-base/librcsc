#!/bin/sh
#
# cmake-helper.sh -- shortcuts for the frequently used CMake commands of librcsc
#
# Run "./cmake-helper.sh help" for the usage.
# The commands are printed before they are run, so that you can learn the
# actual cmake commands.

set -e

usage()
{
    cat <<'EOF'
Usage: ./cmake-helper.sh COMMAND [OPTIONS]

Commands:
  configure                     (re)configure the build directory
  build                         configure (if needed) and build
  test                          build and run the tests
  install [--prefix DIR]        build and install (default prefix: $HOME/.local)
  uninstall                     remove the files installed from the build directory
  check-headers PREFIX          check that each header file of librcsc installed
                                in PREFIX can be compiled alone (also for librcsc
                                installed by Autotools)
  clean                         remove the build directory
  help                          show this help

Environment variables:
  BUILD_DIR   the build directory (default: build)
  CMAKE_ARGS  additional options for configure, e.g.
              CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Debug" ./cmake-helper.sh build
              (configure is run again when CMAKE_ARGS is set)
  JOBS        the number of parallel build jobs (default: the number of CPUs)

The main commands that are run:
  configure      cmake -S . -B build
  build          cmake --build build --parallel JOBS
  test           cd build && ctest --output-on-failure
  install        cmake --install build [--prefix DIR]
  uninstall      cmake --build build --target uninstall
  check-headers  cmake -S test/installed_headers -B build/check-headers -DLIBRCSC_PREFIX=PREFIX
                 cmake --build build/check-headers --parallel JOBS
  clean          rm -rf build

The unit tests need CppUnit (e.g. libcppunit-dev on Ubuntu). Without it, they
are not built, and only the tests of the installation are run.
EOF
}

die()
{
    echo "cmake-helper.sh: $*" >&2
    exit 1
}

# print a command and run it
run_cmd()
{
    printf '+'
    for arg in "$@"; do
        case "$arg" in
            *[[:space:]]*|'') printf " '%s'" "$arg" ;;
            *) printf ' %s' "$arg" ;;
        esac
    done
    printf '\n'
    "$@"
}

# move to the project root (the directory of this script)
cd "$(dirname "$0")"

command="${1:-help}"
[ $# -gt 0 ] && shift

case "$command" in
    help|-h|--help)
        usage
        exit 0
        ;;
    configure|build|test|install|uninstall|check-headers|clean)
        ;;
    *)
        usage >&2
        die "unknown command: $command"
        ;;
esac

build_dir="${BUILD_DIR:-build}"

if [ -z "${JOBS:-}" ]; then
    JOBS=$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)
fi

# configure if the build directory is not configured, or if forced or CMAKE_ARGS is set
configure()
{
    if [ "${1:-}" = force ] || [ -n "${CMAKE_ARGS:-}" ] || [ ! -f "$build_dir/CMakeCache.txt" ]; then
        # CMAKE_ARGS is split into words intentionally.
        # shellcheck disable=SC2086
        run_cmd cmake -S . -B "$build_dir" ${CMAKE_ARGS:-}
    fi
}

build()
{
    configure
    run_cmd cmake --build "$build_dir" --parallel "$JOBS"
}

# the installation directory saved in the cache of the build directory
cached_prefix()
{
    sed -n 's/^CMAKE_INSTALL_PREFIX:[A-Z]*=//p' "$build_dir/CMakeCache.txt"
}

case "$command" in
    configure)
        configure force
        ;;
    build)
        build
        ;;
    test)
        build
        echo "+ cd $build_dir && ctest --output-on-failure"
        (cd "$build_dir" && ctest --output-on-failure)
        ;;
    install)
        prefix=
        while [ $# -gt 0 ]; do
            case "$1" in
                --prefix) [ $# -ge 2 ] || die "--prefix requires a directory"; prefix="$2"; shift 2 ;;
                --prefix=*) prefix="${1#--prefix=}"; shift ;;
                *) die "unknown option for install: $1" ;;
            esac
        done
        build
        if [ -n "$prefix" ]; then
            run_cmd cmake --install "$build_dir" --prefix "$prefix"
        else
            echo "Installing into $(cached_prefix) (CMAKE_INSTALL_PREFIX of $build_dir)"
            run_cmd cmake --install "$build_dir"
        fi
        ;;
    uninstall)
        # The files listed in install_manifest.txt of the build directory are removed.
        [ -f "$build_dir/install_manifest.txt" ] \
            || die "$build_dir/install_manifest.txt is not found. Nothing has been installed from $build_dir."
        run_cmd cmake --build "$build_dir" --target uninstall
        ;;
    check-headers)
        [ $# -ge 1 ] || die "check-headers requires the installation directory (PREFIX)"
        prefix="$1"
        [ -d "$prefix/include/rcsc" ] || die "$prefix/include/rcsc is not found"
        check_dir="$build_dir/check-headers"
        run_cmd cmake -S test/installed_headers -B "$check_dir" -DLIBRCSC_PREFIX="$prefix"
        run_cmd cmake --build "$check_dir" --parallel "$JOBS"
        echo "All the header files in $prefix/include/rcsc can be compiled alone."
        ;;
    clean)
        if [ ! -e "$build_dir" ]; then
            echo "$build_dir does not exist."
        elif [ ! -f "$build_dir/CMakeCache.txt" ]; then
            die "$build_dir is not a CMake build directory (no CMakeCache.txt). It is not removed."
        else
            run_cmd rm -rf -- "$build_dir"
        fi
        ;;
esac
