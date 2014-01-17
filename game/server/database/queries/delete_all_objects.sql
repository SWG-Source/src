whenever sqlerror exit failure
set verify off
set serveroutput on

truncate table players;
truncate table swg_characters;
truncate table messages;

declare
	type curtype is ref cursor;
	tablelist curtype;
	tablename varchar2(200);
begin
	dbms_output.enable(20000);

	open tablelist for
	select distinct table_name
	from user_tab_columns
	where column_name = 'OBJECT_ID'
	and table_name <> 'OBJECTS';

	loop
		fetch tablelist into tablename;
		exit when tablelist%notfound;
		dbms_output.put_line(tablename);
		--dbms_output.put_line('delete ' || tablename || ' where not exists (select * from objects where objects.object_id = '|| tablename || '.object_id);');
		execute immediate 'truncate table ' || tablename ;
	end loop;
	close tablelist;
end;
/

truncate table objects;

exec dbms_output.put_line('If you are using a shared login server database, remember to delete the characters from the login server database.');

set verify on;
exit;
