drop table counting_objects;

alter table objects add name_string_table varchar2(500);
alter table objects add name_string_text varchar2(500);
alter table objects drop column name_string_id;
alter table player_objects add personal_profile_id varchar2(200);
alter table player_objects add character_profile_id varchar2(200);
alter table manf_schematic_objects drop column item_count;
alter table intangible_objects add count int;
alter table tangible_objects add count int;
alter table tangible_objects add condition int;
alter table weapon_objects add damage_radius float;
alter table creature_objects add max_force_power int;
alter table creature_objects add force_power int;

update version_number set version_number=73, min_version_number=73;
