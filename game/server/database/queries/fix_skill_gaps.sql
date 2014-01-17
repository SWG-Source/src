begin
for x in (
    select distinct object_id from property_lists p1
    where sequence_no > 0
    and not exists (select 1 from property_lists p2 where p1.object_id = p2.object_id and p1.list_id = p2.list_id and p1.sequence_no = p2.sequence_no+1))
loop
	begin
		loop
			update property_lists set sequence_no = sequence_no - 1 where object_id = x.object_id
			and sequence_no > 0 and not exists (
				select * from property_lists pl2
				       where property_lists.object_id = pl2.object_id and property_lists.list_id = pl2.list_id
				       and property_lists.sequence_no=pl2.sequence_no+1);

			exit when sql%rowcount=0;
		end loop;
	end;
end loop;
end;
/
