create or replace package body objvar_names
as
	procedure add_name (p_id number, p_name varchar2) as
	begin
		insert into object_variable_names (id,name)
		values (p_id, p_name);
	end;

	function get_name_list return cursortype as
		result_cursor cursortype;
	begin
		open result_cursor for
			select id, name from object_variable_names;

		return result_cursor;
	end;
end;
/
