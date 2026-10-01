"""Compile real server Windows network TUs and their completion-key controls."""
import argparse
import hashlib
import json
import ntpath
import os
from pathlib import Path
import re
import struct
import subprocess

LIB = 'engine/shared/library/sharedNetwork'
TUS = [LIB + '/src/win32/' + name + '.cpp' for name in ('Sock', 'TcpClient', 'TcpServer')]
CMAKE = ['CMakeLists.txt', LIB + '/CMakeLists.txt', LIB + '/src/CMakeLists.txt']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def recipe(root, configuration, arch):
    """Read the selected repository CMake settings, without configuring providers."""
    top = '\n'.join(line.split('#', 1)[0] for line in (root / CMAKE[0]).read_text().splitlines())
    if not re.search(r'set\(CMAKE_CXX_STANDARD\s+17\)', top):
        raise ValueError('Expected server C++17 setting')
    match = re.search(r'set\(CMAKE_CXX_FLAGS_' + configuration.upper() + r'\s+"([^"\n]*\$\{CMAKE_CXX_FLAGS_' + configuration.upper() + r'\}[^\"]*)"\)', top)
    if not match:
        raise ValueError('Expected explicit Windows configuration flags')
    flags = re.sub(r'\$\{[^}]+\}|\\\n', '', match.group(1)).split()
    # CMake's Windows flags assume x86. MSVC forbids this macro on x64.
    if arch == 'amd64':
        flags.remove('-D_USE_32BIT_TIME_T=1')
    includes = []
    for name in CMAKE[1:]:
        file = root / name
        text = file.read_text()
        for block in re.findall(r'include_directories\(([^)]+)\)', text):
            for item in block.split():
                item = item.replace('${CMAKE_CURRENT_SOURCE_DIR}', str(file.parent))
                item = item.replace('${SWG_ENGINE_SOURCE_DIR}', str(root / 'engine'))
                item = item.replace('${SWG_EXTERNALS_SOURCE_DIR}', str(root / 'external'))
                path = Path(item).resolve()
                if path.name != 'linux':
                    includes.append(path)
    return flags, includes


def revert(body):
    needle = 'ULONG_PTR completionKey = 0;'
    if body.count(needle) != 1:
        raise ValueError('Completion-key mutation must bind exactly once')
    changed = body.replace(needle, 'unsigned long int completionKey = 0;')
    calls = list(re.finditer(r'GetQueuedCompletionStatus\s*\([^;]*&completionKey[^;]*;', changed))
    if len(calls) != 1:
        raise ValueError('Expected one genuine completion-key call')
    # Older MSVC reports the final argument line; modern MSVC reports the call start.
    locations = (changed.count('\n', 0, calls[0].start()) + 1,
                 changed.count('\n', 0, calls[0].end()) + 1)
    return changed, locations


def key_failure(text, source, locations):
    found = 0
    for line in text.splitlines():
        if not re.search(r'\b(?:fatal\s+)?error\b|not recognized|cannot find', line, re.I):
            continue
        match = re.match(r'^(.*?)\((\d+)(?:,\d+)?\)\s*:\s*error C2664:\s*(.*)$', line)
        if not match:
            return False
        path, number, cause = match.groups()
        if (ntpath.normcase(ntpath.normpath(path)) != ntpath.normcase(ntpath.normpath(str(source)))
                or int(number) not in locations
                or not re.fullmatch(r".*GetQueuedCompletionStatus.*cannot convert argument 3 from 'unsigned long(?: int)? \*' to 'PULONG_PTR'.*", cause)):
            return False
        found += 1
    return found > 0


def compiler_supported(versions):
    return bool(versions) and all(v.startswith(b'19.') and int(v.decode().split('.')[1]) >= 11
                                  for v in versions)


