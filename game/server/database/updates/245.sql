declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'PLAYER_OBJECTS' and column_name = 'NEXT_GCW_RATING_CALC_TIME';
  if (cnt = 0) then
    execute immediate 'alter table player_objects add current_gcw_points int';
    execute immediate 'alter table player_objects add current_gcw_rating int';
    execute immediate 'alter table player_objects add current_pvp_kills int';
    execute immediate 'alter table player_objects add lifetime_gcw_points int';
    execute immediate 'alter table player_objects add max_gcw_imperial_rating int';
    execute immediate 'alter table player_objects add max_gcw_rebel_rating int';
    execute immediate 'alter table player_objects add lifetime_pvp_kills int';
    execute immediate 'alter table player_objects add next_gcw_rating_calc_time int';
  end if;
end;
/

update version_number set version_number=245, min_version_number=245;
