alter table waypoints modify name varchar2(512);

update version_number set version_number=174, min_version_number=174;
