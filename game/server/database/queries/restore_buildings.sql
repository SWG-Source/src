begin
for x in 
	(select o.object_id, o.deleted_date from objects o, building_objects b
	where deleted=7
	and deleted_date is not null 
	and o.object_id = b.object_id
	and o.bank_balance > 7500)
loop

	update objects set bank_balance = bank_balance + 5000
	where object_id = x.object_id;
	
	for y in
		(select object_id
		from objects
		where deleted = 7 and abs(objects.deleted_date - x.deleted_date) < (10/1440)
		start with objects.object_id = x.object_id
		connect by prior object_id = contained_by
		and player_controlled = 'N')
	loop
		update objects
		set deleted_date = null, deleted = 0, load_with = x.object_id
		where object_id = y.object_id;
	end loop;		
end loop;
end;
