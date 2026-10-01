#!/usr/bin/env python3
"""Real-header Linux overlap regression and exact baseline ambiguity control."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--checkout', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    p.add_argument('--bits', type=int, choices=(32, 64), required=True)
    p.add_argument('--baseline', action='store_true')
    p.add_argument('--expect-ambiguity', action='store_true')
    p.add_argument('--consumers', action='store_true')
    a = p.parse_args()
    root, out = a.checkout.resolve(), a.out.resolve()
    out.mkdir(parents=True, exist_ok=False)
    result = {'passed': False, 'commands': [], 'bits': a.bits, 'baseline': a.baseline}
    env = dict(os.environ, LC_ALL='C')
    def execute(command, name):
        x = subprocess.run(command, cwd=out, env=env, stdout=subprocess.PIPE,
                           stderr=subprocess.STDOUT, timeout=120)
        (out / (name + '.log')).write_bytes(x.stdout)
        result['commands'].append({'argv': command, 'exit': x.returncode, 'log': name + '.log'})
        return x
    try:
        if a.expect_ambiguity and (not a.baseline or a.bits != 64 or a.consumers):
            raise ValueError('negative control requires baseline64 without consumers')
        compiler = shutil.which('g++')
        if not compiler:
            raise ValueError('g++ unavailable')
        result['compiler_version'] = execute([compiler, '--version'], 'compiler').stdout.decode()
        probe = Path(__file__).with_name('probe.cpp').resolve()
        flags = [compiler, '-m' + str(a.bits), '-std=c++17', '-DLINUX',
                 '-DEXPECT_BITS=' + str(a.bits), '-fdiagnostics-color=never']
        for lib in ('sharedFoundation', 'sharedFoundationTypes', 'sharedDebug', 'sharedFile', 'sharedMath'):
            flags.append('-I' + str(root / 'engine/shared/library' / lib / 'include/public'))
        flags.append('-I' + str(root / 'external/ours/library/fileInterface/include/public'))
        if a.baseline:
            flags.append('-DTEST_BASELINE')
        header = root / 'engine/shared/library/sharedFoundation/src/shared/Misc.h'
        result['input_sha256'] = {str(f): hashlib.sha256(f.read_bytes()).hexdigest()
                                  for f in (probe, header, Path(__file__).resolve(), Path(compiler))}
        x = execute(flags + ['-MMD', '-MF', str(out / 'probe.d'), str(probe), '-o', str(out / 'probe')], 'build')
        if a.expect_ambiguity:
            anchors = [(header, 'return memmove(destination, source, static_cast<uint>(length));'),
                       (probe, 'else result = memmove(')]
            expected = set()
            for path, anchor in anchors:
                matches = [i for i, line in enumerate(path.read_text().splitlines(), 1) if anchor in line]
                if len(matches) != 1:
                    raise ValueError('nonunique diagnostic anchor')
                expected.add((str(path.resolve()), matches[0]))
            errors = [line for line in x.stdout.decode().splitlines() if re.search(r'\berror:', line)]
            observed = []
            for line in errors:
                m = re.fullmatch(r"(.+):(\d+):\d+: error: call of overloaded 'memmove\(.*\)' is ambiguous", line)
                if not m:
                    raise ValueError('unexpected error diagnostic: ' + line)
                observed.append((str(Path(m[1]).resolve()), int(m[2])))
            if x.returncode == 0 or len(observed) != 2 or set(observed) != expected:
                raise ValueError('missing or duplicate ambiguity diagnostics')
            result['expected_ambiguity'] = True
        else:
            if x.returncode:
                raise ValueError('probe build failed')
            x = execute([str(out / 'probe')], 'run')
            if x.returncode or x.stdout != b'PASS 30 valid overlap and overload checks\n':
                raise ValueError('unexpected probe result')
            result['checks'] = 30
            if a.consumers:
                for lib, filename in (('sharedFoundation', 'Md5.cpp'), ('sharedFile', 'Iff.cpp')):
                    source = root / 'engine/shared/library' / lib / 'src/shared' / filename
                    result['input_sha256'][str(source)] = hashlib.sha256(source.read_bytes()).hexdigest()
                    x = execute(flags + ['-MMD', '-MF', str(out / (filename + '.d')), '-c', str(source), '-o', str(out / (filename + '.o'))], filename)
                    if x.returncode:
                        raise ValueError('real consumer compile failed: ' + filename)
        result['passed'] = True
    except Exception as error:
        result['error'] = str(error)
    (out / 'results.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))
    return 0 if result['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
