#!/usr/bin/env python3
"""Compile production parsers and compare the existing complete output oracle."""
import argparse
import difflib
import os
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--bits', type=int, choices=(32, 64), required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    test_dir = root / 'engine/shared/library/sharedFoundation/test'
    sources = [
        test_dir / 'integer_parsing_test.cpp',
        root / 'engine/shared/library/sharedCommandParser/src/shared/CommandParser.cpp',
        root / 'engine/shared/library/sharedFoundation/src/shared/dynamicVariable/DynamicVariable.cpp',
        root / 'external/ours/library/unicode/src/shared/UnicodeUtils.cpp',
        root / 'external/ours/library/unicode/src/shared/utf8.cpp',
    ]
    includes = []
    for base in (root / 'engine/shared/library', root / 'external/ours/library'):
        for library in sorted(base.iterdir()):
            for suffix in ('include/public', 'include', 'src/shared', 'src/linux'):
                path = library / suffix
                if path.is_dir():
                    includes.append('-I' + str(path))
    expected = (test_dir / 'integer_parsing_expected.txt').read_text()
    if len(expected.splitlines()) != 41:
        raise SystemExit('Expected the reviewed 41-line parsing oracle')
    with tempfile.TemporaryDirectory(prefix='swg-parsing-') as directory:
        output = str(Path(directory) / 'integer-parsing')
        command = [
            os.environ.get('CXX', 'g++'), '-std=c++17', f'-m{args.bits}',
            '-DLINUX=1', '-Dlinux=1', '-D_USING_STL=1', '-DDEBUG_LEVEL=0',
            '-DPRODUCTION=1', '-O1', '-ffunction-sections', '-fdata-sections',
            '-Wl,--gc-sections', '-pthread', *includes,
            *map(str, sources), '-o', output,
        ]
        subprocess.run(command, check=True)
        result = subprocess.run([output], stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, text=True)
        print(result.stdout, end='')
        if result.returncode:
            raise SystemExit(result.returncode)
        if result.stdout != expected:
            print(''.join(difflib.unified_diff(
                expected.splitlines(True), result.stdout.splitlines(True),
                fromfile='integer_parsing_expected.txt', tofile='actual')), end='')
            raise SystemExit('FAIL: parsing output differs from the oracle')
        print('ALL PASS (41 lines match)')


if __name__ == '__main__':
    main()
