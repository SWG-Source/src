whenever sqlerror exit failure
set verify off
set serveroutput on

declare
	source_version number;
	dest_version number;
begin
	select version_number
	into dest_version
	from version_number;

	select version_number
	into source_version
	from &&schema..version_number;

	if (source_version <> dest_version) then
		raise_application_error(-20000,'The database versions do not match.');
	end if;
end;
/

declare
	type curtype is ref cursor;
	tablelist curtype;
	tablename varchar2(200);
begin
	open tablelist for
	select table_name
	from user_tables
	where table_name <> 'VERSION_NUMBER'
	and table_name not like '%SYS%';

	loop
		fetch tablelist into tablename;
		exit when tablelist%notfound;
		dbms_output.put_line(tablename);
		execute immediate 'truncate table ' || tablename;
		execute immediate 'insert into ' || tablename || ' select * from &&schema..' || tablename;
		commit;
	end loop;
	close tablelist;
end;
/

commit;

update objects
set deleted = (select reason_code from delete_reasons where tag='publish')
where deleted = 0
and exists (select * from universe_objects where universe_objects.object_id = objects.object_id);

commit;

insert into free_object_ids
select * from 
	(select max(object_id+1),NULL
	from objects)
where not exists (select * from free_object_ids where end_id is null);

commit;

declare
	type curtype is ref cursor;
	object_list curtype;
	object_name varchar2(200);
begin
	dbms_output.enable(20000);

	open object_list for
	select object_name
	from user_objects
	where object_type = 'TABLE'
	and object_name not like '%SYS%';

	loop
		fetch object_list into object_name;
		exit when object_list%notfound;
		dbms_output.put_line(object_name);
		execute immediate 'analyze table ' || object_name || ' compute statistics ';
	end loop;
	close object_list;
end;
/

drop sequence next_object_variable_name;

declare 
	max_name number;
begin
	select nvl(max(id)+1,1) into max_name from object_variable_names;
	execute immediate 'create sequence next_object_variable_name start with ' || max_name;
end;
/

undefine schema;
set verify on;
exit
