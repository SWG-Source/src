declare
  cnt number;
begin
  select count(*) into cnt from user_objects
  where object_name = 'RESOURCE_POOL_OBJECTS' and object_type = 'TABLE';
  if (cnt > 0) then
    execute immediate 'drop table resource_pool_objects';
  end if;

  select count(*) into cnt from user_objects
  where object_name = 'RESOURCE_TYPE_OBJECTS' and object_type = 'TABLE';
  if (cnt > 0) then
    execute immediate 'drop table resource_type_objects';
  end if;
end;
/
update version_number set version_number=175, min_version_number=175;
