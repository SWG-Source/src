alter table player_objects add quests varchar2(4000);

update version_number set version_number=179, min_version_number=179;

