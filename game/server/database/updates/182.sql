alter table ship_objects add chassis_cmp_mass_maximum float;
update ship_objects set chassis_cmp_mass_maximum=10000;

update version_number set version_number=182, min_version_number=182;

