declare
  cnt number;
begin
  select count(*) into cnt from user_tab_columns 
  where table_name = 'SHIP_OBJECTS' and column_name = 'CARGO_HOLD_CONTENTS_MAXIMUM';
  if (cnt = 0) then
    execute immediate 'alter table ship_objects add (cargo_hold_contents_maximum number, cargo_hold_contents_current number)';
  end if;
  select count(*) into cnt from user_tab_columns 
  where table_name = 'SHIP_OBJECTS' and column_name = 'CARGO_HOLD_CONTENTS';
  if (cnt = 0) then
    execute immediate 'alter table ship_objects add (cargo_hold_contents varchar2(500))';
  end if;
end;
/
update version_number set version_number=199, min_version_number=199;
