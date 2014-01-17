col novice format 999999
col master format 999999
col skill format a35
set heading off
set feedback off
set trimspool on
set linesize 200

select 'num_players_login_last_90_days' as name, count(*) as num_players_login_last_90_days
from players where last_login_time > sysdate-90;

select novice.skill as skill, novice.skill_count as novice, master.skill_count as master
from 
( select substr(value,1,length(value)-7) as skill, count(*) skill_count
from property_lists pl, players p
where p.last_login_time > sysdate-90
and p.character_object  = pl.object_id
and pl.list_id = 11
and pl.value like '%_novice'
group by substr(value,1,length(value)-7) ) novice,
( select substr(value,1,length(value)-7) as skill, count(*) skill_count
from property_lists pl, players p
where p.last_login_time > sysdate-90
and p.character_object  = pl.object_id
and pl.list_id = 11
and pl.value like '%_master'
group by substr(value,1,length(value)-7) ) master
where novice.skill = master.skill 
order by novice.skill;