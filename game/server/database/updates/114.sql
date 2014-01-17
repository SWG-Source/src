alter table objects add load_with number(20);

update objects set load_with = object_id where contained_by = 0 and deleted = 0;
update objects set load_with = object_id where player_controlled = 'Y' and deleted = 0;
update objects set load_with = contained_by where deleted = 0 and exists (select * from objects o2 where objects.contained_by = o2.object_id and o2.load_contents='N');

declare
	c number;
begin
	loop
		update objects set load_with = (select load_with from objects o2 where objects.contained_by=o2.object_id and o2.load_with is not null)
			where deleted = 0 and load_with is null and exists
				(select load_with from objects o2 where objects.contained_by=o2.object_id and o2.load_with is not null);
		c := sql%rowcount;
		commit;
		exit when c=0;
	end loop;
end;
/

create index load_with_idx on objects (load_with) tablespace indexes;
drop index container_idx;

update version_number set version_number=114, min_version_number=114;
