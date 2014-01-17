begin
for x in (
    select distinct object_id from scripts s1
    where sequence_no > 0
    and not exists (select 1 from scripts s2 where s1.object_id = s2.object_id and s1.sequence_no = s2.sequence_no+1))
loop
	begin
		loop
			update scripts set sequence_no = sequence_no - 1 where object_id = x.object_id
			and sequence_no > 0 and not exists (
				select * from scripts s2
				       where scripts.object_id = s2.object_id
				       and scripts.sequence_no=s2.sequence_no+1);

			exit when sql%rowcount=0;
		end loop;
	end;
end loop;
end;
/
