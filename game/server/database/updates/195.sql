declare
  cnt number;
begin
  select count(*) into cnt from user_tab_columns where table_name = 'WEAPON_OBJECTS' and column_name = 'ZERO_RANGE_MOD';
  if (cnt > 0) then
    execute immediate 'alter table weapon_objects drop column zero_range_mod';
  end if;

  select count(*) into cnt from user_tab_columns where table_name = 'WEAPON_OBJECTS' and column_name = 'MIN_RANGE_MOD';
  if (cnt > 0) then
    execute immediate 'alter table weapon_objects drop column min_range_mod';
  end if;

  select count(*) into cnt from user_tab_columns where table_name = 'WEAPON_OBJECTS' and column_name = 'MID_RANGE_MOD';
  if (cnt > 0) then
    execute immediate 'alter table weapon_objects drop column mid_range_mod';
  end if;

  select count(*) into cnt from user_tab_columns where table_name = 'WEAPON_OBJECTS' and column_name = 'MAX_RANGE_MOD';
  if (cnt > 0) then
    execute immediate 'alter table weapon_objects drop column max_range_mod';
  end if;

  select count(*) into cnt from user_tab_columns where table_name = 'WEAPON_OBJECTS' and column_name = 'ATTACK_MODE';
  if (cnt > 0) then
    execute immediate 'alter table weapon_objects drop column attack_mode';
  end if;

  select count(*) into cnt from user_tab_columns where table_name = 'WEAPON_OBJECTS' and column_name = 'ATTACK_HEALTH_COST';
  if (cnt > 0) then
    execute immediate 'alter table weapon_objects drop column attack_health_cost';
  end if;

  select count(*) into cnt from user_tab_columns where table_name = 'WEAPON_OBJECTS' and column_name = 'ATTACK_ACTION_COST';
  if (cnt > 0) then
    execute immediate 'alter table weapon_objects drop column attack_action_cost';
  end if;

  select count(*) into cnt from user_tab_columns where table_name = 'WEAPON_OBJECTS' and column_name = 'ATTACK_MIND_COST';
  if (cnt > 0) then
    execute immediate 'alter table weapon_objects drop column attack_mind_cost';
  end if;

  select count(*) into cnt from user_tab_columns where table_name = 'WEAPON_OBJECTS' and column_name = 'MIN_RANGE';
  if (cnt = 0) then
    execute immediate 'alter table weapon_objects add (min_range float, max_range float)';
  end if;

  select count(*) into cnt from user_tab_columns where table_name = 'WEAPON_OBJECTS' and column_name = 'DAMAGE_TYPE';
  if (cnt = 0) then
    execute immediate 'alter table weapon_objects add (damage_type int, elemental_type int, elemental_value int, accuracy int, attack_cost int)';
    execute immediate 'update weapon_objects set damage_type = 1, elemental_type = 0, elemental_value = 0, accuracy = 0, attack_cost = 0';
  end if;
end;
/
update version_number set version_number=195, min_version_number=195;
