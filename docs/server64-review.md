# 64-bit Linux server review

This change set completes the current server-side LP64 corrections on top of
`64-bit-types` (`22dd13d4`). It preserves the existing 32-bit client protocol
and supports building the server as either ILP32 or LP64. Client x64/DX11
implementation and optional account re-keying are separate work.

## Review order

The PR retains the original six commits through `0707fd47`, followed by
focused commits for the remaining server changes and their regression tests.

| Area | Contract to review |
| --- | --- |
| Account identity and update 272 | Account-name hashing has a specified, database-recorded scheme; station IDs retain their signed 32-bit database representation. LoginServer owns authentication identity. |
| Wire, file and formatting boundaries | Serialized fields have explicit widths; format arguments match their specifiers; packed-map and delta-counter behavior is covered by literal fixtures. |
| Shutdown | Manifest entries own their values; auction destruction does not invalidate its iteration; JVM receives a standalone `-Xrs`; ServerConsole reads actual input bytes. |
| Code generation | Outputs live in the build directory; server and snapshot test share one `swg_database_codegen` target; Packager publication is atomic. |
| Parsing and narrowing | Checked integer parsing, string search positions, calendar range checks and Linux file-size limits preserve the intended destination contract. |
| Clock and symbol lookup | Millisecond timing stays 64-bit on either ABI; Linux DebugHelp reads the running executable's ELF class. |
| Linux Miff | Integer evaluation follows defined 32-bit behavior, including rejection of invalid operations. Although located under `engine/client`, Miff is a server build/data tool; the separate Windows evaluator is unchanged. |
| Database binding and snapshots | Signed/unsigned 32-bit columns use fixed-width bindings. Invalid narrowing rejects snapshot preparation; NULL policies are explicit; the integer audit checks schema coverage as well as values. |
| Diagnostics | Database connection logging omits the password. |

The fixed-width database commit replaces the earlier `BindableLong` API with
`BindableInt32`; assess the final tree rather than treating the initial six
commits as the final binding design. The snapshot component test establishes
preparation rejection and output behavior; it does not exercise a live fatal
group-send path or prove absence of partial network transmission by itself.

## Operator procedure

1. Back up the login and game schemas and retain the previous binaries and
   configuration. Rehearse on a separate database before production use.
2. Stop every server process. Use the console shutdown path for normal saving.
   SIGTERM is immediate termination; the JVM fix does not make it a save path.
3. Run the normal `ant update_database` procedure, including update 272 on the
   appropriate schemas. Do not repair station IDs with ad hoc arithmetic.
4. Record the login database's identity scheme using the
   [station-ID procedure](../game/server/database/tools/station_id_scheme/README.md).
   Export every applicable identity schema; an empty export from the wrong
   schema is not evidence of a new database. `assert gcc32` or `assert gcc64`
   requires knowledge of how the database was created. Mixed proven schemes
   require separate reconciliation; this PR does not re-key accounts.
5. Run the [integer-column audit](../game/server/database/tools/int32_audit/README.md)
   in dedicated sessions for `game`, `login`, and `sp-character` when used.
   Require both a successful exit and `RESULT: CLEAN` for each applicable
   contract. Investigate missing columns, out-of-range or fractional values;
   do not suppress findings or silently clamp stored data.
6. Start the cluster, check the recorded scheme and database connection, then
   test existing-account login, character loading, saving and restart.

New databases record `gcc32`. Existing databases must be labelled; there is
no configuration fallback that silently changes identity. Roll back using the
backup and matching binaries/configuration, not by adding 2^32 to negative IDs.

## Tests shipped in this PR

| Test | Instructions |
| --- | --- |
| Oracle bindings, snapshots and cluster lists | [Database tests](../game/server/database/tools/int32_audit/test/README.md); use a dedicated scratch schema on both ABIs. |
| Audit generation and coverage | `python3 game/server/database/tools/int32_audit/test/audit_generation_test.py` |
| Wire bytes and decoded state | [Wire fixtures](../tools/test-wire-compatibility/README.md); supply matching server libraries for the mission-list cases. |
| Clock and integer parsing | [Foundation tests](../engine/shared/library/sharedFoundation/test/README.md) |
| Calendar range | `engine/server/library/serverScript/test/calendar-time-test.sh` |
| String-search narrowing | `python3 tools/check-npos-narrowing.py` |
| Linux file size | `engine/shared/library/sharedFile/test/linux/osfile-test.sh` |
| Linux ELF symbols | `engine/shared/library/sharedDebug/test/linux/debughelp-test.sh` |
| Linux Miff arithmetic and corpus | [Miff tests](../engine/client/application/Miff/test/README.md) |

## Current production revision and reproducible checks

