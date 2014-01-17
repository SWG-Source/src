alter table player_objects add skill_title varchar2(200);

update version_number set version_number=84, min_version_number=84;
