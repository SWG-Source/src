declare
  cnt number;
begin
  select count(*) into cnt
  from user_constraints
  where constraint_name = 'MARKET_ATTRIBUTES_ITEM_ID_FK';
  if (cnt > 0) then
    execute immediate 'alter table market_auction_attributes drop constraint market_attributes_item_id_fk';
  end if;
end;
/

declare
	i number;
	cursor c1 is
	select distinct item_id from market_auction_attributes
		where item_id not in (select item_id from market_auctions);
begin
	i := 0;
	for c1_rec in c1 loop
		delete from market_auction_attributes where item_id = c1_rec.item_id;
		i := i + 1;
		if i >= 20000 then
			commit;
			i := 0;
		end if;
	end loop;
	commit;
end;
/

alter table market_auction_attributes add constraint market_attributes_item_id_fk 
  foreign key(item_id) references market_auctions on delete cascade;

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'PLAYER_OBJECTS' and column_name = 'QUESTS3';
  if (cnt = 0) then
    execute immediate 'alter table player_objects add quests3 varchar2(4000)';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'PLAYER_OBJECTS' and column_name = 'QUESTS4';
  if (cnt = 0) then
    execute immediate 'alter table player_objects add quests4 varchar2(4000)';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'PURGE_ACCOUNTS' and column_name = 'PURGE_PHASE_DATE';
  if (cnt = 0) then
    execute immediate 'alter table purge_accounts add purge_phase_date date';
  end if;	
end;
/

update version_number set version_number=218, min_version_number=218;
