create or replace package body objectidmanager
as

	procedure get_ids(how_many in out number, block_start_id out number, block_end_id out number)
	as
		result_cursor cursortype;
	begin
		delete free_object_ids
		where end_id is not null
		and rownum=1
		returning start_id, end_id
		into block_start_id, block_end_id;
			
		if (sql%rowcount = 0) then
			update free_object_ids
			set start_id = start_id + how_many
			where end_id is null
			returning start_id - how_many, start_id - 1
			into block_start_id, block_end_id;
		end if;

		how_many := block_end_id - block_start_id + 1;
	end;

	-- same as get_ids, except guarantees the ids are all contiguous
	-- (therefore it can return just the starting id)
	function get_contiguous_ids(min_count number) return number
	as
		result_cursor cursortype;
		block_start_id number;
		block_end_id number;
	begin
		update free_object_ids
		set start_id = start_id + min_count
		where (end_id - start_id + 1 >= min_count
		or end_id is null)
		and rownum < 2
		returning start_id - min_count, start_id - 1
		into block_start_id, block_end_id;

		if (block_start_id > block_end_id) then -- entire block was consumed
			delete free_object_ids where start_id = block_start_id;
		end if;

		return block_start_id;
	end;

	-- get only one id
	function get_single_id return number
	as
		block_start_id number;
		block_end_id number;
	begin
		update
			free_object_ids
		set
			start_id = start_id + 1
		where
			rownum=1
		returning
			start_id,
			end_id
		into
			block_start_id,
			block_end_id;

		if (block_start_id > block_end_id) then -- entire block was consumed
			delete free_object_ids where start_id = block_start_id;
		end if;

		return block_start_id - 1;
	end;

	procedure rebuild_freelist
	as
		message_count number;
		message_id_block number;
	begin
		delete free_object_ids;

		-- following is not legal in PL/SQL, so it must be "execute immediated"
		execute immediate 'insert into free_object_ids (end_id, start_id) '||
		'select o1.object_id-1 end_id, (select max(o2.object_id) from objects o2 where o1.object_id > o2.object_id)+1 start_id ' ||
		'from objects o1 ' ||
		'where o1.object_id > (select max(o2.object_id)+1 from objects o2 where o1.object_id > o2.object_id)';

		insert into free_object_ids (start_id, end_id)
		select * from
			(select 1,min(object_id)-1
			from objects)
		where not exists (select * from objects where object_id = 1);

		insert into free_object_ids (start_id, end_id)
		select * from
			(select max(object_id)+1,NULL
			from objects)
		where exists (select * from objects);

		--easier to ignore messages while finding free object ids and then fix them up at the end:
		
		select count(*)
		into message_count
		from messages
		where rownum = 1;

		if (message_count <> 0) then
			message_id_block := get_contiguous_ids(message_count);

			update messages
			set message_id = rownum + message_id_block - 1;
		end if;
	end;
end;
/
