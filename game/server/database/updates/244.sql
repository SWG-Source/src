DECLARE
  v_new_obj_var_names_id object_variable_names.ID%TYPE;  
BEGIN
  
  SELECT MAX(o.id) + 1
  INTO v_new_obj_var_names_id
  FROM object_variable_names o;
  
  INSERT INTO object_variables(object_id,
                               name_id,
                               type,
                               value,
                               detached)
  SELECT tangible_objects.owner_id,
         v_new_obj_var_names_id,
         6,
         tangible_objects.object_id,
         0
  FROM tangible_objects,
       accounts,
       players
  WHERE accounts.house_id > 0
  AND tangible_objects.object_id = accounts.house_id
  AND accounts.station_id = players.station_id
  AND tangible_objects.owner_id = players.character_object;
  
  dbms_output.put_line('Rows inserted: ' || SQL%ROWCOUNT);
  
  IF SQL%ROWCOUNT > 0
  THEN
    INSERT INTO object_variable_names(id,name)
    VALUES(v_new_obj_var_names_id,'residenceHouseId');

    dbms_output.put_line('New id for residenceHouseId: ' ||  v_new_obj_var_names_id);
  END IF;
EXCEPTION
  WHEN OTHERS THEN
    RAISE;
END;
/

update version_number set version_number=244, min_version_number=244;
