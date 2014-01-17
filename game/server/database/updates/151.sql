whenever sqlerror exit failure rollback;

alter table objects add script_list varchar2(1000);

declare
	l_last_oid number;
	l_packed_scripts varchar2(1000);
	l_update_count number;
begin
	dbms_output.enable(20000);
	l_update_count := 0;

	for i in (select * from scripts order by object_id, sequence_no)
	loop
		if (l_last_oid is null) then
			l_last_oid := i.object_id;
		end if;

		if ((i.object_id != l_last_oid) and (l_packed_scripts is not null)) then
			update objects
			set script_list = l_packed_scripts
			where object_id = l_last_oid;

			l_packed_scripts := '';
			l_last_oid := i.object_id;
			l_update_count := l_update_count + 1;

			if (l_update_count >= 1000) then
				commit;
				l_update_count := 0;
			end if;
		end if;

		l_packed_scripts := l_packed_scripts || i.script || ':';
	end loop;

	update objects
	set script_list = l_packed_scripts
	where object_id = l_last_oid;

	commit;
end;
/

truncate table scripts;

update version_number set version_number=151, min_version_number=151;
