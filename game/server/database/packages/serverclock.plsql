create or replace package body serverclock
as

	function get_last_save_time return number
	as
		result number;
	begin
		select 
			last_save_time
		into
			result
		from
			clock;
		return result;
	end;

end;
/
