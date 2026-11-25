#!/bin/sh
#
# compare_rcg_output.sh: compare what two builds of librcsc read from game logs.
#
# Usage: test/compare_rcg_output.sh <OldBinDir> <NewBinDir> <RcgFile>[.gz]...
#
# rcgverconv of each build converts each game log into two other formats,
# and the outputs are compared byte by byte:
#   text v4, v5 -> JSON and v6
#   text v6     -> JSON and v5
#   JSON        -> v6 and v5
# rcgverconv writes everything that the parser passes to the handler, so the
# same outputs mean that both builds read the same data from the game log.
#
# <OldBinDir> and <NewBinDir> are the directories that have rcgverconv, e.g. the
# bin directory of an installation, or the src directory of a CMake build tree.
# LD_LIBRARY_PATH is unset, so that each rcgverconv uses librcsc of its own
# build or installation (the recorded RPATH or RUNPATH).
#
# This script is not run by CTest, because it needs real game logs.
# See docs/plan/2026-11-24-02_rcg-parser-speedup.md 4.1.
#
# Exit status: 0 if all the outputs are the same, 1 if not, 2 for a usage error.

set -u

usage()
{
    echo "Usage: $0 <OldBinDir> <NewBinDir> <RcgFile>[.gz]..." >&2
}

if [ $# -lt 3 ]; then
    usage
    exit 2
fi

old_bin=$1
new_bin=$2
shift 2

for dir in "$old_bin" "$new_bin"; do
    if [ ! -x "$dir/rcgverconv" ]; then
        echo "rcgverconv is not found in $dir" >&2
        exit 2
    fi
done

work=$(mktemp -d "${TMPDIR:-/tmp}/compare_rcg_output.XXXXXX") || exit 2
status=0

for log in "$@"; do
    if [ ! -f "$log" ]; then
        echo "$log: not found" >&2
        status=1
        continue
    fi

    header=$(gzip -cdf "$log" 2>/dev/null | head -c 4)
    case "$header" in
        '['*) targets="6 5" ;;
        ULG6) targets="json 5" ;;
        ULG4|ULG5) targets="json 6" ;;
        *)
            echo "$log: unsupported header [$header]" >&2
            status=1
            continue
            ;;
    esac

    name=$(basename "$log")
    for target in $targets; do
        old_out="$work/$name.old.$target"
        new_out="$work/$name.new.$target"
        env -u LD_LIBRARY_PATH "$old_bin/rcgverconv" -v "$target" -o "$old_out" "$log" > "$old_out.log" 2>&1
        env -u LD_LIBRARY_PATH "$new_bin/rcgverconv" -v "$target" -o "$new_out" "$log" > "$new_out.log" 2>&1

        if [ ! -s "$old_out" ] || [ ! -s "$new_out" ]; then
            echo "EMPTY:     $name -> $target (see $old_out.log and $new_out.log)"
            status=1
        elif cmp -s "$old_out" "$new_out"; then
            echo "same:      $name -> $target ($(wc -c < "$new_out" | tr -d ' ') bytes)"
        else
            echo "DIFFERENT: $name -> $target"
            status=1
        fi
    done
done

if [ $status -eq 0 ]; then
    rm -rf "$work"
else
    echo "The outputs are kept in $work"
fi

exit $status
