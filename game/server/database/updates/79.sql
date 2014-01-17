alter table cluster_list add address varchar2(255);
alter table cluster_list add port number;
alter table cluster_list add secret char(1);
update cluster_list set secret='N';

update version_number set version_number=79, min_version_number=78;
