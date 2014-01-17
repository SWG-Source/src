alter table players add uc_character_name varchar2(127);

update version_number set version_number = 10, min_version_number=9;
