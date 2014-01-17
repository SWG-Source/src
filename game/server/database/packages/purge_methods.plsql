CREATE OR REPLACE PACKAGE BODY purge_methods IS

  FUNCTION Purge_objects
    (date_in       in objects.deleted_date%type)
  RETURN NUMBER IS
    v_count NUMBER := 0;
  BEGIN
  --open the cursor for the object ids we want to purge.
  --we will use these ids to delete child records from other tables
    FOR x IN (SELECT /*+ INDEX (OBJECTS DELETED_OBJECT_IDX) */ object_id
              FROM objects
              WHERE deleted_date < date_in) LOOP

       IF v_count = 0 THEN
          dbms_output.put_line ('Starting to delete....');
       END IF;

       --delete all children
       DELETE FROM armor nologging WHERE object_id = x.object_id;

       DELETE FROM battlefield_marker_objects nologging WHERE object_id = x.object_id;
       DELETE FROM battlefield_participants nologging WHERE region_object_id = x.object_id;
       DELETE FROM battlefield_participants nologging WHERE character_object_id = x.object_id ;
       DELETE FROM biographies nologging WHERE object_id = x.object_id ;
       DELETE FROM building_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM cell_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM creature_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM experience_points nologging WHERE object_id = x.object_id ;
       DELETE FROM factory_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM guild_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM harvester_installation_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM installation_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM intangible_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM location_lists nologging WHERE object_id = x.object_id ;
       DELETE FROM manf_schematic_attributes nologging WHERE object_id = x.object_id ;
       DELETE FROM manf_schematic_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM manufacture_inst_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM mission_objects nologging WHERE object_id = x.object_id ;

       DELETE FROM object_variables nologging WHERE object_id = x.object_id ;
       DELETE FROM planet_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM player_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM property_lists nologging WHERE object_id = x.object_id ;
       DELETE FROM resource_container_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM scripts nologging WHERE object_id = x.object_id ;
       DELETE FROM static_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM tangible_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM token_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM universe_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM vehicle_objects nologging WHERE object_id = x.object_id ;
       DELETE FROM waypoints nologging WHERE object_id = x.object_id ;
       DELETE FROM weapon_objects nologging WHERE object_id = x.object_id ;
       --delete messages target(object_id)
       DELETE FROM messages nologging WHERE target = x.object_id ;

       v_count := v_count + 1;

       COMMIT;

    END LOOP;

    --delete parent
    DELETE /*+ INDEX (OBJECTS DELETED_OBJECT_IDX) */ FROM objects
    nologging WHERE deleted_date < date_in;

    dbms_output.put_line (v_count||': iterations');

    COMMIT;
    RETURN 0;

    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      IF v_count = 0 THEN
        dbms_output.put_line ('never deleted');
      END IF;
      dbms_output.put_line (v_count||': iterations');
      RETURN SQLCODE;
  END Purge_objects;
  --
  --
  --
  FUNCTION time_purge
    (date_in          IN VARCHAR2)
  RETURN INTEGER IS
    v_retval NUMBER := 0;
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

end purge_methods;
/
