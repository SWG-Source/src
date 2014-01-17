alter table waypoint_objects add region_name_table varchar2(500);
alter table waypoint_objects add region_name_text varchar2(500);
alter table waypoint_objects add region_id number(20);

update version_number set version_number=55, min_version_number=55;
