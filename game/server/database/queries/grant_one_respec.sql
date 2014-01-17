update objects
set script_list = script_list || 'systems.respec.grant_single_respec_on_login:'
where script_list not like '%systems.respec.grant_respec_on_login:%'
and object_id in (select character_object from players where create_time < '2005-04-27 04:00');
