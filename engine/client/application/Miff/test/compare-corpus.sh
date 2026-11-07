#!/bin/sh
# usage: compare-corpus.sh REFERENCE_MIFF CANDIDATE_MIFF DIR...
# Compiles every .mif under DIR... with both tools and byte-compares the
# .iff files. Use the original 32-bit Miff as the reference and a Miff built
# from this tree (32- and 64-bit) as the candidate, over the shipped dsrc.
# Exit status 1 if any file differs or only one tool rejects it.
[ $# -ge 3 ] || { echo "usage: $0 REFERENCE_MIFF CANDIDATE_MIFF DIR..." >&2; exit 2; }
abs() { echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"; }
REF=$(abs "$1"); CAND=$(abs "$2"); shift 2
[ -x "$REF" ] && [ -x "$CAND" ] || { echo "compiler missing" >&2; exit 2; }
HERE=$(cd "$(dirname "$0")" && pwd)
TMP=$(mktemp -d) || exit 2
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
files=0; same=0; bothreject=0; differ=0
for dir in "$@"; do
	[ -d "$dir" ] || { echo "not a corpus directory: $dir" >&2; exit 2; }
	find "$dir" -name '*.mif' | LC_ALL=C sort > "$TMP/list"
	while IFS= read -r f; do
		files=$((files + 1))
		a=$(sh "$HERE/miff-compile.sh" "$REF" "$f" "$TMP/ref.iff") || exit 2
		b=$(sh "$HERE/miff-compile.sh" "$CAND" "$f" "$TMP/cand.iff") || exit 2
		if [ "$a" = REJECT ] && [ "$b" = REJECT ]; then bothreject=$((bothreject + 1))
		elif [ "$a" != REJECT ] && [ "$b" != REJECT ] && cmp -s "$TMP/ref.iff" "$TMP/cand.iff"; then same=$((same + 1))
		else differ=$((differ + 1)); echo "DIFFER $f: reference=$a candidate=$b"; fi
	done < "$TMP/list"
done
rm -rf "$TMP"
echo "files=$files identical=$same both-rejected=$bothreject differ=$differ"
[ "$files" -gt 0 ] && [ "$differ" -eq 0 ]
