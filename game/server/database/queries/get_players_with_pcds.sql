--------------------------------------------------------------------------------
-- get_players_with_pcds.sql
-- Copyright 2007, Sony Online Entertainment LLC
--
-- This query will list information on all player's characters that have a PCD.
--   The purpose is for QA to do conversion testing for Chapter 6 beast master changes.
-- 
-- Note: Make sure the object_template_ids table is as update to date as possible.
--       You can update it as serverop on the command host using:
-- cd /tmp
-- p4 print //depot/swg/test/dsrc/sku.0/sys.server/built/game/misc/object_template_crc_string_table.tab > t.tab
-- perl /swg/sandbox/sandbox-01/test/src/game/server/database/templates/processTemplateList.pl < t.tab > t.sql
-- sqlplus username/password@dsn @t.sql
-- rm t.*
--------------------------------------------------------------------------------

-- all the crc's and name of pet control devices
create table pcd_ids_temp as select id, name from object_templates where name like 'object/intangible/pet/%';

-- creature objects (load_with) and pcd names for all pcds
create table users_with_pcds_temp as select distinct load_with, p.name
  from objects o, pcd_ids_temp p where o.object_template_id = p.id;

-- player objects and creature objects for all players that have pcds
create table players_with_pcds_temp as select distinct object_id, o.load_with 
  from objects o, users_with_pcds_temp u where o.object_template_id = -640104330 and o.load_with = u.load_with;

-- station id and character id
create table results_temp as select station_id, p1.object_id
  from players_with_pcds_temp p1, player_objects p2 where p1.object_id = p2.object_id;

set heading OFF
set numwidth 12
set linesize 256
set newpage NONE
set feedback OFF

col character_full_name format a40
col name format a96

-- station id, character id, character full name, pcd name
select distinct r.station_id, p.character_object, p.character_full_name, u.name
  from results_temp r, players p, users_with_pcds_temp u
 where r.station_id = p.station_id
   and u.load_with = p.character_object
       order by r.station_id, p.character_object;

-- clean up
drop table pcd_ids_temp;
drop table users_with_pcds_temp;
drop table players_with_pcds_temp;
drop table results_temp;

exit
