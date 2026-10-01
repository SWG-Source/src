"""Portable tests of recipe selection, mutation binding and failure classification."""
from pathlib import Path
from run import recipe, revert, key_failure, compiler_supported, options_supported

checks = 0
def check(condition):
    global checks
    assert condition
    checks += 1

root = Path(__file__).resolve().parents[2]
for config in ('Debug', 'Release'):
    for arch in ('x86', 'amd64'):
        flags, includes = recipe(root, config, arch)
        check(('-D_USE_32BIT_TIME_T=1' in flags) == (arch == 'x86'))
        check(('/MTd' if config == 'Debug' else '/MT') in flags)
        check(all(p.is_dir() for p in includes))
        check(not any(p.name == 'linux' for p in includes))
for stem in ('TcpClient', 'TcpServer'):
    body = (root / ('engine/shared/library/sharedNetwork/src/win32/' + stem + '.cpp')).read_text()
    changed, locations = revert(body)
    start, line = locations
    check(changed.replace('unsigned long int completionKey = 0;', 'ULONG_PTR completionKey = 0;') == body)
    check(changed.splitlines()[line - 1].strip() == ');')
    check('GetQueuedCompletionStatus(' in changed.splitlines()[start - 1])
    source = r'C:\test\reverted-' + stem + '.cpp'
    valid = source + '(%d): error C2664: GetQueuedCompletionStatus: cannot convert argument 3 from \'unsigned long *\' to \'PULONG_PTR\'\n' % line
    check(key_failure(valid, source, locations))
    for invalid in ('', valid.replace(source, r'C:\other.cpp'), valid.replace('argument 3', 'argument 2'),
                    valid.replace('GetQueuedCompletionStatus', 'OtherCall'), valid.replace('C2664', 'C1083'),
                    valid + 'LINK : fatal error LNK1104: missing library\n', valid + "'cl' is not recognized\n",
                    valid + source + '(%d): error C2664: unrelated cause\n' % line):
        check(not key_failure(invalid, source, locations))
    check(not key_failure(valid, source, (start - 1, line + 1)))
    check(key_failure(valid.replace('(%d)' % line, '(%d)' % start), source, locations))
    for bad_line in (start - 1, start + 1, line - 1, line + 1):
        check(not key_failure(valid.replace('(%d)' % line, '(%d)' % bad_line), source, locations))
    for bad in (body.replace('ULONG_PTR completionKey = 0;', ''), body + '\nULONG_PTR completionKey = 0;'):
        try:
            revert(bad)
        except ValueError:
            check(True)
        else:
            raise AssertionError('bad mutation accepted')
for versions, expected in (([], False), ([b'19.10.25017'], False),
                           ([b'19.11.25506'], True), ([b'19.44.35207'], True),
                           ([b'18.00.40629'], False), ([b'19.11.1', b'19.10.1'], False)):
    check(compiler_supported(versions) == expected)
check(options_supported('compile succeeded'))
check(not options_supported("cl : Command line warning D9002 : ignoring unknown option '/std:c++17'"))
check(not options_supported('source.cpp(10): error C2664: expected control\ncl : warning D9002: ignored option'))
print('PASS %d runner checks' % checks)
