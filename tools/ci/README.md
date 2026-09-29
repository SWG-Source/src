# Server regression CI

Both workflows check out the submitted PR **head SHA**, not a fresh clone of
upstream and not GitHub's synthetic merge commit. They run for PRs targeting
`master` or `64-bit-types`, on the `ci/server-regressions` review branch, and
by manual dispatch. This tests the head in isolation; it is not a merge-conflict
or merge-result check. Artifacts record `git rev-parse HEAD`, logs and compiler
or container identity. Shell pipelines use Bash `pipefail`, so `tee` cannot
hide a compiler or test failure. Tokens have read-only repository permission.

## Portable matrix

Ubuntu 22.04 with `g++-multilib` builds fresh executables on both ABIs:

| Runner | 32-bit | 64-bit |
| --- | ---: | ---: |
| `tools/test-wire-compatibility/run.py` | 19 passes | 20 passes |
| `tools/test-wire-compatibility/run-timestamps.py` | 16 passes | 22 passes |
| `tools/ci/run-parsing.py` | 41 oracle lines | 41 oracle lines |

Run the same commands locally with `python3 RUNNER --bits 32` (or `64`).
No cached libraries, Oracle installation, VM or live cluster are needed.
The existing wire runners enforce exact pass totals. Their README documents
stand-ins, compile-only writers, skipped cases and known coverage limits.

The parsing runner compiles the existing `integer_parsing_test.cpp` and four
production translation units: CommandParser, DynamicVariable, UnicodeUtils and
utf8. Unused functions are removed by linker section garbage collection; there
are no parser stand-ins. Its complete combined output must match the checked-in
41-line oracle, and a nonzero process status is always a failure. This is the
same oracle used by the full-library shell runner, not a newly recorded result.
It exercises parser contracts, not every parsing caller or service integration.

## Legacy build

The existing legacy image (amd64 despite the `i386` repository name) receives that exact checkout as a read-only source
mount at the path expected by `build.xml`. `ant clean build_src` discards the
image's build directory before compiling, preventing stale objects from being
used. Checkout and artifact actions run on the modern host rather than inside
an legacy Node environment. The image is pinned by digest and its digest/architecture are recorded on every
run. Ant explicitly requests a 32-bit target build.

A successful legacy build does not establish a full 64-bit server build or live
cluster compatibility. The portable suites omit the optional mission fixtures
that require full same-revision server libraries, full-message and persistence
coverage, oversized containers at real call sites, nested rollback, and
byte-buffer overflow. Native full-cluster acceptance remains a separate gate.

Calendar boundaries, the string-search narrowing scan and SQL audit generation
also run in the portable matrix. The audit test never connects to a database.
Clock, OsFile, DebugHelp and Miff use the freshly built server libraries/tool in
the legacy build via `run-built-regressions.sh`; those checks currently run only
on the legacy 32-bit build. Their 64-bit automation remains open, not a passing
or skipped portable-matrix result. Full-library tests are not replaced with
stand-ins to avoid their real build prerequisites. OsFile requires sparse-file
support; DebugHelp also builds an opposite-class ELF with multilib and compares
its symbol lookup to `addr2line`.
