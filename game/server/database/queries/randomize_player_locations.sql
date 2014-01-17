declare
	type curtype is ref cursor;
	rc curtype;
	objid number;
begin
	open rc for select object_id from objects where player_controlled = 'Y';
	loop
		fetch rc into objid;
		exit when rc%notfound;
	
		update objects set x= (select mod(dbms_random.random,8000) from dual), y=0, z= (select mod(dbms_random.random,8000) from dual)
		where object_id = objid;
	end loop;
end;
/
