-- SQL script to remove gold objects from a database

whenever sqlerror exit failure rollback
set verify off
set serveroutput on

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
	and table_name <> 'OBJECTS'
	and table_name <> 'OBJECT_LIST'
	and table_name <> 'MLOG$_OBJECTS'
	and table_name <> 'OBJECTS$OEM0'
	and table_name <> 'OBJECT_VARIABLES'
	and table_name not like '%VIEW%';

	loop
		fetch tablelist into tablename;
		exit when tablelist%notfound;
		dbms_output.put_line(tablename);
		--dbms_output.put_line('delete ' || tablename || ' where not exists (select * from objects where objects.object_id = '|| tablename || '.object_id);');
		execute immediate 'delete ' || tablename || ' where object_id < &&min_running_object_id';
	end loop;
	close tablelist;
end;
/

delete objects
where object_id < &&min_running_object_id;

delete object_variables
where object_id < &&min_running_object_id;

commit;

@wipe_npcs_no_objvar
