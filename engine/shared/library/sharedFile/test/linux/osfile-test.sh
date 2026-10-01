#!/bin/sh
# usage: osfile-test.sh BUILD_DIR
# Builds osfile_test.cpp with BUILD_DIR's compiler flags and libraries (an
# existing 32- or 64-bit CMake build of this tree) and runs it. It needs
# about 8 GiB of sparse-file support in /tmp, not real disk space.
BUILD=$(cd "$1" && pwd) || { echo "usage: $0 BUILD_DIR" >&2; exit 2; }
HERE=$(cd "$(dirname "$0")" && pwd)
FLAGS=$BUILD/engine/shared/library/sharedFile/src/CMakeFiles/sharedFile.dir/flags.make
[ -f "$FLAGS" ] || { echo "no sharedFile build in $BUILD" >&2; exit 2; }
CXXFLAGS="$(sed -n 's/^CXX_FLAGS = //p' "$FLAGS") $(sed -n 's/^CXX_DEFINES = //p' "$FLAGS") $(sed -n 's/^CXX_INCLUDES = //p' "$FLAGS")"
LIBS=$(find "$BUILD/engine/shared/library" "$BUILD/external/ours/library" -name '*.a' | sort | tr '\n' ' ')
TMP=$(mktemp -d) || exit 2
trap 'rm -rf "$TMP"' EXIT
g++ $CXXFLAGS -D_LARGEFILE64_SOURCE "$HERE/osfile_test.cpp" -o "$TMP/osfile_test" -Wl,--start-group $LIBS -Wl,--end-group -lpthread -ldl -lz || exit 2
"$TMP/osfile_test"
