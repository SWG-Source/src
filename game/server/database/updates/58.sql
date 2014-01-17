update objects set deleted=1 where object_id in (select object_id from mission_data_objects);
update version_number set version_number=58, min_version_number=56;
