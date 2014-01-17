update (select * from cell_objects where is_public='1') set is_public='Y';
update (select * from cell_objects where is_public='0') set is_public='N';
update (select * from building_objects where is_public='1') set is_public='Y';
update (select * from building_objects where is_public='0') set is_public='N';

update version_number set version_number = 27, min_version_number=27;
