update objects
set deleted = 1, deleted_date = sysdate, load_with = null
where player_controlled = 'N'
and deleted = 0
and exists (select 1 from creature_objects c where c.object_id = objects.object_id)
and not exists (select 1 from scripts s where s.object_id = objects.object_id and s.script='terminal.vendor')
and script_list not like '%terminal.vendor%';
