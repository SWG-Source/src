#!/usr/bin/perl
#
# Generates audit.sql, the read-only pre-upgrade audit of the integer
# columns the servers bind as 32-bit values. See README.md.
#
# The column list is derived the way the server derives its bindings:
#
# - Every int/number column of a schema .tab file is bound as
#   DB::BindableInt32 unless the file says otherwise with BIND_AS (the
#   mapping in engine/server/library/codegen/make_schema_h.pl). BIND_AS
#   of DB::BindableUint32 is audited as an unsigned 32-bit column; any
#   other BIND_AS (NetworkId, Int64, Double, ...) and NO_BIND are not
#   32-bit bindings and are skipped. NO_IMPORT tables are bound by
#   hand-written queries; their .tab files carry BIND_AS for the same
#   reason.
# - A persisted member narrower than its 32-bit column (package_data.txt,
#   the list in make_packages.pl's isNarrowInteger) must also fit the
#   member's type, because the encoder refuses a value that does not. A
#   NULL there is reported for information only: it loads as the value a
#   new object gets.
#
# Usage: perl make_audit.pl > audit.sql   (run from this directory)

use strict;
use warnings;
use File::Basename;

my $here = dirname($0);
my $database = "$here/../..";
my $codegen = "$here/../../../../../engine/server/library/codegen";

my @schemaDirectories = ("$database/schema", "$database/login_schema", "$database/sp_character_schema");
my @sourceDirectories = ("$here/../../../../../engine", "$here/../../../../../game");
my $packageData = "$codegen/package_data.txt";
my $makePackages = "$codegen/make_packages.pl";

my %tableSchema;    # table -> game, login, or sp-character
my @entries;        # "TABLE.COLUMN.kind"
my %structToTable;  # generated row struct name -> table, for imported tables
my %columnsOf;      # table -> { column -> 1 }
my %loadRange;      # "table.column" -> 1, for columns annotated LOAD_RANGE
my @nullDefaults;   # "TABLE.COLUMN|what a NULL loads as", for columns annotated NULL_LOADS_AS

# ----------------------------------------------------------------------

sub structNameFor
{
	# The struct name make_schema_h.pl gives an imported table.
	my ($table) = @_;
	my $name = lc($table);
	$name =~ s/s$//;
	$name =~ s/manufacture_inst/manufacture_installation/g;
	$name =~ s/manf_schematic/manufacture_schematic/g;
	$name = join('', map { ucfirst } split(/_/, $name));
	return $name;
}

# ----------------------------------------------------------------------

foreach my $directory (@schemaDirectories)
{
	my $schema = $directory =~ m{/login_schema$} ? "login" : $directory =~ m{/sp_character_schema$} ? "sp-character" : "game";
	foreach my $file (sort glob("$directory/*.tab"))
	{
		open(my $in, '<', $file) or die "$file: $!";
		my $table;
		my $imported = 0;
		my $inColumns = 0;
		while (my $line = <$in>)
		{
			if (!defined $table)
			{
				if ($line =~ /create\s+(?:global\s+temporary\s+)?table\s+"?(\w+)"?/i)
				{
					$table = uc($1);
					die "duplicate table $table in schema inputs" if exists $tableSchema{$table};
					$tableSchema{$table} = $schema;
					# the test make_schema_h.pl applies
					$imported = ($line =~ /create table (\w+)/ && $line !~ /NO_IMPORT/);
					$structToTable{structNameFor($table)} = $table if $imported;
				}
				next;
			}
			if (!$inColumns)
			{
				$inColumns = 1 if ($line =~ /^\s*\(/);
				next;
			}
			last if ($line =~ /^\s*\)/);

			next if ($line =~ /^\s*(constraint|primary\s+key|foreign\s+key)\b/i);
			next unless ($line =~ /^\s*,?\s*"?(\w+)"?\s+(int|integer|number)\b/i);
			my $column = uc($1);
			$columnsOf{$table}{$column} = 1;

			# The loader gives a NULL in this column a value, with a warning.
			if ($line =~ /NULL_LOADS_AS\(([^\)]*)\)/)
			{
				push @nullDefaults, "$table.$column|$1";
			}

			# A hand-written loader narrows this column further (the C++ checks
			# it with DB::checkedNarrow, named "table.column").
			if ($line =~ /--.*LOAD_RANGE\(\s*(-?\d+)\s*,\s*(-?\d+)\s*\)/)
			{
				push @entries, "$table.$column.r:$1:$2";
				$loadRange{lc("$table.$column")} = 1;
			}

			next if ($line =~ /--\s*NO_BIND/);
			my $kind = 'int32';
			if ($line =~ /--\s*BIND_AS\(([^\)]*)\)/)
			{
				my $bindAs = $1;
				$bindAs =~ s/\s+//g;
				if ($bindAs eq 'DB::BindableUint32') { $kind = 'uint32'; }
				elsif ($bindAs eq 'DB::BindableInt32') { $kind = 'int32'; }
				else { next; }
			}
			push @entries, "$table.$column.$kind";
		}
		close($in);
		die "$file: no create table statement" unless defined $table;
	}
}

