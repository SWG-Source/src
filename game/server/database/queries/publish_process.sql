-- SQL script for the publish process

whenever sqlerror exit failure rollback
set verify off
set serveroutput on

-- save player locations
delete publish_player_fixup;

declare
	type curtype is ref cursor;
	playerlist curtype;
	player_id number;
	topmost_container number;
	new_x number;
	new_y number;
	new_z number;
begin
	open playerlist for
	select object_id
	from objects o, players p
	where object_id >= &&min_running_object_id
	and player_controlled = 'Y'
	and deleted = 0
	and contained_by <> 0
	and object_id = character_object;

	loop
		begin
			fetch playerlist into player_id;
			exit when playerlist%notfound;
		
			select x,y,z
			into new_x, new_y, new_z
			from objects
			where contained_by = 0
			start with object_id = player_id
			connect by prior contained_by = object_id;

			insert into publish_player_fixup
			values (player_id, new_x, new_y, new_z);

		exception
			when no_data_found then
				-- put the player at origin (best we can do)
				insert into publish_player_fixup
				values (player_id, 0,0,0);
			when others then
				dbms_output.put_line('Player '|| player_id || ' is contained by his own inventory');
				insert into publish_player_fixup
				values (player_id, 0,0,0);		
		end;		
	end loop;
	close playerlist;
end;
/

-- delete objects that will be replaced by the publish

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

insert into objects
select * 
from &&source_schema..objects
where object_id < &&min_running_object_id;

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
	and table_name not like '%$%'
	and table_name <> 'MLOG$_OBJECTS'
	and table_name <> 'OBJECTS$OEM0'
	and table_name <> 'RUPD$_OBJECTS'
	and table_name <> 'OBJECT_VARIABLES'
	and table_name not like '%VIEW%';

	loop
		fetch tablelist into tablename;
		exit when tablelist%notfound;
		dbms_output.put_line(tablename);
		--dbms_output.put_line('delete ' || tablename || ' where not exists (select * from objects where objects.object_id = '|| tablename || '.object_id);');
		execute immediate 'insert into ' || tablename || ' select * from &&source_schema..' || tablename || ' where object_id < &&min_running_object_id';
	end loop;
	close tablelist;
end;
/

-- copy object variables

delete object_variables
where object_id < &&min_running_object_id;

begin
for x in (select distinct name
	from &&source_schema..object_variable_names n1
	where not exists (select 1 from object_variable_names n2 where n1.name = n2.name))
loop
	insert into object_variable_names (id,name)
	values (next_object_variable_name.nextval,x.name);
end loop;
end;
/	

insert into object_variables
select object_id,dn.id,type,value,sv.detached
from &&source_schema..object_variables sv, &&source_schema..object_variable_names sn, object_variable_names dn
where sv.name_id = sn.id
and (sv.detached = 0 or sv.detached is null)
and sn.name = dn.name;

-- delete all universe objects that were copied

update objects
set deleted = (select reason_code from delete_reasons where tag='Publish')
where deleted = 0
and object_id in 
	(select object_id
	from universe_objects)
and object_id < &&min_running_object_id;

-- fix any players that are now in deleted buildings

update objects
set x=(select x from publish_player_fixup where player_object_id = object_id),
	y=(select y from publish_player_fixup where player_object_id = object_id),
	z=(select z from publish_player_fixup where player_object_id = object_id),
	contained_by = 0
where object_id in
	(select o1.object_id
	from objects o1, objects o2, players p
	where o1.deleted = 0
	and o1.object_id >= &&min_running_object_id
	and o1.player_controlled = 'Y'
	and o1.object_id = p.character_object -- to help it find them faster
	and o1.contained_by = o2.object_id (+)
	and nvl(o2.deleted,1) <> 0)
and contained_by <> 0;

commit;

@wipe_npcs_no_objvar
