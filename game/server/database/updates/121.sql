alter table cluster_list add locked char(1);
update cluster_list set locked = 'N';

update version_number set version_number=121, min_version_number=121;
