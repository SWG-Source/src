whenever sqlerror exit failure
set verify off
set serveroutput on

update objects
set deleted = (select reason_code from delete_reasons where tag='ContainerDeleted')
where deleted = 0
and contained_by <> 0
and player_controlled = 'N'
and not exists
	(select * from objects o2
	where o2.object_id = objects.contained_by);

begin
	loop
		update objects
		set deleted = (select reason_code from delete_reasons where tag='ContainerDeleted')
		where deleted = 0
		and contained_by <> 0
		and player_controlled = 'N'
		and exists 
			(select *
			from objects o2
			where o2.deleted <>0
			and objects.contained_by = o2.object_id);

		exit when sql%rowcount=0;
	end loop;
end;
/

declare
	c number;
begin
	loop
		delete objects where deleted <> 0 and rownum <= 100000;
		c := sql%rowcount;
		commit;
		exit when c=0;
	end loop;
end;
/

delete players
where not exists
	(select *
	from objects
	where objects.object_id = players.character_object);

commit;

delete messages
where not exists (select * from objects where objects.object_id = messages.target);

commit;

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
	and table_name <> 'OBJECT_VARIABLES'
	and table_name <> 'SCRIPTS';

	loop
		fetch tablelist into tablename;
		exit when tablelist%notfound;
		dbms_output.put_line(tablename);
		--dbms_output.put_line('delete ' || tablename || ' where not exists (select * from objects where objects.object_id = '|| tablename || '.object_id);');
		execute immediate 'delete ' || tablename || ' where not exists (select * from objects where objects.object_id = '|| tablename || '.object_id)';

		commit;
	end loop;
	close tablelist;
end;
/

commit;

undefine schema;
set verify on;
exit;
