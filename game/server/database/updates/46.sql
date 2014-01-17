alter table objects add load_contents char(1);
update objects set load_contents='Y';

update version_number set version_number=46, min_version_number=46;
