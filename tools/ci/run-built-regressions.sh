#!/usr/bin/env bash
# Requires a fresh completed same-revision 32-bit server build.
set -euo pipefail
build=$(realpath "${1:?usage: run-built-regressions.sh BUILD_DIR}")
root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root"
# The legacy DebugHelp runner otherwise permits skipping the opposite-class ELF.
scratch=$(mktemp -d)
trap 'rm -rf "$scratch"' EXIT
printf 'int main(void) { return 0; }\n' | gcc -m64 -x c - -o "$scratch/other-class"
command -v addr2line >/dev/null
sh engine/shared/library/sharedFoundation/test/clock-test.sh "$root" "$build"
sh engine/shared/library/sharedFile/test/linux/osfile-test.sh "$build"
sh engine/shared/library/sharedDebug/test/linux/debughelp-test.sh "$build"
sh engine/client/application/Miff/test/run-tests.sh "$build/bin/Miff"
