alter table harvester_installation_objects add hopper_resource number(20);
alter table harvester_installation_objects add hopper_amount int;

update version_number set version_number=125, min_version_number=125;
