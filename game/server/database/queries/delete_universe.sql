update objects set deleted = 1 where deleted = 0 and exists (select * from universe_objects where universe_objects.object_id = objects.object_id);
