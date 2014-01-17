declare
	i number;
	cursor c1 is
	select object_id from player_objects where role_icon_choice IS NULL;
begin
	i := 0;
	for c1_rec in c1 loop
		update player_objects set role_icon_choice = 0 where object_id = c1_rec.object_id;
		i := i + 1;
		if i >= 20000 then
			commit;
			i := 0;
		end if;
	end loop;
	commit;
end;
/

update version_number set version_number=225, min_version_number=225;
