set verify off;
declare
	type curtype is ref cursor;
	tablelist curtype;
	tablename varchar2(200);
begin
	open tablelist for
	select table_name
	from user_tables
	where table_name not like '%IOT_OVER%';

	loop
		fetch tablelist into tablename;
		exit when tablelist%notfound;
		execute immediate 'grant select on ' || tablename || ' to &&schema';
	end loop;
	close tablelist;
end;
/

undefine schema;
set verify on;
