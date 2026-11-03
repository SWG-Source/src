"""Run Linux x64 regression checks without server or Oracle dependencies.

Usage: python3 tests/regression/run_64_bit_types.py --sanitize
Pass --cxx g++ or --cxx clang++ to match swg-main's ANT compiler selection.
Optional: --cmake PATH --build-dir PATH; --skip-cmake if its checks were run separately.
"""
import argparse
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def run(command):
    subprocess.run([str(arg) for arg in command], check=True)


def format_expression(line):
    match = re.search(r',\s*((?:"(?:[^"\\]|\\.)*"\s*|(?:SCN|PRI)\w+\s*)+)[,;)]', line)
    if not match:
        raise ValueError('Cannot locate production format expression: ' + line)
    return match.group(1).strip()


def source_format(path, marker):
    source = (ROOT / path).read_text()
    return format_expression(next(line for line in source.splitlines() if marker in line))


def named_format(path, name):
    source = (ROOT / path).read_text()
    line = next(line for line in source.splitlines() if re.search(r'\b' + name + r'\s*=', line))
    return line.split('=', 1)[1].strip().rstrip(';')


def scan_source():
    # Compile the format expressions from the production call sites. Guard words
    # detect adjacent writes even when a platform's scanf sanitizer is unavailable.
    commands = 'engine/server/library/serverGame/src/shared/command/CommandCppFuncs.cpp'
    queue = 'engine/server/library/serverGame/src/shared/command/CommandQueue.cpp'
    loader = 'engine/server/library/serverDatabase/src/shared/Loader.cpp'
    quest = 'engine/shared/library/sharedGame/src/shared/quest/PlayerQuestData.cpp'
    snapshot = 'game/server/application/SwgDatabaseServer/src/shared/core/SwgSnapshot.cpp'
    archive = 'external/ours/library/archive/src/shared/AutoDeltaPackedMap.h'
    checks = []
    # Include every scalar crafting/batch scan, console PID, and portal CRC scan.
    scalar_sites = [
        (commands, ['&schematicCrc);', '&uServerCrc);', '&uSharedCrc);']),
        ('engine/server/application/CentralServer/src/shared/ConsoleCommandParserGame.cpp', ['sscanf(Unicode::wideToNarrow(argv[1]).c_str()']),
        ('game/server/application/SwgGameServer/src/shared/snapshot/WorldSnapshotParser.cpp', ['&portalLayoutCrc);']),
    ]
    for path, markers in scalar_sites:
        for line in (ROOT / path).read_text().splitlines():
            if 'sscanf' in line and any(marker in line for marker in markers):
                checks.append(f'check(scan32("4294967295", {format_expression(line)}), "Scalar CRC/PID scan");')
    if len(checks) != 7:
        raise ValueError('Expected seven scalar CRC/PID scan call sites')
    for suffix in ['beginTime', 'endTime']:
        expression = source_format(queue, f'if (1 == sscanf(iterObjVar->first.c_str(), "commandCooldown.{suffix}.')
        checks.append(f'check(scan32("commandCooldown.{suffix}.4294967295", {expression}), "Cooldown scan");')
    checks.append(f'check(scan32("get_characters 4294967295", {source_format(loader, "sscanf( msg.getCommandLine()")}), "Account scan");')
    pair = source_format(commands, '&serverCrc, &sharedCrc);')
    deleted = source_format(loader, 'if (sscanf (msg.getCommandLine()')
    quest_scan = source_format(quest, 'numScanned = sscanf')
    old_buff = named_format(snapshot, 'oldschoolBuffFormat')
    buff_scan = named_format(snapshot, 'versionTwoScanFormat')
    buff_print = named_format(snapshot, 'versionTwoFormat')
    archive_lines = (ROOT / archive).read_text().splitlines()
    start = next(i for i, line in enumerate(archive_lines) if 'AutoDeltaPackedMap<uint32_t, uint32_t>::pack(' in line)
    packed_scan = format_expression(archive_lines[start + 2])
    packed_print = format_expression(archive_lines[start + 8])
    # Exercise the actual library aliases and scanf write sizes on Linux x64.
    return r'''
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "engine/shared/library/sharedFoundationTypes/src/linux/FoundationTypesLinux.h"
static_assert(sizeof(void *) == 8 && sizeof(long) == 8, "Run these checks for Linux x64 (LP64)");
void check(bool condition, char const *message) {
    if (!condition) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}
template <typename T> struct Guard {
    uint32 before = 0x12345678U;
    T value = 0;
    uint32 after = 0x87654321U;
    bool valid() const { return before == 0x12345678U && after == 0x87654321U; }
};
bool scan32(char const *input, char const *format) {
    Guard<uint32> number;
    return std::sscanf(input, format, &number.value) == 1 && number.valid() && number.value == UINT32_MAX;
}
int main() {
''' + '\n'.join(checks) + f'''
    Guard<uint32> a, b;
    check(std::sscanf("4294967295 2147483648", {pair}, &a.value, &b.value) == 2, "Draft-slot pair scan");
    check(a.valid() && b.valid() && a.value == UINT32_MAX && b.value == 2147483648U, "Draft-slot write size");
    char name[21] = {{}};
    check(std::sscanf("get_deleted_items 12345 4294967295", {deleted}, name, &a.value) == 2 && a.valid() && a.value == UINT32_MAX, "Deleted-item page scan");
    Guard<uint16> active, completed;
    check(std::sscanf("4294967295 65535 32768 12345", {quest_scan}, &a.value, &active.value, &completed.value, name) == 4, "Quest scan");
    check(a.valid() && active.valid() && completed.valid() && a.value == UINT32_MAX && active.value == UINT16_MAX && completed.value == 32768, "Quest write sizes");
    check(std::sscanf("4294967295 -1 -32768 12345", {quest_scan}, &a.value, &active.value, &completed.value, name) == 4 && active.value == UINT16_MAX && completed.value == 32768, "Legacy signed quest task masks");
    Guard<uint64> oldPacked;
    check(std::sscanf("4294967295 18446744073709551615", {old_buff}, &a.value, &oldPacked.value) == 2, "Legacy buff scan");
    check(a.valid() && oldPacked.valid() && a.value == UINT32_MAX && oldPacked.value == UINT64_MAX, "Legacy buff write sizes");
    Guard<uint32> endTime, duration, stacks;
    Guard<float> value;
    Guard<int64> caster;
    check(std::sscanf("4294967295 2147483648 1.25 4294967295 -9223372036854775807 4294967295:", {buff_scan}, &a.value, &endTime.value, &value.value, &duration.value, &caster.value, &stacks.value) == 6, "Version-two buff scan");
    check(a.valid() && endTime.valid() && value.valid() && duration.valid() && caster.valid() && stacks.valid(), "Version-two buff write sizes");
    check(a.value == UINT32_MAX && endTime.value == 2147483648U && value.value == 1.25f && duration.value == UINT32_MAX && caster.value == INT64_C(-9223372036854775807) && stacks.value == UINT32_MAX, "Version-two buff values");
    char text[256] = {{}};
    std::snprintf(text, sizeof(text), {buff_print}, a.value, endTime.value, value.value, duration.value, caster.value, stacks.value);
    check(std::strcmp(text, "4294967295 2147483648 1.250000 4294967295 -9223372036854775807 4294967295:") == 0, "Version-two persisted representation");
    check(std::sscanf("4294967295 2147483648", {packed_scan}, &a.value, &b.value) == 2 && a.valid() && b.valid() && a.value == UINT32_MAX && b.value == 2147483648U, "Packed-map scan");
    std::snprintf(text, sizeof(text), {packed_print}, a.value, b.value);
    check(std::strcmp(text, "4294967295 2147483648:") == 0, "Packed-map persisted representation");
    std::printf("Integer formats: CRC, cooldown, quest, snapshot and buff checks passed (pointer=%zu, long=%zu)\\n", sizeof(void *), sizeof(long));
}}
'''


