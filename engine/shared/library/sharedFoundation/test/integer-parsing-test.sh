#!/bin/sh
# usage: integer-parsing-test.sh BUILD_DIR [SOURCE.cpp ...]
# Builds integer_parsing_test.cpp with BUILD_DIR's compiler flags and
# libraries (an existing 32- or 64-bit CMake build of this tree), runs it and
# compares the output with integer_parsing_expected.txt; the expected output
# is the same on both ABIs. Any SOURCE.cpp given is compiled into the test in
# place of the library's copy, e.g. an older DynamicVariable.cpp.
[ "$#" -ge 1 ] || { echo "usage: $0 BUILD_DIR [SOURCE.cpp ...]" >&2; exit 2; }
BUILD=$(cd "$1" && pwd) || { echo "usage: $0 BUILD_DIR [SOURCE.cpp ...]" >&2; exit 2; }
shift
HERE=$(cd "$(dirname "$0")" && pwd)
CXXFLAGS=
for target in sharedFoundation/src/CMakeFiles/sharedFoundation.dir sharedCommandParser/src/CMakeFiles/sharedCommandParser.dir; do
	FLAGS=$BUILD/engine/shared/library/$target/flags.make
	[ -f "$FLAGS" ] || { echo "no $target in $BUILD" >&2; exit 2; }
	CXXFLAGS="$CXXFLAGS $(sed -n 's/^CXX_INCLUDES = //p' "$FLAGS")"
done
CXXFLAGS="$(sed -n 's/^CXX_FLAGS = //p' "$FLAGS") $(sed -n 's/^CXX_DEFINES = //p' "$FLAGS") $CXXFLAGS"
LIBS=$(find "$BUILD/engine/shared/library" "$BUILD/external/ours/library" -name '*.a' | sort | tr '\n' ' ')
TMP=$(mktemp -d) || exit 2
trap 'rm -rf "$TMP"' EXIT
"${CXX:-g++}" $CXXFLAGS "$HERE/integer_parsing_test.cpp" "$@" -o "$TMP/integer_parsing_test" -Wl,--start-group $LIBS -Wl,--end-group -lpthread -ldl -lz || exit 2
"$TMP/integer_parsing_test" > "$TMP/out" 2>&1 || { cat "$TMP/out"; exit 1; }
if diff -u "$HERE/integer_parsing_expected.txt" "$TMP/out"; then echo "ALL PASS ($(wc -l < "$TMP/out") lines match)"; else echo "FAILED: output differs from integer_parsing_expected.txt"; exit 1; fi
