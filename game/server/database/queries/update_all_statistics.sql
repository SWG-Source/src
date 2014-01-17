whenever sqlerror exit failure
set serveroutput on

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
exit
