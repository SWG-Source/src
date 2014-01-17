alter table accounts add move_check_time_ms int;
update accounts set move_check_time_ms=10000;

update version_number set version_number = 33, min_version_number=33;
