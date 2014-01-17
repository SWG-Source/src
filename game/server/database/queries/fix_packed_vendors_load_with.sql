/*
	Fix the load_with on all packed vendors.
*/

declare
	load_with number := 0;
begin
	for x in (select location_id from auction_locations where status >= power(2,24))
	loop
		begin
			select load_with into load_with from objects where object_id = x.location_id;
		exception when others then
			load_with := 0;
		end;

		if (load_with <> 0) then
			admin.fix_load_with_depth(x.location_id, load_with, 10); -- containerMaxDepth + 1
		end if;
	end loop;
end;
/

commit;
