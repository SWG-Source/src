declare
	i number;
	cursor c1 is
	select id from object_variable_names where id not in 
		(select name_id from object_variables group by name_id);
begin
	i := 0;
	for c1_rec in c1 loop
		delete from object_variable_names where id = c1_rec.id;
		i := i + 1;
		if i >= 100000 then
			commit;
			i := 0;
		end if;
	end loop;
	commit;
end;
/

