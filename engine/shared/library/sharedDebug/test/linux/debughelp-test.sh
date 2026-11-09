#!/bin/sh
# usage: debughelp-test.sh BUILD_DIR [DebugHelp.cpp]
# Builds debughelp_test.cpp with BUILD_DIR's compiler flags and libraries (an
# existing 32- or 64-bit CMake build of this tree), compiling in DebugHelp.cpp
# (default: this tree's), and runs it with a shared object to look into and an
# ELF file of the other class. The executable's own lookup is also compared
# with addr2line. (The libraries are not built as PIC, so no PIE variant.)
BUILD=$(cd "$1" && pwd) || { echo "usage: $0 BUILD_DIR [DebugHelp.cpp]" >&2; exit 2; }
HERE=$(cd "$(dirname "$0")" && pwd)
SOURCE=$(cd "$(dirname "${2:-$HERE/../../src/linux/DebugHelp.cpp}")" && pwd)/$(basename "${2:-DebugHelp.cpp}")
FLAGS=$BUILD/engine/shared/library/sharedDebug/src/CMakeFiles/sharedDebug.dir/flags.make
[ -f "$FLAGS" ] || { echo "no sharedDebug build in $BUILD" >&2; exit 2; }
CXXFLAGS="$(sed -n 's/^CXX_FLAGS = //p' "$FLAGS") $(sed -n 's/^CXX_DEFINES = //p' "$FLAGS") $(sed -n 's/^CXX_INCLUDES = //p' "$FLAGS") -g -O0"
case " $(echo $CXXFLAGS) " in *" -m32 "*) ABI=-m32; OTHER=-m64 ;; *) ABI=; OTHER=-m32 ;; esac
grep -q 'class MappedElfFile' "$SOURCE" && CXXFLAGS="$CXXFLAGS -DDEBUGHELP_HAS_ELF_CLASS_CHECK"
LIBS=$(find "$BUILD/engine/shared/library" "$BUILD/external/ours/library" -name '*.a' | sort | tr '\n' ' ')
TMP=$(mktemp -d) || exit 2
trap 'rm -rf "$TMP"' EXIT
g++ $ABI -g -O0 -fPIC -shared "$HERE/known_so.cpp" -o "$TMP/libknown.so" || exit 2
SO_FIRST=$(grep -n 'knownSoFunction' "$HERE/known_so.cpp" | cut -d: -f1); SO_LAST=$((SO_FIRST + 4))
echo 'int main(){return 0;}' > "$TMP/other.c"
gcc $OTHER "$TMP/other.c" -o "$TMP/other-class" 2>/dev/null || echo "note: no $OTHER compiler; ELF class check skipped"
fail=0
g++ $CXXFLAGS -no-pie -DDEBUGHELP_SOURCE="\"$SOURCE\"" "$HERE/debughelp_test.cpp" -o "$TMP/debughelp_test" -Wl,--start-group $LIBS -Wl,--end-group -lpthread -ldl -lz || exit 2
"$TMP/debughelp_test" "$TMP/libknown.so" "$SO_FIRST" "$SO_LAST" $( [ -x "$TMP/other-class" ] && echo "$TMP/other-class") > "$TMP/out" 2>&1 || fail=1
grep -v '^ADDR2LINE' "$TMP/out"
# addr2line cross-check for the executable's own function
set -- $(grep '^ADDR2LINE lookupAddress(knownFunction' "$TMP/out" | awk '{print $(NF-1), $NF}')
if [ -n "$1" ]; then
	want=$(addr2line -e "$TMP/debughelp_test" "$1" | sed 's/ (discriminator.*//')
	got=$2
	if [ "$(basename "${want%%:*}"):${want##*:}" = "$(basename "${got%%:*}"):${got##*:}" ]; then echo "PASS  addr2line agrees: $want"; else echo "FAIL  addr2line says $want, DebugHelp says $got"; fail=1; fi
fi
[ $fail -eq 0 ] && echo "ALL PASS" || echo "FAILED"
exit $fail
