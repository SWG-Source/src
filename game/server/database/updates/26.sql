alter table tangible_objects add pvp_type int;
update tangible_objects set pvp_type=0;
alter table tangible_objects add pvp_faction int;
update tangible_objects set pvp_faction=0;
alter table building_objects add is_public char;
update building_objects set is_public='Y';
alter table cell_objects add is_public char;
update cell_objects set is_public='Y';

update version_number set version_number = 26, min_version_number=26;
