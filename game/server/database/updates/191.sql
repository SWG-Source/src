declare
  cnt number;
begin
  select count(*) into cnt from user_tab_columns 
  where table_name = 'AUCTION_LOCATIONS' and column_name = 'SEARCH_ENABLED';
  if (cnt = 0) then
    execute immediate 'alter table auction_locations add search_enabled char(1) default ''Y''';
	execute immediate 'update auction_locations set search_enabled = ''N'' where owner_id <> 0';
  end if;
end;
/
update version_number set version_number=191, min_version_number=191;
