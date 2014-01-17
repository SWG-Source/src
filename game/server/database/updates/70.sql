alter table players add create_time date;
update players set create_time = sysdate;

update version_number set version_number=70, min_version_number=70;
