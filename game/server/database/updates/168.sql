update version_number set version_number=168, min_version_number=168;

declare
  cnt number;
begin
  select count(*) into cnt from user_objects
  where object_name = 'RESOURCE_IMPORTER' and object_type = 'PACKAGE';
  if (cnt > 0) then
    execute immediate 'drop package resource_importer';
  end if;

  select count(*) into cnt from user_objects
  where object_name = 'OBJVAR' and object_type = 'PACKAGE';
  if (cnt > 0) then
    execute immediate 'drop package objvar';
  end if;

  select count(*) into cnt from user_tables
  where table_name = 'ATTRIBUTES';
  if (cnt > 0) then
    execute immediate 'drop table attributes';
  end if;
end;
/
