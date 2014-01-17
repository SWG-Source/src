declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'MARKET_AUCTIONS' and column_name = 'OBJECT_TEMPLATE_ID';
  if (cnt = 0) then
    execute immediate 'alter table market_auctions add object_template_id int';
  end if;	
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'MARKET_AUCTIONS' and column_name = 'OBJECT_TEMPLATE_ID';
  if (cnt = 0) then
  	update market_auctions
	set object_template_id = decode(category,
	8209, (select min(object_template_id) from objects where contained_by = item_id and deleted = 0),
	(select object_template_id from objects where object_id = item_id));
  end if;
end;
/

update version_number set version_number=200, min_version_number=200;
