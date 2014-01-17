update region_objects set pvp_state=3 where pvp_state=0;
update region_objects set pvp_state=0 where pvp_state=1 or pvp_state=2;
update region_objects set pvp_state=1 where pvp_state=3;

update version_number set version_number = 35, min_version_number=35;
