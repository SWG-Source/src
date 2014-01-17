create or replace package body data_cleanup
as
	procedure run_cleanup as
	begin
		delete messages
		where not exists (
			select 1 from objects
			where objects.object_id = messages.target);
	end;
	
	procedure run_fix_bad_cells as
	
		v_count PLS_INTEGER := 0;
	    
	begin
	
	FOR x IN 
	(	
		SELECT * from 
		(
			SELECT a.object_id, a.contained_by, a.load_with, a.deleted,
			(select contained_by from objects where object_id = a.contained_by) contained_by_value
			FROM objects a, cell_objects b
			WHERE a.object_id = b.object_id 
		)
		where contained_by_value = 0 and
		contained_by != load_with
	)
	LOOP
	     
	UPDATE objects 
	SET load_with = x.contained_by
	where object_id = x.object_id;
	
	v_count := v_count + 1;
	IF (MOD(v_count, 1000) = 0) then
		COMMIT;
	END IF;
	      
	END LOOP;
	
	COMMIT;
	
	run_fix_houses_w_bad_cells;
	
	end;

	procedure run_fix_houses_w_bad_cells as
	
		v_count PLS_INTEGER := 0;
		v_last_contained_by NUMBER := -1; 
		v_current_contained_by NUMBER := 0; 
		v_retval NUMBER;
	    
	begin
	
	FOR x IN 
	(	
		select o1.object_id, o1.contained_by
		from cell_objects c, objects o1, objects o2
		where c.object_id = o1.object_id
		and o1.deleted > 0 
		and o2.object_id = o1.contained_by 
		and o2.deleted = 0
		order by o1.contained_by
	)
	LOOP
	
	v_current_contained_by := x.contained_by;
	
	UPDATE objects 
	SET deleted = 13,
	deleted_date = SYSDATE + 100
	where object_id = x.object_id;
	
	IF (((v_current_contained_by != v_last_contained_by) and v_last_contained_by >=0)) THEN
	
		UPDATE objects 
		SET deleted = 13,
		deleted_date = SYSDATE + 100
		where object_id = v_last_contained_by;
		
		v_retval := admin.restore_house (v_last_contained_by); 
		-- result codes:
		--    1 = success
		--    2 = not a character or not deleted
		--    3 = database error

	
	END IF;
	
	
	v_count := v_count + 1;
	IF (MOD(v_count, 1000) = 0) then
		COMMIT;
	END IF;
	
	v_last_contained_by := v_current_contained_by;
	      
	END LOOP;
	
		UPDATE objects 
		SET deleted = 13,
		deleted_date = SYSDATE + 100
		where object_id = v_last_contained_by;
		
		v_retval := admin.restore_house (v_last_contained_by);
	
	
	COMMIT;
	
	end;
	
	function getObjectsWithExpMessages return cursortype as
		result_cursor cursortype;
	begin
		open result_cursor for
			select target from messages where method='C++experience' group by target having count(*) > 100 order by count(*) desc;

		return result_cursor;
	end;

	function getExperienceGrants(p_object_id objectid) return cursortype as
		result_cursor cursortype;
	begin
		open result_cursor for
			select data from messages where target=p_object_id and method='C++experience';

		return result_cursor;
	end;

	procedure delete_experience(p_object_id objectid) as
	begin
		delete messages where target=p_object_id and method='C++experience';
	end;

	procedure grant_experience(p_object_id objectid, p_experience_data varchar2) as
	begin
		insert into messages (message_id, target, method, data, call_time, guaranteed, delivery_type)
		values (objectidmanager.get_single_id(), p_object_id, 'C++experience', p_experience_data, 0, 'Y', 0);
	end;






   -- Delete 10,000 unused object variables at a time
   function object_variable_name_cleanup return number
   as
   begin
       delete from object_variable_names where id not in (select name_id from object_variables group by name_id) and rownum < 10000;
       commit;
       return sql%rowcount;
    end;



   -- Delete 25,000 orphaned rows at a time
   function orphaned_object_cleanup return number
   as
   begin
       update objects set deleted = 7 where contained_by != 0 and contained_by not in (select object_id from objects) and deleted = 0 and rownum < 25000;
       commit;
       return sql%rowcount;
    end;


   -- Delete 10,000 orphaned attribute rows at a time
   function market_attributes_cleanup return number
   as
   begin
       delete from market_auction_attributes where market_auction_attributes.ITEM_ID not in (select market_auctions.ITEM_ID from market_auctions group by market_auctions.ITEM_ID) and rownum < 10000;
       commit;
       return sql%rowcount;
    end;


   -- Delete 50,000 orphaned message rows at a time
   function messages_cleanup return number
   as
   begin

      delete messages
      where not exists (
         select 1 from objects
         where objects.object_id = messages.target) and rownum < 50000;
       commit;
       return sql%rowcount;
    end;
   
   
   -- Delete 25,000 orphaned vendor object rows at a time
   function vendor_object_cleanup return number
   as
   begin
       update objects set deleted = 9 where 
         deleted = 0
         and
         object_id not in (select item_id from market_auctions)
         and
         contained_by in (select object_id from objects where script_list like '%terminal.vendor:%')
         and
         rownum < 25000;
         
         commit;
         return sql%rowcount;
    end;


   
   
   -- Delete 10,000 broken object rows at a time
   function broken_object_cleanup return number
   as
   numrows number;
   begin
      numrows := 0;
      
      
      delete from building_objects where not exists ( select 1 from objects where objects.object_id = building_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
         
       delete from cell_objects where not exists ( select 1 from objects where objects.object_id = cell_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;      
      
       delete from city_objects where not exists ( select 1 from objects where objects.object_id = city_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;        
            
      delete from creature_objects where not exists ( select 1 from objects where objects.object_id = creature_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;    
        
      delete from factory_objects where not exists ( select 1 from objects where objects.object_id = factory_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
         
      delete from harvester_installation_objects where not exists ( select 1 from objects where objects.object_id = harvester_installation_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
         
      delete from installation_objects where not exists ( select 1 from objects where objects.object_id = installation_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;        
      
      delete from intangible_objects where not exists ( select 1 from objects where objects.object_id = intangible_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
      
      delete from manf_schematic_objects where not exists ( select 1 from objects where objects.object_id = manf_schematic_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
      
      delete from manufacture_inst_objects where not exists ( select 1 from objects where objects.object_id = manufacture_inst_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
      
      delete from mission_objects where not exists ( select 1 from objects where objects.object_id = mission_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
      
      delete from planet_objects where not exists ( select 1 from objects where objects.object_id = planet_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
      
       delete from player_objects where not exists ( select 1 from objects where objects.object_id = player_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;   
      
       delete from ship_objects where not exists ( select 1 from objects where objects.object_id = ship_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
      
      delete from static_objects where not exists ( select 1 from objects where objects.object_id = static_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
      
       delete from tangible_objects where not exists ( select 1 from objects where objects.object_id = tangible_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
      
      delete from token_objects where not exists ( select 1 from objects where objects.object_id = token_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;                                                                              
      
      delete from universe_objects where not exists ( select 1 from objects where objects.object_id = universe_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
      
      delete from vehicle_objects where not exists ( select 1 from objects where objects.object_id = vehicle_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;          
      
       delete from weapon_objects where not exists ( select 1 from objects where objects.object_id = weapon_objects.object_id ) and rownum < 10000;
       commit;
      numrows := numrows + sql%rowcount;
      
       return numrows;
    end;   


end;
/
