col skill_count format 999999
col skill_title format a50
set heading off
set feedback off
set trimspool on
set linesize 200


SELECT skill_title, COUNT(*) skill_count
FROM player_objects po
GROUP BY skill_title;
