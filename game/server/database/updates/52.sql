alter table preload_list add interest_radius int;
alter table preload_list add server_id int;
alter table preload_list add secondary_server_id int;

update version_number set version_number=52, min_version_number=52;
