create table temp_convert as select object_id,value from manf_schematic_attributes;

alter table manf_schematic_attributes drop column value;
alter table manf_schematic_attributes add value float;

update manf_schematic_attributes
set value = (select value from temp_convert where temp_convert.object_id = manf_schematic_attributes.object_id);

drop table temp_convert;

update version_number set version_number=56, min_version_number=56;
