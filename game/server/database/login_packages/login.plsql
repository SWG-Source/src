  CREATE OR REPLACE PACKAGE "LOGIN" 
as
	type refcursor is ref cursor;
	subtype objectid is number;

	procedure register_new_cluster(p_cluster_name in varchar2, p_address in varchar2, p_cluster_id out number);
	function get_cluster_list(p_group in number) return refcursor;
	function get_avatar_list (p_station_id number, p_cluster_group number) return refcursor;
	function get_open_character_slots(p_station_id number, p_cluster_id number) return refcursor;
	function get_only_open_character_slots(p_station_id number, p_cluster_id number) return refcursor;
	function is_cluster_at_limit(p_cluster_id number) return number;
	function is_account_at_limit(p_station_id number) return number;
	procedure delete_character(p_cluster_id number, p_character_id number, p_station_id number);
	procedure rename_character(p_cluster_id number, p_character_id number, p_new_name varchar2);
	procedure create_character(p_cluster_id number, p_station_id number, p_character_name varchar2, p_character_id number, p_template_id number, p_character_type number);
	function restore_character(p_cluster_id number, p_station_id number, p_character_name varchar2, p_character_id number, p_template_id number, p_character_type number) return number;
	procedure set_character_slots(p_cluster_id number, p_station_id number, p_slot_type number, p_num_slots number);
	procedure set_character_type(p_cluster_id number, p_station_id number, p_character_id number, p_slot_type number);
	function has_extra_character_slot(p_station_id number, p_character_type number) return number;
	procedure toggle_disable_character(p_cluster_id number, p_character_id number, p_station_id number, p_enabled varchar2);
	function enable_disable_character(p_station_id number, p_character_id number, p_enabled varchar2) return number;
	function get_completed_tutorial(p_station_id number) return refcursor;
	procedure toggle_completed_tutorial(p_station_id number, p_completed varchar2);
	function get_consumed_reward_events(p_station_id number) return refcursor;
	function get_claimed_reward_items(p_station_id number) return refcursor;
	function consume_reward_event(p_station_id number, p_character_id number, p_cluster_id number, p_event_id varchar2) return number;
	function claim_reward_item(p_station_id number, p_character_id number, p_cluster_id number, p_item_id varchar2) return number;
	function get_feature_id_transactions(p_station_id in number, p_cluster_id in number, p_character_id in number) return refcursor;
	function update_feature_id_transaction(p_station_id in number, p_cluster_id in number, p_character_id in number, p_item_id in varchar2, p_count_adjustment in number) return number;
end;

/

  GRANT EXECUTE ON "LOGIN" TO PUBLIC;
--------------------------------------------------------
--  DDL for Package Body LOGIN
--------------------------------------------------------

  CREATE OR REPLACE PACKAGE BODY "LOGIN" 
