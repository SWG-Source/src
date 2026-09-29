#!/usr/bin/env python3
"""Generator tests and optional Oracle fixtures; never connects to a database.

Run normally for local checks. --oracle-fixtures DIRECTORY additionally writes
scripts for a disposable EMPTY schema. Run each case in a separate SQLPlus
session; audit.sql intentionally exits on failure. These fixtures exercise the
SQL audit, not the game's complete DDL or the OCI bindings.
"""
import argparse
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

HERE = Path(__file__).resolve().parent.parent
ROOT = HERE.parents[4]
ENTRY = re.compile(r"^\s*'(game|login|sp-character)\.([A-Z0-9_]+)\.([A-Z0-9_]+)\.([^']+)'[,]?$", re.M)


def generate(root=ROOT):
    return subprocess.run(['perl', str(root / 'game/server/database/tools/int32_audit/make_audit.pl')],
                          check=True, capture_output=True, text=True).stdout


def fixture_root(dest):
    # Minimal real inputs, plus each annotated handwritten loader. This keeps
    # mutation tests isolated from both the checkout and generated artifacts.
    for directory in ('schema', 'login_schema', 'sp_character_schema'):
        shutil.copytree(HERE.parent.parent / directory, dest / 'game/server/database' / directory)
    codegen = Path('engine/server/library/codegen')
    for name in ('package_data.txt', 'make_packages.pl'):
        target = dest / codegen / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ROOT / codegen / name, target)
    target = dest / HERE.relative_to(ROOT) / 'make_audit.pl'
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(HERE / 'make_audit.pl', target)
    for tree in ('engine', 'game'):
        for source in (ROOT / tree).rglob('*.cpp'):
            if '/3rd/' in str(source) or '/generated/' in str(source):
                continue
            text = source.read_text(errors='replace')
            if re.search(r'checkedNarrow\s*\([^;]*?,[^;]*?,\s*"[a-z_0-9]+\.[a-z_0-9]+"', text):
                target = dest / source.relative_to(ROOT)
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, target)


class AuditGeneration(unittest.TestCase):
    def test_checked_in_output(self):
        self.assertEqual(generate(), (HERE / 'audit.sql').read_text())

    def test_schema_partition_and_known_contracts(self):
        entries = set(ENTRY.findall(generate()))
        self.assertIn(('game', 'CREATURE_OBJECTS', 'RANK', 'uint8'), entries)
        self.assertIn(('game', 'WAYPOINTS', 'COLOR', 'r:0:255'), entries)
        self.assertIn(('login', 'CLUSTER_LIST', 'PORT', 'r:0:65535'), entries)
        self.assertIn(('sp-character', 'CHARACTER_PROFILE', 'STATION_ID', 'uint32'), entries)
        self.assertFalse(any(schema == 'login' and table == 'CREATURE_OBJECTS' for schema, table, _, _ in entries))
        self.assertFalse(any(table == 'CHARACTER_PROFILE' and column == 'CASH_BALANCE' for _, table, column, _ in entries))
        self.assertEqual({e[0] for e in entries}, {'game', 'login', 'sp-character'})

    def test_schema_change_updates_contract_and_annotation_drift_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            fixture_root(root)
            added = root / 'game/server/database/login_schema/audit_fixture.tab'
            added.write_text('create table audit_fixture -- NO_IMPORT\n(\n counter number,\n name varchar2(40),\n object_id number -- BIND_AS(DB::BindableNetworkId)\n);\n')
            entries = set(ENTRY.findall(generate(root)))
            self.assertIn(('login', 'AUDIT_FIXTURE', 'COUNTER', 'int32'), entries)
            self.assertFalse(any(e[1] == 'AUDIT_FIXTURE' and e[2] != 'COUNTER' for e in entries))
            added.write_text(added.read_text().replace('counter number,', 'counter number, -- LOAD_RANGE(0,7)'))
            with self.assertRaises(subprocess.CalledProcessError) as result:
                generate(root)
            self.assertIn('has a LOAD_RANGE but no DB::checkedNarrow', result.exception.stderr)


def oracle_fixtures(destination):
    destination.mkdir(parents=True, exist_ok=False)
    shutil.copy2(HERE / 'audit.sql', destination / 'audit.sql')
    entries = ENTRY.findall(generate())
    for schema in ('game', 'login', 'sp-character'):
        columns = {}
        for kind, table, column, _ in entries:
            if kind == schema:
                columns.setdefault(table, set()).add(column)
        setup = 'whenever sqlerror exit failure rollback\n'
        setup += '-- DESTRUCTIVE TEST FIXTURE: use a dedicated EMPTY disposable schema.\n'
        for table, names in sorted(columns.items()):
            setup += 'create table "{}" ({});\n'.format(table, ', '.join('"{}" number'.format(c) for c in sorted(names)))
        (destination / ('setup-' + schema + '.sql')).write_text(setup + 'exit success\n')
    cases = {
        'empty-game': '@audit.sql game\n',
        'empty-login': '@audit.sql login\n',
        'invalid-kind': '@audit.sql wrong\n',
        'clean-game': '@audit.sql game\n',
        'clean-login': '@audit.sql login\n',
        'clean-sp-character': '@audit.sql sp-character\n',
        'wrong-schema': '@audit.sql login\n',
        'missing-column': 'alter table creature_objects drop column rank;\n@audit.sql game\n',
        'out-of-range': 'insert into creature_objects (rank) values (256);\n@audit.sql game\n',
        'fraction': 'insert into creature_objects (rank) values (0.5);\n@audit.sql game\n',
        'null-info': 'insert into creature_objects (rank, posture) values (null, null);\n@audit.sql game\n',
    }
    for name, sql in cases.items():
        (destination / (name + '.sql')).write_text('whenever sqlerror exit failure rollback\n' + sql + 'rollback;\nexit success\n')
    (destination / 'CASES.txt').write_text('''Run from this directory, using SQLPlus sessions connected to disposable schema owners.
Fresh empty schema: empty-game, empty-login, invalid-kind => nonzero, no CLEAN.
After setup-game: clean-game, null-info => zero and CLEAN (null-info also INFO).
After setup-login: clean-login => zero and CLEAN; no game columns required.
After setup-sp-character: clean-sp-character => zero and CLEAN.
After setup-game: wrong-schema => nonzero, missing login columns, no CLEAN.
After setup-game: out-of-range, fraction => nonzero and PROBLEM; audit rolls back.
After setup-game LAST: missing-column => nonzero and missing required column.
missing-column commits DDL; destroy/recreate the disposable schema afterwards.
SQLPlus invocation: sqlplus -L -S /nolog; connect securely, then @CASE.sql.
These tests prove audit control flow, not fidelity of production schema/data.
''')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle-fixtures', type=Path)
    args = parser.parse_args()
    result = unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(AuditGeneration))
    if not result.wasSuccessful():
        raise SystemExit(1)
    if args.oracle_fixtures:
        oracle_fixtures(args.oracle_fixtures)
        print('Oracle fixtures written to', args.oracle_fixtures)
