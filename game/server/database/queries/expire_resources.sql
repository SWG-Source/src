update resource_types set depleted_timestamp=(select last_save_time-1 from clock) where resource_class like 'gas_inert%' and depleted_timestamp >= (select last_save_time from clock);
update resource_types set depleted_timestamp=(select last_save_time-1 from clock) where resource_class like 'ore_siliclastic_%' and depleted_timestamp >= (select last_save_time from clock);
commit;
