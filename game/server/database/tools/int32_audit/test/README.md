# 32-bit database binding tests

Three programs that exercise the 32-bit bindings against a real Oracle
schema. None is built by default.

| Target | What it checks |
| --- | --- |
| `int32_db_binding_test` | checked conversions; unsigned values stored in signed form and read back bit for bit (CRCs, a call time of 0x80000000); NULL varray elements; fixed-width varray pushes (compile-time); bind lengths; array fetches, including a short final batch; a value outside int32 failing the load with ORA-01455, unclamped; a NULL fetched into a reused row not keeping the previous batch's value |
| `int32_snapshot_send_test` | `SwgSnapshot` with the generated encoders: a rank of 256 or a posture of 128 rejects the whole snapshot before anything is sent and keeps no partial output; NULL rank and posture encode exactly as the new-creature defaults |
| `int32_cluster_list_test` | `TaskGetClusterList`: a cluster whose port does not fit uint16 is skipped with a logged error and the other clusters are still served; a NULL port uses the configured port |

## Building

From a configured build directory:

    make int32_db_binding_test int32_snapshot_send_test int32_cluster_list_test

The snapshot and cluster tests compile the SwgDatabaseServer and
LoginServer sources again, without their `main()`.

## Running

Use a scratch schema created with the normal database build scripts
(tables, types and packages). Never point the tests at a live schema. The
connection comes only from the environment; there is no default:

    export SWG_INT32_TEST_DSN=//dbhost/service
    export SWG_INT32_TEST_USER=scratch_owner
    export SWG_INT32_TEST_PASSWORD=...
    ./int32_db_binding_test && ./int32_snapshot_send_test && ./int32_cluster_list_test

Each test prints one `PASS` or `FAIL` line per check and exits non-zero
on a failure. The rows it seeds (ids 770000001 and up, clusters 91 to 93)
are written in a transaction that is rolled back. `int32_db_binding_test`
also creates and drops a scratch table and function named `int32test_*`.
Some checks deliberately provoke errors; the `DatabaseError` lines they
log are expected.

Run the tests on both the 64-bit and the 32-bit build.

## Negative controls

Each of these reverts a fix and must make the named check fail. Rebuild
and rerun the test after each one, then restore the code.

- In `OciQueryImplementation.cpp`, make `postProcessResults()` loop to
  `m_numElements` instead of `rowsFetched`: `int32_db_binding_test` fails
  "short final batch post-processed only its 2 rows".
- In `DbBindableBase.cpp`, remove the `clearValue()` call from
  `Bindable::setNull()`: `int32_db_binding_test` fails the check that a
  NULL in a reused row does not keep the previous batch's value.
- In the generated encoder (`make_packages.pl`), drop the line that sets
  `m_encodeRejected`: `int32_snapshot_send_test` fails "prepareSend
  rejects the snapshot".
- In `TaskGetClusterList.cpp`, replace the `continue` for an out-of-range
  port with `return false`: `int32_cluster_list_test` fails.
