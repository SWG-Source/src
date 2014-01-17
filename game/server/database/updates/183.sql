alter table ship_objects add cmp_creators varchar2(200);

update version_number set version_number=183, min_version_number=183;
