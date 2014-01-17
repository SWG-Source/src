create table temp_players_with_quests as
select c.object_id, 24453256030 quest_id from player_objects p, objects o, objects c where quests like '%2445325630:%' and p.object_id = o.object_id and o.contained_by = c.object_id;

insert into temp_players_with_quests 
select c.object_id, 3338276450 quest_id from player_objects p, objects o, objects c where quests like '%3338276450:%' and p.object_id = o.object_id and o.contained_by = c.object_id;

insert into temp_players_with_quests 
select c.object_id, 3008535013 quest_id from player_objects p, objects o, objects c where quests like '%3008535013:%' and p.object_id = o.object_id and o.contained_by = c.object_id;
