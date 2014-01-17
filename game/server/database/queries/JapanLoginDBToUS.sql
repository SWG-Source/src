update cluster_list set id = 40 where id = 10;
update cluster_list set id = 41 where id = 11;

truncate table account_reward_events;
truncate table account_reward_items;

update extra_character_slots set cluster_id = 40 where cluster_id = 10;
update extra_character_slots set cluster_id = 41 where cluster_id = 11;

delete from extra_character_slots where cluster_id not in (40,41);

update swg_characters set cluster_id = 40 where cluster_id = 10;
update swg_characters set cluster_id = 41 where cluster_id = 11;

delete from swg_characters where cluster_id not in (40,41);

update cluster_list set num_characters = 300000 where id in (40, 41);

