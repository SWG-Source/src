create global temporary table object_list -- NO_IMPORT
(
	object_id number(20),
	container_level number
);

update version_number set version_number = 18, min_version_number=18;
