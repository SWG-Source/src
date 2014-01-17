create table temp_objvar_renum
as select id, right_id
from object_variable_names ovn, (select min(id) right_id, name
	from object_variable_names
	group by name
	having count(*) > 1) subq
where ovn.name = subq.name
and id <> right_id;

update object_variables
set name_id = (select right_id from temp_objvar_renum where object_variables.name_id = temp_objvar_renum.id)
where name_id in (select id from temp_objvar_renum);

delete object_variable_names
where id in (select id from temp_objvar_renum);

drop table temp_objvar_renum;

delete object_variable_names
where name is null;
