alter table creature_objects add shock_wounds int;
update creature_objects set shock_wounds = 0;

update version_number set version_number = 30, min_version_number=30;