The production source at `8e57911e45ce52be88d6b9199bab82aef67ef1fc` was
freshly built and exercised on both Linux ABIs. The following CI and documentation
commits do not change production source. [Published logs and source hashes](https://github.com/Akilleez-QA/client-tools/tree/review/client-x64-evidence/review/client-x64/followups/server)
identify the tested revision, toolchain and limits.

- Fresh full 32/64-bit builds passed using GCC 7.5, CMake 3.17 and Java 11.
  The validation configuration was RELEASE with C++ `-O0`; this is not proof
  of an optimized production release.
- Each ABI passed 62 Oracle binding, 19 snapshot-send and 7 cluster-list checks,
  Clock and OsFile tests, and 33 Miff fixtures.
- Three cluster starts (64-bit, 64-bit restart, 32-bit) each reached 84 processes,
  including 38 GameServers, with running/connected console status and matching
  executable hashes and ELF classes.
- Console shutdown stopped Central/GameServers. Residual TaskManager/LoginServer
  processes were explicitly terminated. Connection-close/broken-pipe notices
  remain in the logs. No final-save acknowledgement is claimed.
- No client connected during this run. Both ABIs used the same copied database
  sequentially; this does not renew historical gameplay evidence or establish
  deterministic comparison of separately initialized databases.

[CI instructions](../tools/ci/README.md) build the submitted revision rather
than cloning another source tree. Portable CI covers wire (19/20), timestamp
and message (16/22), parser (41 expected lines), calendar (5), DebugHelp
(6 plus addr2line), npos scanning and audit generation on both ABIs. The legacy
container job freshly builds the submitted source and runs Clock, OsFile,
DebugHelp and Miff against that build. Both workflows passed at `25816f10`:
[portable run](https://github.com/Akilleez-QA/src/actions/runs/36549084361),
[full 32-bit build and regressions](https://github.com/Akilleez-QA/src/actions/runs/36549084466).
Oracle and cluster runtime remain VM validation, not hosted CI.

### Wire repair history

Review the final tree as well as these additive corrections; history is retained.

| Commit | Correction |
| --- | --- |
| `7ace7d51` | Restores unsigned legacy delta-counter arithmetic; four-byte width alone did not preserve wrap behavior. |
| `4889e6aa` | Restores signed 32-bit wire timestamps, checks narrowing, and keeps internal chat times host-sized. |
| `30cf4531` | Rejects oversized generic container counts instead of silently narrowing them. |
| `8e57911e` | Applies checked count conversion to the reviewed per-message writers. |

[Tracked acceptance work](https://github.com/Akilleez-QA/client-tools/blob/review/client-x64-evidence/review/client-x64/followups/ACCEPTANCE.md)
and the [base dependency assessment](https://github.com/Akilleez-QA/client-tools/blob/review/client-x64-evidence/review/client-x64/followups/server-dependency-plan.md)
keep deployment gaps and the underlying `64-bit-types` integration separate.

## Historical validation before the wire corrections

The results below describe earlier completion-candidate sessions. They are
retained as historical evidence and are not attributed to the current head.

The completion candidate was built and exercised on both Linux ABIs in an
isolated VM. Consolidation preserves its production source, apart from
changed-line whitespace, and adds the previously separate regression tests
and this guide. The full VM sessions preceded commit packaging; they were
not rerun simply because commit identities changed.

On the consolidated checkout, the six standalone wire cases and five calendar
cases passed on each ABI; the three audit-generation tests passed and the
string-search scanner reported zero narrowing sites. Mission-list cases need
the matching VM libraries and retain the earlier nine-check result below.

- Full server builds passed on both ABIs. Oracle tests passed 62 binding,
  19 snapshot and 7 cluster-list checks on each ABI. The independently based
  database-binding candidate also passed these checks.
- Eleven Oracle audit fixtures passed. The copied database passed its
  applicable schema contracts. Code-generation checks regenerated the six
  database outputs once each, without modifying the source tree.
- Clock passed 10 checks per ABI; integer parsing matched all 41 expected
  lines. Miff passed 33 fixtures per ABI and matched all 337 shipped corpus
  outputs against an upstream 32-bit reference. Relevant negative controls
  failed as expected.
- Nine wire checks passed per ABI, including mission-list cases linked to
  the candidate server libraries. These are explicit byte/state fixtures,
  not captured historical client traffic. Upstream 32-bit fails the packed-map
  textual decoding case because of its mismatched format specifier; that case
  establishes the corrected contract rather than exact old-binary behavior.
- Each ABI started 84 observed cluster processes, including 38 GameServers.
  An unchanged 32-bit client loaded an existing high-bit-ID account and a new
  character. A named waypoint survived a full 64-bit cluster restart and the
  new character also loaded through the 32-bit server using the same database.
  Login timestamps and movement coordinates changed as expected during use;
  no deterministic whole-database parity claim is made.
- Repeated console-requested shutdowns had no fatal/assertion/segfault text
  in captured output. The lifecycle category log was empty, so individual
  final-save acknowledgements were not independently observed.
- A separate restore matched 81 tables and 62,014 row values, constraint
  semantics and sequence metadata. One redundant physical index differed.
  Invalid loader compilation was corrected. Replaying update 272 twice
  preserved row values, but this was a 272-to-272 rehearsal.

Still required before full migration acceptance: an authentic pre-272,
32-bit-origin database upgrade; broader combat, travel, inventory, currency,
crafting, resources, auction and mail coverage; controlled crash recovery;
and deterministic logical comparisons from separate identical databases.
The restored schema did not host the gameplay session. Rendering artifacts
appeared in one client capture, so visual parity is not claimed.

JNI static-destruction ordering and the console receiver's completion contract
remain separate defects; this PR's shutdown fixes do not claim to settle them.

## Contribution disclosure

This contribution was developed with Claude Code and Codex assistance,
directed by the submitter, with AI-assisted review and recorded build/runtime
checks. Reviewer agreement is not a substitute for those checks. No third-party
library update or client renderer port is included.
