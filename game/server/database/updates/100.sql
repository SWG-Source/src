update objects set deleted = 1 where deleted = 0 and exists (select * from mission_board_objects b where objects.object_id = b.object_id);
update objects set deleted = 1 where deleted = 0 and exists (select * from mission_data_objects b where objects.object_id = b.object_id);
update objects set deleted = 1 where deleted = 0 and exists (select * from mission_list_entry_objects b where objects.object_id = b.object_id);

drop table mission_board_objects;
drop table mission_data_objects;
drop table mission_list_entry_objects;

update objects set deleted = 1 where deleted = 0 and exists (select * from mission_objects b where objects.object_id = b.object_id);
truncate table mission_objects;

alter table mission_objects drop column mission_data_id;
alter table mission_objects drop column mission_holder_id;

alter table mission_objects add difficulty int;
alter table mission_objects add end_x float;
alter table mission_objects add end_y float;
alter table mission_objects add end_z float;
alter table mission_objects add end_cell number(20);
alter table mission_objects add end_scene int;
alter table mission_objects add mission_creator varchar2(127);
alter table mission_objects add reward int;
alter table mission_objects add root_script_name varchar2(100);
alter table mission_objects add start_x float;
alter table mission_objects add start_y float;
alter table mission_objects add start_z float;
alter table mission_objects add start_cell number(20);
alter table mission_objects add start_scene int;
alter table mission_objects add target number(20);
alter table mission_objects add description_table varchar2(100);
alter table mission_objects add description_text varchar2(100);
alter table mission_objects add title_table varchar2(100);
alter table mission_objects add title_text varchar2(100);
alter table mission_objects add mission_holder_id number(20);
alter table mission_objects add status int;
alter table mission_objects add mission_type int;

update version_number set version_number=100, min_version_number=100;