# ----------------------------------------------------------------------
# Every hand-written DB::checkedNarrow of a column names it "table.column".
# Each such column must carry a LOAD_RANGE annotation, and each annotation
# must have such a check, so the audit and the loaders cannot drift apart.

{
	my %checked;
	my @files;
	foreach my $directory (@sourceDirectories)
	{
		open(my $list, "-|", "find", $directory, "-name", "*.cpp", "-o", "-name", "*.h") or die "find: $!";
		while (my $f = <$list>) { chomp $f; push @files, $f unless $f =~ m{/(3rd|generated)/}; }
		close($list);
	}
	foreach my $file (@files)
	{
		open(my $in, '<', $file) or next;
		local $/;
		my $text = <$in>;
		close($in);
		while ($text =~ /checkedNarrow\s*\([^;]*?,[^;]*?,\s*"([a-z_0-9]+\.[a-z_0-9]+)"/g)
		{
			$checked{$1} = $file;
		}
	}
	foreach my $column (sort keys %checked)
	{
		die "$checked{$column} checks $column with DB::checkedNarrow, but no .tab file gives it a LOAD_RANGE" unless $loadRange{$column};
	}
	foreach my $column (sort keys %loadRange)
	{
		die "$column has a LOAD_RANGE but no DB::checkedNarrow names it" unless $checked{$column};
	}
}

# ----------------------------------------------------------------------
# Narrow persisted members, as make_packages.pl encodes them.

my %narrowKind;
{
	open(my $in, '<', $makePackages) or die "$makePackages: $!";
	local $/;
	my $text = <$in>;
	close($in);
	$text =~ /sub isNarrowInteger\s*\{(.*?)\n\}/s or die "isNarrowInteger not found in $makePackages";
	my $body = $1;
	while ($body =~ /"([\w:]+)"\s*=>\s*1/g)
	{
		my $type = $1;
		my $kind = $type;
		$kind = 'int8' if ($type eq 'Postures::Enumerator'); # typedef int8 Enumerator (Postures.def)
		die "make_audit.pl does not know the range of $type" unless ($kind =~ /^u?int(8|16)$/);
		$narrowKind{$type} = $kind;
	}
}

sub dbIze
{
	# make_packages.pl's member name -> column name mapping.
	my ($name) = @_;
	$name =~ s/m\_//;
	$name =~ s/\s+$//;
	$name =~ s/([a-z])([A-Z])/$1_$2/g;
	$name =~ tr/[A-Z]/[a-z]/;
	$name =~ s/component/cmp/;
	$name =~ s/energy/eng/;
	$name =~ s/hitpoints/hp/;
	$name =~ s/acceleration/acc/;
	$name =~ s/droid_control_device/dcd/;
	$name =~ s/droid_interface_command_speed/droid_if_cmd_speed/;
	$name =~ s/maintenance_requirement/maintenance/;
	return $name;
}

{
	open(my $in, '<', $packageData) or die "$packageData: $!";
	while (my $line = <$in>)
	{
		chomp $line;
		$line =~ s/\#.*//;
		next if ($line =~ /^\s*$/);
		last if ($line =~ /^\s*end\s*$/);
	}
	while (my $line = <$in>)
	{
		chomp $line;
		$line =~ s/\#.*//;
		next if ($line =~ /^\s*$/);
		$line =~ s/\s+/\t/g;
		my ($class, $cName, $package, $datatype, $addCommand, $persistFunction) = split("\t", $line);
		next unless defined $datatype && exists $narrowKind{$datatype};
		# only these packages are persisted by the generated encoder
		$package = 'client' if ($package eq 'authClientServer');
		$package = 'parentClient' if ($package eq 'firstParentAuthClientServer');
		next unless ($package eq 'shared' || $package eq 'server' || $package eq 'client'
			|| ($package eq 'parentClient' && $class eq 'PlayerObject'));
		next if (defined $persistFunction && $persistFunction ne '' && $persistFunction ne '-');

		# the row make_packages.pl encodes the member from
		my $struct;
		if ($class eq 'ServerObject') { $struct = 'Object'; }
		elsif ($class !~ /Object$/) { $struct = $class . 'Object'; }
		else { $struct = $class; }
		my $table = $structToTable{$struct};
		die "no table for $class ($struct)" unless defined $table;
		my $column = uc(dbIze($cName));
		die "$table.$column (member $class.$cName) is not in the schema" unless $columnsOf{$table}{$column};
		push @entries, "$table.$column.$narrowKind{$datatype}";
	}
	close($in);
}

# ----------------------------------------------------------------------

print <<'HEADER';
-- ======================================================================
--
-- audit.sql
--
-- Read-only audit of the integer columns the servers bind as 32-bit
-- values. Run it as the owner of each schema (the game schema and the
-- login schema) before upgrading to servers that bind these columns as
-- DB::BindableInt32 / DB::BindableUint32. It reports:
--
--   - values outside [-2147483648, 2147483647]; loading such a row now
--     fails with ORA-01455 instead of reaching the server truncated;
--   - fractional values, which a 32-bit binding cannot represent;
--   - for columns persisted from a narrower member (int8, uint8, ...),
--     values outside that member's range, which the object encoder
--     refuses. NULLs there are listed as INFO: they load as the value a
--     new object gets, with a warning.
--   - for columns a hand-written loader narrows further (LOAD_RANGE in the
--     .tab file), values outside that range, which fail the load.
--   - as INFO, NULLs in columns whose loader gives a NULL a value
--     (NULL_LOADS_AS in the .tab file), with the value it loads.
--
-- The column list is generated by make_audit.pl from the schema .tab files;
-- do not edit it by hand. Invoke as @audit.sql game (or login, sp-character).
-- Every listed column for the selected schema must exist. The script only reads.
--
-- ======================================================================

whenever oserror exit failure rollback
whenever sqlerror exit failure rollback
set serveroutput on size unlimited
set linesize 250
set trimout on
set feedback off
set verify off

declare
	schema_kind constant varchar2(32) := lower('&1');
	all_entries sys.odcivarchar2list := sys.odcivarchar2list(
HEADER

# One table's columns are adjacent, so the audit reads each table once.
my %order;
my $n = 0;
foreach my $entry (@entries)
{
	my ($table) = split(/\./, $entry);
	$order{$table} = $n++ unless exists $order{$table};
}
my @sorted = sort { my ($ta) = split(/\./, $a); my ($tb) = split(/\./, $b); $order{$ta} <=> $order{$tb} } @entries;
print join(",\n", map { my ($table) = split(/\./, $_); "\t\t'$tableSchema{$table}.$_'" } @sorted), "\n";
print "\t);\n";
print "\n";
print "\t-- columns whose loader gives a NULL a value (NULL_LOADS_AS in the .tab files)\n";
print "\tnull_defaults sys.odcivarchar2list := sys.odcivarchar2list(\n";
print join(",\n", map { my ($table) = split(/\./, $_); my $e = "$tableSchema{$table}.$_"; $e =~ s/'/''/g; "\t\t'$e'" } @nullDefaults), "\n";

print <<'BODY';
	);

	entries     sys.odcivarchar2list := sys.odcivarchar2list();
	type name_list is table of varchar2(128);
	cols        name_list := name_list();
	kinds       name_list := name_list();
	tab         varchar2(128);
	sql_text    varchar2(32767);
	cursor_id   integer := null;
	ignore      integer;
	row_count   number;
	value       number;
	checked     integer := 0;
	missing     integer := 0;
	problems    integer := 0;
	infos       integer := 0;
	tables_seen integer := 0;
	i           integer;
	lo          number;
	hi          number;
	field       integer;

	procedure bounds(kind varchar2, lo out number, hi out number) is
	begin
		if kind like 'r:%' then
			lo := to_number(substr(kind, 3, instr(kind, ':', 3) - 3));
			hi := to_number(substr(kind, instr(kind, ':', 3) + 1));
		elsif kind in ('int32', 'uint32') then lo := -2147483648; hi := 2147483647;
		elsif kind = 'int16' then lo := -32768; hi := 32767;
		elsif kind = 'uint16' then lo := 0; hi := 65535;
		elsif kind = 'int8' then lo := -128; hi := 127;
		elsif kind = 'uint8' then lo := 0; hi := 255;
		else raise_application_error(-20001, 'unknown kind ' || kind);
		end if;
	end;

	-- a persisted member narrower than its column (int8, uint8, ...)
	function narrow(kind varchar2) return boolean is
	begin
		return kind not in ('int32', 'uint32') and kind not like 'r:%';
	end;

	function table_of(entry varchar2) return varchar2 is
	begin
		return substr(entry, 1, instr(entry, '.') - 1);
	end;

	procedure problem(text varchar2) is
	begin
		problems := problems + 1;
		dbms_output.put_line('PROBLEM ' || text);
	end;

begin
	if schema_kind not in ('game', 'login', 'sp-character') then
		raise_application_error(-20002, 'Expected schema argument: game, login, or sp-character');
	end if;
	for k in 1 .. all_entries.count loop
		if substr(all_entries(k), 1, instr(all_entries(k), '.') - 1) = schema_kind then
			entries.extend;
			entries(entries.count) := substr(all_entries(k), instr(all_entries(k), '.') + 1);
		end if;
	end loop;
	if entries.count = 0 then
		raise_application_error(-20003, 'No audit contract for selected schema');
	end if;
	dbms_output.put_line('Auditing ' || schema_kind || ' schema owned by ' || user);
	i := 1;
	while i <= entries.count loop
		tab := table_of(entries(i));
		cols.delete;
		kinds.delete;
		while i <= entries.count and table_of(entries(i)) = tab loop
			declare
				rest varchar2(256) := substr(entries(i), instr(entries(i), '.') + 1);
				col  varchar2(128) := substr(rest, 1, instr(rest, '.') - 1);
				kind varchar2(64)  := substr(rest, instr(rest, '.') + 1);
				n    integer;
			begin
				select count(*) into n from user_tab_columns where table_name = tab and column_name = col;
				if n = 0 then
					missing := missing + 1;
					problem('missing required column ' || tab || '.' || col);
				else
					cols.extend; cols(cols.count) := col;
					kinds.extend; kinds(kinds.count) := kind;
				end if;
			end;
			i := i + 1;
		end loop;

		if cols.count > 0 then
			tables_seen := tables_seen + 1;
			sql_text := 'select count(*)';
			for c in 1 .. cols.count loop
				bounds(kinds(c), lo, hi);
				sql_text := sql_text
					|| ', sum(case when "' || cols(c) || '" < ' || lo || ' or "' || cols(c) || '" > ' || hi || ' then 1 else 0 end)'
					|| ', sum(case when "' || cols(c) || '" <> trunc("' || cols(c) || '") then 1 else 0 end)'
					|| ', min("' || cols(c) || '"), max("' || cols(c) || '")'
					|| ', sum(case when "' || cols(c) || '" is null then 1 else 0 end)';
			end loop;
			sql_text := sql_text || ' from "' || tab || '"';

			cursor_id := dbms_sql.open_cursor;
			dbms_sql.parse(cursor_id, sql_text, dbms_sql.native);
			for f in 1 .. 1 + 5 * cols.count loop
				dbms_sql.define_column(cursor_id, f, value);
			end loop;
			ignore := dbms_sql.execute(cursor_id);
			ignore := dbms_sql.fetch_rows(cursor_id);
			dbms_sql.column_value(cursor_id, 1, row_count);

			for c in 1 .. cols.count loop
				checked := checked + 1;
				bounds(kinds(c), lo, hi);
				field := 2 + 5 * (c - 1);
				declare
					out_of_range number; fractional number; min_v number; max_v number; nulls number;
				begin
					dbms_sql.column_value(cursor_id, field, out_of_range);
					dbms_sql.column_value(cursor_id, field + 1, fractional);
					dbms_sql.column_value(cursor_id, field + 2, min_v);
					dbms_sql.column_value(cursor_id, field + 3, max_v);
					dbms_sql.column_value(cursor_id, field + 4, nulls);
					if nvl(out_of_range, 0) > 0 then
						problem(tab || '.' || cols(c) || ' (' || kinds(c) || '): ' || out_of_range
							|| ' row(s) outside [' || lo || ', ' || hi || '], min ' || min_v || ', max ' || max_v);
					end if;
					if nvl(fractional, 0) > 0 then
						problem(tab || '.' || cols(c) || ' (' || kinds(c) || '): ' || fractional || ' row(s) with a fractional value');
					end if;
					if narrow(kinds(c)) and nvl(nulls, 0) > 0 then
						infos := infos + 1;
						dbms_output.put_line('INFO ' || tab || '.' || cols(c) || ' (' || kinds(c) || '): ' || nulls
							|| ' NULL row(s) will load as the new-object default, with a warning');
					end if;
				end;
			end loop;
			dbms_sql.close_cursor(cursor_id);
		end if;
	end loop;

	-- NULLs a loader replaces with a value: informational
	for k in 1 .. null_defaults.count loop
		declare
			spec varchar2(512) := substr(null_defaults(k), instr(null_defaults(k), '.') + 1);
			col_full varchar2(256) := substr(spec, 1, instr(spec, '|') - 1);
			what varchar2(256) := substr(spec, instr(spec, '|') + 1);
			t varchar2(128) := substr(col_full, 1, instr(col_full, '.') - 1);
			c varchar2(128) := substr(col_full, instr(col_full, '.') + 1);
			n integer;
			nulls number;
		begin
			select count(*) into n from user_tab_columns where table_name = t and column_name = c;
			if substr(null_defaults(k), 1, instr(null_defaults(k), '.') - 1) = schema_kind and n > 0 then
				execute immediate 'select count(*) from "' || t || '" where "' || c || '" is null' into nulls;
				if nulls > 0 then
					infos := infos + 1;
					dbms_output.put_line('INFO ' || col_full || ': ' || nulls || ' NULL row(s) will load as ' || what || ', with a warning');
				end if;
			end if;
		end;
	end loop;

	dbms_output.put_line('int32 audit: ' || checked || ' column(s) in ' || tables_seen || ' table(s) checked, '
		|| missing || ' required column check(s) missing, ' || problems || ' problem(s), ' || infos || ' informational');
	if problems = 0 then
		dbms_output.put_line('RESULT: CLEAN (' || schema_kind || ', complete integer-column coverage)');
	else
		dbms_output.put_line('RESULT: FAILED: ' || problems || ' PROBLEM(S). Correct schema coverage and data before upgrading; nothing was changed.');
		raise_application_error(-20004, 'Integer audit failed: ' || problems || ' problem(s)');
	end if;
exception
	when others then
		if cursor_id is not null and dbms_sql.is_open(cursor_id) then
			dbms_sql.close_cursor(cursor_id);
		end if;
		raise;
end;
/
BODY
