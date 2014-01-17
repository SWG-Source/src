alter table players add character_full_name varchar2(127);
update players set character_full_name = (select object_name from objects where object_id = character_object);
alter table objects modify script_list varchar2(2000);
update version_number set version_number=176, min_version_number=176;

