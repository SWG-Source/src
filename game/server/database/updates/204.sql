alter table ship_objects modify(cmp_names varchar2(4000));

update version_number set version_number=204, min_version_number=204;
