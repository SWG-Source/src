alter table object_variables
add detached int;

update object_variables set detached = 0;
delete from object_variables where name = 'deletehack';

update version_number set version_number=76, min_version_number=76;
