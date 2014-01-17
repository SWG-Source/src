alter table creature_objects drop (max_force_power, force_power);
alter table player_objects add (force_regen_rate float, force_power int, max_force_power int);

update version_number set version_number=130, min_version_number=130;
