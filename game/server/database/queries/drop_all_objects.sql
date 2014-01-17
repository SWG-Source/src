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
	where object_type = 'PACKAGE';

	loop
		fetch object_list into object_name;
		exit when object_list%notfound;
		dbms_output.put_line('drop package ' || object_name);
		execute immediate 'drop package ' || object_name;
	end loop;
	close object_list;

	open object_list for
	select object_name
	from user_objects
	where object_type = 'VIEW';

	loop
		fetch object_list into object_name;
		exit when object_list%notfound;
		dbms_output.put_line('drop view ' || object_name);
		execute immediate 'drop view ' || object_name;
	end loop;
	close object_list;

	open object_list for
	select object_name
	from user_objects
	where object_type = 'TABLE'
	and object_name not like '%SYS%';

	loop
		fetch object_list into object_name;
		exit when object_list%notfound;
		dbms_output.put_line('drop table ' || object_name || ' cascade constraints;');
		execute immediate 'drop table ' || object_name || ' cascade constraints';
	end loop;
	close object_list;

	open object_list for
	select object_name
	from user_objects
	where object_type = 'SEQUENCE';

	loop
		fetch object_list into object_name;
		exit when object_list%notfound;
		dbms_output.put_line('drop sequence ' || object_name || ';');
		execute immediate 'drop sequence ' || object_name;
	end loop;
	close object_list;

	open object_list for
	select object_name
	from user_objects
	where object_type = 'TYPE';

	loop
		fetch object_list into object_name;
		exit when object_list%notfound;
		dbms_output.put_line('drop type ' || object_name || ';');
		execute immediate 'drop type ' || object_name;
	end loop;
	close object_list;
end;
/
exit
