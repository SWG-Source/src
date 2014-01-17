declare
	rc number;
begin
	select count(*) into rc
	from tangible_objects where invulnerable='Y' and condition <> 0;

	if (rc > 0) then
		raise_application_error(-20000,'There are objects in the tangible table with invulnerable set to "Y" but non-zero condition  This is not supported by the update script.  Please update them manually.');
	else
		update tangible_objects set condition = 256 where invulnerable='Y' and condition=0;
		execute immediate 'alter table tangible_objects drop column invulnerable';
	end if;
end;
/

update version_number set version_number=97, min_version_number=97;
