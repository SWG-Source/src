#!/bin/sh
# usage: calendar-time-test.sh [-m32|-m64] [ScriptMethodsScript.cpp]
# Extracts getCalendarTimeSeconds2 from ScriptMethodsScript.cpp (default: this
# tree's), compiles it into calendar_time_test.cpp and runs it. Run it with
# -m32 and -m64; both must print ALL PASS.
ABI=
case $1 in -m32|-m64) ABI=$1; shift ;; esac
HERE=$(cd "$(dirname "$0")" && pwd)
SRC=${1:-$HERE/../src/shared/ScriptMethodsScript.cpp}
TMP=$(mktemp -d) || exit 2
trap 'rm -rf "$TMP"' EXIT
sed -n '/^jint JNICALL ScriptMethodsScriptNamespace::getCalendarTimeSeconds2(/,/^}/p' "$SRC" | tr -d '\r' > "$TMP/function.inc"
[ -s "$TMP/function.inc" ] || { echo "getCalendarTimeSeconds2 not found in $SRC" >&2; exit 2; }
g++ $ABI -std=c++11 -O0 -DCALENDAR_FUNCTION_SOURCE="\"$TMP/function.inc\"" "$HERE/calendar_time_test.cpp" -o "$TMP/calendar_time_test" || exit 2
"$TMP/calendar_time_test"
