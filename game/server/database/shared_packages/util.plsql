create or replace package body util
as
	function unix_time_2_oracle_date_PDT(p_unix_time in number) return date
	as
	begin
		return new_time( to_date('01011970','ddmmyyyy') + 1/24/60/60 * p_unix_time, 'GMT', 'PDT' );
	end;
	
	function unix_time_2_oracle_date_PST(p_unix_time in number) return date
	as
	begin
		return new_time( to_date('01011970','ddmmyyyy') + 1/24/60/60 * p_unix_time, 'GMT', 'PST' );
	end;

	function unix_time_2_oracle_date_CDT(p_unix_time in number) return date
	as
	begin
		return new_time( to_date('01011970','ddmmyyyy') + 1/24/60/60 * p_unix_time, 'GMT', 'CDT' );
	end;
	
	function unix_time_2_oracle_date_CST(p_unix_time in number) return date
	as
	begin
		return new_time( to_date('01011970','ddmmyyyy') + 1/24/60/60 * p_unix_time, 'GMT', 'CST' );
	end;

	function unix_time_2_oracle_date(p_unix_time in number, p_timezone_offset in number) return date
	as
	begin
		return ((to_date('01011970','ddmmyyyy') + 1/24/60/60 * p_unix_time) + p_timezone_offset/24);
	end;
	
	function unix_to_oracle_date(p_unix_time in number, p_timezone_offset in number) return date
	as
	begin
		return unix_time_2_oracle_date(p_unix_time, p_timezone_offset);
	end;
	
end;
/
