create or replace package body gold_override as

	function load_objvar_overrides(p_schema varchar2) return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for 
			'select /*+ ORDERED USE_NL(T)*/ ' ||
			't.object_id, t.name_id, t.type, t.value ' ||
			'from ' ||
				p_schema || 'object_list l, ' ||
				'object_variables t ' ||
			'where l.object_id = t.object_id ' ||
			'and nvl(t.detached,0) = 0';

		return result_cursor;
	end;

end;
/