as

	procedure register_new_cluster(p_cluster_name in varchar2, p_address in varchar2, p_cluster_id out number)
	as
	begin
		select id
		into p_cluster_id
		from cluster_list
		where name = p_cluster_name;

	exception
		when no_data_found then
			insert into cluster_list (id,name,address,secret,locked, not_recommended,group_id,
				online_player_limit,online_free_trial_limit,free_trial_can_create_char, online_tutorial_limit)
			select nvl(max(id)+1,1), p_cluster_name,p_address,'N','N','N',1,
				2500,250,'Y',350
			from cluster_list;

			select id
			into p_cluster_id
			from cluster_list
			where name = p_cluster_name;
	end;

	function get_cluster_list (p_group in number) return refcursor
	as
		result_cursor refcursor;
		maxCharacterPerAccount number;
	begin
		select account_limit into maxCharacterPerAccount
		from default_char_limits;

		open result_cursor for
			select id, name, address, port, secret, locked, not_recommended, maxCharacterPerAccount,
			online_player_limit,online_free_trial_limit,free_trial_can_create_char,online_tutorial_limit
			from cluster_list
			where group_id = p_group;

		return result_cursor;
	end;

	function get_avatar_list (p_station_id number, p_cluster_group number) return refcursor
	as
		result refcursor;
	begin
		open result for
		select
			c.character_name,
			c.template_id,
			c.object_id,
			c.cluster_id,
			c.character_type
		from
			swg_characters c, cluster_list l
		where
			c.cluster_id = l.id and
			l.group_id = p_cluster_group and
			station_id = p_station_id and
			enabled = 'Y';

		return result;
	end;

	function get_open_character_slots(p_station_id number, p_cluster_id number) return refcursor
	as
		result_cursor refcursor;
		over_account_limit number;
		over_cluster_limit number;
	begin
		-- does the account have too many characters, or does the cluster have too many characters?

		select account_limit - account_counter.num, cluster_limit - cluster_list.num_characters
		into over_account_limit, over_cluster_limit
		from
			(select count(*) num from swg_characters
			where (station_id = p_station_id 
			OR station_id IN (SELECT CASE WHEN child_id = p_station_id THEN parent_id 
			ELSE child_id END as station_id FROM account_map WHERE parent_id = p_station_id 
			OR child_id = p_station_id)) and enabled ='Y') account_counter,
			default_char_limits,
			cluster_list
		where
			cluster_list.id = p_cluster_id;

		if (over_account_limit <= 0 or over_cluster_limit <= 0) then
			open result_cursor for
			select 0 character_type_id, 0 remaining_slots
			from dual
			where 1=2;

			return result_cursor;
		end if;

		-- if not, return the number of each type of character we're allowed to create

		open result_cursor for
		select limits.character_type_id, limits.limit - nvl(existing.num,0) remaining_slots
		from
			(select def.character_type_id, def.num_slots + nvl(cls.num_extra_slots,0) limit
			from extra_character_slots cls, default_character_slots def
			where def.character_type_id = cls.character_type_id (+)
			and cls.station_id (+) = p_station_id
			and cls.cluster_id (+) = p_cluster_id) limits,

			(select character_type, count(*) num
			from swg_characters
			where station_id = p_station_id
			and cluster_id = p_cluster_id
			and enabled = 'Y'
			group by character_type) existing
		where
			limits.character_type_id = existing.character_type (+);

		return result_cursor;
	end;

	-- unlike get_open_character_slots, this function ignores account and cluster limits
	function get_only_open_character_slots(p_station_id number, p_cluster_id number) return refcursor
	as
		result_cursor refcursor;
	begin
		-- return the number of each type of character we're allowed to create

		open result_cursor for
		select limits.character_type_id, limits.limit - nvl(existing.num,0) remaining_slots
		from
			(select def.character_type_id, def.num_slots + nvl(cls.num_extra_slots,0) limit
			from extra_character_slots cls, default_character_slots def
			where def.character_type_id = cls.character_type_id (+)
			and cls.station_id (+) = p_station_id
			and cls.cluster_id (+) = p_cluster_id) limits,

			(select character_type, count(*) num
			from swg_characters
			where station_id = p_station_id
			and cluster_id = p_cluster_id
			and enabled = 'Y'
			group by character_type) existing
		where
			limits.character_type_id = existing.character_type (+);

		return result_cursor;
	end;

	function is_cluster_at_limit(p_cluster_id number) return number
	as
		v_cluster_limit number;
		v_num_characters number;
	begin
		select cluster_limit
		into v_cluster_limit
		from default_char_limits;

		select num_characters
		into v_num_characters
		from cluster_list
		where id = p_cluster_id;

		if (v_num_characters >= v_cluster_limit) then
			return 1;
		else
			return 0;
		end if;

	exception
		when others then
			return 0;
	end;

	function is_account_at_limit(p_station_id number) return number
	as
		v_account_limit number;
		v_num_characters number;
	begin
		select account_limit
		into v_account_limit
		from default_char_limits;

		select count(*)
		into v_num_characters
		from swg_characters
		where station_id = p_station_id and enabled ='Y' and cluster_id not in (select id from cluster_list where name in ('Corbantis', 'Europe-Infinity', 'Intrepid', 'Kauri', 'Kettemoor', 'Lowca', 'Naritus', 'Scylla', 'Tarquinas', 'Tempest', 'Valcyn', 'Wanderhome', 'Japan-Katana', 'Japan-Harla'));

		if (v_num_characters >= v_account_limit) then
			return 1;
		else
			return 0;
		end if;

	exception
		when others then
			return 0;
	end;

	procedure delete_character(p_cluster_id number, p_character_id number, p_station_id number)
	as
	begin
		delete from swg_characters
		where station_id = p_station_id
		and cluster_id = p_cluster_id
		and object_id = p_character_id;

		update cluster_list
		set num_characters = num_characters - 1
		where cluster_list.id = p_cluster_id;

	end;

	procedure rename_character(p_cluster_id number, p_character_id number, p_new_name varchar2)
	as
	begin
		update swg_characters
		set character_name = p_new_name
		where cluster_id = p_cluster_id
		and object_id = p_character_id;
	end;

	procedure create_character(p_cluster_id number, p_station_id number, p_character_name varchar2, p_character_id number, p_template_id number, p_character_type number)
	as
	begin
		insert into swg_characters (station_id, cluster_id, character_name, object_id, template_id, character_type)
		values (p_station_id, p_cluster_id, p_character_name, p_character_id, p_template_id, p_character_type);

		update cluster_list
		set num_characters = nvl(num_characters,0) + 1
		where cluster_list.id = p_cluster_id;

		exception when DUP_VAL_ON_INDEX then
			update swg_characters
			set object_id = p_character_id,
			template_id = p_template_id,
			character_type = p_character_type
			where cluster_id = p_cluster_id
			and station_id = p_station_id
			and character_name = p_character_name;
	end;

	function restore_character(p_cluster_id number, p_station_id number, p_character_name varchar2, p_character_id number, p_template_id number, p_character_type number) return number
	-- Return codes:
	-- 1: restored
	-- 2: restored, but too many characters on the account now
	-- 3: database error
	as
		l_num_characters number;
		l_limit number;
	begin
		begin
			insert into swg_characters (station_id, cluster_id, character_name, object_id, template_id, character_type)
			values (p_station_id, p_cluster_id, p_character_name, p_character_id, p_template_id, p_character_type);

			update cluster_list
			set num_characters = nvl(num_characters,0) + 1
			where cluster_list.id = p_cluster_id;

			exception when DUP_VAL_ON_INDEX then
				null; -- already restored
			when others then
				return 3;
		end;

		-- Check the account against the limits
		select count(*)
		into l_num_characters
		from swg_characters
		where station_id = p_station_id
		and character_type = p_character_type
		and enabled = 'Y'
		and cluster_id not in (select id from cluster_list where name in ('Corbantis', 'Europe-Infinity', 'Intrepid', 'Kauri', 'Kettemoor', 'Lowca', 'Naritus', 'Scylla', 'Tarquinas', 'Tempest', 'Valcyn', 'Wanderhome', 'Japan-Katana', 'Japan-Harla'));

		select def.num_slots + nvl(cls.num_extra_slots,0)
		into l_limit
		from extra_character_slots cls, default_character_slots def
		where def.character_type_id = cls.character_type_id (+)
		and cls.station_id (+) = p_station_id
		and cls.cluster_id (+) = p_cluster_id
		and def.character_type_id = p_character_type;

		if (l_num_characters > l_limit) then
			return 2;
		else
			return 1;
		end if;

	exception
		when others then
			return 3;
	end;

	procedure set_character_slots(p_cluster_id number, p_station_id number, p_slot_type number, p_num_slots number)
	as
	begin
		update extra_character_slots
		set num_extra_slots = p_num_slots
		where cluster_id = p_cluster_id
		and station_id = p_station_id
		and character_type_id = p_slot_type;

		if (sql%rowcount = 0) then
			insert into extra_character_slots (station_id, cluster_id, character_type_id, num_extra_slots)
			values (p_station_id, p_cluster_id, p_slot_type, p_num_slots);
		end if;
	end;

	procedure set_character_type(p_cluster_id number, p_station_id number, p_character_id number, p_slot_type number)
	as
	begin
		update swg_characters
		set character_type = p_slot_type
		where station_id = p_station_id
		and cluster_id = p_cluster_id
		and object_id = p_character_id;
	end;

	function has_extra_character_slot(p_station_id number, p_character_type number) return number
	as
		rows number;
		total number;
	begin
		select count(*), sum(num_extra_slots)
		into rows, total
		from extra_character_slots
		where station_id = p_station_id
		and character_type_id = p_character_type;

		if (rows <= 0 or total < 0) then
			total := 0;
		end if;

		return total;
	exception
		when others then
			return 0;
	end;

	procedure toggle_disable_character(p_cluster_id number, p_character_id number, p_station_id number, p_enabled varchar2)
	as
	begin
		update swg_characters
		set enabled = p_enabled
		where station_id = p_station_id
		and cluster_id = p_cluster_id
		and object_id = p_character_id;

	end;

	function enable_disable_character(p_station_id number, p_character_id number, p_enabled varchar2) return number
	as
	begin
		update swg_characters
		set enabled = p_enabled
		where station_id = p_station_id and
		      object_id = p_character_id;
		if (sql%rowcount > 0) then
			return 1;
		else
			return 2;
		end if;
	exception
		when others then
			return 3;
	end;

	function get_completed_tutorial(p_station_id number) return refcursor
	as
		cnt number;
		result_cursor refcursor;
	begin
		select count (*) into cnt
		from account_info
		where station_id = p_station_id;

		if (cnt = 0) then
			insert into account_info (station_id, completed_tutorial)
			values (p_station_id, 'N');
		end if;

		open result_cursor for
		select completed_tutorial
		from account_info
		where station_id = p_station_id;

		return result_cursor;
	end;

	procedure toggle_completed_tutorial(p_station_id number, p_completed varchar2)
	as
	begin
		update account_info
		set completed_tutorial = p_completed
		where station_id = p_station_id;
	end;

	function get_consumed_reward_events(p_station_id number) return refcursor
	as
		result_cursor refcursor;
	begin
		open result_cursor for
		select event_id, cluster_id, character_id
		from account_reward_events
		where station_id = p_station_id;

		return result_cursor;
	end;

	function get_claimed_reward_items(p_station_id number) return refcursor
	as
		result_cursor refcursor;
	begin
		open result_cursor for
		select item_id, cluster_id, character_id
		from account_reward_items
		where station_id = p_station_id;

		return result_cursor;
	end;

	function consume_reward_event(p_station_id number, p_character_id number, p_cluster_id number, p_event_id varchar2) return number
	as
	begin
		insert into account_reward_events (station_id, event_id, date_consumed, cluster_id, character_id)
		values (p_station_id, p_event_id, sysdate, p_cluster_id, p_character_id);

		return 1;

	exception
		when dup_val_on_index then
			-- Check whether previous claim was from the same character and cluster, if so, allow it
			declare
				previous_claim_cluster number;
				previous_claim_character number;
			begin
				select cluster_id, character_id
				into previous_claim_cluster, previous_claim_character
				from account_reward_events
				where station_id = p_station_id
				and event_id = p_event_id;

				if ((p_cluster_id=previous_claim_cluster) and (p_character_id=previous_claim_character)) then
					return 1;
				end if;

				return 0;
			end;
		when others then
			return 0;
	end;

	function claim_reward_item(p_station_id number, p_character_id number, p_cluster_id number, p_item_id varchar2) return number
	as
	begin
		insert into account_reward_items (station_id, item_id, date_claimed, cluster_id, character_id)
		values (p_station_id, p_item_id, sysdate, p_cluster_id, p_character_id);

		return 1;

	exception
		when dup_val_on_index then
			-- Check whether previous claim was from the same character and cluster, if so, allow it
			declare
				previous_claim_cluster number;
				previous_claim_character number;
			begin
				select cluster_id, character_id
				into previous_claim_cluster, previous_claim_character
				from account_reward_items
				where station_id = p_station_id
				and item_id = p_item_id;

				if ((p_cluster_id=previous_claim_cluster) and (p_character_id=previous_claim_character)) then
					return 1;
				end if;

				return 0;
			exception
				when others then
					return 0;
			end;
		when others then
			return 0;
	end;

	function get_feature_id_transactions(p_station_id in number, p_cluster_id in number, p_character_id in number) return refcursor
	as
		result_cursor refcursor;
	begin
		open result_cursor for
		select item_id, count
		from feature_id_transactions
		where station_id = p_station_id and cluster_id = p_cluster_id and character_id = p_character_id;

		return result_cursor;
	end;

	function update_feature_id_transaction(p_station_id in number, p_cluster_id in number, p_character_id in number, p_item_id in varchar2, p_count_adjustment in number) return number
	as
	begin
		update feature_id_transactions set date_updated = sysdate,
		count = count + p_count_adjustment
		where station_id = p_station_id
		and cluster_id = p_cluster_id
		and character_id = p_character_id
		and item_id = p_item_id;

		if (sql%rowcount = 0) then
			insert into feature_id_transactions (station_id, cluster_id, character_id, item_id, date_updated, count)
			values (p_station_id, p_cluster_id, p_character_id, p_item_id, sysdate, p_count_adjustment);
		end if;

		return 1;

	exception
		when others then
			return 0;
	end;
end;

/

  GRANT EXECUTE ON "LOGIN" TO PUBLIC;
  COMMIT;
EXIT;
