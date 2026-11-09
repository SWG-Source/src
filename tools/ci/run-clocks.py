#!/usr/bin/env python3
"""Build clock_robustness_test.cpp against each real clock implementation and run it."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile

# (name, macro, sources, include directories, expected PASS lines)
TCP = 'TcpLibrary'
CASES = [
    ('ServerClock', 'TEST_SERVER_CLOCK',
     ['engine/server/library/serverUtility/src/shared/ServerClock.cpp',
      'tools/test-wire-compatibility/fatal.cpp'], [], 10),
    ('udplibrary', 'TEST_UDP_CLOCK',
     ['external/3rd/library/udplibrary/UdpLibrary.cpp'],
     ['external/3rd/library/udplibrary'], 4),
    ('VChatAPI UdpLibrary', 'TEST_UDP_CLOCK',
     ['external/3rd/library/soePlatform/VChatAPI/utils2.0/utils/UdpLibrary/UdpLibrary.cpp'],
     ['external/3rd/library/soePlatform/VChatAPI/utils2.0/utils/UdpLibrary',
      'external/3rd/library/soePlatform/VChatAPI/utils2.0/utils'], 4),
] + [
    (directory, 'TEST_TCP_CLOCK', [directory + '/Clock.cpp'], [directory], 3)
    for directory in (
        'engine/server/application/CentralServer/src/shared/AuctionTransferGameAPI/' + TCP,
        'external/3rd/library/soePlatform/CTServiceGameAPI/' + TCP,
        'external/3rd/library/soePlatform/CSAssist/utils/' + TCP,
        'external/3rd/library/soePlatform/VChatAPI/utils2.0/utils/' + TCP,
    )
]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--bits', type=int, choices=(32, 64), required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    shared_includes = []
    for base in (root / 'engine/shared/library', root / 'engine/server/library',
                 root / 'external/ours/library'):
        for library in sorted(base.iterdir()):
            for suffix in ('include/public', 'include', 'src/shared', 'src/linux'):
                path = library / suffix
                if path.is_dir():
                    shared_includes.append('-I' + str(path))
    cxx = os.environ.get('CXX', 'g++')
    failed = []
    with tempfile.TemporaryDirectory(prefix='swg-clocks-') as directory:
        for index, (name, macro, sources, includes, expected) in enumerate(CASES):
            print(f'== {name}', flush=True)
            executable = Path(directory) / f'clock-test-{index}'
            flags = ['-I' + str(root / path) for path in includes]
            if macro == 'TEST_SERVER_CLOCK':
                flags += shared_includes
            subprocess.run([
                cxx, '-std=c++17', f'-m{args.bits}', '-DLINUX=1', '-Dlinux=1',
                '-D_USING_STL=1', '-DDEBUG_LEVEL=0', '-DPRODUCTION=1', '-D' + macro,
                '-O1', '-w', '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections',
                '-pthread', *flags, str(root / 'tools/ci/clock_robustness_test.cpp'),
                *[str(root / source) for source in sources], '-o', str(executable),
            ], check=True)
            result = subprocess.run([str(executable)], stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT, text=True)
            print(result.stdout, end='')
            lines = result.stdout.splitlines()
            if (result.returncode or 'ALL PASS' not in lines
                    or sum(line.startswith('PASS  ') for line in lines) != expected):
                failed.append(name)
    if failed:
        raise SystemExit('clock regressions failed: ' + ', '.join(failed))
    print(f'all {len(CASES)} clock implementations pass')


if __name__ == '__main__':
    main()
