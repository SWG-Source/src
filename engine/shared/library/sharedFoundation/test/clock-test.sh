#!/bin/sh
# usage: clock-test.sh SOURCE_DIR BUILD_DIR
# Builds clock_test.cpp against SOURCE_DIR's headers and BUILD_DIR's
# libraries (an existing 32- or 64-bit CMake build of that tree) and runs it.
[ "$#" -ge 2 ] || { echo "usage: $0 SOURCE_DIR BUILD_DIR" >&2; exit 2; }
SRC=$(cd "$1" && pwd) && BUILD=$(cd "$2" && pwd) || { echo "usage: $0 SOURCE_DIR BUILD_DIR" >&2; exit 2; }
HERE=$(cd "$(dirname "$0")" && pwd)
FLAGS=$BUILD/engine/shared/library/sharedFoundation/src/CMakeFiles/sharedFoundation.dir/flags.make
[ -f "$FLAGS" ] || { echo "no sharedFoundation build in $BUILD" >&2; exit 2; }
CXXFLAGS="$(sed -n 's/^CXX_FLAGS = //p' "$FLAGS") $(sed -n 's/^CXX_DEFINES = //p' "$FLAGS") $(sed -n 's/^CXX_INCLUDES = //p' "$FLAGS")"
grep -q 'durationMs' "$SRC/engine/shared/library/sharedFoundation/src/shared/Clock.h" && CXXFLAGS="$CXXFLAGS -DCLOCK_HAS_DURATION_MS"
LIBS=$(find "$BUILD/engine/shared/library" "$BUILD/external/ours/library" -name '*.a' | sort | tr '\n' ' ')
TMP=$(mktemp -d) || exit 2
trap 'rm -rf "$TMP"' EXIT
"${CXX:-g++}" $CXXFLAGS "$HERE/clock_test.cpp" -o "$TMP/clock_test" -Wl,--start-group $LIBS -Wl,--end-group -lpthread -ldl -lz || exit 2
"$TMP/clock_test"
