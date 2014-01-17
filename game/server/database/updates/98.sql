alter table manf_schematic_objects add draft_schematic_id number;

declare
	total number;
	updated number;
begin
	select count(*) into total
	from manf_schematic_objects;

	update manf_schematic_objects
	set draft_schematic_id = (select id from object_templates t where t.name = draft_schematic)
	where exists (select id from object_templates t where t.name = draft_schematic);

	if (sql%rowcount <> total) then
		execute immediate 'alter table manf_schematic_objects drop column draft_schematic_id';
		raise_application_error(-20000,'There are objects in the manf_schematic_objects table with object templates that are not in the object_templates table.  Please correct these before running the update.');
	else
		execute immediate 'alter table manf_schematic_objects drop column draft_schematic';
		execute immediate 'alter table manf_schematic_objects add draft_schematic int';
		execute immediate 'update manf_schematic_objects set draft_schematic = draft_schematic_id';
		execute immediate 'alter table manf_schematic_objects drop column draft_schematic_id';
	end if;
end;
/

update version_number set version_number=98, min_version_number=98;
