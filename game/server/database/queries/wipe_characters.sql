whenever sqlerror exit failure rollback
set verify off

update objects set deleted = 1 where player_controlled='Y' and deleted = 0;
delete from players;
delete from login_server.swg_characters where cluster_id = (select id from cluster_list where name = '&&cluster_name');
delete from swg_characters where cluster_id = (select id from cluster_list where name = '&&cluster_name');

commit;

UNDEFINE cluster_name
set verify on
