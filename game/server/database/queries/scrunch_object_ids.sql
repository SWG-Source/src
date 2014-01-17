-- whenever sqlerror exit failure
set verify off
set serveroutput on

delete scruncher;

insert into scruncher
select rownum new_object_id, object_id old_object_id
from (select object_id from objects
	union select message_id from messages)
order by object_id;

insert into scruncher values (0,0);

commit;

update players
set character_object = (select new_object_id
	from scruncher
	where players.character_object = scruncher.old_object_id);

update messages
set message_id = (select new_object_id
	from scruncher
	where messages.message_id = scruncher.old_object_id),
target = (select new_object_id
	from scruncher
	where messages.target = scruncher.old_object_id);

update objects
set contained_by = (select new_object_id
	from scruncher
	where contained_by = scruncher.old_object_id);

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
	and table_name <> 'SCRUNCHER';

	loop
		fetch tablelist into tablename;
		exit when tablelist%notfound;
		dbms_output.put_line(tablename);
		execute immediate 'update ' || tablename || ' set object_id = (select new_object_id from scruncher where ' || tablename || '.object_id = scruncher.old_object_id)';
	end loop;
	close tablelist;
end;
/

delete free_object_ids;

insert into free_object_ids (start_id)
select max(object_id)+1 from objects;
