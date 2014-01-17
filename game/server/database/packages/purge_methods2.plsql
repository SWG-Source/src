CREATE OR REPLACE PACKAGE BODY PURGE_METHODS2 is
/**********************************************************************/
/*** Purge Objects                                                  ***/
/***                                                                ***/
/*** Deletes objects marked for deletion in object tables and       ***/
/*** then the master objects table based on date value              ***/
/**********************************************************************/

  FUNCTION Purge_objects
    (date_in       in objects.deleted_date%type)
  RETURN PLS_INTEGER
  IS
    v_count PLS_INTEGER := 0;

  Obj_list Obj_list_t;  -- VARRAY object to hold ids to bulk delete for child objects
  Obj_list_index PLS_INTEGER; -- index into VARRAY object

  BEGIN

  Obj_list := Obj_list_t();   -- initialize VARRAY object
  Obj_list.extend(chunk_size); -- allocate VARRAY object memory to chunk size

  -- LOGGING.LOG_STATUS(1,'Starting object deletions.');

  --open the cursor for the object ids we want to purge.
  --we will use these ids to delete records from other tables
    FOR x IN (SELECT /*+ INDEX (OBJECTS DELETED_OBJECT_IDX) */ object_id
              FROM objects
              WHERE deleted_date < date_in) LOOP

       v_count := v_count + 1;
       Obj_list_index := MOD(v_count, chunk_size); -- what obj index for this batch

       If Obj_list_index = 0 then
          Obj_list(chunk_size) := x.object_id; -- make sure we fill last VARRAY element
          Purge_objects_work(Obj_list); -- bulk delete object ids in VARRAY list
          COMMIT;
     --     LOGGING.LOG_STATUS(v_count,'batch purged');
       Else
          Obj_list(Obj_list_index) := x.object_id; -- fill VARRAY with ids
       End If;

    END LOOP;

  -- LOGGING.LOG_STATUS(v_count,'Child object main loop exited.');

    --Now delete remaining partially full object list for objects
    If Obj_list_index != 0 then
        Obj_list.trim(chunk_size - Obj_list_index);
        Purge_objects_work(Obj_list);
        COMMIT;
    --    LOGGING.LOG_STATUS(v_count,'Deleting chunk remainder.');
    End If;

  Obj_list.delete; -- cleanup - delete VARRAY list

--  LOGGING.LOG_STATUS(v_count,'Finished deleting '|| to_char(v_count) ||' references to objects.');          

  RETURN 0;

    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;

      IF v_count = 0 THEN
        dbms_output.put_line ('never deleted');
     --   LOGGING.LOG_ERROR(SQLCODE, SQLERRM);
      END IF;
      dbms_output.put_line (v_count||': iterations');
    --  LOGGING.LOG_ERROR(SQLCODE, SQLERRM);
      RETURN SQLCODE;
  END Purge_objects;

/**********************************************************************/
/*** Purge Objects Work                                             ***/
/***                                                                ***/
/*** Bulk delete objects marked for deletion in child object tables ***/
/*** given a list of object ids passed in from a VARRAY. Called     ***/
/*** purge objects function.                                        ***/
/**********************************************************************/

  PROCEDURE Purge_objects_work
    (Obj_list IN Obj_list_t) IS

  BEGIN

          --delete all children
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM armor WHERE object_id = Obj_list(i);

          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM battlefield_marker_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM battlefield_participants WHERE region_object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM battlefield_participants WHERE character_object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM biographies WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM building_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM cell_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM creature_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM experience_points WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM factory_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM guild_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM harvester_installation_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM installation_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM intangible_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM location_lists WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM manf_schematic_attributes WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM manf_schematic_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM manufacture_inst_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM mission_objects WHERE object_id = Obj_list(i);

          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM object_variables WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM planet_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM player_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM property_lists WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM resource_container_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM scripts WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM static_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM tangible_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM token_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM universe_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM vehicle_objects WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM waypoints WHERE object_id = Obj_list(i);
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM weapon_objects WHERE object_id = Obj_list(i);
          --delete messages target(object_id)
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM messages WHERE target = Obj_list(i);
          
          FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM objects WHERE object_id = Obj_list(i);

  END Purge_objects_work;
  --
  --
  --
  FUNCTION time_purge
    (date_in          IN VARCHAR2)
  RETURN PLS_INTEGER IS
    v_retval PLS_INTEGER := 0;
    v_date VARCHAR2(20) := date_in;
    v_time NUMBER := 1;
  BEGIN

    FOR i IN 1..24 LOOP
      dbms_output.put_line(to_char(to_date(v_date, 'dd-mon-yy hh24:mi:ss') + v_time/24, 'DD-MON-YY hh24:mi:ss'));
      v_retval := purge_objects(to_date(v_date, 'dd-mon-yy hh24:mi:ss') + v_time/24);
      EXIT WHEN v_retval <> 0;

      IF v_time = 23 THEN
       v_time := v_time + (59/60);
      ELSE
        v_time := v_time + 1;
      END IF;
    END LOOP;

    RETURN 0;

    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      dbms_output.put_line('Purge return values: '||v_retval);
      dbms_output.put_line('Time it failed at: '||v_time);
      RETURN SQLCODE;
  END time_purge;

