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

select count(*) from temp_load_with_check where old_value <> load_with;

/* select v.object_id, v.contained_by, v.deleted, v.load_with, v.load_contents, v.object_template from objects_view v, 
	(select level l, object_id from objects start with object_id = 9999999 connect by prior object_id = contained_by and deleted = 0) subq
	where v.object_id = subq.object_id
	order by l,contained_by;  */

-- drop table temp_load_with_check;