def check_cmake(cmake, build):
    source = (ROOT / 'CMakeLists.txt').read_text()
    start = source.index('if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")')
    block = source[start:source.index('# Remove whitespace from flags', start)]
    for compiler_id in ['GNU', 'Clang']:
        script = build / (compiler_id + '.cmake')
        script.write_text(f'set(CMAKE_CXX_COMPILER_ID "{compiler_id}")\n' + block + r'''
foreach(config DEBUG RELWITHDEBINFO MINSIZEREL RELEASE)
    if(config STREQUAL "DEBUG")
        set(expected "-D_DEBUG;-DDEBUG_LEVEL=2;-DPRODUCTION=0")
    elseif(config STREQUAL "RELWITHDEBINFO")
        set(expected "-DDEBUG_LEVEL=0;-DPRODUCTION=0")
    else()
        set(expected "-DDEBUG_LEVEL=0;-DPRODUCTION=1")
    endif()
    foreach(flag ${expected})
        string(FIND "${CMAKE_CXX_FLAGS_${config}}" "${flag}" position)
        if(position EQUAL -1)
            message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID} ${config} is missing ${flag}")
        endif()
    endforeach()
endforeach()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_FLAGS_MINSIZEREL MATCHES "-fwhole-program-vtables")
    message(FATAL_ERROR "GNU production flags contain a Clang-only option")
endif()
''' + f'file(WRITE "{(build / (compiler_id + "-minsizerel.flags")).as_posix()}" "${{CMAKE_CXX_FLAGS_MINSIZEREL}}")\n')
        run([cmake, '-P', script])
        print(f'CMake: {compiler_id} definitions verified in all four configurations', flush=True)


