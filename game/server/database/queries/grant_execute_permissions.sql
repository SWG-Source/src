set verify off;
declare
	type curtype is ref cursor;
	objectlist curtype;
	objectname varchar2(200);
begin
	open objectlist for 
	select object_name
	from user_objects
	where object_type ='PACKAGE' or object_type = 'TYPE';

	loop
		fetch objectlist into objectname;
		exit when objectlist%notfound;
		execute immediate 'grant execute on ' || objectname || ' to &&schema';
	end loop;
	close objectlist;
end;
/

undefine schema;
set verify on;
