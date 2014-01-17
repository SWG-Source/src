declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'AUCTION_LOCATIONS' and column_name = 'ENTRANCE_CHARGE';
  if (cnt = 0) then
    execute immediate 'alter table auction_locations add entrance_charge number default 0 not null';
  end if;
end;
/

declare 
  cursor c1 is select location_id from auction_locations;
  fee number;
begin
  for c1rec in c1 loop
    begin
      select value into fee
      from object_variables_view
      where name = 'turnstile.fee' and object_id in (
            select object_id from objects
            where contained_by = 0
            start with object_id = c1rec.location_id
            connect by object_id = prior contained_by);
      update auction_locations
      set entrance_charge = fee
      where location_id = c1rec.location_id;
    exception
      when no_data_found then
        null;
    end;
  end loop;
end;
/

update version_number set version_number=194, min_version_number=194;


