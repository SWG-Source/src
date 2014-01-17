alter table mission_objects drop column target;
alter table mission_objects add target_appearance int;
alter table mission_objects add target_name_table varchar2(100);
alter table mission_objects add target_name_text varchar2(100);

update version_number set version_number=101, min_version_number=101;
