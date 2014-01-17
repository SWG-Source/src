declare
	i number;

begin
	select count(*) into i
	from object_variable_names
	where name = 'intWeaponType';

	if i = 0 then
		select max(id) into i
		from object_variable_names;

		insert into object_variable_names
		values (i + 1, 'intWeaponType');
	end if;

	select min(id) into i
	from object_variable_names
	where name = 'intWeaponType';

	insert into object_variables
	select 	object_id, i as name_id, 0 as type, '1' as value, 0 as detached
	from objects
	where object_template_id = -635207508; -- object/weapon/ranged/rifle/rifle_proton.iff
	
end;
/
