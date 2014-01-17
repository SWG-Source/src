create table temp_dates_to_change as
select po.object_id, p.create_time, trunc(p.create_time) - to_date('31-DEC-2000') right_born_date, po.born_date /*+ ORDERED */
from player_objects po, objects o2, objects o, players p
where p.character_object = o.object_id
and o.object_id = o2.contained_by
and o2.object_id = po.object_id
and (po.born_date is null or po.born_date <> trunc(p.create_time) - to_date('31-DEC-2000'));

alter table temp_dates_to_change add primary key (object_id);
analyze table temp_dates_to_change estimate statistics;

update player_objects
set born_date = (select right_born_date from temp_dates_to_change t where t.object_id = player_objects.object_id)
where object_id in (select object_id from temp_dates_to_change);

drop table temp_dates_to_change;
