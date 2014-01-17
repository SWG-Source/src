alter table player_objects add quests2 varchar2(4000);

update version_number set version_number=201, min_version_number=201;
