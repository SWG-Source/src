set verify off

delete preload_list where city_server_id > &&number_of_servers;
update preload_list set wilderness_server_id = city_server_id where wilderness_server_id != city_server_id;

undefine number_of_servers
