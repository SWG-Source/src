Linux Miff regression tests

Run from this directory with `sh`:

- `arith-diff.sh`: requires bison and gcc with 32-bit libc. It regenerates the candidate parser and tests its integer helpers against defined ILP32 long arithmetic. The fixed seed runs 3,000,000 operations total, cycling through 12 operators. Writer checks sample the int32 range in strides of 997; they are not exhaustive.
- `run-tests.sh /absolute/path/to/Miff`: 33 fixtures and fixed expected hashes/refusals. Run against candidate binaries of both ABIs. Expected output hashes came from the prior ILP32 reference run; the reasons for intentional refusals are in expected.txt.
- `compare-corpus.sh REFERENCE32 CANDIDATE DIR...`: compare known source corpora. Record the exact compiler revisions, ABI and corpus identity. Both-rejected files are counted separately from identical successful outputs. Zero files is a failure.

The compiler wrapper requires coreutils `timeout` and SHA-256 tools. It accepts only Miff's ERR_PARSER (-7, shell status 249) with no output as REJECT. A crash, timeout, inaccessible tool/input, preprocessing error or successful exit without output is a test error. An upstream negative control may therefore fail by crashing on invalid arithmetic; that is not equivalent to the candidate refusing it.

These tests do not cover the separate Windows evaluator, all preprocessing/include combinations, or live game loading of every emitted asset. Source paths with newlines are unsupported by the corpus walker.
