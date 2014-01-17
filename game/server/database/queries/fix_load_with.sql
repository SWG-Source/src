create table temp_load_with_check as select object_id, contained_by, load_contents, player_controlled, deleted, load_with old_value
from objects;

alter table temp_load_with_check add load_with number;
alter table temp_load_with_check add primary key (object_id);
create index temp_container_idx on temp_load_with_check (contained_by);
	
analyze table temp_load_with_check compute statistics;	
	
update temp_load_with_check set load_with = object_id where contained_by = 0 and deleted = 0 and load_with is null;
update temp_load_with_check set load_with = object_id where player_controlled = 'Y' and deleted = 0 and load_with is null;
update temp_load_with_check set load_with = contained_by where deleted = 0 and load_with is null 
	and exists (select * from temp_load_with_check o2 where temp_load_with_check.contained_by = o2.object_id and o2.load_contents='N');

declare
	c number;
begin
	loop
		update temp_load_with_check set load_with = (select load_with from temp_load_with_check o2 where temp_load_with_check.contained_by=o2.object_id and o2.load_with is not null)
			where deleted = 0 and load_with is null and exists
				(select load_with from temp_load_with_check o2 where temp_load_with_check.contained_by=o2.object_id and o2.load_with is not null);
		c := sql%rowcount;
		commit;
		exit when c=0;
	end loop;
end;
/

commit;

declare
	c number;
begin
	c := 0;

	for x in (select object_id, load_with from temp_load_with_check where old_value <> load_with or (old_value is null and load_with is not null) or (load_with is null and old_value is not null))
	loop
		update objects
		set load_with = x.load_with
		where object_id = x.object_id;
	
		c := c + 1;

		if (c>1000) then
			commit;
			c := 0;
		end if;
	end loop;
end;
/

commit;

drop table temp_load_with_check;
