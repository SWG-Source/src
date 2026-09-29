#!/bin/sh
# usage: run-tests.sh MIFF
# Compiles every cases/*.mif and compares the result with expected.txt:
# the SHA-256 of the .iff the original 32-bit Miff wrote, or REJECT for input
# the original compiled into an ABI-dependent or undefined value.
# Run it with a 32-bit and a 64-bit Miff; both must pass.
[ -x "$1" ] || { echo "usage: $0 MIFF" >&2; exit 2; }
MIFF=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
HERE=$(cd "$(dirname "$0")" && pwd)
TMP=$(mktemp -d) || exit 2
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
fail=0; n=0
while read -r name want rest; do
	case $name in ''|'#'*) continue ;; esac
	n=$((n + 1))
	got=$(sh "$HERE/miff-compile.sh" "$MIFF" "$HERE/cases/$name.mif" "$TMP/$name.iff") || got=TEST_ERROR
	if [ "$got" = "$want" ]; then echo "PASS $name"; else echo "FAIL $name: expected $want, got $got"; fail=$((fail + 1)); fi
done < "$HERE/expected.txt"
rm -rf "$TMP"
echo "$n cases, $fail failed"
[ "$n" -gt 0 ] && [ "$fail" -eq 0 ]
