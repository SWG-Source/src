alter table mission_objects drop column target_name_table;
alter table mission_objects drop column target_name_text;

alter table mission_objects add target_name varchar2(127);

update version_number set version_number=104, min_version_number=104;