def options_supported(text):
    return re.search(r'\bD9002\b', text) is None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--checkout', type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument('--out', required=True, type=Path)
    parser.add_argument('--vcvars', required=True, type=Path)
    args = parser.parse_args()
    if os.name != 'nt':
        parser.error('Native Windows and C++17-capable MSVC required')
    root, out, vcvars = args.checkout.resolve(), args.out.resolve(), args.vcvars.resolve(strict=True)
    out.mkdir(parents=True, exist_ok=False)
    environment = dict(os.environ, VSLANG='1033')
    tracked = subprocess.check_output(['git', '-C', str(root), 'ls-files', '-z']).decode().split('\0')
    inputs = {name: sha(root / name) for name in tracked if name and (root / name).is_file()}
    save(out / 'input-sha256.json', inputs)
    save(out / 'identity.json', {'head': subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD']).decode().strip(),
         'vcvars': str(vcvars), 'vcvars_sha256': sha(vcvars), 'VSLANG': '1033', 'runner_sha256': sha(Path(__file__)),
         'adaptations': ['explicit /std:c++17 /EHsc /c /Y- /showIncludes', 'omit _USE_32BIT_TIME_T only on x64']})
    results, headers, toolchains = [], {}, {}
    for arch in ('x86', 'amd64'):
        script = out / ('toolchain-' + arch + '.cmd')
        script.write_text('@echo off\ncall "' + str(vcvars) + '" ' + arch + '\nif errorlevel 1 exit /b 1\nwhere cl\nwhere link\ncl /Bv\n')
        metadata = subprocess.run(['cmd', '/d', '/c', str(script)], env=environment, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
        (out / ('toolchain-' + arch + '.log')).write_bytes(metadata.stdout)
        versions = re.findall(rb'Compiler Version (19\.\d+[^ \r\n]*)', metadata.stdout)
        if not compiler_supported(versions):
            raise ValueError('Expected C++17-capable MSVC 19.11 or newer; inspect toolchain log')
        executables = {}
        for line in metadata.stdout.decode(errors='replace').splitlines():
            path = Path(line.strip())
            if path.suffix.lower() == '.exe' and path.is_file():
                executables[str(path)] = sha(path)
        toolchains[arch] = {'versions': [v.decode() for v in versions], 'executables': executables}
        save(out / 'toolchains.json', toolchains)
        for config in ('Debug', 'Release'):
            flags, includes = recipe(root, config, arch)
            for name in TUS:
                original = root / name
                modes = [('candidate', original, None)]
                if original.stem != 'Sock':
                    body, location = revert(original.read_text())
                    control = out / ('reverted-' + original.name)
                    control.write_text(body)
                    modes.append(('reverted', control, location))
                positive = False
                for mode, source, location in modes:
                    directory = out / (arch + '-' + config + '-' + original.stem + '-' + mode)
                    directory.mkdir()
                    obj = directory / 'unit.obj'
                    command = ['cl', '/nologo', '/std:c++17', '/EHsc', '/c', '/Y-', '/showIncludes'] + flags
                    command += ['/I' + str(p) for p in includes] + [str(source), '/Fo' + str(obj)]
                    script = directory / 'compile.cmd'
                    script.write_text('@echo off\ncall "' + str(vcvars) + '" ' + arch + '\nif errorlevel 1 exit /b 1\n' + subprocess.list2cmdline(command) + '\nexit /b %errorlevel%\n')
                    result = subprocess.run(['cmd', '/d', '/c', str(script)], cwd=directory, env=environment,
                                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120)
                    (directory / 'compile.log').write_bytes(result.stdout)
                    text = result.stdout.decode(errors='replace')
                    for line in text.splitlines():
                        if 'Note: including file:' in line:
                            path = Path(line.split('Note: including file:', 1)[1].strip()).resolve(strict=True)
                            headers[str(path)] = sha(path)
                    negative = mode == 'reverted' and arch == 'amd64'
                    machine = struct.unpack('<H', obj.read_bytes()[:2])[0] if obj.is_file() else None
                    passed = (result.returncode != 0 and positive and key_failure(text, source, location)) if negative else (
                        result.returncode == 0 and machine == (0x14c if arch == 'x86' else 0x8664))
                    passed = passed and options_supported(text)
                    if mode == 'candidate':
                        positive = passed
                    row = {'case': directory.name, 'command': command, 'source_sha256': sha(source),
                           'compile_exit': result.returncode, 'coff_machine': machine, 'negative': negative, 'passed': passed}
                    results.append(row)
                    save(out / 'results.json', results)
                    save(out / 'included-header-sha256.json', headers)
                    print('%s %s (exit=%d)' % ('PASS' if passed else 'FAIL', directory.name, result.returncode), flush=True)
    unchanged = all((root / name).is_file() and sha(root / name) == value for name, value in inputs.items())
    summary = {'expected': 20, 'observed': len(results), 'passed_cases': sum(r['passed'] for r in results),
               'tracked_inputs_unchanged': unchanged, 'runtime_executions': 0}
    summary['passed'] = len(results) == 20 and all(r['passed'] for r in results) and unchanged
    save(out / 'summary.json', summary)
    return 0 if summary['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
