alter table cluster_list add num_characters number;
update cluster_list set num_characters = 0;

alter table cluster_list
add constraint pk_cluster_list PRIMARY KEY (id)
using index tablespace indexes;

update version_number set version_number = 16, min_version_number=13;
