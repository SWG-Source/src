alter table cluster_list add not_recommended char(1);

update version_number set version_number=124, min_version_number=124;
