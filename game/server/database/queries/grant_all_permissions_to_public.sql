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
		execute immediate 'grant select on ' || tablename || ' to public';
	end loop;
	close tablelist;
end;
/

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
		execute immediate 'grant execute on ' || objectname || ' to public';
	end loop;
	close objectlist;
end;
/

grant all on object_list to public;

exit;
