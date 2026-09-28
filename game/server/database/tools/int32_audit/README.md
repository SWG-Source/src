# 32-bit integer column audit

The servers bind every `int`/`number` column of the schema as a 32-bit
integer: `DB::BindableInt32`, or `DB::BindableUint32` for the columns that
hold CRCs, station ids and other unsigned 32-bit values in their signed
32-bit form. The binding is the same on 32-bit and 64-bit builds.

A stored value that does not fit is never truncated or clamped. Loading the
row fails with ORA-01455, and the server logs the failure. The same applies
to a value that fits the column but not the narrower C++ member it is
persisted from, such as `creature_objects.rank` (uint8). The object encoder
refuses to encode that row. A NULL in such a column is not an error: it
loads as the value a new object gets (rank 0, posture Upright), and the
server logs a warning.

`audit.sql` finds these rows before an upgrade. It only reads.

## Running it

Run it as the owner of each schema, the game schema and the login schema:

    sqlplus /nolog
    -- Connect as the intended schema owner, then choose its contract:
    @audit.sql game
    -- In the login-schema connection:
    @audit.sql login
    -- If the separate character-profile schema is used:
    @audit.sql sp-character

It reports each problem on a `PROBLEM` line:

- values outside [-2147483648, 2147483647];
- fractional values, which a 32-bit binding cannot hold;
- for a column persisted from a narrower member, values outside that
  member's range.

NULLs in those columns are listed on `INFO` lines and are not problems. So are NULLs in the
other columns whose loader gives a NULL a value by the compatibility
policy of this change (waypoint color, message call time), which the
`.tab` file records as `NULL_LOADS_AS(value)`.

The schema argument is required: `game`, `login`, or `sp-character`. Each
selects only the integer columns declared in its corresponding schema source
directory. Every selected column must exist in the connected owner's schema;
a missing table or column is a problem, including on an empty or wrong schema.
The login contract does not require game columns. Extra tables and non-audited
columns are outside this audit's coverage.

A complete contract with no invalid values prints `RESULT: CLEAN` with the
schema kind and coverage boundary. Any data or coverage problem prints
`RESULT: FAILED` and raises an Oracle error. SQLPlus exits nonzero for that
error, other SQL errors, and operating-system errors. Check both its exit
status and the result; never infer success from an empty output file. These
SQLPlus error handlers remain active in the calling session. Invoke the audit
in a dedicated session with no unrelated pending transaction: an error exits
with rollback.

This contract is for the schema version represented by this source revision.
A partially upgraded or intentionally customized schema is not silently
accepted. Review each missing-column report against the deployment's upgrade
sequence; do not bypass it by selecting a different contract. A CLEAN result
covers the declared integer columns and their values, not the whole schema,
SQL packages, application behavior, or successful upgrade.

Fix the reported rows before you start the new servers. The audit does not
suggest values: an out-of-range value is already wrong, and only the
operator knows what it should have been.

## Keeping it exact

`audit.sql` is generated. Do not edit it by hand; regenerate it after
changing a `.tab` file, `package_data.txt` or `make_packages.pl`:

    perl make_audit.pl > audit.sql

`make_audit.pl` uses the rule the server uses to bind each column:

- Each `int`/`number` column of `schema/`, `login_schema/` and
  `sp_character_schema/*.tab` is
  a `DB::BindableInt32`, unless a `-- BIND_AS(...)` comment names another
  type (`make_schema_h.pl` applies the same rule). `BIND_AS(DB::BindableUint32)`
  columns are audited with the same range, in their stored signed form.
  Columns declared `NetworkId`, `Int64`, `Double` and so on, and `NO_BIND`
  columns, are not 32-bit bindings and are skipped.
- `NO_IMPORT` tables are bound by hand-written queries, not by generated
  code. Their `.tab` files carry the same `BIND_AS` comments to record how
  those queries bind each column.
- A column that a hand-written loader narrows further carries
  `-- LOAD_RANGE(lo,hi)` in its `.tab` file (`waypoints.color`,
  `cluster_list.port`, `default_character_slots.character_type_id`) and
  is audited against that range; its NULLs are handled by the loader and
  are not reported. The loader's `DB::checkedNarrow` names the column
  `"table.column"`. `make_audit.pl` fails if a check has no annotation
  or an annotation has no check. This detects missing coverage by column name;
  it does not prove that the annotated bounds equal the C++ destination type.
- Members that `package_data.txt` persists through a narrower type (the
  list in `make_packages.pl`'s `isNarrowInteger`) are also checked against
  that type's range. Their NULLs are counted as informational.

The tests of the bindings themselves are in `test/`.

## Audit regression checks

Run the local generator and contract tests:

    python3 test/audit_generation_test.py

These verify checked-in generation, schema partitioning, representative
contracts, and isolated schema/annotation mutations. They do not execute
PL/SQL or establish Oracle behavior.

Generate runtime cases for a dedicated empty, disposable Oracle schema:

    python3 test/audit_generation_test.py --oracle-fixtures /tmp/int32-audit-cases

Read `CASES.txt` in that directory. The scripts create simplified tables and
one case drops a column; never use them against production or a retained test
database. They exercise empty/wrong/incomplete schemas, each valid schema
contract, invalid values, fractions, and informational NULLs. Run them as
separate SQLPlus sessions and record exit status and output. The real schema
and copied-database upgrade rehearsal remain separate acceptance checks.
