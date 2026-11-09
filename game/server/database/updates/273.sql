-- 273: store resource_types.depleted_timestamp in the signed 32-bit form.
--
-- Update 272 converted the unsigned 32-bit columns that earlier 64-bit
-- builds stored positive, but missed this one. 64-bit builds before
-- September 2023 bound depleted_timestamp as a 64-bit integer, so the
-- "never depletes" value of recycled and permanent resource types
-- (0xFFFFFFFF) was stored as 4294967295 instead of -1. The column is now
-- read as a 4-byte integer, which fails on those rows (ORA-01455) and stops
-- the universe from loading. Current builds write -1; this update converts
-- the rows written by the earlier 64-bit builds.
--
-- On a database only ever used by 32-bit servers, or created after
-- September 2023, there is nothing in that range and this update changes
-- nothing. It is idempotent.
--
-- market_auctions.object_template_id is not converted: every build wrote it
-- from a signed int, so its 64-bit bind already stored the signed form.
--
-- As in 272, the update is one transaction together with the version
-- change: on any error nothing is changed, including version_number.
-- min_version_number stays at 272, because 272-level servers read and write
-- this column in the signed form already.

whenever sqlerror exit failure rollback

declare
	has_version_table number;

	procedure normalise(p_table in varchar2, p_column in varchar2)
	is
		n number;
	begin
		select count(*) into n
		from user_tab_columns
		where table_name = upper(p_table) and column_name = upper(p_column);

		if n = 0 then
			return;
		end if;

		execute immediate
			'update ' || p_table || ' set ' || p_column || ' = ' || p_column || ' - 4294967296' ||
			' where ' || p_column || ' between 2147483648 and 4294967295';
	end;
begin
	normalise('resource_types', 'depleted_timestamp');

	select count(*) into has_version_table from user_tables where table_name = 'VERSION_NUMBER';
	if has_version_table > 0 then
		execute immediate 'update version_number set version_number=273';
	end if;
	commit;
exception
	when others then
		rollback;
		raise;
end;
/
