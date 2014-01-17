update objects set deleted = 6, deleted_date = sysdate, load_with = NULL
	where object_id in (
		select i.object_id from tangible_objects t, objects owner, objects obj, installation_objects i
		where t.owner_id = owner.object_id
		and owner.deleted <> 0
		and t.object_id = i.object_id
		and t.object_id = obj.object_id
		and obj.deleted = 0);
