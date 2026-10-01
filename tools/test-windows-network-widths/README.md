# Server Windows network TU compilation

This compile-only check uses the checkout's actual `Sock.cpp`, `TcpClient.cpp`
and `TcpServer.cpp`, genuine public/forwarding headers and Windows SDK. It does
not link or execute a server, run a socket fixture, or configure Oracle/JNI.
Requires Windows, Python 3, Git and C++17-capable MSVC 19.11 or newer with x86/x64
compiler tools. No provider downloads or replacement declarations are supplied.

```
python tools/test-windows-network-widths/test_runner.py
python tools/test-windows-network-widths/run.py --vcvars "C:\path\VC\Auxiliary\Build\vcvarsall.bat" --out C:\temp\server-network-fresh
```

The output directory must be new. `--checkout` defaults to this script's checkout.
The 20 cases comprise 12 genuine production-TU compiles (three sources × two
architectures × Debug/Release) and eight independent TCP completion-key reversions.
The Win32 reversions must compile. Each x64 reversion must fail only with C2664
at its actual `GetQueuedCompletionStatus` call start or closing argument line, identifying argument 3
and `PULONG_PTR`; its paired unmodified TU must compile. Unrelated errors fail.
Positive objects must have the requested x86/x64 COFF machine type.

The runner reads public includes from the two sharedNetwork CMake files, selects
their Windows include directory, and reads active Windows Debug/Release flags
from root CMake. It retains those definitions, CRT/optimization flags and existing
warning switches. It explicitly adds `/std:c++17 /EHsc /c /Y- /showIncludes`.
Root CMake's `_USE_32BIT_TIME_T=1` is omitted only for x64 because MSVC forbids it
there. This is a documented standalone source-level compile adaptation, not a
claim that the full CMake Windows target configures or links. Client vcxproj and
STLport settings are not imported. Missing dependency closure is a failed check,
not permission to substitute headers or port unrelated source.

`VSLANG=1033` fixes the diagnostic/include-trace language. Outputs retain exact
commands, compiler version/path/hash, raw logs, Git head, tracked-input hashes,
transitive included-header hashes, COFF identities and case/aggregate results.
The workflow uploads text logs and JSON only. No runtime pass, full server build,
binary-provider compatibility or production networking lifecycle is established.
All affected libraries and consumers must be rebuilt together for the Windows
x64 ABI change. The source package depends on the separately reviewed imemmove
prerequisite (upstream server PR37).

Local `test_runner.py` checks recipe selection, exact mutation binding and
negative diagnostic rejection; its printed count derives from executed checks.
Native qualification is separate and must be reported from an actual workflow run.

The minimum is VS2017 15.3 / MSVC 19.11, where Microsoft introduced
[`/std:c++17`](https://learn.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=msvc-170#stdc17);
[compiler version mapping](https://learn.microsoft.com/en-us/cpp/overview/compiler-versions?view=msvc-170)
identifies that release as 1911. Any D9002 ignored-option diagnostic fails a case,
even if compilation succeeds. Console output is one compact outcome per case;
complete commands remain in JSON and batch/log artifacts.
