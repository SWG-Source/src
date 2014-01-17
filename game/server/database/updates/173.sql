alter table player_objects add (active_quests varchar2(512), completed_quests varchar2(512));

update version_number set version_number=173, min_version_number=173;
