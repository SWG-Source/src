#!/bin/sh
# usage: arith-diff.sh
# Generates the parser from ../src/linux/parser.yac and checks its integer
# helpers against a real 32-bit C long: 3,000,000 random and edge-case operations total, cycling over 12
# operators, and sampled 8/16/32-bit writes (stride 997 across int32).
# Needs bison and a gcc that can build -m32.
HERE=$(cd "$(dirname "$0")" && pwd)
TMP=$(mktemp -d) || exit 2
trap 'rm -rf "$TMP"' EXIT
bison -d -o "$TMP/parser.c" "$HERE/../src/linux/parser.yac" || exit 2
gcc -m32 -O1 -w -I"$TMP" "$HERE/arith-diff.c" "$TMP/parser.c" -lm -o "$TMP/arith-diff" || exit 2
"$TMP/arith-diff"
