#!/usr/bin/env python3
"""Regression check: std::string / Unicode::String search results must stay in size_type.

On LP64, npos is 2^64-1. A find() result held in a 32-bit variable (int, unsigned,
uint32...) becomes 0xFFFFFFFF or -1 and never compares equal to npos, so the
"not found" branch never runs. A cast of npos to a 32-bit type hides the same bug
from the compiler. On ILP32 both happen to work, which is why the code looked fine.

This script flags, in C++ sources:
  1. a 32-bit variable declared from a search call
       int pos = s.find(' ');   unsigned p = static_cast<unsigned>(s.rfind('/'));
  2. an assignment of a search call to a name declared 32-bit in the same file
       int pos; ... pos = static_cast<int>(s.find(','));
  3. npos converted to a 32-bit type
       static_cast<unsigned int>(std::string::npos)   int(Unicode::String::npos)
  4. a value narrowed to 32 bits and then compared with npos
       if (static_cast<int>(index) == s.npos)

Usage:  tools/check-npos-narrowing.py [ROOT ...]     (default: engine game external/ours)
Exit status 0 when nothing is found, 1 otherwise. Lines may opt out with a
trailing comment "// npos-narrowing: ok (reason)".
"""
import os
import re
import sys

NARROW = r'(?:unsigned\s+int|unsigned|signed\s+int|int|uint32_t|int32_t|uint32|int32|uint16|int16|short|unsigned\s+short)'
SEARCH = r'\.\s*(?P<method>find|rfind|find_first_of|find_last_of|find_first_not_of|find_last_not_of)\s*\(\s*(?P<arg>.?)'
CAST = r'(?:static_cast\s*<\s*' + NARROW + r'\s*>\s*\(\s*|\(\s*' + NARROW + r'\s*\)\s*|' + NARROW + r'\s*\(\s*)?'

# 1. declaration initialised from a search call (optionally through a cast)
DECL = re.compile(r'\b(?:const\s+)?' + NARROW + r'\s+(?:const\s+)?&?\s*(?P<name>\w+)\s*=\s*' + CAST + r'[\w\.\->\[\]\(\)]*?' + SEARCH)
# 2. a statement "name = [cast] expr.find(" ; the type comes from name's nearest preceding declaration
ASSIGN = re.compile(r'^\s*(?P<name>\w+)\s*=(?!=)\s*' + CAST + r'[\w\.\->\[\]\(\)]*?' + SEARCH)
NARROW_ONLY = re.compile(r'^(?:const\s+)?' + NARROW + r'$')
KEYWORDS = {'return', 'else', 'case', 'delete', 'throw', 'goto', 'new', 'typedef', 'using'}


def declared_type(code, line_index, name):
    decl = re.compile(r'(?:^|[;{(,]\s*|\s)((?:const\s+)?(?:unsigned\s+|signed\s+)?[\w:]+(?:\s*<[^;=]*>)?)\s+(?:const\s+)?[&*]?\s*\b' + re.escape(name) + r'\s*(?:=(?!=)|;|,|\)|\[|\()')
    for i in range(line_index - 1, -1, -1):
        for m in reversed(list(decl.finditer(code[i]))):
            t = m.group(1).strip()
            if t.split()[-1] not in KEYWORDS:
                return t
    return None
# 3. npos narrowed through a cast
NPOS = re.compile(r'(?:static_cast\s*<\s*' + NARROW + r'\s*>\s*\(\s*|\b' + NARROW + r'\s*\(\s*|\(\s*' + NARROW + r'\s*\)\s*)[\w:]*::npos\b')

# 4. a value cast to 32 bits and then compared with npos
CASTCMP = re.compile(r'static_cast\s*<\s*' + NARROW + r'\s*>\s*\([^()]*(?:\([^()]*\))?[^()]*\)\s*[!=]=\s*[\w:\.]*\bnpos\b')

EXTS = ('.cpp', '.h', '.hpp', '.cc', '.inl', '.def')
# third-party code kept verbatim (Crypto++ sources)
SKIP = (os.sep + os.path.join('crypto', 'src', 'shared', 'original') + os.sep,)


def strip_comment(line):
    i = line.find('//')
    return line if i < 0 else line[:i]


def is_string_search(m, name, code, index):
    """Container find() (std::map, AutoDeltaVector...) shares the name; only string searches matter.
    A string search is a find_* / rfind call, a find with a literal argument, or a find whose
    result is later compared with npos."""
    if m.group('method') != 'find' or m.group('arg') in ('"', "'"):
        return True
    if name:
        uses = re.compile(r'\b' + re.escape(name) + r'\b[^;]*npos|npos[^;]*\b' + re.escape(name) + r'\b')
        return any(uses.search(c) for c in code[index:index + 40])
    return False


def scan(path):
    hits = []
    try:
        with open(path, encoding='latin-1') as f:
            lines = f.readlines()
    except OSError:
        return hits
    code = [strip_comment(l) for l in lines]
    for n, (raw, c) in enumerate(zip(lines, code), 1):
        if 'npos-narrowing: ok' in raw:
            continue
        why = None
        d = DECL.search(c)
        if d and is_string_search(d, d.group('name'), code, n - 1):
            why = 'search result stored in a 32-bit variable'
        elif not d:
            m = ASSIGN.search(c)
            if m and is_string_search(m, m.group('name'), code, n - 1):
                t = declared_type(code, n - 1, m.group(1))
                if t and NARROW_ONLY.match(t):
                    why = 'search result assigned to a 32-bit variable'
        if not why and NPOS.search(c):
            why = 'npos converted to a 32-bit type'
        if not why and CASTCMP.search(c):
            why = 'value narrowed to 32 bits before comparing with npos'
        if why:
            hits.append((path, n, why, raw.strip()))
    return hits


def main(argv):
    here = os.path.dirname(os.path.abspath(__file__))
    top = os.path.dirname(here)
    roots = argv[1:] or [os.path.join(top, d) for d in ('engine', 'game', os.path.join('external', 'ours'))]
    hits = []
    for root in roots:
        if os.path.isfile(root):
            hits += scan(root)
            continue
        for d, _, files in os.walk(root):
            for name in sorted(files):
                if name.endswith(EXTS) and not any(k in os.path.join(d, name) for k in SKIP):
                    hits += scan(os.path.join(d, name))
    for path, n, why, text in hits:
        print('%s:%d: %s: %s' % (os.path.relpath(path), n, why, text))
    print('%d npos narrowing site(s)' % len(hits))
    return 1 if hits else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
