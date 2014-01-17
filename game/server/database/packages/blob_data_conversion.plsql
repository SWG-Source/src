CREATE OR REPLACE PACKAGE BODY  "BLOB_DATA_CONVERSION" as

function blob_encodeoob_old(v_blob blob) return varchar2
is
	retstring varchar2(4000);
	b binary_integer;
begin
	if utl_raw.length(v_blob) > 1 then
		for i in 1..trunc(utl_raw.length(v_blob)/2) loop
			begin
				b := utl_raw.CAST_TO_BINARY_INTEGER(utl_raw.SUBSTR(v_blob,(i*2)-1,2),2);
				if b = 0 then
					retstring := retstring || chr(15712191) || chr(1);
				elsif b = 65535 then
					retstring := retstring || chr(15712191) || chr(2);
				elsif b = 27 then
					retstring := retstring || chr(15712191) || chr(3);
				elsif b > 2048 then -- 3-byte encoding
					retstring := retstring || chr(((224 + mod(trunc(b / 4096), 16)) * 65536) + ((128 + mod(trunc(b / 64),64)) * 256) + (128 + mod(b, 64)));
				elsif b > 127 then -- 2-byte encoding
					retstring := retstring || chr((mod(trunc(b / 64),32) + 192) * 256 + (mod(b, 64) + 128));
				else
					retstring := retstring || chr(b);
				end if;
			exception
				when others then
					retstring := retstring || ' ';
			end;
			exit when length(retstring) >= 3996;
		end loop;
		retstring := retstring || chr(15712191) || chr(4);
		return retstring;
	else
		return null;
	end if;
exception
	when others then
		return null;
end;

function blob_encodeoob(v_blob blob) return varchar2
is
	retstring varchar2(4000);
	b binary_integer;
    buf1 raw(10);
    buf2 raw(10);
    buf raw(10);
begin
	if utl_raw.length(v_blob) > 1 then
		for i in 1..trunc(utl_raw.length(v_blob)/2) loop
			b := utl_raw.CAST_TO_BINARY_INTEGER(utl_raw.SUBSTR(v_blob,(i*2)-1,2),2);
                	retstring := retstring || substr(to_char(b, '0XXXX'), 3);
       			exit when length(retstring) >= 3998;
		end loop;
		return retstring;
	else
		return null;
	end if;
exception
	when others then
	return null;
end;

function blob_hexdump(v_blob blob) return varchar2
is
	retstring varchar2(4000);
	b binary_integer;
begin
	if utl_raw.length(v_blob) > 0 then
		for i in 1..utl_raw.length(v_blob) loop
			b := utl_raw.CAST_TO_BINARY_INTEGER(utl_raw.SUBSTR(v_blob,i,1));
			retstring := retstring || to_char(b, '0x');
		end loop;
		return retstring;
	else
		return null;
	end if;
exception
	when others then
		return null;
end;


function blob_to_string(v_blob blob) return varchar2
is
	retstring varchar2(2000);
	c char;
	b binary_integer;
begin
	if utl_raw.length(v_blob) > 0 then
		-- assumes blob is in UTF16
		for i in 1..trunc(utl_raw.length(v_blob)/2) loop
			b := utl_raw.CAST_TO_BINARY_INTEGER(utl_raw.SUBSTR(v_blob,(i*2)-1,2),2);
			if b > 2048 then -- 3-byte encoding
				retstring := retstring || chr(((224 + mod(trunc(b / 4096), 16)) * 65536) + ((128 + mod(trunc(b / 64),64)) * 256) + (128 + mod(b, 64)));
			elsif b > 127 then -- 2-byte encoding
				retstring := retstring || chr((mod(trunc(b / 64),32) + 192) * 256 + (mod(b, 64) + 128));
			else
				retstring := retstring || chr(b);
			end if;
		end loop;
		return retstring;
	else
		return null;
	end if;
exception
	when others then
		return null;
end;


function string_hexdump(v_string varchar2) return varchar2
is
begin
	return blob_hexdump(utl_raw.CAST_TO_RAW(v_string));
exception
	when others then
		return null;
end;


procedure convert_blob_data(v_item_id number)
is
	i number;
begin
--	update market_auctions
--	set new_oob = blob_encodeoob(oob),
--	    new_item_name = blob_to_string(item_name),
--	    new_user_desc = blob_to_string(user_description)
--	where item_id = v_item_id;
--	commit;
	i := 0;
end;

end;
/
