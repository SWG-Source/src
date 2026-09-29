#!/bin/sh
# usage: miff-compile.sh MIFF INPUT.mif OUTPUT.iff
# Miff returns ERR_PARSER (-7, shell status249) for a parse error. A signal,
# timeout, missing input/tool, or contradictory exit/output is a test error.
[ "$#" -eq 3 ] && [ -x "$1" ] && [ -f "$2" ] || { echo "invalid compiler or input" >&2; exit 2; }
MIFF=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
IN=$(cd "$(dirname "$2")" && pwd)/$(basename "$2")
OUT=$3
case $OUT in /*) ;; *) OUT=$(pwd)/$OUT ;; esac
rm -f "$OUT" || exit 2
WD=$(mktemp -d) || exit 2
trap 'rm -rf "$WD"' EXIT HUP INT TERM
(cd "$WD" && timeout 30 "$MIFF" -i "\"$IN\"" -o "\"$OUT\"" > "$WD/log" 2>&1 < /dev/null)
rc=$?
if [ "$rc" -eq 0 ] && [ -s "$OUT" ]; then
    sha256sum < "$OUT" | cut -c1-64
elif [ "$rc" -eq 249 ] && [ ! -s "$OUT" ]; then
    echo REJECT
else
    echo "compiler test error: status=$rc input=$IN" >&2
    cat "$WD/log" >&2
    exit 2
fi
