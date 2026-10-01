#!/usr/bin/env python3
"""Build the existing DebugHelp regression with real source, without server SDKs."""
import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--bits', type=int, choices=(32, 64), required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    test = root / 'engine/shared/library/sharedDebug/test/linux'
    source = root / 'engine/shared/library/sharedDebug/src/linux/DebugHelp.cpp'
    includes = []
    for base in (root / 'engine/shared/library', root / 'external/ours/library'):
        for library in sorted(base.iterdir()):
            for suffix in ('include/public', 'include', 'src/shared', 'src/linux'):
                path = library / suffix
                if path.is_dir():
                    includes.append('-I' + str(path))
    cxx = os.environ.get('CXX', 'g++')
    with tempfile.TemporaryDirectory(prefix='swg-debughelp-') as directory:
        directory = Path(directory)
        library = directory / 'libknown.so'
        other = directory / 'other-class'
        executable = directory / 'debughelp-test'
        subprocess.run([cxx, f'-m{args.bits}', '-g', '-gdwarf-4', '-O0',
                        '-fPIC', '-shared', str(test / 'known_so.cpp'),
                        '-o', str(library)], check=True)
        subprocess.run([cxx, f'-m{96 - args.bits}', '-x', 'c++', '-',
                        '-o', str(other)], input='int main() { return 0; }',
                       text=True, check=True)
        subprocess.run([
            cxx, '-std=c++17', f'-m{args.bits}', '-DLINUX=1', '-Dlinux=1',
            '-D_USING_STL=1', '-DDEBUG_LEVEL=0', '-DPRODUCTION=1',
            '-DDEBUGHELP_HAS_ELF_CLASS_CHECK', '-g', '-gdwarf-4', '-O0',
            '-no-pie', '-ffunction-sections', '-fdata-sections',
            '-Wl,--gc-sections', '-pthread', *includes,
            '-DDEBUGHELP_SOURCE="' + str(source) + '"',
            str(test / 'debughelp_test.cpp'),
            str(root / 'engine/shared/library/sharedSynchronization/src/linux/Mutex.cpp'),
            '-ldl', '-o', str(executable),
        ], check=True)
        first = next(i for i, line in enumerate(
            (test / 'known_so.cpp').read_text().splitlines(), 1)
            if 'knownSoFunction' in line)
        result = subprocess.run([str(executable), str(library), str(first),
                                 str(first + 4), str(other)],
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True)
        print(result.stdout, end='')
        if result.returncode:
            raise SystemExit(result.returncode)
        lines = result.stdout.splitlines()
        if (sum(line.startswith('PASS  ') for line in lines) != 6
                or 'ALL PASS' not in lines or any(line.startswith('FAIL') for line in lines)):
            raise SystemExit('FAIL: expected six DebugHelp checks and ALL PASS')
        lookups = [line for line in lines if line.startswith('ADDR2LINE lookupAddress(knownFunction')]
        if len(lookups) != 1:
            raise SystemExit('FAIL: missing or ambiguous executable address lookup')
        address, got = lookups[0].split()[-2:]
        want = subprocess.check_output(['addr2line', '-e', str(executable), address], text=True).strip()
        want = re.sub(r' \(discriminator.*', '', want)
        if Path(want).name != Path(got).name:
            raise SystemExit(f'FAIL: addr2line says {want}, DebugHelp says {got}')
        print(f'PASS  addr2line agrees: {want}')


if __name__ == '__main__':
    main()
