alter table accounts add max_lots_adjustment int;
update accounts set max_lots_adjustment=0;

update version_number set version_number=67, min_version_number=67;
