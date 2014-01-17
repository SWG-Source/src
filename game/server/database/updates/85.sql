alter table accounts drop column move_check_time_ms;
alter table accounts add cheater_level float;
update accounts set cheater_level=0;

update version_number set version_number=85, min_version_number=85;