def check(args, build):
    cxx = args.cxx or os.environ.get('CXX') or shutil.which('g++') or shutil.which('c++')
    if not cxx:
        raise RuntimeError('Supply a C++ compiler with --cxx')
    run([cxx, '--version'])
    flags = ['-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', '-Wformat=2', '-Wno-format-nonliteral', '-I' + str(ROOT)]
    if args.sanitize:
        flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
    scan = build / 'IntegerFormatsTest.cpp'
    scan.write_text(scan_source())
    for name, source in [('random', Path(__file__).with_name('FastRandomGeneratorTest.cpp')), ('formats', scan)]:
        executable = build / (name + ('.exe' if os.name == 'nt' else ''))
        run([cxx, *flags, source, '-o', executable])
        run([executable])
    if not args.skip_cmake:
        cmake = args.cmake or shutil.which('cmake')
        if not cmake:
            raise RuntimeError('Supply CMake with --cmake, or use --skip-cmake after testing it on another platform')
        check_cmake(cmake, build)
    gnu_flags = build / 'GNU-minsizerel.flags'
    if Path(cxx).name.startswith('g++') and gnu_flags.exists():
        empty = build / 'ProductionFlagTest.cpp'
        empty.write_text('int main() {}\n')
        executable = build / 'production-flags'
        run([cxx, *shlex.split(gnu_flags.read_text()), empty, '-o', executable])
        run([executable])
        print('GCC: production flags compile and link successfully', flush=True)
    print('64-bit-types regression checks passed', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cxx')
    parser.add_argument('--cmake')
    parser.add_argument('--build-dir', type=Path)
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--skip-cmake', action='store_true')
    args = parser.parse_args()
    if args.build_dir:
        args.build_dir.mkdir(parents=True, exist_ok=True)
        check(args, args.build_dir.resolve())
    else:
        with tempfile.TemporaryDirectory(prefix='swg-types-tests-') as directory:
            check(args, Path(directory))


if __name__ == '__main__':
    main()
