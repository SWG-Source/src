-- fix contents with different scene ids than their parents

begin
	loop
		update objects o1 set scene_id=(select scene_id from objects o2 where o1.contained_by=o2.object_id) where contained_by<>0 and object_id in (select object_id from objects o1 where o1.contained_by<>0 and o1.scene_id != (select scene_id from objects o2 where o2.object_id=o1.contained_by));
		exit when sql%rowcount=0;
	end loop;
end;
/

update version_number set version_number=120, min_version_number=120;

