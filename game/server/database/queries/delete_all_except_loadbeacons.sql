update objects set deleted = 1 where deleted = 0 and object_template not like '%loadbeacon%';