/**********************************************************************/
/*** Purge Consumables                                              ***/
/***                                                                ***/
/*** Deletes consumable objects marked for deletion in child object ***/
/*** tables and then the master objects table based on date value   ***/
/**********************************************************************/

  FUNCTION Purge_consumables
    (start_date_in       in objects.deleted_date%type,
     end_date_in         in objects.deleted_date%type)
  RETURN PLS_INTEGER
  IS
    v_count PLS_INTEGER := 0;

  Obj_list Obj_list_t;  -- VARRAY object to hold ids to bulk delete for objects
  Obj_list_index PLS_INTEGER; -- index into VARRAY object

  BEGIN

  Obj_list := Obj_list_t();   -- initialize VARRAY object
  Obj_list.extend(chunk_size); -- allocate VARRAY object memory to chunk size

 -- LOGGING.LOG_STATUS(1,'Starting child object deletions for consumables.');

  --open the cursor for the object ids we want to purge.
  --we will use these ids to delete records from other tables
    FOR x IN (SELECT object_id
              FROM objects
              WHERE deleted_date is not NULL
              AND deleted_date BETWEEN start_date_in AND end_date_in
              and object_template_id in (
           300281711,
           352903310,
          1006954225,
             9343642,
            35614291,
           339194814,
          1001781097,
          1632911215,
          1139268412,
          2059421636,
          1349101341,
         -1657672270,
          1931602976,
         -1440714292,
         -1436615854,
         -1614624973,
         -1556205486,
          -679440489,
         -1397391672,
          -541746271,
          -718648728,
          -789429729,
          -786663538,
          -610128810,
          -132487792,
         -1129945606,
          -104858180,
           -82570045)) LOOP

       v_count := v_count + 1;
       Obj_list_index := MOD(v_count, chunk_size); -- what obj index for this batch

       If Obj_list_index = 0 then
          Obj_list(chunk_size) := x.object_id; -- make sure we fill last VARRAY element
          Purge_consumables_work(Obj_list); -- bulk delete object ids in VARRAY list
          COMMIT;
       --   LOGGING.LOG_STATUS(v_count,'batch purged');
       Else
          Obj_list(Obj_list_index) := x.object_id; -- fill VARRAY with ids
       End If;

    END LOOP;

 -- LOGGING.LOG_STATUS(v_count,'Consumables object main loop exited.');

    --Now delete remaining partially full object list for objects
    If Obj_list_index != 0 then
        Obj_list.trim(chunk_size - Obj_list_index);
        Purge_consumables_work(Obj_list);
        COMMIT;
     --   LOGGING.LOG_STATUS(v_count,'Deleting chunk remainder.');
    End If;

    Obj_list.delete; -- cleanup - delete VARRAY list

 -- LOGGING.LOG_STATUS(v_count,'Finished deleting '|| to_char(v_count) ||' consumable objects.');
  RETURN 0;

    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      IF v_count = 0 THEN
        dbms_output.put_line ('never deleted');
     --   LOGGING.LOG_ERROR(SQLCODE, SQLERRM);
      END IF;
      dbms_output.put_line (v_count||': iterations');
     -- LOGGING.LOG_ERROR(SQLCODE, SQLERRM);
      RETURN SQLCODE;
  END Purge_consumables;

/**********************************************************************/
/*** Purge Consumables Work                                         ***/
/***                                                                ***/
/*** Bulk delete consumable objects marked for deletion in child    ***/
/*** object tables given a list of object ids passed in from a      ***/
/*** VARRAY. Called purge consumables function.                     ***/
/**********************************************************************/

  PROCEDURE Purge_consumables_work
  (Obj_list IN Obj_list_t)
  IS

  BEGIN

        --delete all children
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM armor WHERE object_id = Obj_list(i);

       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM battlefield_marker_objects WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM biographies nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM creature_objects nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM experience_points nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM factory_objects nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM intangible_objects nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM location_lists nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM mission_objects nologging WHERE object_id = Obj_list(i);

       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM object_variables nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM property_lists nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM resource_container_objects nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM scripts nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM tangible_objects nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM token_objects nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM vehicle_objects nologging WHERE object_id = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM weapon_objects nologging WHERE object_id = Obj_list(i);
       --delete messages target(object_id)
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM messages nologging WHERE target = Obj_list(i);
       FORALL i IN Obj_list.First..Obj_list.Last DELETE FROM objects nologging WHERE object_id = Obj_list(i);
  END Purge_consumables_work;


  --
  --
  --
  FUNCTION time_purge_consumables
    (start_date_in       in objects.deleted_date%type,
     end_date_in         in objects.deleted_date%type)
  RETURN PLS_INTEGER IS
    v_retval PLS_INTEGER := 0;
    v_start_date DATE := start_date_in;
    v_end_date DATE := end_date_in;
    v_hours NUMBER := (v_end_date - v_start_date) * 24;
    v_time NUMBER := 1;
  BEGIN
      dbms_output.put_line('Hours between dates: '||v_hours);
    FOR i IN 1..v_hours LOOP
      v_end_date := v_start_date + v_time/24;
      dbms_output.put_line ('Calling proc with Start Date: '||to_char(v_start_date, 'dd-mon-yy hh24:mi:ss')||'-'||'End Date: '||to_char(v_end_date, 'dd-mon-yy hh24:mi:ss'));
      v_retval := purge_consumables(v_start_date, v_end_date);
      v_start_date := v_start_date + v_time/24;

      EXIT WHEN v_retval <> 0;

    END LOOP;
    RETURN 0;

    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      dbms_output.put_line('Purge return value: '||v_retval);
      RETURN SQLCODE;
  END time_purge_consumables;

end purge_methods2;
/
