alter table ship_objects modify cmp_crc varchar2(500);

update version_number set version_number=162, min_version_number=162;
