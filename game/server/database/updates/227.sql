declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'OBJECTS' and column_name = 'CONVERSION_ID';
  if (cnt = 0) then
    execute immediate 'alter table objects add conversion_id int';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'OBJECTS' and column_name = 'STATIC_ITEM_NAME';
  if (cnt = 0) then
    execute immediate 'alter table objects add static_item_name varchar2(128)';
    execute immediate 'alter table objects add static_item_version int';
  end if;
end;
/

DECLARE
	obj_id NUMBER;
	val VARCHAR2(128);
	CURSOR objvar_cur(objvar_name VARCHAR2) IS 
		SELECT v.object_id, v.value
		FROM object_variable_names n, object_variables v
		WHERE n.name = objvar_name
		AND n.id = v.name_id
		ORDER BY v.object_id;

BEGIN
	UPDATE objects 
	SET static_item_name = 
		DECODE (objvar_0_name, 'staticItem', objvar_0_value,
		DECODE (objvar_1_name, 'staticItem', objvar_1_value,
		DECODE (objvar_2_name, 'staticItem', objvar_2_value,
		DECODE (objvar_3_name, 'staticItem', objvar_3_value,
		DECODE (objvar_4_name, 'staticItem', objvar_4_value,
		DECODE (objvar_5_name, 'staticItem', objvar_5_value,
		DECODE (objvar_6_name, 'staticItem', objvar_6_value,
		DECODE (objvar_7_name, 'staticItem', objvar_7_value,
		DECODE (objvar_8_name, 'staticItem', objvar_9_value,
		DECODE (objvar_9_name, 'staticItem', objvar_9_value,
		DECODE (objvar_10_name, 'staticItem', objvar_10_value,
		DECODE (objvar_11_name, 'staticItem', objvar_11_value,
		DECODE (objvar_12_name, 'staticItem', objvar_12_value,
		DECODE (objvar_13_name, 'staticItem', objvar_13_value,
		DECODE (objvar_14_name, 'staticItem', objvar_14_value,
		DECODE (objvar_15_name, 'staticItem', objvar_15_value,
		DECODE (objvar_16_name, 'staticItem', objvar_16_value,
		DECODE (objvar_17_name, 'staticItem', objvar_17_value,
		DECODE (objvar_18_name, 'staticItem', objvar_18_value,
		DECODE (objvar_19_name, 'staticItem', objvar_19_value,
		static_item_name ))))) ))))) ))))) ))))) ,

	static_item_version = 
		DECODE (objvar_0_name, 'version', TO_NUMBER(objvar_0_value),
		DECODE (objvar_1_name, 'version', TO_NUMBER(objvar_1_value),
		DECODE (objvar_2_name, 'version', TO_NUMBER(objvar_2_value),
		DECODE (objvar_3_name, 'version', TO_NUMBER(objvar_3_value),
		DECODE (objvar_4_name, 'version', TO_NUMBER(objvar_4_value),
		DECODE (objvar_5_name, 'version', TO_NUMBER(objvar_5_value),
		DECODE (objvar_6_name, 'version', TO_NUMBER(objvar_6_value),
		DECODE (objvar_7_name, 'version', TO_NUMBER(objvar_7_value),
		DECODE (objvar_8_name, 'version', TO_NUMBER(objvar_9_value),
		DECODE (objvar_9_name, 'version', TO_NUMBER(objvar_9_value),
		DECODE (objvar_10_name, 'version', TO_NUMBER(objvar_10_value),
		DECODE (objvar_11_name, 'version', TO_NUMBER(objvar_11_value),
		DECODE (objvar_12_name, 'version', TO_NUMBER(objvar_12_value),
		DECODE (objvar_13_name, 'version', TO_NUMBER(objvar_13_value),
		DECODE (objvar_14_name, 'version', TO_NUMBER(objvar_14_value),
		DECODE (objvar_15_name, 'version', TO_NUMBER(objvar_15_value),
		DECODE (objvar_16_name, 'version', TO_NUMBER(objvar_16_value),
		DECODE (objvar_17_name, 'version', TO_NUMBER(objvar_17_value),
		DECODE (objvar_18_name, 'version', TO_NUMBER(objvar_18_value),
		DECODE (objvar_19_name, 'version', TO_NUMBER(objvar_19_value),
		static_item_version ))))) ))))) ))))) ))))) ,

	objvar_0_name = DECODE(objvar_0_name, 'staticItem', NULL, 'version', NULL, objvar_0_name),
	objvar_1_name = DECODE(objvar_1_name, 'staticItem', NULL, 'version', NULL, objvar_1_name),
	objvar_2_name = DECODE(objvar_2_name, 'staticItem', NULL, 'version', NULL, objvar_2_name),
	objvar_3_name = DECODE(objvar_3_name, 'staticItem', NULL, 'version', NULL, objvar_3_name),
	objvar_4_name = DECODE(objvar_4_name, 'staticItem', NULL, 'version', NULL, objvar_4_name),
	objvar_5_name = DECODE(objvar_5_name, 'staticItem', NULL, 'version', NULL, objvar_5_name),
	objvar_6_name = DECODE(objvar_6_name, 'staticItem', NULL, 'version', NULL, objvar_6_name),
	objvar_7_name = DECODE(objvar_7_name, 'staticItem', NULL, 'version', NULL, objvar_7_name),
	objvar_8_name = DECODE(objvar_8_name, 'staticItem', NULL, 'version', NULL, objvar_8_name),
	objvar_9_name = DECODE(objvar_9_name, 'staticItem', NULL, 'version', NULL, objvar_9_name),
	objvar_10_name = DECODE(objvar_10_name, 'staticItem', NULL, 'version', NULL, objvar_10_name),
	objvar_11_name = DECODE(objvar_11_name, 'staticItem', NULL, 'version', NULL, objvar_11_name),
	objvar_12_name = DECODE(objvar_12_name, 'staticItem', NULL, 'version', NULL, objvar_12_name),
	objvar_13_name = DECODE(objvar_13_name, 'staticItem', NULL, 'version', NULL, objvar_13_name),
	objvar_14_name = DECODE(objvar_14_name, 'staticItem', NULL, 'version', NULL, objvar_14_name),
	objvar_15_name = DECODE(objvar_15_name, 'staticItem', NULL, 'version', NULL, objvar_15_name),
	objvar_16_name = DECODE(objvar_16_name, 'staticItem', NULL, 'version', NULL, objvar_16_name),
	objvar_17_name = DECODE(objvar_17_name, 'staticItem', NULL, 'version', NULL, objvar_17_name),
	objvar_18_name = DECODE(objvar_18_name, 'staticItem', NULL, 'version', NULL, objvar_18_name),
	objvar_19_name = DECODE(objvar_19_name, 'staticItem', NULL, 'version', NULL, objvar_19_name),

	objvar_0_type = DECODE(objvar_0_name, 'staticItem', -1, 'version', -1, objvar_0_type),
	objvar_1_type = DECODE(objvar_1_name, 'staticItem', -1, 'version', -1, objvar_1_type),
	objvar_2_type = DECODE(objvar_2_name, 'staticItem', -1, 'version', -1, objvar_2_type),
	objvar_3_type = DECODE(objvar_3_name, 'staticItem', -1, 'version', -1, objvar_3_type),
	objvar_4_type = DECODE(objvar_4_name, 'staticItem', -1, 'version', -1, objvar_4_type),
	objvar_5_type = DECODE(objvar_5_name, 'staticItem', -1, 'version', -1, objvar_5_type),
	objvar_6_type = DECODE(objvar_6_name, 'staticItem', -1, 'version', -1, objvar_6_type),
	objvar_7_type = DECODE(objvar_7_name, 'staticItem', -1, 'version', -1, objvar_7_type),
	objvar_8_type = DECODE(objvar_8_name, 'staticItem', -1, 'version', -1, objvar_8_type),
	objvar_9_type = DECODE(objvar_9_name, 'staticItem', -1, 'version', -1, objvar_9_type),
	objvar_10_type = DECODE(objvar_10_name, 'staticItem', -1, 'version', -1, objvar_10_type),
	objvar_11_type = DECODE(objvar_11_name, 'staticItem', -1, 'version', -1, objvar_11_type),
	objvar_12_type = DECODE(objvar_12_name, 'staticItem', -1, 'version', -1, objvar_12_type),
	objvar_13_type = DECODE(objvar_13_name, 'staticItem', -1, 'version', -1, objvar_13_type),
	objvar_14_type = DECODE(objvar_14_name, 'staticItem', -1, 'version', -1, objvar_14_type),
	objvar_15_type = DECODE(objvar_15_name, 'staticItem', -1, 'version', -1, objvar_15_type),
	objvar_16_type = DECODE(objvar_16_name, 'staticItem', -1, 'version', -1, objvar_16_type),
	objvar_17_type = DECODE(objvar_17_name, 'staticItem', -1, 'version', -1, objvar_17_type),
	objvar_18_type = DECODE(objvar_18_name, 'staticItem', -1, 'version', -1, objvar_18_type),
	objvar_19_type = DECODE(objvar_19_name, 'staticItem', -1, 'version', -1, objvar_19_type),

	objvar_0_value = DECODE(objvar_0_name, 'staticItem', NULL, 'version', NULL, objvar_0_value),
	objvar_1_value = DECODE(objvar_1_name, 'staticItem', NULL, 'version', NULL, objvar_1_value),
	objvar_2_value = DECODE(objvar_2_name, 'staticItem', NULL, 'version', NULL, objvar_2_value),
	objvar_3_value = DECODE(objvar_3_name, 'staticItem', NULL, 'version', NULL, objvar_3_value),
	objvar_4_value = DECODE(objvar_4_name, 'staticItem', NULL, 'version', NULL, objvar_4_value),
	objvar_5_value = DECODE(objvar_5_name, 'staticItem', NULL, 'version', NULL, objvar_5_value),
	objvar_6_value = DECODE(objvar_6_name, 'staticItem', NULL, 'version', NULL, objvar_6_value),
	objvar_7_value = DECODE(objvar_7_name, 'staticItem', NULL, 'version', NULL, objvar_7_value),
	objvar_8_value = DECODE(objvar_8_name, 'staticItem', NULL, 'version', NULL, objvar_8_value),
	objvar_9_value = DECODE(objvar_9_name, 'staticItem', NULL, 'version', NULL, objvar_9_value),
	objvar_10_value = DECODE(objvar_10_name, 'staticItem', NULL, 'version', NULL, objvar_10_value),
	objvar_11_value = DECODE(objvar_11_name, 'staticItem', NULL, 'version', NULL, objvar_11_value),
	objvar_12_value = DECODE(objvar_12_name, 'staticItem', NULL, 'version', NULL, objvar_12_value),
	objvar_13_value = DECODE(objvar_13_name, 'staticItem', NULL, 'version', NULL, objvar_13_value),
	objvar_14_value = DECODE(objvar_14_name, 'staticItem', NULL, 'version', NULL, objvar_14_value),
	objvar_15_value = DECODE(objvar_15_name, 'staticItem', NULL, 'version', NULL, objvar_15_value),
	objvar_16_value = DECODE(objvar_16_name, 'staticItem', NULL, 'version', NULL, objvar_16_value),
	objvar_17_value = DECODE(objvar_17_name, 'staticItem', NULL, 'version', NULL, objvar_17_value),
	objvar_18_value = DECODE(objvar_18_name, 'staticItem', NULL, 'version', NULL, objvar_18_value),
	objvar_19_value = DECODE(objvar_19_name, 'staticItem', NULL, 'version', NULL, objvar_19_value)

	WHERE
	objvar_0_name = 'staticItem' OR
	objvar_1_name = 'staticItem' OR
	objvar_2_name = 'staticItem' OR
	objvar_3_name = 'staticItem' OR
	objvar_4_name = 'staticItem' OR
	objvar_5_name = 'staticItem' OR
	objvar_6_name = 'staticItem' OR
	objvar_7_name = 'staticItem' OR
	objvar_8_name = 'staticItem' OR
	objvar_9_name = 'staticItem' OR
	objvar_10_name = 'staticItem' OR
	objvar_11_name = 'staticItem' OR
	objvar_12_name = 'staticItem' OR
	objvar_13_name = 'staticItem' OR
	objvar_14_name = 'staticItem' OR
	objvar_15_name = 'staticItem' OR
	objvar_16_name = 'staticItem' OR
	objvar_17_name = 'staticItem' OR
	objvar_18_name = 'staticItem' OR
	objvar_19_name = 'staticItem' OR
	objvar_0_name = 'version' OR
	objvar_1_name = 'version' OR
	objvar_2_name = 'version' OR
	objvar_3_name = 'version' OR
	objvar_4_name = 'version' OR
	objvar_5_name = 'version' OR
	objvar_6_name = 'version' OR
	objvar_7_name = 'version' OR
	objvar_8_name = 'version' OR
	objvar_9_name = 'version' OR
	objvar_10_name = 'version' OR
	objvar_11_name = 'version' OR
	objvar_12_name = 'version' OR
	objvar_13_name = 'version' OR
	objvar_14_name = 'version' OR
	objvar_15_name = 'version' OR
	objvar_16_name = 'version' OR
	objvar_17_name = 'version' OR
	objvar_18_name = 'version' OR
	objvar_19_name = 'version';

	COMMIT;

	OPEN objvar_cur('staticItem');
	FETCH objvar_cur INTO obj_id, val;
	WHILE objvar_cur%FOUND LOOP
		UPDATE objects SET static_item_name = val WHERE object_id = obj_id;
		FETCH objvar_cur INTO obj_id, val;
	END LOOP;

	DELETE FROM object_variables WHERE name_id = (SELECT id FROM object_variable_names WHERE name = 'staticItem');

	COMMIT;

	CLOSE objvar_cur;

	OPEN objvar_cur('version');
	FETCH objvar_cur INTO obj_id, val;
	WHILE objvar_cur%FOUND LOOP
		UPDATE objects SET static_item_version = TO_NUMBER(val) WHERE object_id = obj_id;
		FETCH objvar_cur INTO obj_id, val;
	END LOOP;

	DELETE FROM object_variables WHERE name_id = (SELECT id FROM object_variable_names WHERE name = 'version');

	COMMIT;

	CLOSE objvar_cur;
END;
/

update version_number set version_number=227, min_version_number=227;
