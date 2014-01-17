alter table building_objects add city_id int;

update version_number set version_number=132, min_version_number=132;
