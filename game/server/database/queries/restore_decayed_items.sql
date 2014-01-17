-- list all the items we're going to have to deal with
create table temp_items
as select o.object_id, owner_id from objects o, tangible_objects t where o.object_id = t.object_id and deleted = 2;

create table temp_owners
as select distinct owner_id, owner_id inventory, owner_id bag from temp_items;

-- figure out the inventory objects for these
update temp_owners
set inventory = admin.get_inventory_for_player(owner_id);

-- delete ones where we couldn't get the inventory
delete temp_owners
where inventory = 0;

-- make bags

declare
	bag_id number;
begin
	for x in (select * from temp_owners)
	loop
	
	bag_id := objectidmanager.get_single_id();

	insert into objects (object_id, x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z, node_x, node_y, node_z, type_id, scene_id, controller_type,
		deleted, object_name, volume, contained_by, slot_arrangement, player_controlled, cache_version, load_contents, cash_balance, bank_balance, complexity,
		name_string_table, name_string_text, object_template_id, load_with)
	values
		(bag_id,0,0,0,1,0,0,0,0,0,0,1413566031,'bogus',0,
		0,'Restored Items from Vendors',1,x.inventory,-1,'N',0,'Y',0,0,1,
		'container_name','satchel',-956527269,x.owner_id);

	insert into tangible_objects (object_id, max_hit_points, owner_id, visible, pvp_type, pvp_faction, damage_taken, count, condition, creator_id, source_draft_schematic)
	values (bag_id, 100, x.owner_id, 'Y', 0,0,0,0,256,0,0);

	update temp_owners
	set bag = bag_id
	where owner_id = x.owner_id;

	end loop;
end;
/


-- put all deleted items into the appropriate bag

begin
	for x in (select t.object_id, t.owner_id, o.bag from temp_items t, temp_owners o where t.owner_id = o.owner_id)
	loop
		update objects
		set
			deleted = 0,
			deleted_date = null,
			load_with = x.owner_id,
			contained_by = x.bag
		where object_id = x.object_id;
	end loop;
end;
/


select station_id
from players p, temp_owners o
where p.character_object = o.owner_id;

drop table temp_items;
drop table temp_owners;
