set verify off
insert into cluster_list (id, name, num_characters, address, secret, locked, not_recommended)
values ((select nvl(max(id)+1,1) from cluster_list), '&cluster_name', 0, '&host', 'N', 'N', 'N');
col name format a20
col address format a20
select id,name,address from cluster_list;
exit;
