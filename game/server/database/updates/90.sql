alter table preload_list add use_for_wilderness char(1);
update preload_list set use_for_wilderness='Y';

update version_number set version_number=90, min_version_number=88;
