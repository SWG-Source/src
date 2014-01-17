CREATE OR REPLACE PACKAGE  "ITEM_CONVERSION" as
	procedure StartItemConversion (rbs_to_use varchar2 default null, conversion_id number default 1);
	procedure DoSingleObject (objectId number, conversion_id number default 1);
end;
/

CREATE OR REPLACE PACKAGE BODY  "ITEM_CONVERSION" as
	-- private type defs
	type objVarType is record (
		object_id number := NULL,
		objvar_name varchar2(500) := NULL,
		objvar_type number := NULL,
		objvar_value varchar2(1000)  := NULL,
		objvar_name_id number := 0,
		old_objvar_name_id number := 0);

	type objRowType is record (
		object_id number,
		object_template_id number,
		object_name varchar2(127), name_string_table varchar2(500), name_string_text varchar2(500),
		objvar_0_name varchar2(50), objvar_0_type number, objvar_0_value varchar2(50),
		objvar_1_name varchar2(50), objvar_1_type number, objvar_1_value varchar2(50),
		objvar_2_name varchar2(50), objvar_2_type number, objvar_2_value varchar2(50),
		objvar_3_name varchar2(50), objvar_3_type number, objvar_3_value varchar2(50),
		objvar_4_name varchar2(50), objvar_4_type number, objvar_4_value varchar2(50),
		objvar_5_name varchar2(50), objvar_5_type number, objvar_5_value varchar2(50),
		objvar_6_name varchar2(50), objvar_6_type number, objvar_6_value varchar2(50),
		objvar_7_name varchar2(50), objvar_7_type number, objvar_7_value varchar2(50),
		objvar_8_name varchar2(50), objvar_8_type number, objvar_8_value varchar2(50),
		objvar_9_name varchar2(50), objvar_9_type number, objvar_9_value varchar2(50),
		objvar_10_name varchar2(50), objvar_10_type number, objvar_10_value varchar2(50),
		objvar_11_name varchar2(50), objvar_11_type number, objvar_11_value varchar2(50),
		objvar_12_name varchar2(50), objvar_12_type number, objvar_12_value varchar2(50),
		objvar_13_name varchar2(50), objvar_13_type number, objvar_13_value varchar2(50),
		objvar_14_name varchar2(50), objvar_14_type number, objvar_14_value varchar2(50),
		objvar_15_name varchar2(50), objvar_15_type number, objvar_15_value varchar2(50),
		objvar_16_name varchar2(50), objvar_16_type number, objvar_16_value varchar2(50),
		objvar_17_name varchar2(50), objvar_17_type number, objvar_17_value varchar2(50),
		objvar_18_name varchar2(50), objvar_18_type number, objvar_18_value varchar2(50),
		objvar_19_name varchar2(50), objvar_19_type number, objvar_19_value varchar2(50),
		scriptlist  varchar2(2000));

	type objVarRowType is record (
		object_id number,
		objvar_name_id number,
		objvar_name varchar2(500), 
		objvar_type number, 
		objvar_value varchar2(1000));
		
	type weaponRowType is record (
		object_id number,
		min_damage number,
		max_damage number,
		attack_speed float,
		wound_chance float,
		accuracy number,
		attack_cost number,
		min_range float,
		max_range float,
		damage_type number,
		elemental_type number,
		elemental_value	number);
	
	type manfSchematicRowType is record (
		object_id number,
		draft_schematic number);
		
	type manfAttributeRowType is record (
		object_id number,
		attribute varchar2(500),
		value float);

	type rangeType is varray(4) of number;

	type rangeTableType is table of rangeType;

	type numTable is table of number;
	
	type varcharTable is table of varchar2(500);

	type numTableVarchar is table of number
		index by varchar2(500);
		
	type varcharTableNum is table of varchar2(500)
		index by binary_integer;

	type objectObjVarArray is varray(20) of objVarType;

	type objVarTable is table of objVarType
		index by varchar2(500);

	type dotObjVarTable is table of objVarType
		index by varchar2(10);

	type dotArrayVarchar is table of dotObjVarTable
		index by varchar2(500);

	type numTableNumber is table of number 
		 index by binary_integer;

	type dotsMatrixType is table of numTableNumber index by varchar2(50);
	
	type manfAttributesTable is table of manfAttributeRowType
		index by varchar2(500);

	-- private global containers
	use_rbs			varchar2(50);
	object_id       number;
	free_slots      numTable;				-- empty OBJECTS ObjVar columns
	slot_map        numTableVarchar;		-- map from ObjVar name to OBJECTS slot
	Obj_ObjVars     objectObjVarArray;		-- array of ObjVars in OBJECTS
	OV_ObjVars      objVarTable;			-- map by name of ObjVars in OBJECT_VARIABLES
	dot_list        dotArrayVarchar;		-- map from extracted DoT name and class to ObjVar
											-- example: dot_list('actionShot1')('type')
	isObjModified   boolean;				-- tracks if any of the OBJECTS data has changed
	updated_objvars varcharTable;			-- list of updated ObjVar names in OBJECT_VARIABLES
	deleted_objvars varcharTable;			-- list of deleted ObjVar names in OBJECT_VARIABLES
	added_objvars   varcharTable;			-- list of added ObjVar names in OBJECT_VARIABLES
	isWeaponObj     boolean;				-- tracks if the object has WEAPON_OBJECTS data
	isSchematic     boolean;				-- tracks if the object has MANF_SCHEMATIC_OBJECTS data
	isFactory       boolean;
	script_list     varchar2(2000);
	object_name     varchar2(127);
	name_table      varchar2(500);
	name_text       varchar2(500);
	template_id     number;
	new_template_id number;
	new_schematic_id number;
	err_num 		number;
	weapon_attribs  weaponRowType;			-- record for weapon attributes for current object
	manf_attributes manfAttributesTable;    -- map of attributes in MANF_SCHEMATIC_ATTRIBUTES
	updated_attribs varcharTable;			-- list of updated attributes in MANF_SCHEMATIC_ATTRIBUTES
	deleted_attribs varcharTable;			-- list of deleted attributes in MANF_SCHEMATIC_ATTRIBUTES
	added_attribs   varcharTable;			-- list of added attributes in MANF_SCHEMATIC_ATTRIBUTES
	
	-- private lookup tables
	templateIdSet   numTableNumber;			-- set of template ids to be converted
	objvarNameIdMap numTableVarchar;		-- map from ObjVar name to OBJECT_VARIABLES_NAMES ID
	dots_matrix     dotsMatrixType;			-- map from DoT type and attribute to ObjVar
											-- example: dots_matrix('fire')(6)
	schematic_map   numTableNumber;			-- map of schematic to created template
	templateNameTable varcharTableNum;		-- map of new template ids to string_name_table
	templateNameText  varcharTableNum;		-- map of new template ids to string_name_text
	
	
	-- private constants
	blankObjVar     objVarType;				-- can't use CONSTANT properly with records :(
	
	-- private function declaration
	procedure DoItemConversionBatch(start_objectid number, end_objectid number, conversion_id number);
	function InitializeData return boolean;
	
	procedure AddObjVar (objvar_name varchar2, objvar_type number, objvar_value varchar2);
	procedure DeleteObjVar (objVarName in varchar2);
	procedure ModifyObjVar (oldObjVarName varchar2, objvar_name varchar2, objvar_type number, objvar_value varchar2);
	procedure ModifyObjVar (oldObjVarName varchar2, objvar_name varchar2, objvar_type number, rangeTable rangeTableType);
	procedure RenameObjVar (oldObjVarName varchar2, objvar_name varchar2);
	function GetObjVarValue (objvar_name varchar2) return varchar2;
	procedure AddScript (newScript varchar2);
	procedure RemoveScript (oldScript varchar2);
	procedure SetNewTemplateId (newTemplateId number);
	procedure SetTemplateText (newTemplateId number);
	procedure InvalidateSchematic;
	procedure AddAttrib (attrib_name varchar2, attrib_value float);
	procedure DeleteAttrib (attrib_name in varchar2);
	procedure ModifyAttrib (old_attrib_name varchar2, attrib_name varchar2, attrib_value float);

	procedure DeleteOldArmorObjvars;
	procedure SetGenProtection(rangeTable rangeTableType);
	procedure SetArmorCondLevelCat(condition varchar2, lev varchar2, category varchar2);
	procedure SetArmorLevel(lev varchar);
	procedure SetArmorCategory(category varchar2);
	procedure SetGpAndCond(gp_name varchar2, gp_val varchar2, cond_name varchar2, cond_val varchar2);
	
	procedure DeleteCraftingComponents;
	procedure CopySchematicAttributes;
	procedure HandleFoodSchematic;
	procedure HandleFoodCrate;
	procedure ConvertToStimpack(stim_template number,healing_power number, combat_level number);
	procedure ConvertToEnhancer(enh_template number, enh_name varchar2, enh_power number);
	procedure ConvertToPetStimpack(stim_template number,healing_power number);
	procedure ConvertSpice(spice_name varchar2);
	
	procedure HandleWeaponDots;
	procedure SetWeaponBaseStats(attack number, acc number, minrange number, maxrange number, damagetype number);
	procedure SetMinDamage (min_damage number);
	procedure SetMaxDamage (max_damage number);
	procedure SetAttackSpeed (attack_speed number);
	procedure SetWoundChance (wound_chance number);
	procedure SetAccuracy (accuracy number);
	procedure SetAttackCost (attack_cost number);
	procedure SetMinRange (min_range number);
	procedure SetMaxRange (max_range number);
	procedure SetDamageType (dmg_type number);
	procedure SetElementalType (ele_type number);
	procedure SetElementalValue (ele_value number);
	procedure ConvertAttribValue(attribute varchar2, rangeTable rangeTableType);
	procedure ConvertMinDamage (rangeTable rangeTableType);
	procedure ConvertMaxDamage (rangeTable rangeTableType);
	procedure ConvertAttackSpeed (rangeTable rangeTableType);
	procedure ConvertWoundChance (rangeTable rangeTableType);
	
	procedure ConvertRangedPowerup;
	procedure ConvertMeleePowerup;
	procedure ConvertMinePowerup;

	function ConvertValue (oldvalue number, rangeTable rangeTableType) return number;

	procedure CheckObjVarForDot (objvar objVarType);

	function DotsMatrix(dot_type varchar2, dot_attrib number)
		return number;
	function SchematicMap(schematic_id number)
		return number;

	-- public function definitions
	procedure StartItemConversion (rbs_to_use varchar2, conversion_id number)
	is
		maxObjectId number;
		startObjectId number := 0;
		starttime date := sysdate;
	begin
		use_rbs := rbs_to_use;
		if use_rbs is not null then
			execute immediate 'set transaction use rollback segment ' || use_rbs;
		end if;
		if (InitializeData) then
			-- loop through all objects in batches
			select max(object_id) into maxObjectId from objects;
			while startObjectId < maxObjectId loop
				dbms_output.put_line('starting batch: ' || startObjectId || ' time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
				DoItemConversionBatch(startObjectId, startObjectId + 19999999999, conversion_id);
				commit;
				if use_rbs is not null then
					execute immediate 'set transaction use rollback segment ' || use_rbs;
				end if;
				startObjectId := startObjectId + 20000000000;
			end loop;
		end if;
		dbms_output.put_line('start time = ' || to_char(starttime, 'MM/DD/YY HH24:MI:SS'));
		dbms_output.put_line('final time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
		dbms_output.put_line('total time = ' || to_char(trunc((sysdate - starttime)*24*60)) || ' mins');
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error in StartItemConversion - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in StartItemConversion');
	end;

	procedure DoSingleObject (objectId number, conversion_id number)
	is
	begin
		if (InitializeData) then
			DoItemConversionBatch(objectId, objectId, conversion_id);
		end if;
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error in DoSingleObject - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in ObjVarUpdates');
	end;

	-- private function definitions
	procedure DoItemConversionBatch(start_objectid number, end_objectid number, conversion_id number)
	is
		-- cursors
		objRow objRowType;
		cursor objCursor (start_id number, end_id number) is
		select
			o.object_id,
			o.object_template_id,
			o.object_name, o.name_string_table, o.name_string_text,
			o.objvar_0_name, o.objvar_0_type, o.objvar_0_value,
			o.objvar_1_name, o.objvar_1_type, o.objvar_1_value,
			o.objvar_2_name, o.objvar_2_type, o.objvar_2_value,
			o.objvar_3_name, o.objvar_3_type, o.objvar_3_value,
			o.objvar_4_name, o.objvar_4_type, o.objvar_4_value,
			o.objvar_5_name, o.objvar_5_type, o.objvar_5_value,
			o.objvar_6_name, o.objvar_6_type, o.objvar_6_value,
			o.objvar_7_name, o.objvar_7_type, o.objvar_7_value,
			o.objvar_8_name, o.objvar_8_type, o.objvar_8_value,
			o.objvar_9_name, o.objvar_9_type, o.objvar_9_value,
			o.objvar_10_name, o.objvar_10_type, o.objvar_10_value,
			o.objvar_11_name, o.objvar_11_type, o.objvar_11_value,
			o.objvar_12_name, o.objvar_12_type, o.objvar_12_value,
			o.objvar_13_name, o.objvar_13_type, o.objvar_13_value,
			o.objvar_14_name, o.objvar_14_type, o.objvar_14_value,
			o.objvar_15_name, o.objvar_15_type, o.objvar_15_value,
			o.objvar_16_name, o.objvar_16_type, o.objvar_16_value,
			o.objvar_17_name, o.objvar_17_type, o.objvar_17_value,
			o.objvar_18_name, o.objvar_18_type, o.objvar_18_value,
			o.objvar_19_name, o.objvar_19_type, o.objvar_19_value,
			script_list
		from objects o
		where object_id between start_id and end_id
		order by object_id;

		objVarRow objVarRowType;
		cursor objVarCursor (start_id number, end_id number) is
		select
			v.object_id,
			v.name_id,
  			nvl(n.name, to_char(n.id)),
			v.type,
			v.value
  		from object_variables v, object_variable_names n
		where v.object_id between start_id and end_id
			and nvl(v.detached,0) = 0
			and v.name_id = n.id
		order by v.object_id;

		weaponRow weaponRowType;
		cursor weaponCursor (start_id number, end_id number) is
		select
			object_id,
			min_damage,
			max_damage,
			attack_speed,
			wound_chance,
			accuracy,
			attack_cost,
			min_range,
			max_range,
			damage_type,
			elemental_type,
			elemental_value
		from weapon_objects
		where object_id between start_id and end_id
		order by object_id;

		manfSchematicRow manfSchematicRowType;
		cursor schematicCursor (start_id number, end_id number) is
		select
			object_id,
			draft_schematic
		from manf_schematic_objects
		where object_id between start_id and end_id
		order by object_id;

		manfAttributeRow manfAttributeRowType;
		cursor attributeCursor (start_id number, end_id number) is
		select
			object_id,
			attribute_type,
			value
		from manf_schematic_attributes
		where object_id between start_id and end_id
		order by object_id;

		-- local variables
		i          number;
		n          number;
		idx        number := 0;
		cnt        number := 0;
		cvt        number := 0;
		bool       boolean;
		tempObjVar objVarType;
		stmt       varchar2(4000);
		prefix     varchar2(100);
	begin
		dbms_output.put_line('start time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
		open objCursor (start_objectid, end_objectid);
		fetch objCursor into objRow;
		dbms_output.put_line('object cursor time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
		open objVarCursor (start_objectid, end_objectid);
		fetch objVarCursor into objVarRow;
		dbms_output.put_line('objvar cursor time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
		open weaponCursor (start_objectid, end_objectid);
		fetch weaponCursor into weaponRow;
		dbms_output.put_line('weapon cursor time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
		open schematicCursor (start_objectid, end_objectid);
		fetch schematicCursor into manfSchematicRow;
		dbms_output.put_line('schematic cursor time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
		open attributeCursor (start_objectid, end_objectid);
		fetch attributeCursor into manfAttributeRow;
		dbms_output.put_line('attribute cursor time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));

		while objCursor%found loop
			begin
			if templateIdSet.exists(objRow.object_template_id) then
				-- Clear Data
				free_slots.delete;
				free_slots := numTable(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20);
				slot_map.delete;
				for i in 1..20 loop
					Obj_ObjVars(i) := blankObjVar;
				end loop;
				OV_ObjVars.delete;
				dot_list.delete;
				updated_objvars.trim(updated_objvars.count);
				deleted_objvars.trim(deleted_objvars.count);
				added_objvars.trim(added_objvars.count);
				manf_attributes.delete;
				updated_attribs.trim(updated_attribs.count);
				deleted_attribs.trim(deleted_attribs.count);
				added_attribs.trim(added_attribs.count);
				isObjModified := FALSE;
				isWeaponObj := FALSE;
				isSchematic := FALSE;
				isFactory := FALSE;
				prefix := 'crafting_components.';
				new_schematic_id := 0;
				weapon_attribs := null;
				
				-- Load the data into variables
				object_id := objRow.object_id;
				script_list := objRow.scriptList;
				object_name := objRow.object_name;
				name_table := objRow.name_string_table;
				name_text := objRow.name_string_text;
				template_id := objRow.object_template_id;
				new_template_id := template_id;

				Obj_ObjVars(1).object_id := objRow.object_id;    -- might be able to eliminate
				Obj_ObjVars(1).objvar_name := objRow.objvar_0_name;
				Obj_ObjVars(1).objvar_type := objRow.objvar_0_type;
				Obj_ObjVars(1).objvar_value := objRow.objvar_0_value;
				if ((objRow.objvar_0_name is not null) and (objRow.objvar_0_type != -1))then
					slot_map(objRow.objvar_0_name) := 1;
					CheckObjVarForDot(Obj_ObjVars(1));
					free_slots.delete(1);
				end if;

				Obj_ObjVars(2).object_id := objRow.object_id;
				Obj_ObjVars(2).objvar_name := objRow.objvar_1_name;
				Obj_ObjVars(2).objvar_type := objRow.objvar_1_type;
				Obj_ObjVars(2).objvar_value := objRow.objvar_1_value;
				if ((objRow.objvar_1_name is not null) and (objRow.objvar_1_type != -1))then
					slot_map(objRow.objvar_1_name) := 2;
					CheckObjVarForDot(Obj_ObjVars(2));
					free_slots.delete(2);
				end if;

				Obj_ObjVars(3).object_id := objRow.object_id;
				Obj_ObjVars(3).objvar_name := objRow.objvar_2_name;
				Obj_ObjVars(3).objvar_type := objRow.objvar_2_type;
				Obj_ObjVars(3).objvar_value := objRow.objvar_2_value;
				if ((objRow.objvar_2_name is not null) and (objRow.objvar_2_type != -1))then
					slot_map(objRow.objvar_2_name) := 3;
					CheckObjVarForDot(Obj_ObjVars(3));
					free_slots.delete(3);
				end if;

				Obj_ObjVars(4).object_id := objRow.object_id;
				Obj_ObjVars(4).objvar_name := objRow.objvar_3_name;
				Obj_ObjVars(4).objvar_type := objRow.objvar_3_type;
				Obj_ObjVars(4).objvar_value := objRow.objvar_3_value;
				if ((objRow.objvar_3_name is not null) and (objRow.objvar_3_type != -1))then
					slot_map(objRow.objvar_3_name) := 4;
					CheckObjVarForDot(Obj_ObjVars(4));
					free_slots.delete(4);
				end if;

				Obj_ObjVars(5).object_id := objRow.object_id;
				Obj_ObjVars(5).objvar_name := objRow.objvar_4_name;
				Obj_ObjVars(5).objvar_type := objRow.objvar_4_type;
				Obj_ObjVars(5).objvar_value := objRow.objvar_4_value;
				if ((objRow.objvar_4_name is not null) and (objRow.objvar_4_type != -1))then
					slot_map(objRow.objvar_4_name) := 5;
					CheckObjVarForDot(Obj_ObjVars(5));
					free_slots.delete(5);
				end if;

				Obj_ObjVars(6).object_id := objRow.object_id;
				Obj_ObjVars(6).objvar_name := objRow.objvar_5_name;
				Obj_ObjVars(6).objvar_type := objRow.objvar_5_type;
				Obj_ObjVars(6).objvar_value := objRow.objvar_5_value;
				if ((objRow.objvar_5_name is not null) and (objRow.objvar_5_type != -1))then
					slot_map(objRow.objvar_5_name) := 6;
					CheckObjVarForDot(Obj_ObjVars(6));
					free_slots.delete(6);
				end if;

				Obj_ObjVars(7).object_id := objRow.object_id;
				Obj_ObjVars(7).objvar_name := objRow.objvar_6_name;
				Obj_ObjVars(7).objvar_type := objRow.objvar_6_type;
				Obj_ObjVars(7).objvar_value := objRow.objvar_6_value;
				if ((objRow.objvar_6_name is not null) and (objRow.objvar_6_type != -1))then
					slot_map(objRow.objvar_6_name) := 7;
					CheckObjVarForDot(Obj_ObjVars(7));
					free_slots.delete(7);
				end if;

				Obj_ObjVars(8).object_id := objRow.object_id;
				Obj_ObjVars(8).objvar_name := objRow.objvar_7_name;
				Obj_ObjVars(8).objvar_type := objRow.objvar_7_type;
				Obj_ObjVars(8).objvar_value := objRow.objvar_7_value;
				if ((objRow.objvar_7_name is not null) and (objRow.objvar_7_type != -1))then
					slot_map(objRow.objvar_7_name) := 8;
					CheckObjVarForDot(Obj_ObjVars(8));
					free_slots.delete(8);
				end if;

				Obj_ObjVars(9).object_id := objRow.object_id;
				Obj_ObjVars(9).objvar_name := objRow.objvar_8_name;
				Obj_ObjVars(9).objvar_type := objRow.objvar_8_type;
				Obj_ObjVars(9).objvar_value := objRow.objvar_8_value;
				if ((objRow.objvar_8_name is not null) and (objRow.objvar_8_type != -1))then
					slot_map(objRow.objvar_8_name) := 9;
					CheckObjVarForDot(Obj_ObjVars(9));
					free_slots.delete(9);
				end if;

				Obj_ObjVars(10).object_id := objRow.object_id;
				Obj_ObjVars(10).objvar_name := objRow.objvar_9_name;
				Obj_ObjVars(10).objvar_type := objRow.objvar_9_type;
				Obj_ObjVars(10).objvar_value := objRow.objvar_9_value;
				if ((objRow.objvar_9_name is not null) and (objRow.objvar_9_type != -1))then
					slot_map(objRow.objvar_9_name) := 10;
					CheckObjVarForDot(Obj_ObjVars(10));
					free_slots.delete(10);
				end if;

				Obj_ObjVars(11).object_id := objRow.object_id;
				Obj_ObjVars(11).objvar_name := objRow.objvar_10_name;
				Obj_ObjVars(11).objvar_type := objRow.objvar_10_type;
				Obj_ObjVars(11).objvar_value := objRow.objvar_10_value;
				if ((objRow.objvar_10_name is not null) and (objRow.objvar_10_type != -1))then
					slot_map(objRow.objvar_10_name) := 11;
					CheckObjVarForDot(Obj_ObjVars(11));
					free_slots.delete(11);
				end if;

				Obj_ObjVars(12).object_id := objRow.object_id;
				Obj_ObjVars(12).objvar_name := objRow.objvar_11_name;
				Obj_ObjVars(12).objvar_type := objRow.objvar_11_type;
				Obj_ObjVars(12).objvar_value := objRow.objvar_11_value;
				if ((objRow.objvar_11_name is not null) and (objRow.objvar_11_type != -1))then
					slot_map(objRow.objvar_11_name) := 12;
					CheckObjVarForDot(Obj_ObjVars(12));
					free_slots.delete(12);
				end if;

				Obj_ObjVars(13).object_id := objRow.object_id;
				Obj_ObjVars(13).objvar_name := objRow.objvar_12_name;
				Obj_ObjVars(13).objvar_type := objRow.objvar_12_type;
				Obj_ObjVars(13).objvar_value := objRow.objvar_12_value;
				if ((objRow.objvar_12_name is not null) and (objRow.objvar_12_type != -1))then
					slot_map(objRow.objvar_12_name) := 13;
					CheckObjVarForDot(Obj_ObjVars(13));
					free_slots.delete(13);
				end if;

				Obj_ObjVars(14).object_id := objRow.object_id;
				Obj_ObjVars(14).objvar_name := objRow.objvar_13_name;
				Obj_ObjVars(14).objvar_type := objRow.objvar_13_type;
				Obj_ObjVars(14).objvar_value := objRow.objvar_13_value;
				if ((objRow.objvar_13_name is not null) and (objRow.objvar_13_type != -1))then
					slot_map(objRow.objvar_13_name) := 14;
					CheckObjVarForDot(Obj_ObjVars(14));
					free_slots.delete(14);
				end if;

				Obj_ObjVars(15).object_id := objRow.object_id;
				Obj_ObjVars(15).objvar_name := objRow.objvar_14_name;
				Obj_ObjVars(15).objvar_type := objRow.objvar_14_type;
				Obj_ObjVars(15).objvar_value := objRow.objvar_14_value;
				if ((objRow.objvar_14_name is not null) and (objRow.objvar_14_type != -1))then
					slot_map(objRow.objvar_14_name) := 15;
					CheckObjVarForDot(Obj_ObjVars(15));
					free_slots.delete(15);
				end if;

				Obj_ObjVars(16).object_id := objRow.object_id;
				Obj_ObjVars(16).objvar_name := objRow.objvar_15_name;
				Obj_ObjVars(16).objvar_type := objRow.objvar_15_type;
				Obj_ObjVars(16).objvar_value := objRow.objvar_15_value;
				if ((objRow.objvar_15_name is not null) and (objRow.objvar_15_type != -1))then
					slot_map(objRow.objvar_15_name) := 16;
					CheckObjVarForDot(Obj_ObjVars(16));
					free_slots.delete(16);
				end if;

				Obj_ObjVars(17).object_id := objRow.object_id;
				Obj_ObjVars(17).objvar_name := objRow.objvar_16_name;
				Obj_ObjVars(17).objvar_type := objRow.objvar_16_type;
				Obj_ObjVars(17).objvar_value := objRow.objvar_16_value;
				if ((objRow.objvar_16_name is not null) and (objRow.objvar_16_type != -1))then
					slot_map(objRow.objvar_16_name) := 17;
					CheckObjVarForDot(Obj_ObjVars(17));
					free_slots.delete(17);
				end if;

				Obj_ObjVars(18).object_id := objRow.object_id;
				Obj_ObjVars(18).objvar_name := objRow.objvar_17_name;
				Obj_ObjVars(18).objvar_type := objRow.objvar_17_type;
				Obj_ObjVars(18).objvar_value := objRow.objvar_17_value;
				if ((objRow.objvar_17_name is not null) and (objRow.objvar_17_type != -1))then
					slot_map(objRow.objvar_17_name) := 18;
					CheckObjVarForDot(Obj_ObjVars(18));
					free_slots.delete(18);
				end if;

				Obj_ObjVars(19).object_id := objRow.object_id;
				Obj_ObjVars(19).objvar_name := objRow.objvar_18_name;
				Obj_ObjVars(19).objvar_type := objRow.objvar_18_type;
				Obj_ObjVars(19).objvar_value := objRow.objvar_18_value;
				if ((objRow.objvar_18_name is not null) and (objRow.objvar_18_type != -1))then
					slot_map(objRow.objvar_18_name) := 19;
					CheckObjVarForDot(Obj_ObjVars(19));
					free_slots.delete(19);
				end if;

				Obj_ObjVars(20).object_id := objRow.object_id;
				Obj_ObjVars(20).objvar_name := objRow.objvar_19_name;
				Obj_ObjVars(20).objvar_type := objRow.objvar_19_type;
				Obj_ObjVars(20).objvar_value := objRow.objvar_19_value;
				if ((objRow.objvar_19_name is not null) and (objRow.objvar_19_type != -1))then
					slot_map(objRow.objvar_19_name) := 20;
					CheckObjVarForDot(Obj_ObjVars(20));
					free_slots.delete(20);
				end if;


				while (objVarCursor%found and objVarRow.object_id <= objRow.object_id) loop
					if objVarRow.object_id = objRow.object_id then
						tempObjVar := blankObjVar;
						tempObjVar.object_id := objVarRow.object_id;
						tempObjVar.objvar_name := objVarRow.objvar_name;
						tempObjVar.objvar_type := objVarRow.objvar_type;
						tempObjVar.objvar_value := objVarRow.objvar_value;
						tempObjVar.objvar_name_id := objVarRow.objvar_name_id;
						OV_ObjVars(objVarRow.objvar_name) := tempObjVar;
						slot_map(objVarRow.objvar_name) := 21;
						CheckObjVarForDot(tempObjVar);
					end if;
					fetch objVarCursor into objVarRow;
				end loop;

				while (weaponCursor%found and weaponRow.object_id <= objRow.object_id) loop
					if weaponRow.object_id = objRow.object_id then
						weapon_attribs := weaponRow;
						isWeaponObj := TRUE;
					end if;
					fetch weaponCursor into weaponRow;
				end loop;

				-- check to see if it is a manufacturing schematic
				if template_id = -144833342 then
					while (schematicCursor%found and manfSchematicRow.object_id <= objRow.object_id) loop
						if manfSchematicRow.object_id = objRow.object_id then
							template_id := SchematicMap(manfSchematicRow.draft_schematic);
							if template_id != 0 then
								isSchematic := TRUE;
								prefix := 'crafting_attributes.crafting:';
							end if;
						end if;
						fetch schematicCursor into manfSchematicRow;
					end loop;
					while (attributeCursor%found and manfAttributeRow.object_id <= objRow.object_id) loop
						if manfAttributeRow.object_id = objRow.object_id then
							manf_attributes(manfAttributeRow.attribute) := manfAttributeRow;
						end if;
						fetch attributeCursor into manfAttributeRow;
					end loop;
				-- check to see if it a weapon (162889564) or armor (1120303977) factory crate
				-- or food (-200781577) or chemicals (-1966544754)
				-- or electronics (-754298423) or clothing (-1207917085)
				elsif template_id = 162889564 or template_id = 1120303977 
						or template_id = -200781577 or template_id = -1966544754 
						or template_id = -754298423 or template_id = -1207917085 then
					if slot_map.exists('draftSchematic') then
						if slot_map('draftSchematic') < 21 then
							template_id := SchematicMap(to_number(Obj_ObjVars(slot_map('draftSchematic')).objvar_value));
						else
							template_id := SchematicMap(to_number(OV_ObjVars('draftSchematic').objvar_value));
						end if;
						isFactory := TRUE;
						prefix := 'crafting_attributes.crafting:';
						DeleteObjVar('ingr.*');
					end if;
				end if;

				-- Do the conversions
-- ****** BEGIN ARMOR CONVERSION RULES ******
				--object/tangible/wearables/armor/bone/armor_bone_s01_bicep_l.iff
				if template_id = 1386184862 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1324608047);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_bicep_r.iff
				elsif template_id = 575325453 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1042722750);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_boots.iff
				elsif template_id = 1652891098 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1870545359);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_bracer_l.iff
				elsif template_id = 1185807341 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,500347981);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_bracer_r.iff
				elsif template_id = 913882750 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1829237214);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_chest_plate.iff
				elsif template_id = 1994709940 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1282818768);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_gloves.iff
				elsif template_id = -1489599533 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1839665152);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_helmet.iff
				elsif template_id = 301359835 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-613945080);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_leggings.iff
				elsif template_id = -913758478 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1829344942);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_bicep_l.iff
				elsif template_id = 306929321 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1324608047);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_bicep_r.iff
				elsif template_id = 1654528826 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1042722750);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_boots.iff
				elsif template_id = -1981228116 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1870545359);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_bracer_l.iff
				elsif template_id = -1493844838 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,500347981);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_bracer_r.iff
				elsif template_id = -702482167 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1829237214);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_chest_plate.iff
				elsif template_id = 1360182635 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1282818768);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_gloves.iff
				elsif template_id = 605165771 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1839665152);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_helmet.iff
				elsif template_id = -1831667261 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-613945080);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_leggings.iff
				elsif template_id = 701587845 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1829344942);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_belt.iff
				elsif template_id = -1634646786 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
					SetNewTemplateId(2099054288);		--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_crafted_belt.iff
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_bicep_l.iff
				elsif template_id = 819427966 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
					SetNewTemplateId(-852325331);		--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_crafted_bicep_l.iff
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_bicep_r.iff
				elsif template_id = 1073889261 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
					SetNewTemplateId(-1108883010);		--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_crafted_bicep_r.iff
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_boots.iff
				elsif template_id = 1449316778 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
					SetNewTemplateId(315293450);		--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_crafted_boots.iff
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_bracer_l.iff
				elsif template_id = -1418664428 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
					SetNewTemplateId(-151793970);		--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_crafted_bracer_l.iff
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_bracer_r.iff
				elsif template_id = -609901689 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
					SetNewTemplateId(-2044294307);		--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_crafted_bracer_r.iff
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_chest_plate.iff
				elsif template_id = -1782480140 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
					SetNewTemplateId(1649659380);		--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_crafted_chest_plate.iff
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_gloves.iff
				elsif template_id = -2016614241 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
					SetNewTemplateId(140529204);		--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_crafted_gloves.iff
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_helmet.iff
				elsif template_id = 822872471 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
					SetNewTemplateId(-1096765636);		--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_crafted_helmet.iff
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_leggings.iff
				elsif template_id = 609499915 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
					SetNewTemplateId(2044138449);		--object/tangible/wearables/armor/bounty_hunter/armor_bounty_hunter_crafted_leggings.iff
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_bicep_l.iff
				elsif template_id = 1026806068 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1366672887);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_bicep_r.iff
				elsif template_id = 1306956967 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,564166756);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_boots.iff
				elsif template_id = 1503133620 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1773284434);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_bracer_l.iff
				elsif template_id = 2115381959 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1582582732);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_bracer_r.iff
				elsif template_id = 247689044 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-780209753);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_chest_plate.iff
				elsif template_id = 725211246 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1253715791);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_gloves.iff
				elsif template_id = 1252870946 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-180061178);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_helmet.iff
				elsif template_id = -59921878 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1132764430);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_leggings.iff
				elsif template_id = -248317992 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,781150507);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_bicep_l.iff
				elsif template_id = -171534277 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1366672887);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_bicep_r.iff
				elsif template_id = -2062295640 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,564166756);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_boots.iff
				elsif template_id = -127140925 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1773284434);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_bracer_l.iff
				elsif template_id = -253929242 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1582582732);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_bracer_r.iff
				elsif template_id = -2146951819 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-780209753);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_chest_plate.iff
				elsif template_id = 1042717208 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1253715791);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_gloves.iff
				elsif template_id = 1128757147 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-180061178);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_helmet.iff
				elsif template_id = -175663469 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1132764430);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_leggings.iff
				elsif template_id = 2147059193 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,781150507);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_bicep_l.iff
				elsif template_id = 748768172 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						RemoveScript('systems.armor_rehue.composite_rehue');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-293142117);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_bicep_r.iff
				elsif template_id = 1551109695 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						RemoveScript('systems.armor_rehue.composite_rehue');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1638775800);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_boots.iff
				elsif template_id = 1302427111 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						RemoveScript('systems.armor_rehue.composite_rehue');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,953524984);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_bracer_l.iff
				elsif template_id = -1537444864 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						RemoveScript('systems.armor_rehue.composite_rehue');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-702617353);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_bracer_r.iff
				elsif template_id = -729206381 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						RemoveScript('systems.armor_rehue.composite_rehue');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1496601244);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_chest_plate.iff
				elsif template_id = 1231025524 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						RemoveScript('systems.armor_rehue.composite_rehue');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,467509735);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_gloves.iff
				elsif template_id = 590525838 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						RemoveScript('systems.armor_rehue.composite_rehue');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1600857086);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_helmet.iff
				elsif template_id = -1779280762 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						RemoveScript('systems.armor_rehue.composite_rehue');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,374619402);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_leggings.iff
				elsif template_id = 729327903 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						RemoveScript('systems.armor_rehue.composite_rehue');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1496987112);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_bicep_l.iff
				elsif template_id = -255400918 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1735043340);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_bicep_r.iff
				elsif template_id = -2146197063 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,398453919);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_boots.iff
				elsif template_id = -1390425255 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,985400715);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_bracer_l.iff
				elsif template_id = -412867955 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1886613270);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_bracer_r.iff
				elsif template_id = -1749981410 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,10889861);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_chest_plate.iff
				elsif template_id = 1756768751 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,431228564);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_gloves.iff
				elsif template_id = 1478831607 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1062103956);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_helmet.iff
				elsif template_id = -286929665 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1987147108);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_leggings.iff
				elsif template_id = 1749090194 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-11030007);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_bicep_l.iff
				elsif template_id = 183123522 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,647086555);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_bicep_r.iff
				elsif template_id = 2050980817 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1447330888);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_boots.iff
				elsif template_id = 2062241693 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('0');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-772711695);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_bracer_l.iff
				elsif template_id = 1837916498 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1151543850);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_bracer_r.iff
				elsif template_id = 492381377 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-880177083);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_chest_plate.iff
				elsif template_id = 108052567 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-218539538);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_gloves.iff
				elsif template_id = -1552027109 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('0');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1661731700);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_helmet.iff
				elsif template_id = 364712723 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-707992964);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_leggings.iff
				elsif template_id = -492258227 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,880281801);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_bicep_l.iff
				elsif template_id = 1926737359 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-977049079);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_bicep_r.iff
				elsif template_id = 33744988 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1256837222);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_boots.iff
				elsif template_id = 1205005532 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-186991275);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_bracer_l.iff
				elsif template_id = -1767695347 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,555550582);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_bracer_r.iff
				elsif template_id = -428451426 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1372078821);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_chest_plate.iff
				elsif template_id = -1520174324 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-674054582);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_gloves.iff
				elsif template_id = 2132891736 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-955417857);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_helmet.iff
				elsif template_id = -907953840 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1909300215);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_leggings.iff
				elsif template_id = 428327186 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1371940247);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_black_mtn/armor_kashyyykian_black_mtn_bracer_l.iff
				elsif template_id = -1877836182 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1701039966);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_black_mtn/armor_kashyyykian_black_mtn_bracer_r.iff
				elsif template_id = -523813895 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-364284623);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_black_mtn/armor_kashyyykian_black_mtn_chest_plate.iff
				elsif template_id = 66083222 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-937806356);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_black_mtn/armor_kashyyykian_black_mtn_leggings.iff
				elsif template_id = 523674485 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,364441021);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_ceremonial/armor_kashyyykian_ceremonial_bracer_l.iff
				elsif template_id = 1974187109 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-678680574);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_ceremonial/armor_kashyyykian_ceremonial_bracer_r.iff
				elsif template_id = 92205558 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1487311471);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_ceremonial/armor_kashyyykian_ceremonial_chest_plate.iff
				elsif template_id = 856491201 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,708645782);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_ceremonial/armor_kashyyykian_ceremonial_leggings.iff
				elsif template_id = -91264646 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1487467805);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_hunting/armor_kashyyykian_hunting_bracer_l.iff
				elsif template_id = 811367898 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,2003645176);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_hunting/armor_kashyyykian_hunting_bracer_r.iff
				elsif template_id = 1082734665 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,129529707);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_hunting/armor_kashyyykian_hunting_chest_plate.iff
				elsif template_id = 1193819287 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-212755131);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_hunting/armor_kashyyykian_hunting_leggings.iff
				elsif template_id = -1082613563 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer15',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-129372185);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/mandalorian/armor_mandalorian_belt.iff
				elsif template_id = -1640376935 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						AddObjVar('armor.mandDeconstruct',0,'1');
						AddScript('item.conversion.armor_mand');
					end if;
				--object/tangible/wearables/armor/mandalorian/armor_mandalorian_bicep_l.iff
				elsif template_id = 1584198075 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.mandDeconstruct',0,'1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_mand');
					end if;
				--object/tangible/wearables/armor/mandalorian/armor_mandalorian_bicep_r.iff
				elsif template_id = 783922216 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.mandDeconstruct',0,'1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_mand');
					end if;
				--object/tangible/wearables/armor/mandalorian/armor_mandalorian_bracer_l.iff
				elsif template_id = -2133834359 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.mandDeconstruct',0,'1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_mand');
					end if;
				--object/tangible/wearables/armor/mandalorian/armor_mandalorian_bracer_r.iff
				elsif template_id = -268107750 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.mandDeconstruct',0,'1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_mand');
					end if;
				--object/tangible/wearables/armor/mandalorian/armor_mandalorian_chest_plate.iff
				elsif template_id = 1631264204 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.mandDeconstruct',0,'1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_mand');
					end if;
				--object/tangible/wearables/armor/mandalorian/armor_mandalorian_gloves.iff
				elsif template_id = -1047768279 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						AddObjVar('armor.mandDeconstruct',0,'1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_mand');
					end if;
				--object/tangible/wearables/armor/mandalorian/armor_mandalorian_helmet.iff
				elsif template_id = 2001515041 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.mandDeconstruct',0,'1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_mand');
					end if;
				--object/tangible/wearables/armor/mandalorian/armor_mandalorian_leggings.iff
				elsif template_id = 267688086 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.mandDeconstruct',0,'1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_mand');
					end if;
				--object/tangible/wearables/armor/mandalorian/armor_mandalorian_shoes.iff
				elsif template_id = 1065077663 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						AddObjVar('armor.mandDeconstruct',0,'1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_mand');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s01_belt.iff
				elsif template_id = 57117299 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s01_bicep_l.iff
				elsif template_id = 866000377 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bicep_l_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s01_bicep_r.iff
				elsif template_id = 1129011306 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bicep_r_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s01_bracer_l.iff
				elsif template_id = -279689267 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bracer_l_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s01_bracer_r.iff
				elsif template_id = -1618935202 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bracer_r_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s01_chest_plate.iff
				elsif template_id = -1377743586 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_chest_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s01_gloves.iff
				elsif template_id = 1440200840 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_gloves_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s01_helmet.iff
				elsif template_id = -485009024 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_helmet_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s01_leggings.iff
				elsif template_id = 1617992402 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_leggings_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s02_bicep_l.iff
				elsif template_id = 246676033 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bicep_l_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s02_bicep_r.iff
				elsif template_id = 2120662994 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bicep_r_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s02_boots.iff
				elsif template_id = -1169656734 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('0');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_boots_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s02_bracer_l.iff
				elsif template_id = 659297422 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bracer_l_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s02_bracer_r.iff
				elsif template_id = 1469665565 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bracer_r_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s02_chest_plate.iff
				elsif template_id = -1785454855 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_chest_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s02_chest_plate_quest.iff
				elsif template_id = 522917746 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_chest_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s02_leggings.iff
				elsif template_id = -1469527663 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_leggings_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s03_bandolier_double.iff
				elsif template_id = 824817548 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s03_bicep_l.iff
				elsif template_id = -434656188 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bicep_l_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s03_bicep_r.iff
				elsif template_id = -1765611049 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bicep_r_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s03_boots.iff
				elsif template_id = 1139078028 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_boots_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s03_bracer_l.iff
				elsif template_id = 1622923855 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bracer_l_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s03_bracer_r.iff
				elsif template_id = 275685340 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bracer_r_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s03_chest_plate.iff
				elsif template_id = -2109263964 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_chest_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s03_gloves.iff
				elsif template_id = 1513782916 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('2');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_gloves_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s03_helmet.iff
				elsif template_id = -319119476 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_helmet_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/armor_marauder_s03_leggings.iff
				elsif template_id = -274790576 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','2');
						AddObjVar('armor.layer15',2,'0.454545');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_leggings_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/energy_armor_marauder_belt.iff
				elsif template_id = -1751431536 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
					end if;
				--object/tangible/wearables/armor/marauder/energy_armor_marauder_bicep_l.iff
				elsif template_id = 1928484082 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bicep_l_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/energy_armor_marauder_bicep_r.iff
				elsif template_id = 36114785 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bicep_r_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/energy_armor_marauder_boots.iff
				elsif template_id = 2125025189 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_boots_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/energy_armor_marauder_bracer_l.iff
				elsif template_id = -1285938931 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bracer_l_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/energy_armor_marauder_bracer_r.iff
				elsif template_id = -1014012770 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_bracer_r_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/energy_armor_marauder_chest_plate.iff
				elsif template_id = -698743461 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_chest_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/energy_armor_marauder_gloves.iff
				elsif template_id = -226902841 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_gloves_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/energy_armor_marauder_helmet.iff
				elsif template_id = 1152998863 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_helmet_conversion');
					end if;
				--object/tangible/wearables/armor/marauder/energy_armor_marauder_leggings.iff
				elsif template_id = 1014955026 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddScript('item.armor.new_armor');
						AddScript('item.conversion.armor_leggings_conversion');
					end if;
				--object/tangible/wearables/armor/marine/armor_marine_bicep_l.iff
				elsif template_id = -1762554943 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/marine/armor_marine_bicep_r.iff
				elsif template_id = -433795502 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/marine/armor_marine_boots.iff
				elsif template_id = 1463740430 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('0');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/marine/armor_marine_chest_plate.iff
				elsif template_id = -2101932344 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/marine/armor_marine_chest_plate_rebel.iff
				elsif template_id = -1721490790 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/marine/armor_marine_helmet.iff
				elsif template_id = 1777643552 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/marine/armor_marine_leggings.iff
				elsif template_id = -385048649 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddObjVar('armor.layer14',2,'0.454545');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/nightsister/armor_nightsister_bicep_r_s01.iff
				elsif template_id = -761139333 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_belt.iff
				elsif template_id = -932988328 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1971625869);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_bicep_l.iff
				elsif template_id = 525967921 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-439640191);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_bicep_r.iff
				elsif template_id = 1871505314 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1793140206);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_boots.iff
				elsif template_id = -963800801 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-481038074);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_bracer_l.iff
				elsif template_id = -2050710967 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1312081514);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_bracer_r.iff
				elsif template_id = -183375910 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1054966779);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_chest_plate.iff
				elsif template_id = 689622929 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1067716071);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_gloves.iff
				elsif template_id = 1301374465 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,471310986);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_helmet.iff
				elsif template_id = -78543095 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1428733054);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_leggings.iff
				elsif template_id = 182484822 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1055909001);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_belt.iff
				elsif template_id = 1051253941 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1971625869);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_bicep_l.iff
				elsif template_id = -954052477 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-439640191);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_bicep_r.iff
				elsif template_id = -1208543984 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1793140206);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_boots.iff
				elsif template_id = -1630155612 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-481038074);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_bracer_l.iff
				elsif template_id = -911886824 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1312081514);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_bracer_r.iff
				elsif template_id = -1183779957 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1054966779);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_chest_plate.iff
				elsif template_id = -343994769 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1067716071);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_gloves.iff
				elsif template_id = 1263770558 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,471310986);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_helmet.iff
				elsif template_id = -40666442 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1428733054);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_leggings.iff
				elsif template_id = 1182886663 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1055909001);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_bicep_l.iff
				elsif template_id = 774778471 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1388916234);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_bicep_r.iff
				elsif template_id = 1593535476 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-572287899);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_boots.iff
				elsif template_id = -306804975 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1862370746);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_bracer_l.iff
				elsif template_id = 582714478 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1577833305);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_bracer_r.iff
				elsif template_id = 1382990333 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,786339530);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_chest_plate.iff
				elsif template_id = -1832625049 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1275692711);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_gloves.iff
				elsif template_id = -974187136 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1550571956);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_helmet.iff
				elsif template_id = 1932522632 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-357877572);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_leggings.iff
				elsif template_id = -1382065807 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-785445306);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/singing_mountain_clan/armor_smc_s01_bicep_l.iff
				elsif template_id = 1841408054 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/singing_mountain_clan/armor_smc_s01_bicep_r.iff
				elsif template_id = 487910821 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/singing_mountain_clan/armor_smc_s01_bracer_l.iff
				elsif template_id = -185039936 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/singing_mountain_clan/armor_smc_s01_bracer_r.iff
				elsif template_id = -2077374893 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_bicep_l.iff
				elsif template_id = 303274938 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_bicep_r.iff
				elsif template_id = 1657200169 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_boots.iff
				elsif template_id = 1224577523 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_bracer_l.iff
				elsif template_id = -30363750 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_bracer_r.iff
				elsif template_id = -1897534967 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_chest_plate.iff
				elsif template_id = 567515627 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_chest_plate_quest.iff
				elsif template_id = 1362436589 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_gloves.iff
				elsif template_id = 1746127589 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_helmet.iff
				elsif template_id = -556586003 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_helmet_quest.iff
				elsif template_id = 1630705486 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_leggings.iff
				elsif template_id = 1897165445 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_leggings_quest.iff
				elsif template_id = -2065356471 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/armor_stormtrooper_utility_belt.iff
				elsif template_id = 487215056 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/blast_armor_stormtrooper_bicep_l.iff
				elsif template_id = -483121900 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/blast_armor_stormtrooper_bicep_r.iff
				elsif template_id = -1813946233 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/blast_armor_stormtrooper_boots.iff
				elsif template_id = 830219516 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/blast_armor_stormtrooper_bracer_l.iff
				elsif template_id = 1425531172 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/blast_armor_stormtrooper_bracer_r.iff
				elsif template_id = 606250167 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/blast_armor_stormtrooper_chest_plate.iff
				elsif template_id = -1175884434 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/blast_armor_stormtrooper_helmet.iff
				elsif template_id = -1659123451 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/blast_armor_stormtrooper_leggings.iff
				elsif template_id = -606913477 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,10,0.2,0.5),rangeType(10,20,0.5,0.83),rangeType(20,30,0.83,0.85),rangeType(30,40,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','1');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('item.armor.new_armor');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/stormtrooper/blast_armor_stormtrooper_utility_belt.iff
				elsif template_id = 619388150 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						AddObjVar('biolink.faction_points',0,'5');
						AddScript('npc.faction_recruiter.faction_item');
						AddScript('npc.faction_recruiter.biolink_item');
					end if;
				--object/tangible/wearables/armor/tantel/armor_tantel_skreej_boots.iff
				elsif template_id = -2061397693 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('0');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2118749659);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/tantel/armor_tantel_skreej_boots_quest.iff
				elsif template_id = -914141938 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('0');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2118749659);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/tantel/armor_tantel_skreej_chest_plate.iff
				elsif template_id = -653428823 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1564577478);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/tantel/armor_tantel_skreej_chest_plate_quest.iff
				elsif template_id = -1993339022 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1564577478);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/tantel/armor_tantel_skreej_helmet.iff
				elsif template_id = 1855229351 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-361521797);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/tantel/armor_tantel_skreej_helmet_quest.iff
				elsif template_id = 275564031 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-361521797);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_bandolier.iff
				elsif template_id = -247678631 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1855746937);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_boots.iff
				elsif template_id = 1261688149 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('0');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2009182529);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_bracer_l.iff
				elsif template_id = -779152232 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.8),rangeType(50,80,0.8,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1330065271);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_bracer_r.iff
				elsif template_id = -1587916533 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.8),rangeType(50,80,0.8,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1066560230);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_gloves.iff
				elsif template_id = -1424733636 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('0');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-163178884);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_helmet.iff
				elsif template_id = 500476724 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.8),rangeType(50,80,0.8,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1082586996);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_helmet_quest.iff
				elsif template_id = -320086751 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.8),rangeType(50,80,0.8,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1082586996);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_jacket.iff
				elsif template_id = 678412834 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.8),rangeType(50,80,0.8,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1422504544);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_jacket_quest.iff
				elsif template_id = 1436153762 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.8),rangeType(50,80,0.8,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1422504544);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_pants.iff
				elsif template_id = -177674793 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.8),rangeType(50,80,0.8,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1067191702);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_pants_quest.iff
				elsif template_id = -1886457751 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.8),rangeType(50,80,0.8,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,1067191702);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_shirt.iff
				elsif template_id = 569575162 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-486630128);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_belt.iff
				elsif template_id = -392461625 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2077797017);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_boots.iff
				elsif template_id = -1500071681 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('0');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2081929972);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_boots_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_chest_plate.iff
				elsif template_id = -420003189 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1595914733);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_chest_plate_quest.iff
				elsif template_id = 309375847 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1595914733);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_gloves.iff
				elsif template_id = -98746218 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						DeleteObjVar('armor.effectiveness');
						SetArmorLevel('2');
						SetArmorCategory('0');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,184970781);
							AddScript('systems.crafting.armor.component.crafting_armor_nostats');
						else
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_gloves_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_helmet.iff
				elsif template_id = 1289593246 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1111060715);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_helmet_quest.iff
				elsif template_id = 103036882 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1111060715);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_pants.iff
				elsif template_id = 416123005 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1657354457);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_pants_quest.iff
				elsif template_id = -469920034 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						SetGenProtection(rangeTableType(rangeType(0,50,0.2,0.5),rangeType(50,80,0.5,0.83),rangeType(80,90,0.83,0.85),rangeType(90,100,0.85,0.86)));
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.8','2','0');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer14',2,'0.454545');
							ModifyObjVar('draftSchematic','draftSchematic',0,-1657354457);
							AddScript('systems.crafting.armor.component.crafting_new_armor_final');
						else
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.armor.new_armor');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/component/armor/shield_generator_personal.iff
				elsif template_id = -817576610 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.5','0','3');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer13',2,'0.4444');
							AddObjVar('crafting_components.recharge_rate',2,'60');
							AddScript('systems.crafting.armor.crafting_psg');
						else
							AddObjVar('armor.layer13',2,'0.4444');
							AddObjVar('armor.recharge_rate',2,'60');
							AddScript('item.armor.new_armor');
							AddScript('item.armor.psg');
						end if;
					end if;
				--object/tangible/component/armor/shield_generator_personal_b.iff
				elsif template_id = -38886533 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.5','1','3');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer13',2,'0.54545');
							AddObjVar('crafting_components.recharge_rate',2,'60');
							AddScript('systems.crafting.armor.crafting_psg');
						else
							AddObjVar('armor.layer13',2,'0.54545');
							AddObjVar('armor.recharge_rate',2,'60');
							AddScript('item.armor.new_armor');
							AddScript('item.armor.psg');
						end if;
					end if;
				--object/tangible/component/armor/shield_generator_personal_c.iff
				elsif template_id = -1264335626 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.5','2','3');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer13',2,'0.759167');
							AddObjVar('crafting_components.recharge_rate',2,'60');
							AddScript('systems.crafting.armor.crafting_psg');
						else
							AddObjVar('armor.layer13',2,'0.759167');
							AddObjVar('armor.recharge_rate',2,'60');
							AddScript('item.armor.new_armor');
							AddScript('item.armor.psg');
						end if;
					end if;
				--object/tangible/component/armor/shield_generator_personal_imperial_test.iff
				elsif template_id = 2144735852 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteOldArmorObjvars;
						SetArmorCondLevelCat('0.5','2','3');
						if isFactory = TRUE then
							AddObjVar('crafting_components.layer13',2,'0.759167');
							AddObjVar('crafting_components.recharge_rate',2,'65');
							AddScript('systems.crafting.armor.crafting_psg');
						else
							AddObjVar('armor.layer13',2,'0.759167');
							AddObjVar('armor.recharge_rate',2,'65');
							AddScript('item.armor.new_armor');
							AddScript('item.armor.psg');
						end if;
					end if;
-- ****** BEGIN ARMOR COMPONENT CONVERSION RULES ******
				--object/tangible/component/armor/armor_layer_acid.iff
				elsif template_id = 1875688539 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar(prefix || 'armor_layer6',2,'0.700000');
						if isFactory = TRUE then
							script_list := 'systems.crafting.armor.component.crafting_new_armor_advanced_layer_1' || ':';
							isObjModified := TRUE;
						else
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_layer_blast.iff
				elsif template_id = 192857538 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar(prefix || 'armor_layer0',2,'0.700000');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-190320536);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_advanced_layer_1' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-983864428);		--object/tangible/component/armor/armor_layer_kinetic.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_layer_cold.iff
				elsif template_id = -442303678 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar(prefix || 'armor_layer5',2,'0.700000');
						if isFactory = TRUE then
							script_list := 'systems.crafting.armor.component.crafting_new_armor_advanced_layer_1' || ':';
							isObjModified := TRUE;
						else
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_layer_electrical.iff
				elsif template_id = -2072795932 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar(prefix || 'armor_layer2',2,'0.700000');
						if isFactory = TRUE then
							script_list := 'systems.crafting.armor.component.crafting_new_armor_advanced_layer_1' || ':';
							isObjModified := TRUE;
						else
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_layer_energy.iff
				elsif template_id = 983989372 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar(prefix || 'armor_layer1',2,'0.700000');
						if isFactory = TRUE then
							script_list := 'systems.crafting.armor.component.crafting_new_armor_advanced_layer_1' || ':';
							isObjModified := TRUE;
						else
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_layer_heat.iff
				elsif template_id = 490845618 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar(prefix || 'armor_layer4',2,'0.700000');
						if isFactory = TRUE then
							script_list := 'systems.crafting.armor.component.crafting_new_armor_advanced_layer_1' || ':';
							isObjModified := TRUE;
						else
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_layer_kinetic.iff
				elsif template_id = -983864428 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar(prefix || 'armor_layer0',2,'0.700000');
						if isFactory = TRUE then
							script_list := 'systems.crafting.armor.component.crafting_new_armor_advanced_layer_1' || ':';
							isObjModified := TRUE;
						else
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_layer_nightsister.iff
				elsif template_id = 2086609620 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'20.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.component.component_attrib_mod');
					end if;
				--object/tangible/component/armor/armor_layer_ris.iff
				elsif template_id = 110363758 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'25.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.component.component_attrib_mod');
					end if;
				--object/tangible/component/armor/armor_layer_stun.iff
				elsif template_id = -1424880637 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar(prefix || 'armor_layer0',2,'0.700000');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-190320536);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_advanced_layer_1' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-983864428);		--object/tangible/component/armor/armor_layer_kinetic.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment.iff
				elsif template_id = -448030425 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1861772866);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(1594162622);		--object/tangible/component/armor/armor_segment_recon.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_bone.iff
				elsif template_id = -786663538 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_bone_advanced.iff
				elsif template_id = -702291727 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_bone_donkuwah.iff
				elsif template_id = 50152844 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.6', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_bone_kimogila.iff
				elsif template_id = -132357748 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.8', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_bone_voritor_lizard.iff
				elsif template_id = 1590091204 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.6', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_chitin.iff
				elsif template_id = -212848001 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1138822246);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_chitin_advanced.iff
				elsif template_id = 1415954991 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1138822246);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_chitin_brackaset.iff
				elsif template_id = 1401689522 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1138822246);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_chitin_kliknick.iff
				elsif template_id = 1656768558 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.6', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1138822246);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_chitin_kliknick_adv.iff
				elsif template_id = 944293988 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.8', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1138822246);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_chitin_sharnaff.iff
				elsif template_id = -1764803069 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.5', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1138822246);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_composite.iff
				elsif template_id = 2026707684 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1138822246);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_composite_advanced.iff
				elsif template_id = 1741906969 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.7', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1138822246);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_composite_krayt.iff
				elsif template_id = 1862066444 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.8', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1138822246);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_enhancement_brackaset.iff
				elsif template_id = 981227534 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'5.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.component.component_attrib_mod');
					end if;
				--object/tangible/component/armor/armor_segment_enhancement_fambaa.iff
				elsif template_id = -613110139 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'5.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.component.component_attrib_mod');
					end if;
				--object/tangible/component/armor/armor_segment_enhancement_janta.iff
				elsif template_id = 188520444 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'10.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.component.component_attrib_mod');
					end if;
				--object/tangible/component/armor/armor_segment_enhancement_kimogila.iff
				elsif template_id = -589817340 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'10.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.component.component_attrib_mod');
					end if;
				--object/tangible/component/armor/armor_segment_enhancement_krayt.iff
				elsif template_id = 1740191859 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'15.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.component.component_attrib_mod');
					end if;
				--object/tangible/component/armor/armor_segment_enhancement_nightsister.iff
				elsif template_id = -1464343893 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'15.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.component.component_attrib_mod');
					end if;
				--object/tangible/component/armor/armor_segment_enhancement_rancor.iff
				elsif template_id = -2022305605 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'15.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.component.component_attrib_mod');
					end if;
				--object/tangible/component/armor/armor_segment_enhancement_sharnaff.iff
				elsif template_id = 810379093 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'5.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.component.component_attrib_mod');
					end if;
				--object/tangible/component/armor/armor_segment_enhancement_voritor_lizard.iff
				elsif template_id = -965865145 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'5.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.component.component_attrib_mod');
					end if;
				--object/tangible/component/armor/armor_segment_kashyyykian_black_mtn.iff
				elsif template_id = 775748867 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_kashyyykian_black_mtn_advanced.iff
				elsif template_id = -1876464536 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_kashyyykian_ceremonial.iff
				elsif template_id = -483088632 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1861772866);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(1594162622);		--object/tangible/component/armor/armor_segment_recon.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_kashyyykian_ceremonial_advanced.iff
				elsif template_id = 901906289 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1861772866);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(1594162622);		--object/tangible/component/armor/armor_segment_recon.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_kashyyykian_hunting.iff
				elsif template_id = -2134198239 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1138822246);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_kashyyykian_hunting_advanced.iff
				elsif template_id = -1655034745 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.7', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-1138822246);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_padded.iff
				elsif template_id = 1080722273 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_padded_acklay.iff
				elsif template_id = 1220774872 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.8', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_padded_advanced.iff
				elsif template_id = 1141198733 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.7', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_padded_fambaa.iff
				elsif template_id = -309965447 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.6', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_padded_rancor.iff
				elsif template_id = -1316687033 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.8', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_ris.iff
				elsif template_id = -1322827874 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.8', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_ubese.iff
				elsif template_id = 810339444 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1861772866);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(1594162622);		--object/tangible/component/armor/armor_segment_recon.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_ubese_advanced.iff
				elsif template_id = 1102511316 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.7', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1861772866);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(1594162622);		--object/tangible/component/armor/armor_segment_recon.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_zam.iff
				elsif template_id = 1804298795 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1861772866);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(1594162622);		--object/tangible/component/armor/armor_segment_recon.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_zam_advanced.iff
				elsif template_id = 1505456979 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.7', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,1861772866);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(1594162622);		--object/tangible/component/armor/armor_segment_recon.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/bone_fragment_woolamander_harrower.iff
				elsif template_id = -90182809 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/deflector_shield_emitter_assembly.iff
				elsif template_id = -1915271095 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/deflector_shield_generator_energy_ray.iff
				elsif template_id = -1188519487 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/deflector_shield_generator_particle.iff
				elsif template_id = -1173568585 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/deflector_shield_projector_plate.iff
				elsif template_id = 1906195493 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/feather_peko_albatross.iff
				elsif template_id = -1958737672 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'20.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
					end if;
				--object/tangible/component/armor/hide_gurk_king.iff
				elsif template_id = 665098706 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/scale_giant_dune_kimogila.iff
				elsif template_id = -1170494820 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/heavy_armor_layer.iff
				elsif template_id = 831150616 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/heavy_armor_layer_acid.iff
				elsif template_id = 1762601318 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/heavy_armor_layer_blast.iff
				elsif template_id = -764562064 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/heavy_armor_layer_cold.iff
				elsif template_id = -480153985 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/heavy_armor_layer_electrical.iff
				elsif template_id = -784284994 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/heavy_armor_layer_energy.iff
				elsif template_id = 444888474 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/heavy_armor_layer_environmental.iff
				elsif template_id = 1618598853 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/heavy_armor_layer_heat.iff
				elsif template_id = 461512847 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/heavy_armor_layer_kinetic.iff
				elsif template_id = 2138127220 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/heavy_armor_layer_restraint.iff
				elsif template_id = 1938675644 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/heavy_armor_layer_stun.iff
				elsif template_id = -1378772674 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/shield_generator_facility.iff
				elsif template_id = -1109531580 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/deflector_shield_ion_feed_unit.iff
				elsif template_id = 155551742 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/deflector_shield_overload_discharge_unit.iff
				elsif template_id = -623854688 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/armor_module_heavy.iff
				elsif template_id = -617341983 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/armor_module_light.iff
				elsif template_id = 443295551 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/armor_layer_environmental.iff
				elsif template_id = -1554492115 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar(prefix || 'armor_layer0',2,'0.700000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/armor_layer_restraint.iff
				elsif template_id = 201334650 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar(prefix || 'armor_layer12',2,'0.700000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/armor_segment_advanced.iff
				elsif template_id = 78081465 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						if isFactory = TRUE then
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_bone_acklay.iff
				elsif template_id = 1261011254 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.8', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_enhancement_dragonet.iff
				elsif template_id = 227937004 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						AddObjVar('attribute.bonus.0',2,'5.000000');
						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
						AddScript('item.armor.new_armor');
					end if;
				--object/tangible/component/armor/armor_segment_padded_dune_lizard.iff
				elsif template_id = -1185633502 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.4', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_ris_acklay.iff
				elsif template_id = -1920361299 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.8', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/clothing/padding_segment.iff
				elsif template_id = 349677024 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						DeleteObjVar(prefix || 'armor*');
						SetGpAndCond(prefix || 'general_protection', '0.7', prefix || 'condition', '0.2');
						if isFactory = TRUE then
							ModifyObjVar('draftSchematic','draftSchematic',0,-2043680571);
							script_list := 'systems.crafting.armor.component.crafting_new_armor_segment' || ':';
							isObjModified := TRUE;
						else
							SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
							RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
							AddScript('item.armor.new_armor');
						end if;
					end if;
				--object/tangible/component/weapon/geonosian_power_cube_yellow.iff
				elsif template_id = 1210744947 then
					if isSchematic = TRUE then
						InvalidateSchematic;
					else
						AddObjVar(prefix || 'general_protection',2,'0.175');
						AddScript('item.armor.new_armor');
					end if;
-- ****** BEGIN WEAPON CONVERSION RULES ******
				--component/blaster_pistol_barrel
				elsif template_id = 1369190307 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,10,8,14),rangeType(10,20,14,16)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,10,21,26),rangeType(10,20,26,30)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-10,-9,-10,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,5,0,5),rangeType(5,6,5,6)));
					SetAttackCost(-3);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/blaster_pistol_barrel_advanced
				elsif template_id = -1438464920 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,25,13,26),rangeType(25,50,26,30)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,25,42,53),rangeType(25,50,53,60)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-12,0,-20,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,15,0,15),rangeType(15,16,15,18)));
					SetAttackCost(-6);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/blaster_power_handler
				elsif template_id = 702649693 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,10,8,14),rangeType(10,20,14,16)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,20,21,26),rangeType(20,40,26,30)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-5,0,-10,0)));
					SetAttackCost(-3);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/blaster_power_handler_advanced
				elsif template_id = 34919568 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,25,13,26),rangeType(25,50,26,30)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,50,42,53),rangeType(50,100,53,60)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-5,0,-20,0)));
					SetAttackCost(-6);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/blaster_rifle_barrel
				elsif template_id = -1565904814 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,10,8,14),rangeType(10,20,14,16)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,10,21,26),rangeType(10,20,26,30)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-5,0,-10,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,5,0,5),rangeType(5,10,5,6)));
					SetAttackCost(-3);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/blaster_rifle_barrel_advanced
				elsif template_id = -778602894 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,25,13,26),rangeType(25,50,26,30)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,25,42,53),rangeType(25,50,53,60)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-7,0,-20,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,11,0,10)));
					SetAttackCost(-6);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/chemical_dispersion_mechanism
				elsif template_id = -1382616459 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,10,8,14),rangeType(10,20,14,16)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,10,21,26),rangeType(10,20,26,30)));
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/chemical_dispersion_mechanism_advanced
				elsif template_id = -900058289 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,20,13,26),rangeType(20,40,26,30)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,20,42,53),rangeType(20,40,53,60)));
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/geonosian_reinforcement_core
				elsif template_id = -1566891826 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,50,13,26),rangeType(50,100,26,30)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,50,42,53),rangeType(50,100,53,60)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-7,0,-20,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,15,0,0),rangeType(15,40,0,0)));
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/projectile_feed_mechanism
				elsif template_id = -1474613882 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,10,8,14),rangeType(10,20,14,16)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,20,21,26),rangeType(20,40,26,30)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-5,0,-10,0)));
					SetAttackCost(-3);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/projectile_feed_mechanism_advanced
				elsif template_id = 590597969 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,25,13,26),rangeType(25,50,26,30)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,50,42,53),rangeType(50,100,53,60)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-5,0,-20,0)));
					SetAttackCost(-6);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/projectile_pistol_barrel
				elsif template_id = 6493761 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,10,8,14),rangeType(10,20,14,16)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,20,21,26),rangeType(20,40,26,30)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-5,-4,-10,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,5,0,5),rangeType(5,20,5,6)));
					SetAttackCost(-3);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/projectile_pistol_barrel_advanced
				elsif template_id = 2132239519 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,25,13,26),rangeType(25,50,26,30)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,25,42,53),rangeType(25,50,53,60)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-5,0,-20,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,11,0,10)));
					SetAttackCost(-6);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/projectile_rifle_barrel
				elsif template_id = -721077210 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,10,8,14),rangeType(10,20,14,16)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,10,21,26),rangeType(10,20,26,30)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-7,0,-10,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,5,0,5),rangeType(5,15,5,6)));
					SetAttackCost(-3);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/projectile_rifle_barrel_advanced
				elsif template_id = 320220025 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,25,13,26),rangeType(25,50,26,30)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,25,42,53),rangeType(25,50,53,60)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-7,0,-20,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,10,0,10),rangeType(10,35,10,12)));
					SetAttackCost(-6);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/reinforcement_core
				elsif template_id = -225492986 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,20,8,14),rangeType(20,40,14,16)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,20,21,26),rangeType(20,40,26,30)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-5,0,-10,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,11,0,10)));
					SetAttackCost(-3);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/reinforcement_core_advanced
				elsif template_id = 1995174592 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,50,13,26),rangeType(50,100,26,30)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,50,42,53),rangeType(50,100,53,60)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-7,0,-20,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,10,0,15),rangeType(10,20,15,18)));
					SetAttackCost(-6);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/scope_weapon
				elsif template_id = 403078591 then
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(2,2,0,10)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,10,0,10),rangeType(10,15,10,12)));
					SetAccuracy(8);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/scope_weapon_advanced
				elsif template_id = -269033971 then
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(10,10,0,20)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,20,0,20),rangeType(20,40,20,24)));
					SetAccuracy(16);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/stock
				elsif template_id = -651693115 then
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(10,20,0,15)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,5,0,5),rangeType(5,20,5,6)));
					SetAttackCost(-12);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/stock_advanced
				elsif template_id = -1571384798 then
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(10,20,0,25)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,15,0,15),rangeType(15,30,15,18)));
					SetAttackCost(-20);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/sword_core
				elsif template_id = 716551990 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,20,8,14),rangeType(20,40,14,16)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,20,21,26),rangeType(20,40,26,30)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-5,0,-10,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,10,0,10),rangeType(10,11,10,12)));
					SetAttackCost(-3);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/sword_core_advanced
				elsif template_id = 1944034997 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,50,13,26),rangeType(50,100,26,30)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,50,42,53),rangeType(50,100,53,60)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-7,0,-20,0)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,15,0,15),rangeType(15,30,15,18)));
					SetAttackCost(-6);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--component/vibro_unit_nightsister
				elsif template_id = 1855721592 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(0,75,15,29),rangeType(75,150,29,34)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(0,90,46,58),rangeType(90,180,58,66)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-4,-1,-20,5)));
					ModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,rangeTableType(rangeType(0,30,10,30),rangeType(30,60,30,36)));
					SetAttackCost(-7);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
					end if;
				--quest_rifle_projectile_tusken
				elsif template_id = 981088834 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(57,66,217,413),rangeType(66,89,413,472)));
					ConvertMaxDamage(rangeTableType(rangeType(195,243,648,824),rangeType(243,359,824,942)));
					ConvertAttackSpeed(rangeTableType(rangeType(76.6,127.2,225,250),rangeType(127.2,146.8,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(29,29,1,17),rangeType(29,30,17,20.4)));
					SetWeaponBaseStats(104,0,0,65,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--battleaxe
				elsif template_id = -637703116 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(76,160,30,56),rangeType(160,308,56,64)));
					ConvertMaxDamage(rangeTableType(rangeType(124,232,87,110),rangeType(232,425,110,126)));
					ConvertAttackSpeed(rangeTableType(rangeType(19.3,30,225,250),rangeType(30,49.5,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(19,38,5,25),rangeType(38,45,25,30)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--battleaxe_quest
				elsif template_id = -1779174489 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(82,95,30,58),rangeType(95,146,58,66)));
					ConvertMaxDamage(rangeTableType(rangeType(137,160,91,116),rangeType(160,251,116,132)));
					ConvertAttackSpeed(rangeTableType(rangeType(24.5,40,225,250),rangeType(40,47.4,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(19,19,5,25),rangeType(19,23,25,30)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--executioners_hack
				elsif template_id = 1840338268 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(132,190,178,339),rangeType(190,214,339,388)));
					ConvertMaxDamage(rangeTableType(rangeType(263,383,532,677),rangeType(383,474,677,774)));
					ConvertAttackSpeed(rangeTableType(rangeType(19.5,24.1,225,250),rangeType(24.1,36.6,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(31,38,2,24),rangeType(38,46,24,28.8)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--cleaver
				elsif template_id = 237038605 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(80,130,57,108),rangeType(130,158,108,124)));
					ConvertMaxDamage(rangeTableType(rangeType(196,290,169,215),rangeType(290,539,215,246)));
					ConvertAttackSpeed(rangeTableType(rangeType(15.2,24.1,225,250),rangeType(24.1,40.1,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(39,52,9,35),rangeType(52,78,35,42)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--katana
				elsif template_id = 653612816 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(39,83,85,163),rangeType(83,140,163,186)));
					ConvertMaxDamage(rangeTableType(rangeType(211,331,256,326),rangeType(331,656,326,372)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,17.4,180,200),rangeType(17.4,32.8,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(32,47,5,27),rangeType(47,81,27,32.4)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--katana_quest
				elsif template_id = 1608037452 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(16,19,87,166),rangeType(19,26,166,190)));
					ConvertMaxDamage(rangeTableType(rangeType(174,207,261,331),rangeType(207,329,331,378)));
					ConvertAttackSpeed(rangeTableType(rangeType(19.7,31,180,200),rangeType(31,36.9,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(24,24,7,30),rangeType(24,28,30,36)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--maul
				elsif template_id = -613167286 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(118,199,283,541),rangeType(199,445,541,618)));
					ConvertMaxDamage(rangeTableType(rangeType(430,659,850,1082),rangeType(659,1105,1082,1236)));
					ConvertAttackSpeed(rangeTableType(rangeType(12.4,37.2,225,250),rangeType(37.2,58.6,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(26,37,1,20),rangeType(37,62,20,24)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--2h_sword_scythe
				elsif template_id = -245341807 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(171,252,224,428),rangeType(252,301,428,490)));
					ConvertMaxDamage(rangeTableType(rangeType(300,448,673,856),rangeType(448,579,856,978)));
					ConvertAttackSpeed(rangeTableType(rangeType(9.7,17.3,225,250),rangeType(17.3,27.7,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(22,29,10,25),rangeType(29,47,25,30)));
					SetWeaponBaseStats(104,0,0,5,1);
					SetElementalType(128);
					SetElementalValue(81);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--quest_battleaxe
				elsif template_id = -1019635633 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(50,55,32,61),rangeType(55,67,61,70)));
					ConvertMaxDamage(rangeTableType(rangeType(84,92,96,121),rangeType(92,113,121,138)));
					ConvertAttackSpeed(rangeTableType(rangeType(38.9,54.2,225,250),rangeType(54.2,60,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(11,11,5,25),rangeType(11,13,25,30)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--quest_maul
				elsif template_id = -449112830 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(50,56,286,546),rangeType(56,66,546,624)));
					ConvertMaxDamage(rangeTableType(rangeType(225,253,858,1092),rangeType(253,299,1092,1248)));
					ConvertAttackSpeed(rangeTableType(rangeType(58,78.9,225,250),rangeType(78.9,86.9,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(10,10,5,25),rangeType(10,12,25,30)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--axe
				elsif template_id = -526113597 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(5,57,24,45),rangeType(57,610,45,52)));
					ConvertMaxDamage(rangeTableType(rangeType(81,143,70,89),rangeType(143,620,89,102)));
					ConvertAttackSpeed(rangeTableType(rangeType(13.6,54.4,225,250),rangeType(54.4,69.4,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(8,14,1,20),rangeType(14,22,20,24)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--axe_vibroaxe
				elsif template_id = 589649861 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(42,201,178,339),rangeType(201,590,339,388)));
					ConvertMaxDamage(rangeTableType(rangeType(148,331,532,677),rangeType(331,634,677,774)));
					ConvertAttackSpeed(rangeTableType(rangeType(12,28.1,225,250),rangeType(28.1,62,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(15,29,1,14),rangeType(29,66,14,16.8)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--baton_gaderiffi
				elsif template_id = -20005656 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(39,178,85,163),rangeType(178,610,163,186)));
					ConvertMaxDamage(rangeTableType(rangeType(90,297,256,326),rangeType(297,677,326,372)));
					ConvertAttackSpeed(rangeTableType(rangeType(15.9,23.1,180,200),rangeType(23.1,60.3,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(9,40,1,21),rangeType(40,74,21,25.2)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--baton_stun
				elsif template_id = 257663710 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(57,173,209,400),rangeType(173,783,400,457)));
					ConvertMaxDamage(rangeTableType(rangeType(96,254,629,800),rangeType(254,626,800,914)));
					ConvertAttackSpeed(rangeTableType(rangeType(10.9,16.8,180,200),rangeType(16.8,30.1,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(6,21,1,3),rangeType(21,42,3,3.6)));
					SetWeaponBaseStats(84,0,0,5,2);
					SetElementalType(256);
					SetElementalValue(58);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--victor_baton_gaderiffi
				elsif template_id = -639753330 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(48,48,85,163),rangeType(48,48,163,186)));
					ConvertMaxDamage(rangeTableType(rangeType(148,148,256,326),rangeType(148,148,326,372)));
					ConvertAttackSpeed(rangeTableType(rangeType(38,38,180,200),rangeType(38,38,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(11,11,2,25),rangeType(11,11,25,30)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--knife_twilek_dagger
				elsif template_id = -730685311 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(6,28,19,36),rangeType(28,460,36,41)));
					ConvertMaxDamage(rangeTableType(rangeType(23,50,56,71),rangeType(50,470,71,82)));
					ConvertAttackSpeed(rangeTableType(rangeType(10.3,32.7,180,200),rangeType(32.7,41.3,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(5,9,1,12),rangeType(9,38,12,14.4)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--knife_donkuwah
				elsif template_id = -130243512 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(9,35,19,36),rangeType(35,54,36,41)));
					ConvertMaxDamage(rangeTableType(rangeType(15,54,56,71),rangeType(54,81,71,82)));
					ConvertAttackSpeed(rangeTableType(rangeType(12.3,21.3,180,200),rangeType(21.3,49.4,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(3,10,1,11),rangeType(10,16,11,13.2)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--knife_janta
				elsif template_id = 1756821101 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(15,26,19,36),rangeType(26,46,36,41)));
					ConvertMaxDamage(rangeTableType(rangeType(35,58,56,70),rangeType(58,103,70,80)));
					ConvertAttackSpeed(rangeTableType(rangeType(16.2,32.4,180,200),rangeType(32.4,42.7,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(3,6,1,13),rangeType(6,12,13,15.6)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--knife_stone
				elsif template_id = -1501822488 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(4,30,19,36),rangeType(30,640,36,41)));
					ConvertMaxDamage(rangeTableType(rangeType(12,37,56,71),rangeType(37,650,71,82)));
					ConvertAttackSpeed(rangeTableType(rangeType(14.6,40.9,180,200),rangeType(40.9,47.2,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(4,5,1,10),rangeType(5,14,10,12)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--knife_survival
				elsif template_id = -132487792 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(9,22,19,36),rangeType(22,640,36,41)));
					ConvertMaxDamage(rangeTableType(rangeType(23,37,56,71),rangeType(37,650,71,82)));
					ConvertAttackSpeed(rangeTableType(rangeType(8.6,39.3,180,200),rangeType(39.3,43.4,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(4,6,1,11),rangeType(6,20,11,13.2)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--knife_vibroblade
				elsif template_id = 1982554682 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(-17,104,46,87),rangeType(104,640,87,100)));
					ConvertMaxDamage(rangeTableType(rangeType(55,204,136,173),rangeType(204,650,173,198)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,17.9,180,200),rangeType(17.9,42.1,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(7,28,1,15),rangeType(28,34,15,18)));
					SetWeaponBaseStats(84,0,0,5,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--knife_vibroblade_quest
				elsif template_id = -913097330 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(14,16,46,89),rangeType(16,22,89,102)));
					ConvertMaxDamage(rangeTableType(rangeType(86,99,141,179),rangeType(99,131,179,204)));
					ConvertAttackSpeed(rangeTableType(rangeType(19.1,29.2,180,200),rangeType(29.2,33.4,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(8,8,1,20),rangeType(8,10,20,24)));
					SetWeaponBaseStats(84,0,0,5,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--lance_vibro_controller_fp
				elsif template_id = -402333683 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(0,49,96,182),rangeType(49,370,182,208)));
					ConvertMaxDamage(rangeTableType(rangeType(119,338,285,362),rangeType(338,1411,362,414)));
					ConvertAttackSpeed(rangeTableType(rangeType(19.9,27.6,203,225),rangeType(27.6,69.9,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(7,34,1,18),rangeType(34,65,18,21.6)));
					SetWeaponBaseStats(94,0,0,5,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--lance_vibro_nightsister
				elsif template_id = -1811261900 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(5,16,160,305),rangeType(16,52,305,348)));
					ConvertMaxDamage(rangeTableType(rangeType(96,328,479,609),rangeType(328,1010,609,696)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,33.9,203,225),rangeType(33.9,67.1,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(6,20,1,13),rangeType(20,90,13,15.6)));
					SetWeaponBaseStats(94,0,0,5,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--lance_nightsister
				elsif template_id = -1178706532 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(54,95,255,486),rangeType(95,113,486,556)));
					ConvertMaxDamage(rangeTableType(rangeType(298,478,763,971),rangeType(478,592,971,1110)));
					ConvertAttackSpeed(rangeTableType(rangeType(22,26.6,203,225),rangeType(26.6,41.7,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(29,47,10,35),rangeType(47,56,35,42)));
					SetWeaponBaseStats(94,0,0,5,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--staff_janta
				elsif template_id = -1137089652 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(20,36,25,48),rangeType(36,58,48,55)));
					ConvertMaxDamage(rangeTableType(rangeType(52,93,76,97),rangeType(93,148,97,110)));
					ConvertAttackSpeed(rangeTableType(rangeType(24.3,38.9,203,225),rangeType(38.9,53.6,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(3,4,1,10),rangeType(4,7,10,12)));
					SetWeaponBaseStats(94,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--staff_metal
				elsif template_id = -473813792 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(41,97,39,74),rangeType(97,440,74,84)));
					ConvertMaxDamage(rangeTableType(rangeType(63,142,116,147),rangeType(142,450,147,168)));
					ConvertAttackSpeed(rangeTableType(rangeType(20,44.5,203,225),rangeType(44.5,65.4,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(3,7,1,7),rangeType(7,11,7,8.4)));
					SetWeaponBaseStats(94,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--staff
				elsif template_id = -718648728 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(22,31,21,40),rangeType(31,67,40,46)));
					ConvertMaxDamage(rangeTableType(rangeType(51,72,63,79),rangeType(72,157,79,90)));
					ConvertAttackSpeed(rangeTableType(rangeType(18,51.5,203,225),rangeType(51.5,60.3,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(3,4,1,7),rangeType(4,11,7,8.4)));
					SetWeaponBaseStats(94,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--staff_reinforced
				elsif template_id = 238936831 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(21,49,26,51),rangeType(49,330,51,59)));
					ConvertMaxDamage(rangeTableType(rangeType(50,99,81,103),rangeType(99,340,103,118)));
					ConvertAttackSpeed(rangeTableType(rangeType(8.6,48.3,203,225),rangeType(48.3,61.1,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(2,7,1,7),rangeType(7,24,7,8.4)));
					SetWeaponBaseStats(94,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--lance_vibro
				elsif template_id = 1358850609 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(33,122,255,486),rangeType(122,383,486,556)));
					ConvertMaxDamage(rangeTableType(rangeType(142,401,763,971),rangeType(401,1407,971,1110)));
					ConvertAttackSpeed(rangeTableType(rangeType(9.5,27.8,203,225),rangeType(27.8,59.8,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(22,52,4,26),rangeType(52,133,26,31.2)));
					SetWeaponBaseStats(94,0,0,5,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--axe_vibro
				elsif template_id = -1088166271 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(58,165,178,339),rangeType(165,509,339,388)));
					ConvertMaxDamage(rangeTableType(rangeType(238,568,532,677),rangeType(568,2041,677,774)));
					ConvertAttackSpeed(rangeTableType(rangeType(11,26.3,225,250),rangeType(26.3,59.7,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(29,65,1,14),rangeType(65,156,14,16.8)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--knuckler_vibro
				elsif template_id = 102348899 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(21,59,118,224),rangeType(59,440,224,256)));
					ConvertMaxDamage(rangeTableType(rangeType(114,206,351,446),rangeType(206,450,446,510)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,13.9,248,275),rangeType(13.9,26.6,275,325)));
					ConvertWoundChance(rangeTableType(rangeType(25,48,4,26),rangeType(48,58,26,31.2)));
					SetWeaponBaseStats(122,0,0,5,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--sword
				elsif template_id = -673780945 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(-1,50,30,56),rangeType(50,640,56,64)));
					ConvertMaxDamage(rangeTableType(rangeType(46,122,87,110),rangeType(122,650,110,126)));
					ConvertAttackSpeed(rangeTableType(rangeType(13.1,34.7,225,250),rangeType(34.7,48.5,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(5,21,1,18),rangeType(21,59,18,21.6)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--sword_curved
				elsif template_id = 213992376 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(23,108,85,163),rangeType(108,590,163,186)));
					ConvertMaxDamage(rangeTableType(rangeType(96,240,256,326),rangeType(240,600,326,372)));
					ConvertAttackSpeed(rangeTableType(rangeType(8.4,13.8,180,200),rangeType(13.8,36.4,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(18,47,1,24),rangeType(47,78,24,28.8)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--sword_ryyk_blade
				elsif template_id = -1330392160 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(17,111,198,378),rangeType(111,440,378,432)));
					ConvertMaxDamage(rangeTableType(rangeType(113,232,594,756),rangeType(232,507,756,864)));
					ConvertAttackSpeed(rangeTableType(rangeType(9.8,16.5,180,200),rangeType(16.5,39.1,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(20,52,5,29),rangeType(52,74,29,34.8)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--sword_curved_nyax
				elsif template_id = 1834662895 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(16,19,171,326),rangeType(19,24,326,372)));
					ConvertMaxDamage(rangeTableType(rangeType(74,88,512,651),rangeType(88,117,651,744)));
					ConvertAttackSpeed(rangeTableType(rangeType(26.4,36.9,180,200),rangeType(36.9,41.6,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(11,13,5,27),rangeType(13,19,27,32.4)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--sword_nyax
				elsif template_id = -1781845673 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(24,28,142,271),rangeType(28,40,271,310)));
					ConvertMaxDamage(rangeTableType(rangeType(66,80,426,541),rangeType(80,113,541,618)));
					ConvertAttackSpeed(rangeTableType(rangeType(27.6,41.7,225,250),rangeType(41.7,46.8,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(9,10,1,20),rangeType(10,14,20,24)));
					SetWeaponBaseStats(104,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--sword_rantok
				elsif template_id = 1214765971 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(42,51,112,215),rangeType(51,70,215,246)));
					ConvertMaxDamage(rangeTableType(rangeType(141,171,339,431),rangeType(171,229,431,492)));
					ConvertAttackSpeed(rangeTableType(rangeType(19.4,28.7,180,200),rangeType(28.7,34.9,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(20,20,5,23),rangeType(20,24,23,27.6)));
					SetWeaponBaseStats(84,0,0,5,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--carbine_blaster_cdef
				elsif template_id = -1213235742 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(15,24,22,42),rangeType(24,171,42,48)));
					ConvertMaxDamage(rangeTableType(rangeType(31,49,66,83),rangeType(49,269,83,95)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,36.1,203,225),rangeType(36.1,44.1,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(4,11,1,5),rangeType(11,75,5,6)));
					SetWeaponBaseStats(94,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--carbine_blaster_cdef_corsec
				elsif template_id = -1498919413 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(14,22,24,45),rangeType(22,60,45,52)));
					ConvertMaxDamage(rangeTableType(rangeType(33,49,70,89),rangeType(49,134,89,102)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,31,203,225),rangeType(31,37.1,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(8,11,2,4),rangeType(11,25,4,4.8)));
					SetWeaponBaseStats(94,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_light_blaster_dh17_carbine
				elsif template_id = -1109444980 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(14,95,26,49),rangeType(95,315,49,56)));
					ConvertMaxDamage(rangeTableType(rangeType(60,218,77,98),rangeType(218,581,98,112)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,22.7,203,225),rangeType(22.7,46.1,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(2,23,1,12),rangeType(23,51,12,14.4)));
					SetWeaponBaseStats(94,0,0,40,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_light_blaster_dh17_carbine_black
				elsif template_id = -379196495 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(17,46,25,49),rangeType(46,440,49,56)));
					ConvertMaxDamage(rangeTableType(rangeType(68,104,78,99),rangeType(104,578,99,113)));
					ConvertAttackSpeed(rangeTableType(rangeType(18.7,36.7,203,225),rangeType(36.7,41.8,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(6,7,1,15),rangeType(7,17,15,18)));
					SetWeaponBaseStats(94,0,0,30,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_light_blaster_dh17_carbine_snubnose
				elsif template_id = 648215202 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(27,118,52,98),rangeType(118,440,98,112)));
					ConvertMaxDamage(rangeTableType(rangeType(87,260,153,194),rangeType(260,506,194,222)));
					ConvertAttackSpeed(rangeTableType(rangeType(10.8,17.9,203,225),rangeType(17.9,43.2,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(5,28,1,12),rangeType(28,48,12,14.4)));
					SetWeaponBaseStats(94,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_disrupter_dxr6
				elsif template_id = -279076105 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(102,188,213,406),rangeType(188,290,406,464)));
					ConvertMaxDamage(rangeTableType(rangeType(161,291,638,812),rangeType(291,552,812,928)));
					ConvertAttackSpeed(rangeTableType(rangeType(15.7,27.3,225,250),rangeType(27.3,44.6,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(19,42,1,20),rangeType(42,60,20,24)));
					SetWeaponBaseStats(104,0,0,65,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--carbine_e11_mk2
				elsif template_id = -2138350593 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(22,128,255,486),rangeType(128,399,486,556)));
					ConvertMaxDamage(rangeTableType(rangeType(70,224,763,971),rangeType(224,419,971,1110)));
					ConvertAttackSpeed(rangeTableType(rangeType(12.5,22,203,225),rangeType(22,40.6,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(5,31,2,25),rangeType(31,46,25,30)));
					SetWeaponBaseStats(94,0,0,50,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
							new_schematic_id := -1108671633;
						else
							ModifyObjVar('draftSchematic','draftSchematic',0,-1108671633);
						end if;
						SetTemplateText(-746051337);
					else
						SetNewTemplateId(-746051337);  --convert to E11 Mark II
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_light_blaster_e11_carbine_quest
				elsif template_id = -1839044042 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(66,133,99,189),rangeType(133,187,189,216)));
					ConvertMaxDamage(rangeTableType(rangeType(136,254,297,378),rangeType(254,531,378,432)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,19.6,203,225),rangeType(19.6,33.4,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(14,36,1,13),rangeType(36,46,13,15.6)));
					SetWeaponBaseStats(94,0,0,40,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_light_blaster_e11_carbine_victor
				elsif template_id = -1601393927 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(50,50,101,192),rangeType(50,50,192,220)));
					ConvertMaxDamage(rangeTableType(rangeType(350,350,301,383),rangeType(350,350,383,438)));
					ConvertAttackSpeed(rangeTableType(rangeType(40,40,203,225),rangeType(40,40,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(20,20,2,20),rangeType(20,20,20,24)));
					SetWeaponBaseStats(94,0,0,40,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_light_blaster_ee3
				elsif template_id = 418740352 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(39,147,129,245),rangeType(147,290,245,280)));
					ConvertMaxDamage(rangeTableType(rangeType(111,304,384,488),rangeType(304,556,488,558)));
					ConvertAttackSpeed(rangeTableType(rangeType(12.5,-28.1,203,225),rangeType(-28.1,83.6,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(3,29,1,9),rangeType(29,44,9,10.8)));
					SetWeaponBaseStats(94,0,0,50,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--carbine_e5
				elsif template_id = 1778522271 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(96,180,255,486),rangeType(180,290,486,556)));
					ConvertMaxDamage(rangeTableType(rangeType(165,298,763,971),rangeType(298,650,971,1110)));
					ConvertAttackSpeed(rangeTableType(rangeType(12.4,25.6,203,225),rangeType(25.6,41.7,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(15,39,1,9),rangeType(39,48,9,10.8)));
					SetWeaponBaseStats(94,0,0,50,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_light_blaster_laser_carbine
				elsif template_id = 764829944 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(10,104,206,392),rangeType(104,460,392,448)));
					ConvertMaxDamage(rangeTableType(rangeType(191,433,615,782),rangeType(433,834,782,894)));
					ConvertAttackSpeed(rangeTableType(rangeType(10.1,24.6,203,225),rangeType(24.6,46.4,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(11,39,1,16),rangeType(39,66,16,19.2)));
					SetWeaponBaseStats(94,0,0,50,2);
					SetElementalType(32);
					SetElementalValue(63);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--carbine_nym_slugthrower
				elsif template_id = -627137144 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(95,122,180,344),rangeType(122,181,344,394)));
					ConvertMaxDamage(rangeTableType(rangeType(198,270,541,688),rangeType(270,468,688,786)));
					ConvertAttackSpeed(rangeTableType(rangeType(17.9,32.8,203,225),rangeType(32.8,42.9,225,275)));
					ConvertWoundChance(rangeTableType(rangeType(16,16,6,16),rangeType(16,22,16,19.2)));
					SetWeaponBaseStats(94,0,0,50,1);
					SetElementalType(128);
					SetElementalValue(144);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--heavy_acid_beam
				elsif template_id = 100102071 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(914,1250,178,339),rangeType(1250,1426,339,388)));
					ConvertMaxDamage(rangeTableType(rangeType(2744,3876,532,677),rangeType(3876,4317,677,774)));
					ConvertAttackSpeed(rangeTableType(rangeType(30.5,35.3,248,275),rangeType(35.3,59.7,275,325)));
					ConvertWoundChance(rangeTableType(rangeType(31,45,9,35),rangeType(45,49,35,42)));
					SetWeaponBaseStats(122,0,0,65,2);
					SetElementalType(128);
					SetElementalValue(189);
					AddObjVar('intAOEDamagePercent',2,'0.0');
					AddObjVar('intWeaponType',0,'6');
					AddObjVar('strWeaponType',4,'acid_beam');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--heavy_lightning_beam
				elsif template_id = 949442296 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(850,1707,212,405),rangeType(1707,1706,405,463)));
					ConvertMaxDamage(rangeTableType(rangeType(2052,3895,637,811),rangeType(3895,4057,811,926)));
					ConvertAttackSpeed(rangeTableType(rangeType(36.2,44.5,248,275),rangeType(44.5,73.1,275,325)));
					ConvertWoundChance(rangeTableType(rangeType(20,34,3,23),rangeType(34,37,23,27.6)));
					SetWeaponBaseStats(122,0,0,40,2);
					SetElementalType(256);
					SetElementalValue(200);
					AddObjVar('intAOEDamagePercent',2,'1.0');
					AddObjVar('intWeaponType',0,'7');
					AddObjVar('strWeaponType',4,'lightning_beam');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--heavy_particle_beam
				elsif template_id = 180006972 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(1839,2296,155,296),rangeType(2296,3015,296,338)));
					ConvertMaxDamage(rangeTableType(rangeType(2775,3892,465,591),rangeType(3892,4930,591,676)));
					ConvertAttackSpeed(rangeTableType(rangeType(25.8,31.9,248,275),rangeType(31.9,66.8,275,325)));
					ConvertWoundChance(rangeTableType(rangeType(16,29,1,20),rangeType(29,34,20,24)));
					SetWeaponBaseStats(122,0,0,40,2);
					SetElementalType(32);
					SetElementalValue(135);
					AddObjVar('intAOEDamagePercent',2,'1.0');
					AddObjVar('intWeaponType',0,'4');
					AddObjVar('strWeaponType',4,'particle_beam');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--heavy_rocket_launcher
				elsif template_id = -926822510 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(477,1567,101,194),rangeType(1567,1739,194,222)));
					ConvertMaxDamage(rangeTableType(rangeType(865,4849,306,389),rangeType(4849,5334,389,444)));
					ConvertAttackSpeed(rangeTableType(rangeType(44.6,53.5,248,275),rangeType(53.5,134.7,275,325)));
					ConvertWoundChance(rangeTableType(rangeType(14,41,7,31),rangeType(41,47,31,37.2)));
					SetWeaponBaseStats(122,0,0,64,1);
					SetElementalType(32);
					SetElementalValue(180);
					AddObjVar('intWeaponType',0,'2');
					AddObjVar('strWeaponType',4,'rocket_launcher');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--quest_heavy_acid_beam
				elsif template_id = -633473500 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(61,68,178,339),rangeType(68,84,339,388)));
					ConvertMaxDamage(rangeTableType(rangeType(439,489,532,677),rangeType(489,609,677,774)));
					ConvertAttackSpeed(rangeTableType(rangeType(95.9,126.4,248,275),rangeType(126.4,142.6,275,325)));
					ConvertWoundChance(rangeTableType(rangeType(19,19,4,25),rangeType(19,19,25,30)));
					SetWeaponBaseStats(122,0,0,65,2);
					SetElementalType(128);
					SetElementalValue(198);
					AddObjVar('intAOEDamagePercent',2,'0.0');
					AddObjVar('intWeaponType',0,'6');
					AddObjVar('strWeaponType',4,'acid_beam');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--quest_heavy_particle_beam
				elsif template_id = 451466626 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(135,149,160,305),rangeType(149,189,305,348)));
					ConvertMaxDamage(rangeTableType(rangeType(203,223,479,609),rangeType(223,283,609,696)));
					ConvertAttackSpeed(rangeTableType(rangeType(70.7,93.2,248,275),rangeType(93.2,105.1,275,325)));
					ConvertWoundChance(rangeTableType(rangeType(11,11,8,25),rangeType(11,11,25,30)));
					SetWeaponBaseStats(122,0,0,40,2);
					SetElementalType(32);
					SetElementalValue(126);
					AddObjVar('intAOEDamagePercent',2,'1.0');
					AddObjVar('intWeaponType',0,'4');
					AddObjVar('strWeaponType',4,'particle_beam');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_blaster_cdef
				elsif template_id = -82570045 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(13,24,19,36),rangeType(24,590,36,41)));
					ConvertMaxDamage(rangeTableType(rangeType(30,45,56,70),rangeType(45,600,70,80)));
					ConvertAttackSpeed(rangeTableType(rangeType(14.2,37.6,180,200),rangeType(37.6,41.8,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(1,3,1,5),rangeType(3,13,5,6)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_blaster_cdef_corsec
				elsif template_id = -860173634 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(15,20,19,36),rangeType(20,33,36,41)));
					ConvertMaxDamage(rangeTableType(rangeType(35,46,56,71),rangeType(46,76,71,82)));
					ConvertAttackSpeed(rangeTableType(rangeType(18.1,31.4,180,200),rangeType(31.4,37,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(2,2,1,12),rangeType(2,5,12,14.4)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_blaster_d18
				elsif template_id = 252668529 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(-6,46,22,42),rangeType(46,640,42,48)));
					ConvertMaxDamage(rangeTableType(rangeType(24,99,66,84),rangeType(99,650,84,96)));
					ConvertAttackSpeed(rangeTableType(rangeType(5.4,21.3,180,200),rangeType(21.3,35.7,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(1,13,1,8),rangeType(13,51,8,9.6)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_de_10
				elsif template_id = 1866781530 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(129,198,230,439),rangeType(198,247,439,502)));
					ConvertMaxDamage(rangeTableType(rangeType(263,534,690,877),rangeType(534,772,877,1002)));
					ConvertAttackSpeed(rangeTableType(rangeType(15.4,24.9,180,200),rangeType(24.9,42.1,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(32,61,1,20),rangeType(61,91,20,24)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_blaster_dh17
				elsif template_id = -245080147 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(30,135,112,215),rangeType(135,440,215,246)));
					ConvertMaxDamage(rangeTableType(rangeType(58,191,339,431),rangeType(191,450,431,492)));
					ConvertAttackSpeed(rangeTableType(rangeType(4.2,9.7,180,200),rangeType(9.7,37.5,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(7,28,1,13),rangeType(28,58,13,15.6)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_blaster_dl44
				elsif template_id = 1684501073 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(0,75,35,66),rangeType(75,490,66,76)));
					ConvertMaxDamage(rangeTableType(rangeType(52,186,103,131),rangeType(186,536,131,150)));
					ConvertAttackSpeed(rangeTableType(rangeType(8.1,16.3,180,200),rangeType(16.3,43,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(1,22,1,9),rangeType(22,54,9,10.8)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_blaster_dl44_metal
				elsif template_id = -1925049902 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(-5,95,228,434),rangeType(95,640,434,496)));
					ConvertMaxDamage(rangeTableType(rangeType(62,216,681,866),rangeType(216,650,866,990)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,16,180,200),rangeType(16,41.3,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(2,24,1,9),rangeType(24,54,9,10.8)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_disrupter_dx2
				elsif template_id = -837691545 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(87,141,228,434),rangeType(141,290,434,496)));
					ConvertMaxDamage(rangeTableType(rangeType(135,240,681,866),rangeType(240,552,866,990)));
					ConvertAttackSpeed(rangeTableType(rangeType(8.8,16.5,180,200),rangeType(16.5,26.7,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(18,34,1,16),rangeType(34,60,16,19.2)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_flechette_fwg5
				elsif template_id = -1792584100 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(28,94,228,434),rangeType(94,440,434,496)));
					ConvertMaxDamage(rangeTableType(rangeType(116,313,681,866),rangeType(313,665,866,990)));
					ConvertAttackSpeed(rangeTableType(rangeType(4.2,7.5,180,200),rangeType(7.5,31.9,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(10,27,1,12),rangeType(27,49,12,14.4)));
					SetWeaponBaseStats(84,0,0,35,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_flechette_fwg5_quest
				elsif template_id = -1340116130 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(53,79,231,441),rangeType(79,108,441,504)));
					ConvertMaxDamage(rangeTableType(rangeType(146,230,693,882),rangeType(230,382,882,1008)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,16.6,180,200),rangeType(16.6,25.9,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(14,32,1,12),rangeType(32,45,12,14.4)));
					SetWeaponBaseStats(84,0,0,35,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_geo_sonic_blaster
				elsif template_id = -1400940656 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(56,163,142,271),rangeType(163,300,271,310)));
					ConvertMaxDamage(rangeTableType(rangeType(87,343,426,541),rangeType(343,747,541,618)));
					ConvertAttackSpeed(rangeTableType(rangeType(9.1,18.5,180,200),rangeType(18.5,37.6,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(-3,36,1,16),rangeType(36,63,16,19.2)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_launcher
				elsif template_id = 1980615506 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(49,92,63,121),rangeType(92,133,121,138)));
					ConvertMaxDamage(rangeTableType(rangeType(174,309,190,242),rangeType(309,592,242,276)));
					ConvertAttackSpeed(rangeTableType(rangeType(5.2,16.7,180,200),rangeType(16.7,33.3,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(22,41,3,35),rangeType(41,56,35,42)));
					SetWeaponBaseStats(84,0,0,45,1);
					SetElementalType(32);
					SetElementalValue(72);
					AddObjVar('intAOEDamagePercent',2,'0.5');
					AddObjVar('intWeaponType',0,'1');
					AddObjVar('weaponType',0,'3');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_blaster_power5
				elsif template_id = 1687234741 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(-4,114,171,326),rangeType(114,350,326,372)));
					ConvertMaxDamage(rangeTableType(rangeType(85,247,512,651),rangeType(247,623,651,744)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,19.4,180,200),rangeType(19.4,41,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(5,29,1,13),rangeType(29,56,13,15.6)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_republic_blaster
				elsif template_id = -2032619870 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(36,95,217,413),rangeType(95,121,413,472)));
					ConvertMaxDamage(rangeTableType(rangeType(134,306,648,824),rangeType(306,628,824,942)));
					ConvertAttackSpeed(rangeTableType(rangeType(8.5,12.8,180,200),rangeType(12.8,37.9,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(17,36,1,20),rangeType(36,62,20,24)));
					SetWeaponBaseStats(84,0,0,35,2);
					SetElementalType(128);
					SetElementalValue(48);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_republic_blaster_quest
				elsif template_id = 1070818136 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(59,89,228,434),rangeType(89,127,434,496)));
					ConvertMaxDamage(rangeTableType(rangeType(181,303,681,866),rangeType(303,569,866,990)));
					ConvertAttackSpeed(rangeTableType(rangeType(10.1,17.4,180,200),rangeType(17.4,27.5,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(21,40,3,25),rangeType(40,62,25,30)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_scatter
				elsif template_id = 322519616 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(83,173,228,434),rangeType(173,251,434,496)));
					ConvertMaxDamage(rangeTableType(rangeType(104,290,681,866),rangeType(290,647,866,990)));
					ConvertAttackSpeed(rangeTableType(rangeType(0.1,9.8,180,200),rangeType(9.8,30.3,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(21,48,5,27),rangeType(48,68,27,32.4)));
					SetWeaponBaseStats(84,0,0,35,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_blaster_scout_trooper
				elsif template_id = 129228399 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(25,98,46,87),rangeType(98,290,87,100)));
					ConvertMaxDamage(rangeTableType(rangeType(59,206,136,173),rangeType(206,869,173,198)));
					ConvertAttackSpeed(rangeTableType(rangeType(5.4,7.9,180,200),rangeType(7.9,37.8,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(4,25,1,9),rangeType(25,55,9,10.8)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_blaster_scout_trooper_corsec
				elsif template_id = -1929515589 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(22,28,46,87),rangeType(28,51,87,100)));
					ConvertMaxDamage(rangeTableType(rangeType(55,70,136,173),rangeType(70,129,173,198)));
					ConvertAttackSpeed(rangeTableType(rangeType(23.4,31.4,180,200),rangeType(31.4,36.8,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(4,5,1,11),rangeType(5,9,11,13.2)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_blaster_short_range_combat
				elsif template_id = 1691705957 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(4,124,162,310),rangeType(124,420,310,354)));
					ConvertMaxDamage(rangeTableType(rangeType(63,213,487,620),rangeType(213,659,620,708)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,22.8,180,200),rangeType(22.8,41.2,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(7,29,1,17),rangeType(29,59,17,20.4)));
					SetWeaponBaseStats(84,0,0,25,2);
					SetElementalType(64);
					SetElementalValue(27);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--pistol_projectile_striker
				elsif template_id = -1919723538 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(6,124,85,163),rangeType(124,440,163,186)));
					ConvertMaxDamage(rangeTableType(rangeType(31,187,256,326),rangeType(187,450,326,372)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,13.1,180,200),rangeType(13.1,42.4,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(4,29,1,13),rangeType(29,53,13,15.6)));
					SetWeaponBaseStats(84,0,0,35,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_tanlge_gun7
				elsif template_id = -937385248 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(49,85,178,339),rangeType(85,107,339,388)));
					ConvertMaxDamage(rangeTableType(rangeType(90,153,532,677),rangeType(153,242,677,774)));
					ConvertAttackSpeed(rangeTableType(rangeType(12.8,22.6,225,250),rangeType(22.6,35.4,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(3,4,2,4),rangeType(4,6,4,4.8)));
					SetWeaponBaseStats(104,0,0,35,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--quest_pistol_launcher
				elsif template_id = 685231719 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(17,19,99,189),rangeType(19,23,189,216)));
					ConvertMaxDamage(rangeTableType(rangeType(87,100,297,378),rangeType(100,121,378,432)));
					ConvertAttackSpeed(rangeTableType(rangeType(38.1,51.9,248,275),rangeType(51.9,59.6,275,325)));
					ConvertWoundChance(rangeTableType(rangeType(12,13,4,35),rangeType(13,15,35,42)));
					SetWeaponBaseStats(122,0,0,45,1);
					SetElementalType(32);
					SetElementalValue(72);
					AddObjVar('intAOEDamagePercent',2,'0.5');
					AddObjVar('intWeaponType',0,'1');
					AddObjVar('skillmod.bonus.pistol_speed',0,'3');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--quest_pistol_republic_blaster_quest
				elsif template_id = -1340981329 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(20,22,228,436),rangeType(22,27,436,498)));
					ConvertMaxDamage(rangeTableType(rangeType(93,109,685,872),rangeType(109,152,872,996)));
					ConvertAttackSpeed(rangeTableType(rangeType(32.2,42.1,180,200),rangeType(42.1,48,200,250)));
					ConvertWoundChance(rangeTableType(rangeType(15,15,3,25),rangeType(15,18,25,30)));
					SetWeaponBaseStats(84,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--quest_rifle_flame_thrower
				elsif template_id = -1238955746 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(14,15,277,530),rangeType(15,17,530,606)));
					ConvertMaxDamage(rangeTableType(rangeType(149,165,834,1061),rangeType(165,196,1061,1212)));
					ConvertAttackSpeed(rangeTableType(rangeType(49,67.5,248,275),rangeType(67.5,74.5,275,325)));
					ConvertWoundChance(rangeTableType(rangeType(14,14,2,19),rangeType(14,14,19,22.8)));
					SetWeaponBaseStats(122,0,0,25,2);
					SetElementalType(32);
					SetElementalValue(126);
					AddObjVar('intAOEDamagePercent',2,'1.0');
					AddObjVar('intWeaponType',0,'3');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--quest_rifle_lightning
				elsif template_id = -656409820 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(13,14,197,376),rangeType(14,17,376,430)));
					ConvertMaxDamage(rangeTableType(rangeType(125,140,591,751),rangeType(140,175,751,858)));
					ConvertAttackSpeed(rangeTableType(rangeType(44.9,61.7,225,250),rangeType(61.7,71.2,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(11,11,1,17),rangeType(11,12,17,20.4)));
					SetWeaponBaseStats(104,0,0,65,2);
					SetElementalType(256);
					SetElementalValue(58);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_acid_beam
				elsif template_id = -67541093 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(239,480,121,231),rangeType(480,608,231,264)));
					ConvertMaxDamage(rangeTableType(rangeType(549,1071,363,462),rangeType(1071,1393,462,528)));
					ConvertAttackSpeed(rangeTableType(rangeType(29.9,36.9,248,275),rangeType(36.9,64.3,275,325)));
					ConvertWoundChance(rangeTableType(rangeType(29,55,5,30),rangeType(55,67,30,36)));
					SetWeaponBaseStats(122,0,0,45,2);
					SetElementalType(128);
					SetElementalValue(117);
					AddObjVar('intWeaponType',0,'5');
					AddObjVar('weaponType',0,'3');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_beam
				elsif template_id = 1223042704 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(53,174,145,276),rangeType(174,246,276,316)));
					ConvertMaxDamage(rangeTableType(rangeType(103,287,433,551),rangeType(287,416,551,630)));
					ConvertAttackSpeed(rangeTableType(rangeType(20.2,32.2,225,250),rangeType(32.2,61.2,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(9,61,5,30),rangeType(61,67,30,36)));
					SetWeaponBaseStats(104,0,0,65,2);
					SetElementalType(32);
					SetElementalValue(108);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_berserker
				elsif template_id = 1646698389 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(143,227,212,405),rangeType(227,259,405,463)));
					ConvertMaxDamage(rangeTableType(rangeType(305,559,637,811),rangeType(559,699,811,926)));
					ConvertAttackSpeed(rangeTableType(rangeType(26.9,33.1,225,250),rangeType(33.1,56.2,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(39,67,5,35),rangeType(67,92,35,42)));
					SetWeaponBaseStats(104,0,0,65,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--bowcaster_assault
				elsif template_id = -1930572145 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(64,189,283,541),rangeType(189,486,541,618)));
					ConvertMaxDamage(rangeTableType(rangeType(109,347,850,1082),rangeType(347,911,1082,1236)));
					ConvertAttackSpeed(rangeTableType(rangeType(15.3,41.6,225,250),rangeType(41.6,83.5,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(7,25,2,25),rangeType(25,43,25,30)));
					SetWeaponBaseStats(104,0,0,65,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
							new_schematic_id := -847903116;
						else
							ModifyObjVar('draftSchematic','draftSchematic',0,-847903116);
						end if;
						SetTemplateText(-1631144444);
					else
						SetNewTemplateId(-1631144444);  --convert to heavy bowcaster
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_blaster_cdef
				elsif template_id = -1437726662 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(15,23,24,45),rangeType(23,85,45,52)));
					ConvertMaxDamage(rangeTableType(rangeType(31,49,70,89),rangeType(49,178,89,102)));
					ConvertAttackSpeed(rangeTableType(rangeType(10,36.3,225,250),rangeType(36.3,44.9,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(4,11,1,5),rangeType(11,44,5,6)));
					SetWeaponBaseStats(104,0,0,35,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_blaster_dlt20
				elsif template_id = -2118141076 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(47,144,30,56),rangeType(144,305,56,64)));
					ConvertMaxDamage(rangeTableType(rangeType(65,228,87,110),rangeType(228,466,110,126)));
					ConvertAttackSpeed(rangeTableType(rangeType(16.4,48.3,225,250),rangeType(48.3,76.3,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(1,23,1,12),rangeType(23,47,12,14.4)));
					SetWeaponBaseStats(104,0,0,40,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_blaster_dlt20a
				elsif template_id = -1354942232 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(52,153,43,82),rangeType(153,446,82,94)));
					ConvertMaxDamage(rangeTableType(rangeType(79,241,129,163),rangeType(241,551,163,186)));
					ConvertAttackSpeed(rangeTableType(rangeType(14,45,225,250),rangeType(45,77.9,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(3,23,1,12),rangeType(23,53,12,14.4)));
					SetWeaponBaseStats(104,0,0,45,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_blaster_e11
				elsif template_id = 709369610 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(35,148,142,271),rangeType(148,290,271,310)));
					ConvertMaxDamage(rangeTableType(rangeType(82,258,426,541),rangeType(258,469,541,618)));
					ConvertAttackSpeed(rangeTableType(rangeType(18.7,37.4,225,250),rangeType(37.4,63.4,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(4,30,1,13),rangeType(30,46,13,15.6)));
					SetWeaponBaseStats(104,0,0,45,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_flame_thrower
				elsif template_id = -205520309 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(251,711,274,523),rangeType(711,771,523,598)));
					ConvertMaxDamage(rangeTableType(rangeType(560,1283,822,1045),rangeType(1283,1432,1045,1194)));
					ConvertAttackSpeed(rangeTableType(rangeType(16.7,30.1,248,275),rangeType(30.1,60.2,275,325)));
					ConvertWoundChance(rangeTableType(rangeType(15,28,1,16),rangeType(28,50,16,19.2)));
					SetWeaponBaseStats(122,0,0,25,2);
					SetElementalType(32);
					SetElementalValue(126);
					AddObjVar('intAOEDamagePercent',2,'0.4');
					AddObjVar('intWeaponType',0,'3');
					AddObjVar('weaponType',0,'3');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_blaster_ionization_jawa
				elsif template_id = -603889252 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(59,187,213,407),rangeType(187,257,407,466)));
					ConvertMaxDamage(rangeTableType(rangeType(109,384,640,814),rangeType(384,721,814,930)));
					ConvertAttackSpeed(rangeTableType(rangeType(16.6,35,225,250),rangeType(35,79.1,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(3,13,1,5),rangeType(13,22,5,6)));
					SetWeaponBaseStats(104,0,0,65,2);
					SetElementalType(256);
					SetElementalValue(117);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_blaster_laser_rifle
				elsif template_id = -1321212581 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(12,101,283,541),rangeType(101,610,541,618)));
					ConvertMaxDamage(rangeTableType(rangeType(296,605,850,1082),rangeType(605,1157,1082,1236)));
					ConvertAttackSpeed(rangeTableType(rangeType(18.8,39,225,250),rangeType(39,71.5,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(10,35,1,13),rangeType(35,51,13,15.6)));
					SetWeaponBaseStats(104,0,0,65,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_lightning
				elsif template_id = 397924793 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(285,622,151,287),rangeType(622,723,287,328)));
					ConvertMaxDamage(rangeTableType(rangeType(561,1100,450,572),rangeType(1100,1656,572,654)));
					ConvertAttackSpeed(rangeTableType(rangeType(19.2,28,225,250),rangeType(28,52.3,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(13,38,1,16),rangeType(38,50,16,19.2)));
					SetWeaponBaseStats(104,0,0,65,2);
					SetElementalType(256);
					SetElementalValue(90);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_sonic_sg82
				elsif template_id = -1148251186 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(69,173,57,109),rangeType(173,514,109,125)));
					ConvertMaxDamage(rangeTableType(rangeType(110,280,172,218),rangeType(280,668,218,250)));
					ConvertAttackSpeed(rangeTableType(rangeType(20,36,225,250),rangeType(36,68.9,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(10,20,1,18),rangeType(20,33,18,21.6)));
					SetWeaponBaseStats(104,0,0,40,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_spray_stick_stohli
				elsif template_id = 2069010987 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(3,61,77,147),rangeType(61,86,147,168)));
					ConvertMaxDamage(rangeTableType(rangeType(87,242,231,294),rangeType(242,415,294,336)));
					ConvertAttackSpeed(rangeTableType(rangeType(7.1,14,225,250),rangeType(14,22.3,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(13,23,1,22),rangeType(23,71,22,26.4)));
					SetWeaponBaseStats(104,0,0,65,1);
					SetElementalType(32);
					SetElementalValue(99);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_t21
				elsif template_id = -1374316705 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(122,243,283,541),rangeType(243,725,541,618)));
					ConvertMaxDamage(rangeTableType(rangeType(333,606,850,1082),rangeType(606,2787,1082,1236)));
					ConvertAttackSpeed(rangeTableType(rangeType(20,43.7,225,250),rangeType(43.7,92.1,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(23,46,1,29),rangeType(46,68,29,34.8)));
					SetWeaponBaseStats(104,0,0,65,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_disruptor_dxr6
				elsif template_id = -1428709338 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(120,274,213,406),rangeType(274,607,406,464)));
					ConvertMaxDamage(rangeTableType(rangeType(262,585,638,812),rangeType(585,1659,812,928)));
					ConvertAttackSpeed(rangeTableType(rangeType(22.2,35.7,225,250),rangeType(35.7,67.9,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(37,67,1,20),rangeType(67,96,20,24)));
					SetWeaponBaseStats(104,0,0,65,2);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_projectile_tusken
				elsif template_id = -2024883534 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(44,149,212,405),rangeType(149,465,405,463)));
					ConvertMaxDamage(rangeTableType(rangeType(71,225,637,811),rangeType(225,799,811,926)));
					ConvertAttackSpeed(rangeTableType(rangeType(12.5,65.6,225,250),rangeType(65.6,98,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(0,22,1,12),rangeType(22,62,12,14.4)));
					SetWeaponBaseStats(104,0,0,65,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--rifle_victor_projectile_tusken
				elsif template_id = -896708611 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(68,103,215,410),rangeType(103,114,410,468)));
					ConvertMaxDamage(rangeTableType(rangeType(282,513,644,819),rangeType(513,565,819,936)));
					ConvertAttackSpeed(rangeTableType(rangeType(36.3,42.3,225,250),rangeType(42.3,59.6,250,300)));
					ConvertWoundChance(rangeTableType(rangeType(6,6,1,17),rangeType(6,6,17,20.4)));
					SetWeaponBaseStats(104,0,0,65,1);
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--bug_bomb
				elsif template_id = 1325615183 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(420,524,178,339),rangeType(524,579,339,388)));
					ConvertMaxDamage(rangeTableType(rangeType(553,639,532,677),rangeType(639,766,677,774)));
					ConvertAttackSpeed(rangeTableType(rangeType(48.9,56.8,1000,1000),rangeType(56.8,61.7,1000,1200)));
					ConvertWoundChance(rangeTableType(rangeType(17,34,0,5),rangeType(34,31,5,6)));
					SetAttackCost(65);
					SetAccuracy(0);
					SetMinRange(0);
					AddObjVar('weapon.intNoCertRequired',0,'1');
					AddObjVar('effect_class',4,'bug');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--cryoban
				elsif template_id = 1007504804 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(3,400,142,271),rangeType(400,556,271,310)));
					ConvertMaxDamage(rangeTableType(rangeType(158,1045,426,541),rangeType(1045,4018,541,618)));
					ConvertAttackSpeed(rangeTableType(rangeType(7.1,15.2,1000,1000),rangeType(15.2,89.7,1000,1200)));
					ConvertWoundChance(rangeTableType(rangeType(7,19,0,5),rangeType(19,31,5,6)));
					SetAttackCost(65);
					SetAccuracy(0);
					SetMinRange(0);
					AddObjVar('effect_class',4,'cryoban');
					AddObjVar('slowDuration',0,'10');
					AddObjVar('slowIntensity',0,'10');
					AddObjVar('reuseTimer',0,'15');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--cryoban_loot_medium
				elsif template_id = 1775069676 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(110,123,106,203),rangeType(123,135,203,232)));
					ConvertMaxDamage(rangeTableType(rangeType(374,419,319,405),rangeType(419,461,405,463)));
					ConvertAttackSpeed(rangeTableType(rangeType(71.1,82.1,1000,1000),rangeType(82.1,94,1000,1200)));
					ConvertWoundChance(rangeTableType(rangeType(7,7,0,5),rangeType(7,7,5,6)));
					SetAttackCost(65);
					SetAccuracy(0);
					SetMinRange(0);
					AddObjVar('effect_class',4,'cryoban');
					AddObjVar('slowDuration',0,'10');
					AddObjVar('slowIntensity',0,'10');
					AddObjVar('reuseTimer',0,'15');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--fragmentation
				elsif template_id = -1629773375 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(4,402,77,147),rangeType(402,451,147,168)));
					ConvertMaxDamage(rangeTableType(rangeType(108,1855,231,294),rangeType(1855,5521,294,336)));
					ConvertAttackSpeed(rangeTableType(rangeType(3.4,32.5,1000,1000),rangeType(32.5,51.8,1000,1200)));
					ConvertWoundChance(rangeTableType(rangeType(6,17,20,40),rangeType(17,33,40,48)));
					SetAttackCost(65);
					SetAccuracy(0);
					SetMinRange(0);
					AddObjVar('effect_class',4,'fragmentation');
					AddObjVar('reuseTimer',0,'5');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--fragmentation_light
				elsif template_id = 1333331720 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(65,281,22,41),rangeType(281,323,41,47)));
					ConvertMaxDamage(rangeTableType(rangeType(250,1284,64,81),rangeType(1284,1456,81,92)));
					ConvertAttackSpeed(rangeTableType(rangeType(4.1,17.2,1000,1000),rangeType(17.2,48.2,1000,1200)));
					ConvertWoundChance(rangeTableType(rangeType(8,20,10,30),rangeType(20,20,30,36)));
					SetAttackCost(33);
					SetAccuracy(0);
					SetMinRange(0);
					AddObjVar('effect_class',4,'fragmentation');
					AddObjVar('reuseTimer',0,'5');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--glop
				elsif template_id = 869624722 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(4,219,178,339),rangeType(219,505,339,388)));
					ConvertMaxDamage(rangeTableType(rangeType(-62,1000,532,677),rangeType(1000,5015,677,774)));
					ConvertAttackSpeed(rangeTableType(rangeType(5.4,48.2,1000,1000),rangeType(48.2,65.2,1000,1200)));
					ConvertWoundChance(rangeTableType(rangeType(5,11,5,10),rangeType(11,32,10,12)));
					SetAttackCost(88);
					SetAccuracy(0);
					SetMinRange(0);
					AddObjVar('effect_class',4,'glop');
					AddObjVar('blindDuration',0,'10');
					AddObjVar('blindChance',0,'50');
					AddObjVar('reuseTimer',0,'25');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--imperial_detonator
				elsif template_id = 247923643 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(636,1356,215,410),rangeType(1356,1430,410,468)));
					ConvertMaxDamage(rangeTableType(rangeType(1515,3154,644,819),rangeType(3154,3762,819,936)));
					ConvertAttackSpeed(rangeTableType(rangeType(7.4,15.6,1000,1000),rangeType(15.6,50,1000,1200)));
					ConvertWoundChance(rangeTableType(rangeType(19,36,5,10),rangeType(36,35,10,12)));
					SetAttackCost(94);
					SetAccuracy(0);
					SetMinRange(0);
					AddObjVar('effect_class',4,'imperial_detonator');
					AddObjVar('reuseTimer',0,'20');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--proton
				elsif template_id = -631441882 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(566,2218,286,546),rangeType(2218,2137,546,624)));
					ConvertMaxDamage(rangeTableType(rangeType(1342,4718,858,1092),rangeType(4718,5430,1092,1248)));
					ConvertAttackSpeed(rangeTableType(rangeType(10.6,17.7,1000,1000),rangeType(17.7,101.5,1000,1200)));
					ConvertWoundChance(rangeTableType(rangeType(14,39,0,0),rangeType(39,37,0,0)));
					SetAttackCost(108);
					SetAccuracy(0);
					SetMinRange(0);
					AddObjVar('effect_class',4,'proton');
					AddObjVar('burnDuration',0,'30');
					AddObjVar('burnIntensity',0,'10');
					AddObjVar('reuseTimer',0,'25');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
				--thermal_detonator
				elsif template_id = -1328960537 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(445,1248,252,481),rangeType(1248,1451,481,550)));
					ConvertMaxDamage(rangeTableType(rangeType(1096,2946,756,961),rangeType(2946,3207,961,1098)));
					ConvertAttackSpeed(rangeTableType(rangeType(6.4,19.6,1000,1000),rangeType(19.6,47.8,1000,1200)));
					ConvertWoundChance(rangeTableType(rangeType(15,36,5,15),rangeType(36,34,15,18)));
					SetAttackCost(100);
					SetAccuracy(0);
					SetMinRange(0);
					AddObjVar('effect_class',4,'thermal_detonator');
					AddObjVar('burnDuration',0,'40');
					AddObjVar('burnIntensity',0,'30');
					AddObjVar('reuseTimer',0,'60');
					if isSchematic = TRUE or isFactory = TRUE then
						DeleteCraftingComponents;
						if isSchematic = TRUE then
							DeleteObjVar('crafting_attributes.crafting:*');
						end if;
					else
						AddScript('systems.combat.combat_weapon');
					end if;
-- ****** BEGIN POWERUP CONVERSION RULES ******
				--object/tangible/powerup/weapon/melee_generic.iff
				elsif template_id = 1327818409 then
					ConvertMeleePowerup;
				--object/tangible/powerup/weapon/mine_explosive.iff
				elsif template_id = -2117547605 then
					ConvertMinePowerup;
				--object/tangible/powerup/weapon/ranged_barrel.iff
				elsif template_id = 352903310 then
					ConvertRangedPowerup;
				--object/tangible/powerup/weapon/ranged_grip.iff
				elsif template_id = 130303407 then
					ConvertRangedPowerup;
				--object/tangible/powerup/weapon/ranged_muzzle.iff
				elsif template_id = 1006954225 then
					ConvertRangedPowerup;
				--object/tangible/powerup/weapon/ranged_power.iff
				elsif template_id = -1957995430 then
					ConvertRangedPowerup;
				--object/tangible/powerup/weapon/ranged_scope.iff
				elsif template_id = 200431483 then
					ConvertRangedPowerup;
				--object/tangible/powerup/weapon/ranged_stock.iff
				elsif template_id = 1219521873 then
					ConvertRangedPowerup;
-- ****** BEGIN SABER CONVERSION RULES ******
				--object/tangible/component/weapon/lightsaber/lightsaber_lance_module_force_crystal.iff
				elsif template_id = 1567297626 then
					DeleteObjVar('jedi.crystal.stats.mid_rng');
					DeleteObjVar('jedi.crystal.stats.zero_mod');
					DeleteObjVar('jedi.crystal.stats.min_mod');
					DeleteObjVar('jedi.crystal.stats.mid_mod');
					DeleteObjVar('jedi.crystal.stats.max_mod');
					DeleteObjVar('jedi.crystal.stats.action');
					DeleteObjVar('jedi.crystal.stats.health');
					DeleteObjVar('jedi.crystal.stats.mind');
					if not(slot_map.exists('jedi.crystal.stats.color')) and slot_map.exists('jedi.crystal.owner.name') then
						n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.min_dmg'),0)),rangeTableType(rangeType(-5,50,15,110)));
						AddObjVar ('jedi.crystal.stats.min_dmg', 0, n);
						n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.max_dmg'),0)),rangeTableType(rangeType(-5,50,15,110)));
						AddObjVar ('jedi.crystal.stats.max_dmg', 0, n);
						n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.speed'),100)),rangeTableType(rangeType(-0.6,0,-0.3,0)));
						AddObjVar ('jedi.crystal.stats.speed', 2, n);
					end if;
				--object/tangible/component/weapon/lightsaber/lightsaber_module_force_crystal.iff
				elsif template_id = 1062687370 then
					DeleteObjVar('jedi.crystal.stats.mid_rng');
					DeleteObjVar('jedi.crystal.stats.zero_mod');
					DeleteObjVar('jedi.crystal.stats.min_mod');
					DeleteObjVar('jedi.crystal.stats.mid_mod');
					DeleteObjVar('jedi.crystal.stats.max_mod');
					DeleteObjVar('jedi.crystal.stats.action');
					DeleteObjVar('jedi.crystal.stats.health');
					DeleteObjVar('jedi.crystal.stats.mind');
					if not(slot_map.exists('jedi.crystal.stats.color')) and slot_map.exists('jedi.crystal.owner.name') then
						n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.min_dmg'),0)),rangeTableType(rangeType(-5,50,15,110)));
						AddObjVar ('jedi.crystal.stats.min_dmg', 0, n);
						n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.max_dmg'),0)),rangeTableType(rangeType(-5,50,15,110)));
						AddObjVar ('jedi.crystal.stats.max_dmg', 0, n);
						n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.speed'),100)),rangeTableType(rangeType(-0.6,0,-0.3,0)));
						AddObjVar ('jedi.crystal.stats.speed', 2, n);
					end if;
				--object/tangible/component/weapon/lightsaber/lightsaber_module_krayt_dragon_pearl.iff
				elsif template_id = -1774596602 then
					DeleteObjVar('jedi.crystal.stats.mid_rng');
					DeleteObjVar('jedi.crystal.stats.zero_mod');
					DeleteObjVar('jedi.crystal.stats.min_mod');
					DeleteObjVar('jedi.crystal.stats.mid_mod');
					DeleteObjVar('jedi.crystal.stats.max_mod');
					DeleteObjVar('jedi.crystal.stats.action');
					DeleteObjVar('jedi.crystal.stats.health');
					DeleteObjVar('jedi.crystal.stats.mind');
					if not(slot_map.exists('jedi.crystal.stats.color')) and slot_map.exists('jedi.crystal.owner.name') then
						n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.min_dmg'),0)),rangeTableType(rangeType(10,50,25,120)));
						AddObjVar ('jedi.crystal.stats.min_dmg', 0, n);
						n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.max_dmg'),0)),rangeTableType(rangeType(10,50,25,120)));
						AddObjVar ('jedi.crystal.stats.max_dmg', 0, n);
						n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.speed'),100)),rangeTableType(rangeType(-0.6,-0.2,-0.3,-0.1)));
						AddObjVar ('jedi.crystal.stats.speed', 2, n);
					end if;
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_training.iff
				elsif template_id = 1530854406 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(50,70,50,80)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(130,170,110,185)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.5,4.8,3.7,4.0)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_one_handed_gen1.iff
				elsif template_id = -255701924 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(70,90,70,90)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(160,200,170,210)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.2,4.5,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_one_handed_gen2.iff
				elsif template_id = 735438027 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(80,100,90,110)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(170,210,180,220)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.2,4.5,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_one_handed_gen3.iff
				elsif template_id = 1658356550 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(130,150,140,160)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(220,260,230,270)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.2,4.5,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_one_handed_gen4.iff
				elsif template_id = -1724143534 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(140,160,150,170)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(230,270,240,280)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.2,4.5,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_one_handed_gen5.iff
				elsif template_id = -801714209 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(140,160,150,170)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(230,270,240,280)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.2,4.5,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/polearm/crafted_saber/sword_lightsaber_polearm_gen1.iff
				elsif template_id = -181151130 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(105,125,80,100)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(195,235,170,210)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.8,5.1,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/polearm/crafted_saber/sword_lightsaber_polearm_gen2.iff
				elsif template_id = 774140657 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(125,145,90,110)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(215,255,180,220)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.8,5.1,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/polearm/crafted_saber/sword_lightsaber_polearm_gen3.iff
				elsif template_id = 1730745724 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(195,215,140,160)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(285,305,230,270)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.8,5.1,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/polearm/crafted_saber/sword_lightsaber_polearm_gen4.iff
				elsif template_id = -1664467352 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(225,235,150,170)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(305,325,240,280)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.8,5.1,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/2h_sword/crafted_saber/sword_lightsaber_two_handed_gen1.iff
				elsif template_id = 431995434 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(90,110,70,90)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(180,220,170,210)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.5,4.8,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/2h_sword/crafted_saber/sword_lightsaber_two_handed_gen2.iff
				elsif template_id = -1029172547 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(100,120,80,100)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(190,230,180,220)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.5,4.8,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/2h_sword/crafted_saber/sword_lightsaber_two_handed_gen3.iff
				elsif template_id = -1952090832 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(175,185,130,150)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(255,295,230,270)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.5,4.8,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
				--object/weapon/melee/2h_sword/crafted_saber/sword_lightsaber_two_handed_gen4.iff
				elsif template_id = 1883659812 then
					DeleteObjVar('jedi.saber.base_stats.mid_rng');
					DeleteObjVar('jedi.saber.base_stats.zero_mod');
					DeleteObjVar('jedi.saber.base_stats.min_mod');
					DeleteObjVar('jedi.saber.base_stats.mid_mod');
					DeleteObjVar('jedi.saber.base_stats.max_mod');
					DeleteObjVar('jedi.saber.base_stats.action');
					DeleteObjVar('jedi.saber.base_stats.health');
					DeleteObjVar('jedi.saber.base_stats.mind');
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),rangeTableType(rangeType(185,195,140,160)));
					AddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);
					weapon_attribs.min_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),rangeTableType(rangeType(265,305,240,280)));
					AddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);
					weapon_attribs.max_damage := n;
					n := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),rangeTableType(rangeType(4.5,4.8,3.0,3.3)));
					AddObjVar ('jedi.saber.base_stats.speed', 2, n);
					weapon_attribs.attack_speed := n;
					AddObjVar ('jedi.saber.base_stats.attack_cost', 2, 100);
					weapon_attribs.attack_cost := 100;
					AddObjVar ('jedi.saber.base_stats.min_rng', 2, 0);
					weapon_attribs.min_range := 0;
					AddObjVar ('jedi.saber.base_stats.max_rng', 2, 5);
					weapon_attribs.max_range := 5;
					AddObjVar ('jedi.saber.base_stats.damage_type', 0, 2);
					weapon_attribs.damage_type := 2;
					AddScript('systems.combat.combat_weapon');
-- ****** BEGIN FOOD CONVERSION RULES ******
				--object/tangible/food/crafted/dessert_air_cake.iff
				elsif template_id = -980521968 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_air_cake');
						if slot_map.exists('duration.eff') then
							ModifyObjVar('duration.eff','effectiveness',2,rangeTableType(rangeType(0,40,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('duration.dur') then
							ModifyObjVar('duration.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('duration.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_almond_kwevvu_crisp_munchies.iff
				elsif template_id = -306844601 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_almond_kwevvu_crisp_munchies');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_blob_candy.iff
				elsif template_id = -2039935369 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_blob_candy');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_bofa_treat.iff
				elsif template_id = -23647699 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_bofa_treat');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_cavaellin_creams.iff
				elsif template_id = -1933405050 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_cavaellin_creams');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,120,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_chandad.iff
				elsif template_id = -1329737896 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_chandad');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_citros_snow_cake.iff
				elsif template_id = 144581616 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_citros_snow_cake');
						if slot_map.exists('duration.eff') then
							ModifyObjVar('duration.eff','effectiveness',2,rangeTableType(rangeType(0,40,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('duration.dur') then
							ModifyObjVar('duration.dur','duration',2,rangeTableType(rangeType(0,3200,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('duration.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_deneelian_fizz_pudding.iff
				elsif template_id = -1122191702 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_deneelian_fizz_pudding');
						if slot_map.exists('duration.eff') then
							ModifyObjVar('duration.eff','effectiveness',2,rangeTableType(rangeType(0,80,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('duration.dur') then
							ModifyObjVar('duration.dur','duration',2,rangeTableType(rangeType(0,240,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('duration.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_dweezel.iff
				elsif template_id = 22306368 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_dweezel');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_gorrnar.iff
				elsif template_id = 281064494 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_gorrnar');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,30,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_kiwik_clusjo_swirl.iff
				elsif template_id = -1320167407 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_kiwik_clusjo_swirl');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_parwan_nutricake.iff
				elsif template_id = 1061303036 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_parwan_nutricake');
						if slot_map.exists('instant.v1') then
							ModifyObjVar('instant.v1','effectiveness',2,rangeTableType(rangeType(0,130,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('instant.v2') then
							ModifyObjVar('instant.v2','duration',2,rangeTableType(rangeType(0,130,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('instant.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_pikatta_pie.iff
				elsif template_id = 507210122 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_pikatta_pie');
						if slot_map.exists('duration.eff') then
							ModifyObjVar('duration.eff','effectiveness',2,rangeTableType(rangeType(0,50,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('duration.dur') then
							ModifyObjVar('duration.dur','duration',2,rangeTableType(rangeType(0,3200,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('duration.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_pkneb.iff
				elsif template_id = -88500234 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_pkneb');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_pyollian_cake.iff
				elsif template_id = 1472463975 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_pyollian_cake');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,14,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_smugglers_delight.iff
				elsif template_id = 2080884324 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_smugglers_delight');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,80,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_vagnerian_canape.iff
				elsif template_id = -2052487231 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_vagnerian_canape');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_won_won.iff
				elsif template_id = -528877582 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_won_won');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_ahrisa.iff
				elsif template_id = -135648971 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_ahrisa');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_bivoli_tempari.iff
				elsif template_id = 230572551 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_bivoli_tempari');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_blood_chowder.iff
				elsif template_id = 1057167832 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_blood_chowder');
						if slot_map.exists('duration.eff') then
							ModifyObjVar('duration.eff','effectiveness',2,rangeTableType(rangeType(0,30,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('duration.dur') then
							ModifyObjVar('duration.dur','duration',2,rangeTableType(rangeType(0,4000,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('duration.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_cho_nor_hoola.iff
				elsif template_id = 536694865 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_cho_nor_hoola');
						if slot_map.exists('duration.eff') then
							ModifyObjVar('duration.eff','effectiveness',2,rangeTableType(rangeType(0,30,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('duration.dur') then
							ModifyObjVar('duration.dur','duration',2,rangeTableType(rangeType(0,4000,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('duration.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_crispic.iff
				elsif template_id = 437634302 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_crispic');
						if slot_map.exists('duration.eff') then
							ModifyObjVar('duration.eff','effectiveness',2,rangeTableType(rangeType(0,30,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('duration.dur') then
							ModifyObjVar('duration.dur','duration',2,rangeTableType(rangeType(0,1800,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('duration.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_dustcrepe.iff
				elsif template_id = 2054517358 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_dustcrepe');
						if slot_map.exists('instant.v1') then
							ModifyObjVar('instant.v1','effectiveness',2,rangeTableType(rangeType(0,30,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('instant.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_exo_protein_wafers.iff
				elsif template_id = -1337924041 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_exo_protein_wafers');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,30,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('delayed.dur') then
							ModifyObjVar('delayed.dur','duration',2,rangeTableType(rangeType(0,40,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_gruuvan_shaal.iff
				elsif template_id = 1353161551 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_gruuvan_shaal');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_havla.iff
				elsif template_id = -2137759113 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_havla');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,100,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('delayed.dur') then
							ModifyObjVar('delayed.dur','duration',2,rangeTableType(rangeType(0,60,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_kanali_wafers.iff
				elsif template_id = 179201581 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_kanali_wafers');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_karkan_ribenes.iff
				elsif template_id = -322799884 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_karkan_ribenes');
						if slot_map.exists('instant.v1') then
							ModifyObjVar('instant.v1','effectiveness',2,rangeTableType(rangeType(0,150,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('instant.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_ormachek.iff
				elsif template_id = 1599302627 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_ormachek');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,10,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('delayed.dur') then
							ModifyObjVar('delayed.dur','duration',2,rangeTableType(rangeType(0,40,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_rakririan_burnout_sauce.iff
				elsif template_id = -1655466225 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_rakririan_burnout_sauce');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_rations.iff
				elsif template_id = 993643963 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_rations');
						if slot_map.exists('instant.v1') then
							ModifyObjVar('instant.v1','effectiveness',2,rangeTableType(rangeType(0,50,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('instant.v3') then
							ModifyObjVar('instant.v3','duration',2,rangeTableType(rangeType(0,50,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('instant.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_scrimpi.iff
				elsif template_id = 428493251 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_scrimpi');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_synthsteak.iff
				elsif template_id = 1834531933 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_synthsteak');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,50,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('delayed.dur') then
							ModifyObjVar('delayed.dur','duration',2,rangeTableType(rangeType(0,80,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_teltier_noodles.iff
				elsif template_id = 1664157751 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_teltier_noodles');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_terratta.iff
				elsif template_id = -1989431522 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_terratta');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_thakitillo.iff
				elsif template_id = -237557333 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_thakitillo');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,80,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,1800,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_travel_biscuits.iff
				elsif template_id = -400228634 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_travel_biscuits');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_trimpian.iff
				elsif template_id = 1550199637 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_trimpian');
						if slot_map.exists('duration.eff') then
							ModifyObjVar('duration.eff','effectiveness',2,rangeTableType(rangeType(0,40,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('duration.dur') then
							ModifyObjVar('duration.dur','duration',2,rangeTableType(rangeType(0,4000,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('duration.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_vegeparsine.iff
				elsif template_id = -1129312673 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_vegeparsine');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_veghash.iff
				elsif template_id = 1602372839 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_veghash');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,30,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,1800,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_vercupti_of_agazza_boleruuee.iff
				elsif template_id = 2036727535 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_vercupti_of_agazza_boleruuee');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_accarragm.iff
				elsif template_id = -2066158902 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_accarragm');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_aitha.iff
				elsif template_id = 1706352715 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_aitha');
						if slot_map.exists('mind_heal') then
							ModifyObjVar('mind_heal','effectiveness',2,rangeTableType(rangeType(0,300,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_bespin_port.iff
				elsif template_id = -821627577 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_bespin_port');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,10,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_blue_milk.iff
				elsif template_id = 1731904862 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_blue_milk');
						if slot_map.exists('mind_heal') then
							ModifyObjVar('mind_heal','effectiveness',2,rangeTableType(rangeType(0,800,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_breath_of_heaven.iff
				elsif template_id = 1492257317 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_breath_of_heaven');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_caf.iff
				elsif template_id = 1543376981 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_caf');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_corellian_ale.iff
				elsif template_id = -453063519 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_corellian_ale');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_corellian_brandy.iff
				elsif template_id = -1135348098 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_corellian_brandy');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_deuterium_pyro.iff
				elsif template_id = -180524642 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_deuterium_pyro');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_durindfire.iff
				elsif template_id = -1234286053 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_durindfire');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_elshandruu_pica_thundercloud.iff
				elsif template_id = -966588406 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_elshandruu_pica_thundercloud');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_flameout.iff
				elsif template_id = 391653777 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_flameout');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,180,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('delayed.dur') then
							ModifyObjVar('delayed.dur','duration',2,rangeTableType(rangeType(0,10,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_garrmorl.iff
				elsif template_id = 1833668435 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_garrmorl');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_gralinyn_juice.iff
				elsif template_id = -1632342596 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_gralinyn_juice');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_ithorian_mist.iff
				elsif template_id = 1468700561 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_ithorian_mist');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_jaar.iff
				elsif template_id = 1186404816 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_jaar');
						if slot_map.exists('instant.v1') then
							ModifyObjVar('instant.v1','effectiveness',2,rangeTableType(rangeType(0,50,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('instant.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_jawa_beer.iff
				elsif template_id = -1236344303 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_jawa_beer');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_mandalorian_wine.iff
				elsif template_id = 870515892 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_mandalorian_wine');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_ruby_bliel.iff
				elsif template_id = -609603936 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_ruby_bliel');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,60,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('delayed.dur') then
							ModifyObjVar('delayed.dur','duration',2,rangeTableType(rangeType(0,40,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_spiced_tea.iff
				elsif template_id = -108612773 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_spiced_tea');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_starshine_surprise.iff
				elsif template_id = -723081254 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_starshine_surprise');
						if slot_map.exists('delayed.eff') then
							ModifyObjVar('delayed.eff','effectiveness',2,rangeTableType(rangeType(0,60,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('delayed.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_tilla_tiil.iff
				elsif template_id = -1482529718 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_tilla_tiil');
						if slot_map.exists('instant.v1') then
							ModifyObjVar('instant.v1','effectiveness',2,rangeTableType(rangeType(0,30,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('instant.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_tssolok.iff
				elsif template_id = -702596630 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_tssolok');
						if slot_map.exists('duration.eff') then
							ModifyObjVar('duration.eff','effectiveness',2,rangeTableType(rangeType(0,40,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('duration.dur') then
							ModifyObjVar('duration.dur','duration',2,rangeTableType(rangeType(0,1200,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('duration.type');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_vasarian_brandy.iff
				elsif template_id = -191532605 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_vasarian_brandy');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_vayerbok.iff
				elsif template_id = 1368313349 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_vayerbok');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_veronian_berry_wine.iff
				elsif template_id = -1149904183 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_veronian_berry_wine');
						if slot_map.exists('skill_mod.amount') then
							ModifyObjVar('skill_mod.amount','effectiveness',2,rangeTableType(rangeType(0,20,0.0,2.0)));
						else
							AddObjVar('effectiveness',2,'0.98');
						end if;
						if slot_map.exists('skill_mod.dur') then
							ModifyObjVar('skill_mod.dur','duration',2,rangeTableType(rangeType(0,2400,0.0,2.0)));
						else
							AddObjVar('duration',2,'0.9');
						end if;
						DeleteObjVar('skill_mod.name');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/bread_loaf_full_s1.iff
				elsif template_id = -104858180 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'newbie_bread');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_blap_biscuit.iff
				elsif template_id = 1901385280 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_blap_biscuit');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_corellian_fried_icecream.iff
				elsif template_id = -519120500 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_vagnerian_canape');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_felbar.iff
				elsif template_id = -1030442907 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_felbar');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_glazed_glucose_pate.iff
				elsif template_id = -272947641 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_cavaellin_creams');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_nanana_twist.iff
				elsif template_id = 1239922273 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_parwan_nutricake');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_pastebread.iff
				elsif template_id = 920119228 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_pastebread');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_puffcake.iff
				elsif template_id = 1460469297 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_won_won');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_ryshcate.iff
				elsif template_id = 1098635748 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_kiwik_clusjo_swirl');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_sweesonberry_rolls.iff
				elsif template_id = 1461830070 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_sweesonberry_rolls');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dessert_wedding_cake.iff
				elsif template_id = -1864503130 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dessert_pikatta_pie');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_braised_canron.iff
				elsif template_id = 1801195805 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_rakririan_burnout_sauce');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_fire_stew.iff
				elsif template_id = -410420377 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_trimpian');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_fried_endwa.iff
				elsif template_id = 877344239 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_blood_chowder');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_meatlump.iff
				elsif template_id = 1434285776 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_ahrisa');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_patot_panak.iff
				elsif template_id = 558263029 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_gruuvan_shaal');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_puk.iff
				elsif template_id = 22531863 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_karkan_ribenes');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_ramorrean_capanata.iff
				elsif template_id = 1828317055 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_scrimpi');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_stewed_gwouch.iff
				elsif template_id = -1670164093 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_cho_nor_hoola');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_wastril_bread.iff
				elsif template_id = 284923026 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_synthsteak');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/dish_xermaauc.iff
				elsif template_id = -250957874 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_vegeparsine');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_aludium_pu36.iff
				elsif template_id = 1454424858 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_flameout');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_angerian_fishak_surprise.iff
				elsif template_id = 51214754 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'dish_vercupti_of_agazza_boleruuee');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_antakarian_fire_dancer.iff
				elsif template_id = 540146307 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_garrmorl');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_bantha_blaster.iff
				elsif template_id = -635575777 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_corellian_ale');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_charde.iff
				elsif template_id = 1166887647 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_deuterium_pyro');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_cortyg.iff
				elsif template_id = -23601230 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_veronian_berry_wine');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_double_dip_outer_rim_rumdrop.iff
				elsif template_id = -1365202497 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_elshandruu_pica_thundercloud');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_ice_blaster.iff
				elsif template_id = 1714967517 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_vasarian_brandy');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_kylessian_fruit_distillate.iff
				elsif template_id = 99972710 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_breath_of_heaven');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_skannbult_likker.iff
				elsif template_id = -1722035741 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_tilla_tiil');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_sullustan_gin.iff
				elsif template_id = -1746165408 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_mandalorian_wine');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/crafted/drink_tatooine_sunburn.iff
				elsif template_id = 329225748 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'drink_ithorian_mist');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/edible_jar_berries.iff
				elsif template_id = 1301924307 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_berries');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/edible_jar_bugs.iff
				elsif template_id = -846740906 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_bugs');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/edible_jar_fungus.iff
				elsif template_id = 471067493 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_fungus');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/edible_jar_funk.iff
				elsif template_id = -1483553925 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_funk');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/foraged_fruit_s1.iff
				elsif template_id = -1938284398 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_fruit_1');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/foraged_fruit_s2.iff
				elsif template_id = 1466927109 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_fruit_2');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/foraged_fruit_s3.iff
				elsif template_id = 509800328 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_fruit_3');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/foraged_fruit_s4.iff
				elsif template_id = -444480356 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_fruit_4');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/foraged_fruit_s5.iff
				elsif template_id = -1400069359 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_fruit_5');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/foraged_vegetable_s2.iff
				elsif template_id = -589619825 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_vegetable_2');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/foraged_vegetable_s3.iff
				elsif template_id = -1781106174 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_vegetable_3');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/foraged_vegetable_s4.iff
				elsif template_id = 1848987926 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_vegetable_4');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/foraged/foraged_vegetable_s5.iff
				elsif template_id = 657995419 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'foraged_vegetable_5');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.foraged');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/fruit_melon.iff
				elsif template_id = 1931602976 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'newbie_melon');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/meat_kabob.iff
				elsif template_id = -679440489 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'newbie_meat');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
				--object/tangible/food/nectar.iff
				elsif template_id = 2059421636 then
					if isSchematic = TRUE then
						HandleFoodSchematic;
					elsif isFactory = TRUE then
						HandleFoodCrate;
					else
						AddObjVar('buff_name',4,'newbie_nectar');
						AddObjVar('effectiveness',2,'0.98');
						AddObjVar('duration',2,'0.9');
						DeleteObjVar('consumable.mods');
						DeleteObjVar('food_type');
						RemoveScript('item.comestible.crafted');
						AddScript('item.food');
						RenameObjVar('consumable.stomachValues','filling');
					end if;
-- ****** BEGIN MEDICINE CONVERSION RULES ******
				--object/tangible/component/chemistry/quest_stimpack_a.iff
				elsif template_id = -737833670 then
					ConvertToStimpack(1910273415,100,1);
				--object/tangible/medicine/antidote_sm_s1.iff
				elsif template_id = -893992793 then
					ConvertToStimpack(1910273415,100,1);
				--object/tangible/medicine/crafted/crafted_medpack_blinded_a.iff
				elsif template_id = -1107035087 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/crafted/crafted_medpack_blinded_b.iff
				elsif template_id = 1695831206 then
					ConvertToEnhancer(-102559156,'stabilizers',500);
				--object/tangible/medicine/crafted/crafted_medpack_damage_a.iff
				elsif template_id = -1266575809 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted/crafted_medpack_damage_b.iff
				elsif template_id = 1872117416 then
					ConvertToEnhancer(392779336,'bactaToss',300);
				--object/tangible/medicine/crafted/crafted_medpack_damage_c.iff
				elsif template_id = 647715109 then
					ConvertToEnhancer(1389874386,'bactaJab',500);
				--object/tangible/medicine/crafted/crafted_medpack_damage_d.iff
				elsif template_id = -579309007 then
					ConvertToEnhancer(1389874386,'bactaJab',700);
				--object/tangible/medicine/crafted/crafted_medpack_damage_e.iff
				elsif template_id = -1804266052 then
					ConvertToEnhancer(1389874386,'bactaJab',900);
				--object/tangible/medicine/crafted/crafted_medpack_dizzy_a.iff
				elsif template_id = 527556965 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/crafted/crafted_medpack_dizzy_b.iff
				elsif template_id = -999929358 then
					ConvertToEnhancer(-102559156,'stabilizers',500);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_action_a.iff
				elsif template_id = 2005513882 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',200);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_action_b.iff
				elsif template_id = -1398923763 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',400);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_action_c.iff
				elsif template_id = -443319936 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',600);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_action_d.iff
				elsif template_id = 510677652 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',800);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_action_triad_a.iff
				elsif template_id = 1045341716 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',200);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_action_triad_b.iff
				elsif template_id = -447148413 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',400);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_action_triad_c.iff
				elsif template_id = -1403754226 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',600);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_action_triad_d.iff
				elsif template_id = 1471627802 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',800);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_constitution_a.iff
				elsif template_id = 2049606520 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',200);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_constitution_b.iff
				elsif template_id = -1589826577 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',400);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_constitution_c.iff
				elsif template_id = -399487902 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',600);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_constitution_d.iff
				elsif template_id = 332621686 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',800);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_disease_a.iff
				elsif template_id = -106672157 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',200);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_disease_b.iff
				elsif template_id = 582215540 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',400);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_disease_c.iff
				elsif template_id = 1807646969 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',600);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_health_a.iff
				elsif template_id = -231292762 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',200);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_health_b.iff
				elsif template_id = 690055217 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',400);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_health_c.iff
				elsif template_id = 1613500348 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',600);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_health_d.iff
				elsif template_id = -1680923480 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',800);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_health_triad_a.iff
				elsif template_id = -132280259 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',200);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_health_triad_b.iff
				elsif template_id = 587872426 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',400);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_health_triad_c.iff
				elsif template_id = 1778868007 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',600);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_health_triad_d.iff
				elsif template_id = -1847299021 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',800);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_poison_a.iff
				elsif template_id = -220731710 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',200);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_poison_b.iff
				elsif template_id = 700468821 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',400);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_poison_c.iff
				elsif template_id = 1624058328 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',600);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_quickness_a.iff
				elsif template_id = -1634928232 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',200);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_quickness_b.iff
				elsif template_id = 1167806735 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',400);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_quickness_c.iff
				elsif template_id = 211170946 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',600);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_quickness_d.iff
				elsif template_id = -143319658 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',800);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_stamina_a.iff
				elsif template_id = 949453622 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',200);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_stamina_b.iff
				elsif template_id = -478139487 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',400);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_stamina_c.iff
				elsif template_id = -1433580500 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',600);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_stamina_d.iff
				elsif template_id = 1366189880 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',800);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_strength_a.iff
				elsif template_id = 1240046010 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',200);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_strength_b.iff
				elsif template_id = -1828835027 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',400);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_strength_c.iff
				elsif template_id = -604809568 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',600);
				--object/tangible/medicine/crafted/crafted_medpack_enhance_strength_d.iff
				elsif template_id = 537951668 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',800);
				--object/tangible/medicine/crafted/crafted_medpack_fire_blanket.iff
				elsif template_id = 310195605 then
					ConvertToEnhancer(-1598227139,'disinfect',500);
				--object/tangible/medicine/crafted/crafted_medpack_grenade_area_a.iff
				elsif template_id = 672875303 then
					ConvertToEnhancer(1665648240,'bactaSpray',250);
				--object/tangible/medicine/crafted/crafted_medpack_grenade_area_b.iff
				elsif template_id = -217257040 then
					ConvertToEnhancer(1665648240,'bactaSpray',500);
				--object/tangible/medicine/crafted/crafted_medpack_grenade_area_c.iff
				elsif template_id = -1174303683 then
					ConvertToEnhancer(1665648240,'bactaSpray',750);
				--object/tangible/medicine/crafted/crafted_medpack_grenade_damage_a.iff
				elsif template_id = -2011612965 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted/crafted_medpack_grenade_damage_b.iff
				elsif template_id = 1393463372 then
					ConvertToEnhancer(392779336,'bactaToss',300);
				--object/tangible/medicine/crafted/crafted_medpack_grenade_damage_c.iff
				elsif template_id = 436467649 then
					ConvertToEnhancer(392779336,'bactaToss',500);
				--object/tangible/medicine/crafted/crafted_medpack_grenade_damage_d.iff
				elsif template_id = -505357099 then
					ConvertToEnhancer(392779336,'bactaToss',700);
				--object/tangible/medicine/crafted/crafted_medpack_grenade_damage_e.iff
				elsif template_id = -1460815016 then
					ConvertToEnhancer(392779336,'bactaToss',900);
				--object/tangible/medicine/crafted/crafted_medpack_intimidated_a.iff
				elsif template_id = -97283480 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/crafted/crafted_medpack_intimidated_b.iff
				elsif template_id = 556022527 then
					ConvertToEnhancer(-102559156,'stabilizers',500);
				--object/tangible/medicine/crafted/crafted_medpack_stunned_a.iff
				elsif template_id = 1415748921 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/crafted/crafted_medpack_stunned_b.iff
				elsif template_id = -1888143954 then
					ConvertToEnhancer(-102559156,'stabilizers',500);
				--object/tangible/medicine/crafted/crafted_medpack_wound_action_a.iff
				elsif template_id = -1676060831 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted/crafted_medpack_wound_action_b.iff
				elsif template_id = 1192165366 then
					ConvertToEnhancer(392779336,'bactaToss',300);
				--object/tangible/medicine/crafted/crafted_medpack_wound_action_c.iff
				elsif template_id = 235116667 then
					ConvertToEnhancer(1389874386,'bactaJab',500);
				--object/tangible/medicine/crafted/crafted_medpack_wound_action_d.iff
				elsif template_id = -169823377 then
					ConvertToEnhancer(1389874386,'bactaJab',700);
				--object/tangible/medicine/crafted/crafted_medpack_wound_action_e.iff
				elsif template_id = -1125264158 then
					ConvertToEnhancer(1389874386,'bactaJab',900);
				--object/tangible/medicine/crafted/crafted_medpack_wound_constitution_a.iff
				elsif template_id = -328757929 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted/crafted_medpack_wound_constitution_b.iff
				elsif template_id = 930096576 then
					ConvertToEnhancer(392779336,'bactaToss',300);
				--object/tangible/medicine/crafted/crafted_medpack_wound_constitution_c.iff
				elsif template_id = 2122138189 then
					ConvertToEnhancer(1389874386,'bactaJab',500);
				--object/tangible/medicine/crafted/crafted_medpack_wound_constitution_d.iff
				elsif template_id = -2053215911 then
					ConvertToEnhancer(1389874386,'bactaJab',700);
				--object/tangible/medicine/crafted/crafted_medpack_wound_constitution_e.iff
				elsif template_id = -862777644 then
					ConvertToEnhancer(1389874386,'bactaJab',900);
				--object/tangible/medicine/crafted/crafted_medpack_wound_health_a.iff
				elsif template_id = 430317917 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted/crafted_medpack_wound_health_b.iff
				elsif template_id = -1028552246 then
					ConvertToEnhancer(392779336,'bactaToss',300);
				--object/tangible/medicine/crafted/crafted_medpack_wound_health_c.iff
				elsif template_id = -1950552505 then
					ConvertToEnhancer(1389874386,'bactaJab',500);
				--object/tangible/medicine/crafted/crafted_medpack_wound_health_d.iff
				elsif template_id = 1885324627 then
					ConvertToEnhancer(1389874386,'bactaJab',700);
				--object/tangible/medicine/crafted/crafted_medpack_wound_health_e.iff
				elsif template_id = 961716958 then
					ConvertToEnhancer(1389874386,'bactaJab',900);
				--object/tangible/medicine/crafted/crafted_medpack_wound_quickness_a.iff
				elsif template_id = -1677860148 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted/crafted_medpack_wound_quickness_b.iff
				elsif template_id = 1089096283 then
					ConvertToEnhancer(392779336,'bactaToss',300);
				--object/tangible/medicine/crafted/crafted_medpack_wound_quickness_c.iff
				elsif template_id = 166145494 then
					ConvertToEnhancer(1389874386,'bactaJab',500);
				--object/tangible/medicine/crafted/crafted_medpack_wound_quickness_d.iff
				elsif template_id = -234617150 then
					ConvertToEnhancer(1389874386,'bactaJab',700);
				--object/tangible/medicine/crafted/crafted_medpack_wound_quickness_e.iff
				elsif template_id = -1157013169 then
					ConvertToEnhancer(1389874386,'bactaJab',900);
				--object/tangible/medicine/crafted/crafted_medpack_wound_stamina_a.iff
				elsif template_id = 1184142638 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted/crafted_medpack_wound_stamina_b.iff
				elsif template_id = -1652343367 then
					ConvertToEnhancer(392779336,'bactaToss',300);
				--object/tangible/medicine/crafted/crafted_medpack_wound_stamina_c.iff
				elsif template_id = -728868300 then
					ConvertToEnhancer(1389874386,'bactaJab',500);
				--object/tangible/medicine/crafted/crafted_medpack_wound_stamina_d.iff
				elsif template_id = 795701536 then
					ConvertToEnhancer(1389874386,'bactaJab',700);
				--object/tangible/medicine/crafted/crafted_medpack_wound_stamina_e.iff
				elsif template_id = 1717573293 then
					ConvertToEnhancer(1389874386,'bactaJab',900);
				--object/tangible/medicine/crafted/crafted_medpack_wound_strength_a.iff
				elsif template_id = -1765458857 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted/crafted_medpack_wound_strength_b.iff
				elsif template_id = 1305646272 then
					ConvertToEnhancer(392779336,'bactaToss',300);
				--object/tangible/medicine/crafted/crafted_medpack_wound_strength_c.iff
				elsif template_id = 81785677 then
					ConvertToEnhancer(1389874386,'bactaJab',500);
				--object/tangible/medicine/crafted/crafted_medpack_wound_strength_d.iff
				elsif template_id = -12789671 then
					ConvertToEnhancer(1389874386,'bactaJab',700);
				--object/tangible/medicine/crafted/crafted_medpack_wound_strength_e.iff
				elsif template_id = -1238253612 then
					ConvertToEnhancer(1389874386,'bactaJab',900);
				--object/tangible/medicine/crafted/crafted_stimpack_sm_s1_a.iff
				elsif template_id = -1657672270 then
					ConvertToStimpack(1910273415,100,1);
				--object/tangible/medicine/crafted/crafted_stimpack_sm_s1_b.iff
				elsif template_id = 1176913189 then
					ConvertToStimpack(-1429480176,188,15);
				--object/tangible/medicine/crafted/crafted_stimpack_sm_s1_c.iff
				elsif template_id = 254485160 then
					ConvertToStimpack(-473515363,250,35);
				--object/tangible/medicine/crafted/crafted_stimpack_sm_s1_d.iff
				elsif template_id = -188216900 then
					ConvertToStimpack(405115273,425,60);
				--object/tangible/medicine/crafted/crafted_stimpack_sm_s1_e.iff
				elsif template_id = -1111134671 then
					ConvertToStimpack(405115273,600,60);
				--object/tangible/medicine/crafted/medpack_cure_disease_a.iff
				elsif template_id = 157634799 then
					ConvertToEnhancer(-1598227139,'disinfect',250);
				--object/tangible/medicine/crafted/medpack_cure_disease_area_a.iff
				elsif template_id = -1983000237 then
					ConvertToEnhancer(-1598227139,'disinfect',250);
				--object/tangible/medicine/crafted/medpack_cure_disease_area_b.iff
				elsif template_id = 1390049732 then
					ConvertToEnhancer(-1598227139,'disinfect',500);
				--object/tangible/medicine/crafted/medpack_cure_disease_area_c.iff
				elsif template_id = 467082825 then
					ConvertToEnhancer(-1598227139,'disinfect',750);
				--object/tangible/medicine/crafted/medpack_cure_disease_b.iff
				elsif template_id = -764217224 then
					ConvertToEnhancer(-1598227139,'disinfect',500);
				--object/tangible/medicine/crafted/medpack_cure_disease_c.iff
				elsif template_id = -1686134795 then
					ConvertToEnhancer(-1598227139,'disinfect',750);
				--object/tangible/medicine/crafted/medpack_cure_poison_a.iff
				elsif template_id = 95475824 then
					ConvertToEnhancer(-1598227139,'disinfect',250);
				--object/tangible/medicine/crafted/medpack_cure_poison_area_a.iff
				elsif template_id = 90200724 then
					ConvertToEnhancer(-1598227139,'disinfect',250);
				--object/tangible/medicine/crafted/medpack_cure_poison_area_b.iff
				elsif template_id = -562564605 then
					ConvertToEnhancer(-1598227139,'disinfect',500);
				--object/tangible/medicine/crafted/medpack_cure_poison_area_c.iff
				elsif template_id = -1753574002 then
					ConvertToEnhancer(-1598227139,'disinfect',750);
				--object/tangible/medicine/crafted/medpack_cure_poison_b.iff
				elsif template_id = -559451929 then
					ConvertToEnhancer(-1598227139,'disinfect',500);
				--object/tangible/medicine/crafted/medpack_cure_poison_c.iff
				elsif template_id = -1750460566 then
					ConvertToEnhancer(-1598227139,'disinfect',750);
				--object/tangible/medicine/crafted/medpack_disease_action_a.iff
				elsif template_id = 1506389102 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',250);
				--object/tangible/medicine/crafted/medpack_disease_action_b.iff
				elsif template_id = -2099378951 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',500);
				--object/tangible/medicine/crafted/medpack_disease_action_c.iff
				elsif template_id = -875338892 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',750);
				--object/tangible/medicine/crafted/medpack_disease_area_action_a.iff
				elsif template_id = 53231194 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',250);
				--object/tangible/medicine/crafted/medpack_disease_area_action_b.iff
				elsif template_id = -667183411 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',500);
				--object/tangible/medicine/crafted/medpack_disease_area_action_c.iff
				elsif template_id = -1858670272 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',750);
				--object/tangible/medicine/crafted/medpack_disease_area_constitution_a.iff
				elsif template_id = -1166687602 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',250);
				--object/tangible/medicine/crafted/medpack_disease_area_constitution_b.iff
				elsif template_id = 1633839641 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',500);
				--object/tangible/medicine/crafted/medpack_disease_area_constitution_c.iff
				elsif template_id = 678366612 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',750);
				--object/tangible/medicine/crafted/medpack_disease_area_focus_a.iff
				elsif template_id = -1206446259 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',250);
				--object/tangible/medicine/crafted/medpack_disease_area_focus_b.iff
				elsif template_id = 1660993498 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',500);
				--object/tangible/medicine/crafted/medpack_disease_area_focus_c.iff
				elsif template_id = 705550423 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',750);
				--object/tangible/medicine/crafted/medpack_disease_area_health_a.iff
				elsif template_id = -2037168026 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',250);
				--object/tangible/medicine/crafted/medpack_disease_area_health_b.iff
				elsif template_id = 1568989425 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',500);
				--object/tangible/medicine/crafted/medpack_disease_area_health_c.iff
				elsif template_id = 344557436 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',750);
				--object/tangible/medicine/crafted/medpack_disease_area_mind_a.iff
				elsif template_id = 810684832 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',250);
				--object/tangible/medicine/crafted/medpack_disease_area_mind_b.iff
				elsif template_id = -347752137 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',500);
				--object/tangible/medicine/crafted/medpack_disease_area_mind_c.iff
				elsif template_id = -1572282694 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',750);
				--object/tangible/medicine/crafted/medpack_disease_area_quickness_a.iff
				elsif template_id = 1934373193 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',250);
				--object/tangible/medicine/crafted/medpack_disease_area_quickness_b.iff
				elsif template_id = -1470391842 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',500);
				--object/tangible/medicine/crafted/medpack_disease_area_quickness_c.iff
				elsif template_id = -514392493 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',750);
				--object/tangible/medicine/crafted/medpack_disease_area_stamina_a.iff
				elsif template_id = 1856972813 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',250);
				--object/tangible/medicine/crafted/medpack_disease_area_stamina_b.iff
				elsif template_id = -1246199654 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',500);
				--object/tangible/medicine/crafted/medpack_disease_area_stamina_c.iff
				elsif template_id = -55187689 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',750);
				--object/tangible/medicine/crafted/medpack_disease_area_strength_a.iff
				elsif template_id = 324224015 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',250);
				--object/tangible/medicine/crafted/medpack_disease_area_strength_b.iff
				elsif template_id = -935007080 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',500);
				--object/tangible/medicine/crafted/medpack_disease_area_strength_c.iff
				elsif template_id = -2125886699 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',750);
				--object/tangible/medicine/crafted/medpack_disease_area_willpower_a.iff
				elsif template_id = -1564805563 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',250);
				--object/tangible/medicine/crafted/medpack_disease_area_willpower_b.iff
				elsif template_id = 2041405138 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',500);
				--object/tangible/medicine/crafted/medpack_disease_area_willpower_c.iff
				elsif template_id = 815806815 then
					ConvertToEnhancer(-1974700596,'thyroidRupture',750);
				--object/tangible/medicine/crafted/medpack_disease_constitution_a.iff
				elsif template_id = -1636411245 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',250);
				--object/tangible/medicine/crafted/medpack_disease_constitution_b.iff
				elsif template_id = 1164046340 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',500);
				--object/tangible/medicine/crafted/medpack_disease_constitution_c.iff
				elsif template_id = 208443273 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',750);
				--object/tangible/medicine/crafted/medpack_disease_focus_a.iff
				elsif template_id = -979596476 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',250);
				--object/tangible/medicine/crafted/medpack_disease_focus_b.iff
				elsif template_id = 512435155 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',500);
				--object/tangible/medicine/crafted/medpack_disease_focus_c.iff
				elsif template_id = 1468417118 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',750);
				--object/tangible/medicine/crafted/medpack_disease_health_a.iff
				elsif template_id = -596190638 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',250);
				--object/tangible/medicine/crafted/medpack_disease_health_b.iff
				elsif template_id = 123826885 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',500);
				--object/tangible/medicine/crafted/medpack_disease_health_c.iff
				elsif template_id = 1315706184 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',750);
				--object/tangible/medicine/crafted/medpack_disease_mind_a.iff
				elsif template_id = -1062853951 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',250);
				--object/tangible/medicine/crafted/medpack_disease_mind_b.iff
				elsif template_id = 464620118 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',500);
				--object/tangible/medicine/crafted/medpack_disease_mind_c.iff
				elsif template_id = 1388113371 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',750);
				--object/tangible/medicine/crafted/medpack_disease_quickness_a.iff
				elsif template_id = 2035451832 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',250);
				--object/tangible/medicine/crafted/medpack_disease_quickness_b.iff
				elsif template_id = -1572524241 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',500);
				--object/tangible/medicine/crafted/medpack_disease_quickness_c.iff
				elsif template_id = -347584350 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',750);
				--object/tangible/medicine/crafted/medpack_disease_stamina_a.iff
				elsif template_id = -601857572 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',250);
				--object/tangible/medicine/crafted/medpack_disease_stamina_b.iff
				elsif template_id = 121096523 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',500);
				--object/tangible/medicine/crafted/medpack_disease_stamina_c.iff
				elsif template_id = 1312468678 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',750);
				--object/tangible/medicine/crafted/medpack_disease_strength_a.iff
				elsif template_id = 681976607 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',250);
				--object/tangible/medicine/crafted/medpack_disease_strength_b.iff
				elsif template_id = -206468216 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',500);
				--object/tangible/medicine/crafted/medpack_disease_strength_c.iff
				elsif template_id = -1162023931 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',750);
				--object/tangible/medicine/crafted/medpack_disease_willpower_a.iff
				elsif template_id = -1465630540 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',250);
				--object/tangible/medicine/crafted/medpack_disease_willpower_b.iff
				elsif template_id = 1941170211 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',500);
				--object/tangible/medicine/crafted/medpack_disease_willpower_c.iff
				elsif template_id = 985567150 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',750);
				--object/tangible/medicine/crafted/medpack_poison_action_a.iff
				elsif template_id = -1647095649 then
					ConvertToEnhancer(-273618931,'neurotoxin',250);
				--object/tangible/medicine/crafted/medpack_poison_action_b.iff
				elsif template_id = 1187313672 then
					ConvertToEnhancer(-273618931,'neurotoxin',500);
				--object/tangible/medicine/crafted/medpack_poison_action_c.iff
				elsif template_id = 264871813 then
					ConvertToEnhancer(-273618931,'neurotoxin',750);
				--object/tangible/medicine/crafted/medpack_poison_area_action_a.iff
				elsif template_id = 235278722 then
					ConvertToEnhancer(-1930462643,'traumatize',250);
				--object/tangible/medicine/crafted/medpack_poison_area_action_b.iff
				elsif template_id = -720258795 then
					ConvertToEnhancer(-1930462643,'traumatize',500);
				--object/tangible/medicine/crafted/medpack_poison_area_action_c.iff
				elsif template_id = -1675829608 then
					ConvertToEnhancer(-1930462643,'traumatize',750);
				--object/tangible/medicine/crafted/medpack_poison_area_health_a.iff
				elsif template_id = -1950783554 then
					ConvertToEnhancer(-1930462643,'traumatize',250);
				--object/tangible/medicine/crafted/medpack_poison_area_health_b.iff
				elsif template_id = 1353633577 then
					ConvertToEnhancer(-1930462643,'traumatize',500);
				--object/tangible/medicine/crafted/medpack_poison_area_health_c.iff
				elsif template_id = 430155940 then
					ConvertToEnhancer(-1930462643,'traumatize',750);
				--object/tangible/medicine/crafted/medpack_poison_area_mind_a.iff
				elsif template_id = -636296737 then
					ConvertToEnhancer(-1930462643,'traumatize',250);
				--object/tangible/medicine/crafted/medpack_poison_area_mind_b.iff
				elsif template_id = 17123656 then
					ConvertToEnhancer(-1930462643,'traumatize',500);
				--object/tangible/medicine/crafted/medpack_poison_area_mind_c.iff
				elsif template_id = 1208495813 then
					ConvertToEnhancer(-1930462643,'traumatize',750);
				--object/tangible/medicine/crafted/medpack_poison_health_a.iff
				elsif template_id = 409740963 then
					ConvertToEnhancer(-273618931,'neurotoxin',250);
				--object/tangible/medicine/crafted/medpack_poison_health_b.iff
				elsif template_id = -1015312844 then
					ConvertToEnhancer(-273618931,'neurotoxin',500);
				--object/tangible/medicine/crafted/medpack_poison_health_c.iff
				elsif template_id = -1971918407 then
					ConvertToEnhancer(-273618931,'neurotoxin',750);
				--object/tangible/medicine/crafted/medpack_poison_mind_a.iff
				elsif template_id = -1085939198 then
					ConvertToEnhancer(-273618931,'neurotoxin',250);
				--object/tangible/medicine/crafted/medpack_poison_mind_b.iff
				elsif template_id = 1683114645 then
					ConvertToEnhancer(-273618931,'neurotoxin',500);
				--object/tangible/medicine/crafted/medpack_poison_mind_c.iff
				elsif template_id = 761212184 then
					ConvertToEnhancer(-273618931,'neurotoxin',750);
				--object/tangible/medicine/crafted/medpack_revive.iff
				elsif template_id = 794688942 then
					ConvertToStimpack(1910273415,100,1);
				--object/tangible/medicine/crafted_medpack_blinded.iff
				elsif template_id = -2053523137 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/crafted_medpack_dizzy.iff
				elsif template_id = -1092500879 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/crafted_medpack_enhance_action.iff
				elsif template_id = -1730770782 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',200);
				--object/tangible/medicine/crafted_medpack_enhance_constitution.iff
				elsif template_id = 208928229 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',200);
				--object/tangible/medicine/crafted_medpack_enhance_health.iff
				elsif template_id = 807772545 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',200);
				--object/tangible/medicine/crafted_medpack_enhance_quickness.iff
				elsif template_id = -596014267 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',200);
				--object/tangible/medicine/crafted_medpack_enhance_stamina.iff
				elsif template_id = -1106535119 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',200);
				--object/tangible/medicine/crafted_medpack_enhance_strength.iff
				elsif template_id = -1599002060 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',200);
				--object/tangible/medicine/crafted_medpack_grenade_area.iff
				elsif template_id = -1680302670 then
					ConvertToEnhancer(1665648240,'bactaSpray',250);
				--object/tangible/medicine/crafted_medpack_grenade_damage.iff
				elsif template_id = 144304422 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted_medpack_intimidated.iff
				elsif template_id = 1089638525 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/crafted_medpack_stunned.iff
				elsif template_id = 1491099597 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/crafted_medpack_wound_action.iff
				elsif template_id = 1896411640 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted_medpack_wound_constitution.iff
				elsif template_id = 656430760 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted_medpack_wound_health.iff
				elsif template_id = -637806373 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted_medpack_wound_quickness.iff
				elsif template_id = 1450479489 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted_medpack_wound_stamina.iff
				elsif template_id = -2028859065 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/crafted_medpack_wound_strength.iff
				elsif template_id = 1338425771 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/medikit_tool_advanced.iff
				elsif template_id = -508865592 then
					ConvertToStimpack(-1429480176,188,15);
				--object/tangible/medicine/medikit_tool_basic.iff
				elsif template_id = 186405659 then
					ConvertToStimpack(1910273415,100,1);
				--object/tangible/medicine/medpack_blinded.iff
				elsif template_id = 1367605438 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/medpack_cure_disease.iff
				elsif template_id = 766575814 then
					ConvertToEnhancer(-1598227139,'disinfect',250);
				--object/tangible/medicine/medpack_cure_poison.iff
				elsif template_id = -1695066256 then
					ConvertToEnhancer(-1598227139,'disinfect',250);
				--object/tangible/medicine/medpack_damage.iff
				elsif template_id = -1026091453 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/medpack_disease_health.iff
				elsif template_id = -2032640577 then
					ConvertToEnhancer(-1636338954,'deuteriumToss',250);
				--object/tangible/medicine/medpack_dizzy.iff
				elsif template_id = 1362576287 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/medpack_enhance_action.iff
				elsif template_id = 430277184 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',200);
				--object/tangible/medicine/medpack_enhance_constitution.iff
				elsif template_id = 148075865 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',200);
				--object/tangible/medicine/medpack_enhance_health.iff
				elsif template_id = -1319720093 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',200);
				--object/tangible/medicine/medpack_enhance_quickness.iff
				elsif template_id = -14537666 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',200);
				--object/tangible/medicine/medpack_enhance_stamina.iff
				elsif template_id = -1597210008 then
					ConvertToEnhancer(-1891703777,'endorphineInjection',200);
				--object/tangible/medicine/medpack_enhance_strength.iff
				elsif template_id = 389089358 then
					ConvertToEnhancer(-1453898665,'nutrientInjection',200);
				--object/tangible/medicine/medpack_grenade_area.iff
				elsif template_id = 143113263 then
					ConvertToEnhancer(1665648240,'bactaSpray',250);
				--object/tangible/medicine/medpack_grenade_damage.iff
				elsif template_id = -1981094972 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/medpack_intimidated.iff
				elsif template_id = 787604538 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/medpack_poison_health.iff
				elsif template_id = 233382897 then
					ConvertToEnhancer(-273618931,'neurotoxin',250);
				--object/tangible/medicine/medpack_sm_s1.iff
				elsif template_id = -820961364 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/medpack_stunned.iff
				elsif template_id = -1929517492 then
					ConvertToEnhancer(-102559156,'stabilizers',250);
				--object/tangible/medicine/medpack_wound.iff
				elsif template_id = 1825046689 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/medpack_wound_action.iff
				elsif template_id = -497561499 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/medpack_wound_constitution.iff
				elsif template_id = -370688623 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/medpack_wound_health.iff
				elsif template_id = 1252325702 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/medpack_wound_quickness.iff
				elsif template_id = 1219727576 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/medpack_wound_stamina.iff
				elsif template_id = -261254912 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/medpack_wound_strength.iff
				elsif template_id = -826946743 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/newbie_medpack_damage.iff
				elsif template_id = -307530442 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/newbie_medpack_wound_action.iff
				elsif template_id = -126968979 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/newbie_medpack_wound_health.iff
				elsif template_id = 1352512078 then
					ConvertToEnhancer(392779336,'bactaToss',100);
				--object/tangible/medicine/pet/pet_stimpack_a.iff
				elsif template_id = 38436739 then
					ConvertToPetStimpack(1955421157,100);
				--object/tangible/medicine/pet/pet_stimpack_b.iff
				elsif template_id = -648161516 then
					ConvertToPetStimpack(-1348799630,225);
				--object/tangible/medicine/pet/pet_stimpack_c.iff
				elsif template_id = -1873760103 then
					ConvertToPetStimpack(-426273537,475);
				--object/tangible/medicine/pet/pet_stimpack_d.iff
				elsif template_id = 1806932877 then
					ConvertToPetStimpack(494190571,700);
				--object/tangible/medicine/stimpack_sm_s1.iff
				elsif template_id = -2043897502 then
					ConvertToStimpack(1910273415,100,1);
-- ****** BEGIN SPICE CONVERSION RULES ******
				--object/tangible/food/spice/spice_booster_blue.iff
				elsif template_id = 1672986063 then
					ConvertSpice('booster_blue');
				--object/tangible/food/spice/spice_crash_n_burn.iff
				elsif template_id = -201478902 then
					ConvertSpice('crash_n_burn');
				--object/tangible/food/spice/spice_droid_lube.iff
				elsif template_id = -320858653 then
					ConvertSpice('droid_lube');
				--object/tangible/food/spice/spice_giggledust.iff
				elsif template_id = -428819171 then
					ConvertSpice('giggledust');
				--object/tangible/food/spice/spice_grey_gabaki.iff
				elsif template_id = 649015964 then
					ConvertSpice('grey_gabaki');
				--object/tangible/food/spice/spice_gunjack.iff
				elsif template_id = -1588852672 then
					ConvertSpice('gunjack');
				--object/tangible/food/spice/spice_kliknik_boost.iff
				elsif template_id = 1589268411 then
					ConvertSpice('kliknik_boost');
				--object/tangible/food/spice/spice_kwi_boost.iff
				elsif template_id = -2063383877 then
					ConvertSpice('kwi_boost');
				--object/tangible/food/spice/spice_muon_gold.iff
				elsif template_id = -184506838 then
					ConvertSpice('muon_gold');
				--object/tangible/food/spice/spice_neutron_pixey.iff
				elsif template_id = 205254138 then
					ConvertSpice('neutron_pixey');
				--object/tangible/food/spice/spice_pyrepenol.iff
				elsif template_id = 1936734520 then
					ConvertSpice('pyrepenol');
				--object/tangible/food/spice/spice_scramjet.iff
				elsif template_id = 1577598037 then
					ConvertSpice('scramjet');
				--object/tangible/food/spice/spice_sedative_h4b.iff
				elsif template_id = 83272939 then
					ConvertSpice('sedative_h4b');
				--object/tangible/food/spice/spice_shadowpaw.iff
				elsif template_id = -383871740 then
					ConvertSpice('shadowpaw');
				--object/tangible/food/spice/spice_sweetblossom.iff
				elsif template_id = -1338608150 then
					ConvertSpice('sweetblossom');
				--object/tangible/food/spice/spice_thruster_head.iff
				elsif template_id = -132717072 then
					ConvertSpice('thruster_head');
				--object/tangible/food/spice/spice_yarrock.iff
				elsif template_id = 375006515 then
					ConvertSpice('yarrock');
				--object/tangible/wearables/necklace/necklace_nyax.iff
				elsif template_id = 1717727484 then
					DeleteObjVar('skillmod.bonus.combat_medic_effectiveness');
					AddObjVar('skillmod.bonus.dot_efficiency',0,'5');
				end if;

				-- Save the changes
				if isObjModified = TRUE then
					begin
						stmt := 'update objects set '
							|| 'objvar_0_name = :o0n, objvar_0_type = :o0t, objvar_0_value = :o0v, '
							|| 'objvar_1_name = :o1n, objvar_1_type = :o1t, objvar_1_value = :o1v, '
							|| 'objvar_2_name = :o2n, objvar_2_type = :o2t, objvar_2_value = :o2v, '
							|| 'objvar_3_name = :o3n, objvar_3_type = :o3t, objvar_3_value = :o3v, '
							|| 'objvar_4_name = :o4n, objvar_4_type = :o4t, objvar_4_value = :o4v, '
							|| 'objvar_5_name = :o5n, objvar_5_type = :o5t, objvar_5_value = :o5v, '
							|| 'objvar_6_name = :o6n, objvar_6_type = :o6t, objvar_6_value = :o6v, '
							|| 'objvar_7_name = :o7n, objvar_7_type = :o7t, objvar_7_value = :o7v, '
							|| 'objvar_8_name = :o8n, objvar_8_type = :o8t, objvar_8_value = :o8v, '
							|| 'objvar_9_name = :o9n, objvar_9_type = :o9t, objvar_9_value = :o9v, '
							|| 'objvar_10_name = :o10n, objvar_10_type = :o10t, objvar_10_value = :o10v, '
							|| 'objvar_11_name = :o11n, objvar_11_type = :o11t, objvar_11_value = :o11v, '
							|| 'objvar_12_name = :o12n, objvar_12_type = :o12t, objvar_12_value = :o12v, '
							|| 'objvar_13_name = :o13n, objvar_13_type = :o13t, objvar_13_value = :o13v, '
							|| 'objvar_14_name = :o14n, objvar_14_type = :o14t, objvar_14_value = :o14v, '
							|| 'objvar_15_name = :o15n, objvar_15_type = :o15t, objvar_15_value = :o15v, '
							|| 'objvar_16_name = :o16n, objvar_16_type = :o16t, objvar_16_value = :o16v, '
							|| 'objvar_17_name = :o17n, objvar_17_type = :o17t, objvar_17_value = :o17v, '
							|| 'objvar_18_name = :o18n, objvar_18_type = :o18t, objvar_18_value = :o18v, '
							|| 'objvar_19_name = :o19n, objvar_19_type = :o19t, objvar_19_value = :o19v, '
							|| 'script_list = :sl, object_template_id = :otid, '
							|| 'object_name = :obn, name_string_table = :nstb, name_string_text = :nstx, '
							|| 'conversion_id = :version '
							|| 'where object_id = :id';

						execute immediate stmt using
							Obj_ObjVars(1).objvar_name, Obj_ObjVars(1).objvar_type, Obj_ObjVars(1).objvar_value,
							Obj_ObjVars(2).objvar_name, Obj_ObjVars(2).objvar_type, Obj_ObjVars(2).objvar_value,
							Obj_ObjVars(3).objvar_name, Obj_ObjVars(3).objvar_type, Obj_ObjVars(3).objvar_value,
							Obj_ObjVars(4).objvar_name, Obj_ObjVars(4).objvar_type, Obj_ObjVars(4).objvar_value,
							Obj_ObjVars(5).objvar_name, Obj_ObjVars(5).objvar_type, Obj_ObjVars(5).objvar_value,
							Obj_ObjVars(6).objvar_name, Obj_ObjVars(6).objvar_type, Obj_ObjVars(6).objvar_value,
							Obj_ObjVars(7).objvar_name, Obj_ObjVars(7).objvar_type, Obj_ObjVars(7).objvar_value,
							Obj_ObjVars(8).objvar_name, Obj_ObjVars(8).objvar_type, Obj_ObjVars(8).objvar_value,
							Obj_ObjVars(9).objvar_name, Obj_ObjVars(9).objvar_type, Obj_ObjVars(9).objvar_value,
							Obj_ObjVars(10).objvar_name, Obj_ObjVars(10).objvar_type, Obj_ObjVars(10).objvar_value,
							Obj_ObjVars(11).objvar_name, Obj_ObjVars(11).objvar_type, Obj_ObjVars(11).objvar_value,
							Obj_ObjVars(12).objvar_name, Obj_ObjVars(12).objvar_type, Obj_ObjVars(12).objvar_value,
							Obj_ObjVars(13).objvar_name, Obj_ObjVars(13).objvar_type, Obj_ObjVars(13).objvar_value,
							Obj_ObjVars(14).objvar_name, Obj_ObjVars(14).objvar_type, Obj_ObjVars(14).objvar_value,
							Obj_ObjVars(15).objvar_name, Obj_ObjVars(15).objvar_type, Obj_ObjVars(15).objvar_value,
							Obj_ObjVars(16).objvar_name, Obj_ObjVars(16).objvar_type, Obj_ObjVars(16).objvar_value,
							Obj_ObjVars(17).objvar_name, Obj_ObjVars(17).objvar_type, Obj_ObjVars(17).objvar_value,
							Obj_ObjVars(18).objvar_name, Obj_ObjVars(18).objvar_type, Obj_ObjVars(18).objvar_value,
							Obj_ObjVars(19).objvar_name, Obj_ObjVars(19).objvar_type, Obj_ObjVars(19).objvar_value,
							Obj_ObjVars(20).objvar_name, Obj_ObjVars(20).objvar_type, Obj_ObjVars(20).objvar_value,
							script_list, new_template_id,
							object_name, name_table, name_text,
							conversion_id,
							object_id;
					exception
						when others then
							err_num := SQLCODE;
							dbms_output.put_line('Error in object update(' || object_id || ') - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
							db_error_logger.dblogerror(err_num, 'Error in object update');
					end;
				end if;

				if isWeaponObj = TRUE then
					begin
						stmt := 'update weapon_objects set '
							|| 'min_damage = :mindam, '
							|| 'max_damage = :maxdam, '
							|| 'attack_speed = :spd, '
							|| 'wound_chance = :wnd, '
							|| 'accuracy = :accuracy, '
							|| 'attack_cost = :ac, '
							|| 'min_range = :mindrng, '
							|| 'max_range = :maxrng, '
							|| 'damage_type = :dmgtype, '
							|| 'elemental_type = :eletype, '
							|| 'elemental_value = :elevalue '
							|| 'where object_id = :id';

						execute immediate stmt using
							weapon_attribs.min_damage,
							weapon_attribs.max_damage,
							weapon_attribs.attack_speed,
							weapon_attribs.wound_chance,
							weapon_attribs.accuracy,
							weapon_attribs.attack_cost,
							weapon_attribs.min_range,
							weapon_attribs.max_range,				
							weapon_attribs.damage_type,				
							weapon_attribs.elemental_type,				
							weapon_attribs.elemental_value,				
							object_id;
					exception
						when others then
							err_num := SQLCODE;
							dbms_output.put_line('Error in weapon update(' || object_id || ') - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
							db_error_logger.dblogerror(err_num, 'Error in weapon update');
					end;
				end if;

				i := deleted_objvars.first;
				while i is not null loop
					begin
						stmt := 'delete from object_variables '
							|| 'where object_id = :id and name_id = :oldname';

						execute immediate stmt using
							object_id,
							OV_ObjVars(deleted_objvars(i)).old_objvar_name_id;

						i := deleted_objvars.next(i);
					exception
						when others then
							err_num := SQLCODE;
							dbms_output.put_line('Error (' || SQLERRM(err_num) || ') with deleted_objvars(' || object_id || ') - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
							db_error_logger.dblogerror(err_num, 'Error with deleted_objvars');
							i := deleted_objvars.next(i);
					end;
				end loop;

				i := added_objvars.first;
				while i is not null loop
					begin
						stmt := 'insert into object_variables values '
							|| '(:id, :name, :type, :value, 0)';

						execute immediate stmt using
							object_id,
							OV_ObjVars(added_objvars(i)).objvar_name_id,
							OV_ObjVars(added_objvars(i)).objvar_type,
							OV_ObjVars(added_objvars(i)).objvar_value;

						i := added_objvars.next(i);
					exception
						when others then
							err_num := SQLCODE;
							dbms_output.put_line('Error (' || SQLERRM(err_num) || ') with added_objvars(' || object_id || ') - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
							db_error_logger.dblogerror(err_num, 'Error with added_objvars');
						i := added_objvars.next(i);
					end;
				end loop;

				i := updated_objvars.first;
				while i is not null loop
					begin
						stmt := 'update object_variables set '
							|| 'name_id = :name, type = :type, value = :value '
							|| 'where object_id = :id and name_id = :oldname';

						execute immediate stmt using
							objvarNameIdMap(OV_ObjVars(updated_objvars(i)).objvar_name),
							OV_ObjVars(updated_objvars(i)).objvar_type,
							OV_ObjVars(updated_objvars(i)).objvar_value,
							object_id,
							OV_ObjVars(updated_objvars(i)).old_objvar_name_id;

						i := updated_objvars.next(i);
					exception
						when others then
							err_num := SQLCODE;
							dbms_output.put_line('Error (' || SQLERRM(err_num) || ') with updated_objvars(' || object_id || ') - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
							db_error_logger.dblogerror(err_num, 'Error with updated_objvars');
							i := updated_objvars.next(i);
					end;
				end loop;

				i := deleted_attribs.first;
				while i is not null loop
					begin
						stmt := 'delete from manf_schematic_attributes '
							|| 'where object_id = :id and attribute_type = :attribute';

						execute immediate stmt using
							object_id,
							deleted_attribs(i);

						i := deleted_attribs.next(i);
					exception
						when others then
							err_num := SQLCODE;
							dbms_output.put_line('Error (' || SQLERRM(err_num) || ') with  deleted_attribs(' || object_id || ') - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
							db_error_logger.dblogerror(err_num, 'Error with  deleted_attribs');
							i := deleted_attribs.next(i);
					end;
				end loop;

				i := added_attribs.first;
				while i is not null loop
					begin
						stmt := 'insert into manf_schematic_attributes values '
							|| '(:id, :attrib, :value)';

						execute immediate stmt using
							object_id,
							manf_attributes(added_attribs(i)).attribute,
							manf_attributes(added_attribs(i)).value;

						i := added_attribs.next(i);
					exception
						when others then
							err_num := SQLCODE;
							dbms_output.put_line('Error (' || SQLERRM(err_num) || ') with  added_attribs(' || object_id || ') - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
							db_error_logger.dblogerror(err_num, 'Error with  added_attribs');
						i := added_attribs.next(i);
					end;
				end loop;

				i := updated_attribs.first;
				while i is not null loop
					begin
						stmt := 'update manf_schematic_attributes set '
							|| 'attribute_type = :attribute, value = :value '
							|| 'where object_id = :id and attribute_type = :oldname';

						execute immediate stmt using
							manf_attributes(updated_attribs(i)).attribute,
							manf_attributes(updated_attribs(i)).value,
							object_id,
							updated_attribs(i);

						i := updated_attribs.next(i);
					exception
						when others then
							err_num := SQLCODE;
							dbms_output.put_line('Error (' || SQLERRM(err_num) || ') with  updated_attribs(' || object_id || ') - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
							db_error_logger.dblogerror(err_num, 'Error with  updated_attribs');
							i := updated_attribs.next(i);
					end;
				end loop;
				
				if (isSchematic = TRUE) and (new_schematic_id != 0) then
					begin
						stmt := 'update manf_schematic_objects set '
							|| 'draft_schematic = :new_id '
							|| 'where object_id = :id';

						execute immediate stmt using
							new_schematic_id,
							object_id;
					exception
						when others then
							err_num := SQLCODE;
							dbms_output.put_line('Error (' || SQLERRM(err_num) || ') with  update_schematic(' || object_id || ') - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
							db_error_logger.dblogerror(err_num, 'Error with  update_schematic');
							i := updated_attribs.next(i);
					end;
				end if;

				cvt := cvt + 1;
			end if;
				idx := idx + 1;
				-- commit every 100,000 records
				if idx > 99999 then
					commit;
					if use_rbs is not null then
						execute immediate 'set transaction use rollback segment ' || use_rbs;
					end if;
					dbms_output.put_line('100,000 rows = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
					cnt := cnt + idx;
					idx := 0;
				end if;					
			end;
			fetch objCursor into objRow;
		end loop;
		--commit;
		close attributeCursor;
		close schematicCursor;
		close weaponCursor;
		close objVarCursor;
		close objCursor;
		cnt := cnt + idx;
		dbms_output.put_line('end time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
		dbms_output.put_line('converted = ' || to_char(cvt));
		dbms_output.put_line('total count = ' || to_char(cnt));
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error (' || SQLERRM(err_num) || ') in DoItemConversionBatch (' || object_id || ') - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in DoItemConversionBatch');
	end;

	function InitializeData return boolean
	is
		objvarNameIdRow object_variable_names%rowtype;
  		-- ALL added/updated objvar names MUST be in this list
		cursor objvarNameIdCursor is
		select id, name from object_variable_names
		where name in (
			'armor.armorCategory',
			'armor.armorLevel',
			'armor.armorencumbrance',
			'armor.condition',
			'armor.effectiveness',
			'armor.encumbrance',
			'armor.encumbranceSplit',
			'armor.general_protection',
			'armor.layer0',
			'armor.layer13',
			'armor.layer14',
			'armor.layer15',
			'armor.mandDeconstruct',
			'armor.rating',
			'armor.recharge_rate',
			'armor.risDeconstruct',
			'armor.special.1',
			'armor.special.1024',
			'armor.special.128',
			'armor.special.16',
			'armor.special.2',
			'armor.special.2048',
			'armor.special.256',
			'armor.special.32',
			'armor.special.4',
			'armor.special.4096',
			'armor.special.512',
			'armor.special.64',
			'armor.special.8',
			'armor.vulnerability',
			'armorCategory',
			'armorLevel',
			'biolink.faction_points',
			'blindChance',
			'blindDuration',
			'buff_name',
			'burnDuration',
			'burnIntensity',
			'crafting_attributes.crafting:accuracy',
			'crafting_attributes.crafting:armor_condition',
			'crafting_attributes.crafting:armor_general_protection',
			'crafting_attributes.crafting:armor_layer0',
			'crafting_attributes.crafting:armor_layer1',
			'crafting_attributes.crafting:armor_layer2',
			'crafting_attributes.crafting:armor_layer4',
			'crafting_attributes.crafting:armor_layer5',
			'crafting_attributes.crafting:armor_layer6',
			'crafting_attributes.crafting:armor_layer7',
			'crafting_attributes.crafting:armor_layer12',
			'crafting_attributes.crafting:attackActionCost',
			'crafting_attributes.crafting:attackCost',
			'crafting_attributes.crafting:attackHealthCost',
			'crafting_attributes.crafting:attackMindCost',
			'crafting_attributes.crafting:attackSpeed',
			'crafting_attributes.crafting:condition',
			'crafting_attributes.crafting:damageType',
			'crafting_attributes.crafting:effect',
			'crafting_attributes.crafting:efficiency',
			'crafting_attributes.crafting:elementalType',
			'crafting_attributes.crafting:elementalValue',
			'crafting_attributes.crafting:flavor',
			'crafting_attributes.crafting:general_protection',
			'crafting_attributes.crafting:hitPoints',
			'crafting_attributes.crafting:maxDamage',
			'crafting_attributes.crafting:maxRange',
			'crafting_attributes.crafting:maxRangeMod',
			'crafting_attributes.crafting:midRange',
			'crafting_attributes.crafting:midRangeMod',
			'crafting_attributes.crafting:minDamage',
			'crafting_attributes.crafting:minRange',
			'crafting_attributes.crafting:minRangeMod',
			'crafting_attributes.crafting:nutrition',
			'crafting_attributes.crafting:power',
			'crafting_attributes.crafting:woundChance',
			'crafting_attributes.crafting:zeroRangeMod',
			'crafting_components.accuracy',
			'crafting_components.armor_armorencumbrance',
			'crafting_components.armor_condition',
			'crafting_components.armor_general_protection',
			'crafting_components.armor_layer0',
			'crafting_components.armor_layer1',
			'crafting_components.armor_layer2',
			'crafting_components.armor_layer4',
			'crafting_components.armor_layer5',
			'crafting_components.armor_layer6',
			'crafting_components.armor_layer7',
			'crafting_components.armor_layer12',
			'crafting_components.attackActionCost',
			'crafting_components.attackCost',
			'crafting_components.attackHealthCost',
			'crafting_components.attackMindCost',
			'crafting_components.attackSpeed',
			'crafting_components.attribute.bonus.0',
			'crafting_components.attribute.bonus.1',
			'crafting_components.attribute.bonus.2',
			'crafting_components.attribute.bonus.3',
			'crafting_components.attribute.bonus.4',
			'crafting_components.attribute.bonus.5',
			'crafting_components.attribute.bonus.6',
			'crafting_components.attribute.bonus.7',
			'crafting_components.attribute.bonus.8',
			'crafting_components.condition',
			'crafting_components.damageType',
			'crafting_components.elementalType',
			'crafting_components.elementalValue',
			'crafting_components.general_protection',
			'crafting_components.hitPoints',
			'crafting_components.layer13',
			'crafting_components.layer14',
			'crafting_components.layer15',
			'crafting_components.maxDamage',
			'crafting_components.maxRange',
			'crafting_components.maxRangeMod',
			'crafting_components.midRange',
			'crafting_components.midRangeMod',
			'crafting_components.minDamage',
			'crafting_components.minRange',
			'crafting_components.minRangeMod',
			'crafting_components.recharge_rate',
			'crafting_components.woundChance',
			'crafting_components.zeroRangeMod',
			'draftSchematic',
			'duration',
			'effect_class',
			'effectiveness',
			'filling',
			'ImAnInvalidSchematic',
			'intAOEDamagePercent',
			'intWeaponType',
			'jedi.crystal.stats.accuracy',
			'jedi.crystal.stats.attack_cost',
			'jedi.crystal.stats.damage_type',
			'jedi.crystal.stats.elemental_type',
			'jedi.crystal.stats.elemental_value',
			'jedi.crystal.stats.force',
			'jedi.crystal.stats.max_dmg',
			'jedi.crystal.stats.max_rng',
			'jedi.crystal.stats.min_dmg',
			'jedi.crystal.stats.min_rng',
			'jedi.crystal.stats.speed',
			'jedi.crystal.stats.wound',
			'jedi.saber.base_stats.accuracy',
			'jedi.saber.base_stats.attack_cost',
			'jedi.saber.base_stats.damage_type',
			'jedi.saber.base_stats.elemental_type',
			'jedi.saber.base_stats.elemental_value',
			'jedi.saber.base_stats.force',
			'jedi.saber.base_stats.max_dmg',
			'jedi.saber.base_stats.max_rng',
			'jedi.saber.base_stats.min_dmg',
			'jedi.saber.base_stats.min_rng',
			'jedi.saber.base_stats.speed',
			'jedi.saber.base_stats.wound',
			'jedi.saber.force',
			'healing.power',
			'powerup.accuracy',
			'powerup.actionCost',
			'powerup.damage',
			'powerup.effect',
			'powerup.efficiency',
			'powerup.speed',
			'powerup.usesLeft',
			'powerup.wound',
			'reuseTimer',
			'skillmod.bonus.pistol_speed',
			'slowDuration',
			'slowIntensity',
			'spice.name',
			'strWeaponType',
			'weapon.intNoCertRequired',
			'weaponType'
		);
		maxNameId number;
		
		schematicRow schematic_templates%rowtype;
		cursor schematicCursor is
		select schematic_id, object_template_id
		from schematic_templates;
		
		v varchar2(500);
		stmt varchar2(4000);
	begin
		-- initialize
		free_slots  := numTable(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20);
		Obj_ObjVars := objectObjVarArray();
		Obj_Objvars.extend(20);
		updated_objvars  := varcharTable();
		deleted_objvars := varcharTable();
		added_objvars   := varcharTable();
		updated_attribs  := varcharTable();
		deleted_attribs := varcharTable();
		added_attribs   := varcharTable();
		-- Data Table for relative weapon DoT powers
		dots_matrix('fire')(6) := 12;
		dots_matrix('fire')(0) := 9;
		dots_matrix('fire')(3) := 6;
		dots_matrix('poison')(6) := 11;
		dots_matrix('poison')(0) := 8;
		dots_matrix('poison')(3) := 5;
		dots_matrix('disease')(6) := 10;
		dots_matrix('disease')(0) := 7;
		dots_matrix('disease')(3) := 4;
		dots_matrix('bleeds')(6) := 3;
		dots_matrix('bleeds')(0) := 2;
		dots_matrix('bleeds')(3) := 1;

		objvarNameIdMap('armor.armorCategory') := 0;
		objvarNameIdMap('armor.armorLevel') := 0;
		objvarNameIdMap('armor.armorencumbrance') := 0;
		objvarNameIdMap('armor.condition') := 0;
		objvarNameIdMap('armor.effectiveness') := 0;
		objvarNameIdMap('armor.encumbrance') := 0;
		objvarNameIdMap('armor.encumbranceSplit') := 0;
		objvarNameIdMap('armor.general_protection') := 0;
		objvarNameIdMap('armor.layer0') := 0;
		objvarNameIdMap('armor.layer14') := 0;
		objvarNameIdMap('armor.layer13') := 0;
		objvarNameIdMap('armor.layer15') := 0;
		objvarNameIdMap('armor.mandDeconstruct') := 0;
		objvarNameIdMap('armor.rating') := 0;
		objvarNameIdMap('armor.recharge_rate') := 0;
		objvarNameIdMap('armor.risDeconstruct') := 0;
		objvarNameIdMap('armor.special.1') := 0;
		objvarNameIdMap('armor.special.1024') := 0;
		objvarNameIdMap('armor.special.128') := 0;
		objvarNameIdMap('armor.special.16') := 0;
		objvarNameIdMap('armor.special.2') := 0;
		objvarNameIdMap('armor.special.2048') := 0;
		objvarNameIdMap('armor.special.256') := 0;
		objvarNameIdMap('armor.special.32') := 0;
		objvarNameIdMap('armor.special.4') := 0;
		objvarNameIdMap('armor.special.4096') := 0;
		objvarNameIdMap('armor.special.512') := 0;
		objvarNameIdMap('armor.special.64') := 0;
		objvarNameIdMap('armor.special.8') := 0;
		objvarNameIdMap('armor.vulnerability') := 0;
		objvarNameIdMap('armorCategory') := 0;
		objvarNameIdMap('armorLevel') := 0;
		objvarNameIdMap('biolink.faction_points') := 0;
		objvarNameIdMap('blindChance') := 0;
		objvarNameIdMap('blindDuration') := 0;
		objvarNameIdMap('buff_name') := 0;
		objvarNameIdMap('burnDuration') := 0;
		objvarNameIdMap('burnIntensity') := 0;
		objvarNameIdMap('crafting_attributes.crafting:accuracy') := 0;
		objvarNameIdMap('crafting_attributes.crafting:armor_general_protection') := 0;
		objvarNameIdMap('crafting_attributes.crafting:armor_condition') := 0;
		objvarNameIdMap('crafting_attributes.crafting:armor_layer0') := 0;
		objvarNameIdMap('crafting_attributes.crafting:armor_layer1') := 0;
		objvarNameIdMap('crafting_attributes.crafting:armor_layer2') := 0;
		objvarNameIdMap('crafting_attributes.crafting:armor_layer4') := 0;
		objvarNameIdMap('crafting_attributes.crafting:armor_layer5') := 0;
		objvarNameIdMap('crafting_attributes.crafting:armor_layer6') := 0;
		objvarNameIdMap('crafting_attributes.crafting:armor_layer7') := 0;
		objvarNameIdMap('crafting_attributes.crafting:armor_layer12') := 0;
		objvarNameIdMap('crafting_attributes.crafting:attackActionCost') := 0;
		objvarNameIdMap('crafting_attributes.crafting:attackCost') := 0;
		objvarNameIdMap('crafting_attributes.crafting:attackHealthCost') := 0;
		objvarNameIdMap('crafting_attributes.crafting:attackMindCost') := 0;
		objvarNameIdMap('crafting_attributes.crafting:attackSpeed') := 0;
		objvarNameIdMap('crafting_attributes.crafting:condition') := 0;
		objvarNameIdMap('crafting_attributes.crafting:damageType') := 0;
		objvarNameIdMap('crafting_attributes.crafting:effect') := 0;
		objvarNameIdMap('crafting_attributes.crafting:efficiency') := 0;
		objvarNameIdMap('crafting_attributes.crafting:elementalType') := 0;
		objvarNameIdMap('crafting_attributes.crafting:elementalValue') := 0;
		objvarNameIdMap('crafting_attributes.crafting:flavor') := 0;
		objvarNameIdMap('crafting_attributes.crafting:general_protection') := 0;
		objvarNameIdMap('crafting_attributes.crafting:hitPoints') := 0;
		objvarNameIdMap('crafting_attributes.crafting:maxDamage') := 0;
		objvarNameIdMap('crafting_attributes.crafting:maxRange') := 0;
		objvarNameIdMap('crafting_attributes.crafting:maxRangeMod') := 0;
		objvarNameIdMap('crafting_attributes.crafting:midRange') := 0;
		objvarNameIdMap('crafting_attributes.crafting:midRangeMod') := 0;
		objvarNameIdMap('crafting_attributes.crafting:minDamage') := 0;
		objvarNameIdMap('crafting_attributes.crafting:minRange') := 0;
		objvarNameIdMap('crafting_attributes.crafting:minRangeMod') := 0;
		objvarNameIdMap('crafting_attributes.crafting:nutrition') := 0;
		objvarNameIdMap('crafting_attributes.crafting:power') := 0;
		objvarNameIdMap('crafting_attributes.crafting:woundChance') := 0;
		objvarNameIdMap('crafting_attributes.crafting:zeroRangeMod') := 0;
		objvarNameIdMap('crafting_components.accuracy') := 0;
		objvarNameIdMap('crafting_components.armor_armorencumbrance') := 0;
		objvarNameIdMap('crafting_components.armor_condition') := 0;
		objvarNameIdMap('crafting_components.armor_general_protection') := 0;
		objvarNameIdMap('crafting_components.armor_layer0') := 0;
		objvarNameIdMap('crafting_components.armor_layer1') := 0;
		objvarNameIdMap('crafting_components.armor_layer2') := 0;
		objvarNameIdMap('crafting_components.armor_layer4') := 0;
		objvarNameIdMap('crafting_components.armor_layer5') := 0;
		objvarNameIdMap('crafting_components.armor_layer6') := 0;
		objvarNameIdMap('crafting_components.armor_layer7') := 0;
		objvarNameIdMap('crafting_components.armor_layer12') := 0;
		objvarNameIdMap('crafting_components.attackActionCost') := 0;
		objvarNameIdMap('crafting_components.attackCost') := 0;
		objvarNameIdMap('crafting_components.attackHealthCost') := 0;
		objvarNameIdMap('crafting_components.attackMindCost') := 0;
		objvarNameIdMap('crafting_components.attackSpeed') := 0;
		objvarNameIdMap('crafting_components.attribute.bonus.0') := 0;
		objvarNameIdMap('crafting_components.attribute.bonus.1') := 0;
		objvarNameIdMap('crafting_components.attribute.bonus.2') := 0;
		objvarNameIdMap('crafting_components.attribute.bonus.3') := 0;
		objvarNameIdMap('crafting_components.attribute.bonus.4') := 0;
		objvarNameIdMap('crafting_components.attribute.bonus.5') := 0;
		objvarNameIdMap('crafting_components.attribute.bonus.6') := 0;
		objvarNameIdMap('crafting_components.attribute.bonus.7') := 0;
		objvarNameIdMap('crafting_components.attribute.bonus.8') := 0;
		objvarNameIdMap('crafting_components.condition') := 0;
		objvarNameIdMap('crafting_components.damageType') := 0;
		objvarNameIdMap('crafting_components.elementalType') := 0;
		objvarNameIdMap('crafting_components.elementalValue') := 0;
		objvarNameIdMap('crafting_components.general_protection') := 0;
		objvarNameIdMap('crafting_components.hitPoints') := 0;
		objvarNameIdMap('crafting_components.layer13') := 0;
		objvarNameIdMap('crafting_components.layer14') := 0;
		objvarNameIdMap('crafting_components.layer15') := 0;
		objvarNameIdMap('crafting_components.maxDamage') := 0;
		objvarNameIdMap('crafting_components.maxRange') := 0;
		objvarNameIdMap('crafting_components.maxRangeMod') := 0;
		objvarNameIdMap('crafting_components.midRange') := 0;
		objvarNameIdMap('crafting_components.midRangeMod') := 0;
		objvarNameIdMap('crafting_components.minDamage') := 0;
		objvarNameIdMap('crafting_components.minRange') := 0;
		objvarNameIdMap('crafting_components.minRangeMod') := 0;
		objvarNameIdMap('crafting_components.recharge_rate') := 0;
		objvarNameIdMap('crafting_components.woundChance') := 0;
		objvarNameIdMap('crafting_components.zeroRangeMod') := 0;
		objvarNameIdMap('draftSchematic') := 0;
		objvarNameIdMap('duration') := 0;
		objvarNameIdMap('effect_class') := 0;
		objvarNameIdMap('effectiveness') := 0;
		objvarNameIdMap('filling') := 0;
		objvarNameIdMap('ImAnInvalidSchematic') := 0;
		objvarNameIdMap('intAOEDamagePercent') := 0;
		objvarNameIdMap('intWeaponType') := 0;
		objvarNameIdMap('jedi.crystal.stats.accuracy') := 0;
		objvarNameIdMap('jedi.crystal.stats.attack_cost') := 0;
		objvarNameIdMap('jedi.crystal.stats.damage_type') := 0;
		objvarNameIdMap('jedi.crystal.stats.elemental_type') := 0;
		objvarNameIdMap('jedi.crystal.stats.elemental_value') := 0;
		objvarNameIdMap('jedi.crystal.stats.force') := 0;
		objvarNameIdMap('jedi.crystal.stats.max_dmg') := 0;
		objvarNameIdMap('jedi.crystal.stats.max_rng') := 0;
		objvarNameIdMap('jedi.crystal.stats.min_dmg') := 0;
		objvarNameIdMap('jedi.crystal.stats.min_rng') := 0;
		objvarNameIdMap('jedi.crystal.stats.speed') := 0;
		objvarNameIdMap('jedi.crystal.stats.wound') := 0;
		objvarNameIdMap('jedi.saber.base_stats.accuracy') := 0;
		objvarNameIdMap('jedi.saber.base_stats.attack_cost') := 0;
		objvarNameIdMap('jedi.saber.base_stats.damage_type') := 0;
		objvarNameIdMap('jedi.saber.base_stats.elemental_type') := 0;
		objvarNameIdMap('jedi.saber.base_stats.elemental_value') := 0;
		objvarNameIdMap('jedi.saber.base_stats.force') := 0;
		objvarNameIdMap('jedi.saber.base_stats.max_dmg') := 0;
		objvarNameIdMap('jedi.saber.base_stats.max_rng') := 0;
		objvarNameIdMap('jedi.saber.base_stats.min_dmg') := 0;
		objvarNameIdMap('jedi.saber.base_stats.min_rng') := 0;
		objvarNameIdMap('jedi.saber.base_stats.speed') := 0;
		objvarNameIdMap('jedi.saber.base_stats.wound') := 0;
		objvarNameIdMap('jedi.saber.force') := 0;
		objvarNameIdMap('healing.power') := 0;
		objvarNameIdMap('powerup.accuracy') := 0;
		objvarNameIdMap('powerup.actionCost') := 0;
		objvarNameIdMap('powerup.damage') := 0;
		objvarNameIdMap('powerup.effect') := 0;
		objvarNameIdMap('powerup.efficiency') := 0;
		objvarNameIdMap('powerup.speed') := 0;
		objvarNameIdMap('powerup.usesLeft') := 0;
		objvarNameIdMap('powerup.wound') := 0;
		objvarNameIdMap('reuseTimer') := 0;
		objvarNameIdMap('skillmod.bonus.pistol_speed') := 0;
		objvarNameIdMap('slowDuration') := 0;
		objvarNameIdMap('slowIntensity') := 0;
		objvarNameIdMap('spice.name') := 0;
		objvarNameIdMap('strWeaponType') := 0;
		objvarNameIdMap('weapon.intNoCertRequired') := 0;
		objvarNameIdMap('weaponType') := 0;

		open objvarNameIdCursor;
		fetch objvarNameIdCursor into objvarNameIdRow;
		while objvarNameIdCursor%found loop
			  objvarNameIdMap(objvarNameIdRow.name) := objvarNameIdRow.id;
			  fetch objvarNameIdCursor into objvarNameIdRow;
		end loop;
		close objvarNameIdCursor;

		select max(id) into maxNameId from object_variable_names;

		v := objvarNameIdMap.first;
		while v is not null loop
			if objvarNameIdMap(v) = 0 then
				maxNameId := maxNameId + 1;
				stmt := 'insert into object_variable_names values (:id, :name)';
				execute immediate stmt using maxNameId, v;
				objvarNameIdMap(v) := maxNameId;
			end if;
			v := objvarNameIdMap.next(v);
		end loop;
		commit;
		if use_rbs is not null then
			execute immediate 'set transaction use rollback segment ' || use_rbs;
		end if;

		open schematicCursor;
		fetch schematicCursor into schematicRow;
		while schematicCursor%found loop
			  schematic_map(schematicRow.schematic_id) := schematicRow.object_template_id;
			  fetch schematicCursor into schematicRow;
		end loop;
		close schematicCursor;
	
-- ****** BEGIN TEMPLATE ID SET ******
		templateIdSet(1386184862) := 1386184862;
		templateIdSet(575325453) := 575325453;
		templateIdSet(1652891098) := 1652891098;
		templateIdSet(1185807341) := 1185807341;
		templateIdSet(913882750) := 913882750;
		templateIdSet(1994709940) := 1994709940;
		templateIdSet(-1489599533) := -1489599533;
		templateIdSet(301359835) := 301359835;
		templateIdSet(-913758478) := -913758478;
		templateIdSet(306929321) := 306929321;
		templateIdSet(1654528826) := 1654528826;
		templateIdSet(-1981228116) := -1981228116;
		templateIdSet(-1493844838) := -1493844838;
		templateIdSet(-702482167) := -702482167;
		templateIdSet(1360182635) := 1360182635;
		templateIdSet(605165771) := 605165771;
		templateIdSet(-1831667261) := -1831667261;
		templateIdSet(701587845) := 701587845;
		templateIdSet(-1634646786) := -1634646786;
		templateIdSet(819427966) := 819427966;
		templateIdSet(1073889261) := 1073889261;
		templateIdSet(1449316778) := 1449316778;
		templateIdSet(-1418664428) := -1418664428;
		templateIdSet(-609901689) := -609901689;
		templateIdSet(-1782480140) := -1782480140;
		templateIdSet(-2016614241) := -2016614241;
		templateIdSet(822872471) := 822872471;
		templateIdSet(609499915) := 609499915;
		templateIdSet(1026806068) := 1026806068;
		templateIdSet(1306956967) := 1306956967;
		templateIdSet(1503133620) := 1503133620;
		templateIdSet(2115381959) := 2115381959;
		templateIdSet(247689044) := 247689044;
		templateIdSet(725211246) := 725211246;
		templateIdSet(1252870946) := 1252870946;
		templateIdSet(-59921878) := -59921878;
		templateIdSet(-248317992) := -248317992;
		templateIdSet(-171534277) := -171534277;
		templateIdSet(-2062295640) := -2062295640;
		templateIdSet(-127140925) := -127140925;
		templateIdSet(-253929242) := -253929242;
		templateIdSet(-2146951819) := -2146951819;
		templateIdSet(1042717208) := 1042717208;
		templateIdSet(1128757147) := 1128757147;
		templateIdSet(-175663469) := -175663469;
		templateIdSet(2147059193) := 2147059193;
		templateIdSet(748768172) := 748768172;
		templateIdSet(1551109695) := 1551109695;
		templateIdSet(1302427111) := 1302427111;
		templateIdSet(-1537444864) := -1537444864;
		templateIdSet(-729206381) := -729206381;
		templateIdSet(1231025524) := 1231025524;
		templateIdSet(590525838) := 590525838;
		templateIdSet(-1779280762) := -1779280762;
		templateIdSet(729327903) := 729327903;
		templateIdSet(-255400918) := -255400918;
		templateIdSet(-2146197063) := -2146197063;
		templateIdSet(-1390425255) := -1390425255;
		templateIdSet(-412867955) := -412867955;
		templateIdSet(-1749981410) := -1749981410;
		templateIdSet(1756768751) := 1756768751;
		templateIdSet(1478831607) := 1478831607;
		templateIdSet(-286929665) := -286929665;
		templateIdSet(1749090194) := 1749090194;
		templateIdSet(183123522) := 183123522;
		templateIdSet(2050980817) := 2050980817;
		templateIdSet(2062241693) := 2062241693;
		templateIdSet(1837916498) := 1837916498;
		templateIdSet(492381377) := 492381377;
		templateIdSet(108052567) := 108052567;
		templateIdSet(-1552027109) := -1552027109;
		templateIdSet(364712723) := 364712723;
		templateIdSet(-492258227) := -492258227;
		templateIdSet(1926737359) := 1926737359;
		templateIdSet(33744988) := 33744988;
		templateIdSet(1205005532) := 1205005532;
		templateIdSet(-1767695347) := -1767695347;
		templateIdSet(-428451426) := -428451426;
		templateIdSet(-1520174324) := -1520174324;
		templateIdSet(2132891736) := 2132891736;
		templateIdSet(-907953840) := -907953840;
		templateIdSet(428327186) := 428327186;
		templateIdSet(-1877836182) := -1877836182;
		templateIdSet(-523813895) := -523813895;
		templateIdSet(66083222) := 66083222;
		templateIdSet(523674485) := 523674485;
		templateIdSet(1974187109) := 1974187109;
		templateIdSet(92205558) := 92205558;
		templateIdSet(856491201) := 856491201;
		templateIdSet(-91264646) := -91264646;
		templateIdSet(811367898) := 811367898;
		templateIdSet(1082734665) := 1082734665;
		templateIdSet(1193819287) := 1193819287;
		templateIdSet(-1082613563) := -1082613563;
		templateIdSet(-1640376935) := -1640376935;
		templateIdSet(1584198075) := 1584198075;
		templateIdSet(783922216) := 783922216;
		templateIdSet(-2133834359) := -2133834359;
		templateIdSet(-268107750) := -268107750;
		templateIdSet(1631264204) := 1631264204;
		templateIdSet(-1047768279) := -1047768279;
		templateIdSet(2001515041) := 2001515041;
		templateIdSet(267688086) := 267688086;
		templateIdSet(1065077663) := 1065077663;
		templateIdSet(57117299) := 57117299;
		templateIdSet(866000377) := 866000377;
		templateIdSet(1129011306) := 1129011306;
		templateIdSet(-279689267) := -279689267;
		templateIdSet(-1618935202) := -1618935202;
		templateIdSet(-1377743586) := -1377743586;
		templateIdSet(1440200840) := 1440200840;
		templateIdSet(-485009024) := -485009024;
		templateIdSet(1617992402) := 1617992402;
		templateIdSet(246676033) := 246676033;
		templateIdSet(2120662994) := 2120662994;
		templateIdSet(-1169656734) := -1169656734;
		templateIdSet(659297422) := 659297422;
		templateIdSet(1469665565) := 1469665565;
		templateIdSet(-1785454855) := -1785454855;
		templateIdSet(522917746) := 522917746;
		templateIdSet(-1469527663) := -1469527663;
		templateIdSet(824817548) := 824817548;
		templateIdSet(-434656188) := -434656188;
		templateIdSet(-1765611049) := -1765611049;
		templateIdSet(1139078028) := 1139078028;
		templateIdSet(1622923855) := 1622923855;
		templateIdSet(275685340) := 275685340;
		templateIdSet(-2109263964) := -2109263964;
		templateIdSet(1513782916) := 1513782916;
		templateIdSet(-319119476) := -319119476;
		templateIdSet(-274790576) := -274790576;
		templateIdSet(-1751431536) := -1751431536;
		templateIdSet(1928484082) := 1928484082;
		templateIdSet(36114785) := 36114785;
		templateIdSet(2125025189) := 2125025189;
		templateIdSet(-1285938931) := -1285938931;
		templateIdSet(-1014012770) := -1014012770;
		templateIdSet(-698743461) := -698743461;
		templateIdSet(-226902841) := -226902841;
		templateIdSet(1152998863) := 1152998863;
		templateIdSet(1014955026) := 1014955026;
		templateIdSet(-1762554943) := -1762554943;
		templateIdSet(-433795502) := -433795502;
		templateIdSet(1463740430) := 1463740430;
		templateIdSet(-2101932344) := -2101932344;
		templateIdSet(-1721490790) := -1721490790;
		templateIdSet(1777643552) := 1777643552;
		templateIdSet(-385048649) := -385048649;
		templateIdSet(-761139333) := -761139333;
		templateIdSet(-932988328) := -932988328;
		templateIdSet(525967921) := 525967921;
		templateIdSet(1871505314) := 1871505314;
		templateIdSet(-963800801) := -963800801;
		templateIdSet(-2050710967) := -2050710967;
		templateIdSet(-183375910) := -183375910;
		templateIdSet(689622929) := 689622929;
		templateIdSet(1301374465) := 1301374465;
		templateIdSet(-78543095) := -78543095;
		templateIdSet(182484822) := 182484822;
		templateIdSet(1051253941) := 1051253941;
		templateIdSet(-954052477) := -954052477;
		templateIdSet(-1208543984) := -1208543984;
		templateIdSet(-1630155612) := -1630155612;
		templateIdSet(-911886824) := -911886824;
		templateIdSet(-1183779957) := -1183779957;
		templateIdSet(-343994769) := -343994769;
		templateIdSet(1263770558) := 1263770558;
		templateIdSet(-40666442) := -40666442;
		templateIdSet(1182886663) := 1182886663;
		templateIdSet(774778471) := 774778471;
		templateIdSet(1593535476) := 1593535476;
		templateIdSet(-306804975) := -306804975;
		templateIdSet(582714478) := 582714478;
		templateIdSet(1382990333) := 1382990333;
		templateIdSet(-1832625049) := -1832625049;
		templateIdSet(-974187136) := -974187136;
		templateIdSet(1932522632) := 1932522632;
		templateIdSet(-1382065807) := -1382065807;
		templateIdSet(1841408054) := 1841408054;
		templateIdSet(487910821) := 487910821;
		templateIdSet(-185039936) := -185039936;
		templateIdSet(-2077374893) := -2077374893;
		templateIdSet(303274938) := 303274938;
		templateIdSet(1657200169) := 1657200169;
		templateIdSet(1224577523) := 1224577523;
		templateIdSet(-30363750) := -30363750;
		templateIdSet(-1897534967) := -1897534967;
		templateIdSet(567515627) := 567515627;
		templateIdSet(1362436589) := 1362436589;
		templateIdSet(1746127589) := 1746127589;
		templateIdSet(-556586003) := -556586003;
		templateIdSet(1630705486) := 1630705486;
		templateIdSet(1897165445) := 1897165445;
		templateIdSet(-2065356471) := -2065356471;
		templateIdSet(487215056) := 487215056;
		templateIdSet(-483121900) := -483121900;
		templateIdSet(-1813946233) := -1813946233;
		templateIdSet(830219516) := 830219516;
		templateIdSet(1425531172) := 1425531172;
		templateIdSet(606250167) := 606250167;
		templateIdSet(-1175884434) := -1175884434;
		templateIdSet(-1659123451) := -1659123451;
		templateIdSet(-606913477) := -606913477;
		templateIdSet(619388150) := 619388150;
		templateIdSet(-2061397693) := -2061397693;
		templateIdSet(-914141938) := -914141938;
		templateIdSet(-653428823) := -653428823;
		templateIdSet(-1993339022) := -1993339022;
		templateIdSet(1855229351) := 1855229351;
		templateIdSet(275564031) := 275564031;
		templateIdSet(-247678631) := -247678631;
		templateIdSet(1261688149) := 1261688149;
		templateIdSet(-779152232) := -779152232;
		templateIdSet(-1587916533) := -1587916533;
		templateIdSet(-1424733636) := -1424733636;
		templateIdSet(500476724) := 500476724;
		templateIdSet(-320086751) := -320086751;
		templateIdSet(678412834) := 678412834;
		templateIdSet(1436153762) := 1436153762;
		templateIdSet(-177674793) := -177674793;
		templateIdSet(-1886457751) := -1886457751;
		templateIdSet(569575162) := 569575162;
		templateIdSet(-392461625) := -392461625;
		templateIdSet(-1500071681) := -1500071681;
		templateIdSet(-420003189) := -420003189;
		templateIdSet(309375847) := 309375847;
		templateIdSet(-98746218) := -98746218;
		templateIdSet(1289593246) := 1289593246;
		templateIdSet(103036882) := 103036882;
		templateIdSet(416123005) := 416123005;
		templateIdSet(-469920034) := -469920034;
		templateIdSet(-817576610) := -817576610;
		templateIdSet(-38886533) := -38886533;
		templateIdSet(-1264335626) := -1264335626;
		templateIdSet(2144735852) := 2144735852;
		templateIdSet(1875688539) := 1875688539;
		templateIdSet(192857538) := 192857538;
		templateIdSet(-442303678) := -442303678;
		templateIdSet(-2072795932) := -2072795932;
		templateIdSet(983989372) := 983989372;
		templateIdSet(490845618) := 490845618;
		templateIdSet(-983864428) := -983864428;
		templateIdSet(2086609620) := 2086609620;
		templateIdSet(110363758) := 110363758;
		templateIdSet(-1424880637) := -1424880637;
		templateIdSet(-448030425) := -448030425;
		templateIdSet(-786663538) := -786663538;
		templateIdSet(-702291727) := -702291727;
		templateIdSet(50152844) := 50152844;
		templateIdSet(-132357748) := -132357748;
		templateIdSet(1590091204) := 1590091204;
		templateIdSet(-212848001) := -212848001;
		templateIdSet(1415954991) := 1415954991;
		templateIdSet(1401689522) := 1401689522;
		templateIdSet(1656768558) := 1656768558;
		templateIdSet(944293988) := 944293988;
		templateIdSet(-1764803069) := -1764803069;
		templateIdSet(2026707684) := 2026707684;
		templateIdSet(1741906969) := 1741906969;
		templateIdSet(1862066444) := 1862066444;
		templateIdSet(981227534) := 981227534;
		templateIdSet(-613110139) := -613110139;
		templateIdSet(188520444) := 188520444;
		templateIdSet(-589817340) := -589817340;
		templateIdSet(1740191859) := 1740191859;
		templateIdSet(-1464343893) := -1464343893;
		templateIdSet(-2022305605) := -2022305605;
		templateIdSet(810379093) := 810379093;
		templateIdSet(-965865145) := -965865145;
		templateIdSet(775748867) := 775748867;
		templateIdSet(-1876464536) := -1876464536;
		templateIdSet(-483088632) := -483088632;
		templateIdSet(901906289) := 901906289;
		templateIdSet(-2134198239) := -2134198239;
		templateIdSet(-1655034745) := -1655034745;
		templateIdSet(1080722273) := 1080722273;
		templateIdSet(1220774872) := 1220774872;
		templateIdSet(1141198733) := 1141198733;
		templateIdSet(-309965447) := -309965447;
		templateIdSet(-1316687033) := -1316687033;
		templateIdSet(-1322827874) := -1322827874;
		templateIdSet(810339444) := 810339444;
		templateIdSet(1102511316) := 1102511316;
		templateIdSet(1804298795) := 1804298795;
		templateIdSet(1505456979) := 1505456979;
		templateIdSet(-90182809) := -90182809;
		templateIdSet(-1915271095) := -1915271095;
		templateIdSet(-1188519487) := -1188519487;
		templateIdSet(-1173568585) := -1173568585;
		templateIdSet(1906195493) := 1906195493;
		templateIdSet(-1958737672) := -1958737672;
		templateIdSet(665098706) := 665098706;
		templateIdSet(-1170494820) := -1170494820;
		templateIdSet(831150616) := 831150616;
		templateIdSet(1762601318) := 1762601318;
		templateIdSet(-764562064) := -764562064;
		templateIdSet(-480153985) := -480153985;
		templateIdSet(-784284994) := -784284994;
		templateIdSet(444888474) := 444888474;
		templateIdSet(1618598853) := 1618598853;
		templateIdSet(461512847) := 461512847;
		templateIdSet(2138127220) := 2138127220;
		templateIdSet(1938675644) := 1938675644;
		templateIdSet(-1378772674) := -1378772674;
		templateIdSet(-1109531580) := -1109531580;
		templateIdSet(155551742) := 155551742;
		templateIdSet(-623854688) := -623854688;
		templateIdSet(-617341983) := -617341983;
		templateIdSet(443295551) := 443295551;
		templateIdSet(-1554492115) := -1554492115;
		templateIdSet(201334650) := 201334650;
		templateIdSet(78081465) := 78081465;
		templateIdSet(1261011254) := 1261011254;
		templateIdSet(227937004) := 227937004;
		templateIdSet(-1185633502) := -1185633502;
		templateIdSet(-1920361299) := -1920361299;
		templateIdSet(349677024) := 349677024;
		templateIdSet(1210744947) := 1210744947;
		templateIdSet(1369190307) := 1369190307;
		templateIdSet(-1438464920) := -1438464920;
		templateIdSet(702649693) := 702649693;
		templateIdSet(34919568) := 34919568;
		templateIdSet(-1565904814) := -1565904814;
		templateIdSet(-778602894) := -778602894;
		templateIdSet(-1382616459) := -1382616459;
		templateIdSet(-900058289) := -900058289;
		templateIdSet(-1566891826) := -1566891826;
		templateIdSet(-1474613882) := -1474613882;
		templateIdSet(590597969) := 590597969;
		templateIdSet(6493761) := 6493761;
		templateIdSet(2132239519) := 2132239519;
		templateIdSet(-721077210) := -721077210;
		templateIdSet(320220025) := 320220025;
		templateIdSet(-225492986) := -225492986;
		templateIdSet(1995174592) := 1995174592;
		templateIdSet(403078591) := 403078591;
		templateIdSet(-269033971) := -269033971;
		templateIdSet(-651693115) := -651693115;
		templateIdSet(-1571384798) := -1571384798;
		templateIdSet(716551990) := 716551990;
		templateIdSet(1944034997) := 1944034997;
		templateIdSet(1855721592) := 1855721592;
		templateIdSet(981088834) := 981088834;
		templateIdSet(-637703116) := -637703116;
		templateIdSet(-1779174489) := -1779174489;
		templateIdSet(1840338268) := 1840338268;
		templateIdSet(237038605) := 237038605;
		templateIdSet(653612816) := 653612816;
		templateIdSet(1608037452) := 1608037452;
		templateIdSet(-613167286) := -613167286;
		templateIdSet(-245341807) := -245341807;
		templateIdSet(-1019635633) := -1019635633;
		templateIdSet(-449112830) := -449112830;
		templateIdSet(-526113597) := -526113597;
		templateIdSet(589649861) := 589649861;
		templateIdSet(-20005656) := -20005656;
		templateIdSet(257663710) := 257663710;
		templateIdSet(-639753330) := -639753330;
		templateIdSet(-730685311) := -730685311;
		templateIdSet(-130243512) := -130243512;
		templateIdSet(1756821101) := 1756821101;
		templateIdSet(-1501822488) := -1501822488;
		templateIdSet(-132487792) := -132487792;
		templateIdSet(1982554682) := 1982554682;
		templateIdSet(-913097330) := -913097330;
		templateIdSet(-402333683) := -402333683;
		templateIdSet(-1811261900) := -1811261900;
		templateIdSet(-1178706532) := -1178706532;
		templateIdSet(-1137089652) := -1137089652;
		templateIdSet(-473813792) := -473813792;
		templateIdSet(-718648728) := -718648728;
		templateIdSet(238936831) := 238936831;
		templateIdSet(1358850609) := 1358850609;
		templateIdSet(-1088166271) := -1088166271;
		templateIdSet(102348899) := 102348899;
		templateIdSet(-673780945) := -673780945;
		templateIdSet(213992376) := 213992376;
		templateIdSet(-1330392160) := -1330392160;
		templateIdSet(1834662895) := 1834662895;
		templateIdSet(-1781845673) := -1781845673;
		templateIdSet(1214765971) := 1214765971;
		templateIdSet(-1213235742) := -1213235742;
		templateIdSet(-1498919413) := -1498919413;
		templateIdSet(-1109444980) := -1109444980;
		templateIdSet(-379196495) := -379196495;
		templateIdSet(648215202) := 648215202;
		templateIdSet(-279076105) := -279076105;
		templateIdSet(-2138350593) := -2138350593;
		templateIdSet(-1839044042) := -1839044042;
		templateIdSet(-1601393927) := -1601393927;
		templateIdSet(418740352) := 418740352;
		templateIdSet(1778522271) := 1778522271;
		templateIdSet(764829944) := 764829944;
		templateIdSet(-627137144) := -627137144;
		templateIdSet(100102071) := 100102071;
		templateIdSet(949442296) := 949442296;
		templateIdSet(180006972) := 180006972;
		templateIdSet(-926822510) := -926822510;
		templateIdSet(-633473500) := -633473500;
		templateIdSet(451466626) := 451466626;
		templateIdSet(-82570045) := -82570045;
		templateIdSet(-860173634) := -860173634;
		templateIdSet(252668529) := 252668529;
		templateIdSet(1866781530) := 1866781530;
		templateIdSet(-245080147) := -245080147;
		templateIdSet(1684501073) := 1684501073;
		templateIdSet(-1925049902) := -1925049902;
		templateIdSet(-837691545) := -837691545;
		templateIdSet(-1792584100) := -1792584100;
		templateIdSet(-1340116130) := -1340116130;
		templateIdSet(-1400940656) := -1400940656;
		templateIdSet(1980615506) := 1980615506;
		templateIdSet(1687234741) := 1687234741;
		templateIdSet(-2032619870) := -2032619870;
		templateIdSet(1070818136) := 1070818136;
		templateIdSet(322519616) := 322519616;
		templateIdSet(129228399) := 129228399;
		templateIdSet(-1929515589) := -1929515589;
		templateIdSet(1691705957) := 1691705957;
		templateIdSet(-1919723538) := -1919723538;
		templateIdSet(-937385248) := -937385248;
		templateIdSet(685231719) := 685231719;
		templateIdSet(-1340981329) := -1340981329;
		templateIdSet(-1238955746) := -1238955746;
		templateIdSet(-656409820) := -656409820;
		templateIdSet(-67541093) := -67541093;
		templateIdSet(1223042704) := 1223042704;
		templateIdSet(1646698389) := 1646698389;
		templateIdSet(-1930572145) := -1930572145;
		templateIdSet(-1437726662) := -1437726662;
		templateIdSet(-2118141076) := -2118141076;
		templateIdSet(-1354942232) := -1354942232;
		templateIdSet(709369610) := 709369610;
		templateIdSet(-205520309) := -205520309;
		templateIdSet(-603889252) := -603889252;
		templateIdSet(-1321212581) := -1321212581;
		templateIdSet(397924793) := 397924793;
		templateIdSet(-1148251186) := -1148251186;
		templateIdSet(2069010987) := 2069010987;
		templateIdSet(-1374316705) := -1374316705;
		templateIdSet(-1428709338) := -1428709338;
		templateIdSet(-2024883534) := -2024883534;
		templateIdSet(-896708611) := -896708611;
		templateIdSet(1325615183) := 1325615183;
		templateIdSet(1007504804) := 1007504804;
		templateIdSet(1775069676) := 1775069676;
		templateIdSet(-1629773375) := -1629773375;
		templateIdSet(1333331720) := 1333331720;
		templateIdSet(869624722) := 869624722;
		templateIdSet(247923643) := 247923643;
		templateIdSet(-631441882) := -631441882;
		templateIdSet(-1328960537) := -1328960537;
		templateIdSet(1327818409) := 1327818409;
		templateIdSet(-2117547605) := -2117547605;
		templateIdSet(352903310) := 352903310;
		templateIdSet(130303407) := 130303407;
		templateIdSet(1006954225) := 1006954225;
		templateIdSet(-1957995430) := -1957995430;
		templateIdSet(200431483) := 200431483;
		templateIdSet(1219521873) := 1219521873;
		templateIdSet(1567297626) := 1567297626;
		templateIdSet(1062687370) := 1062687370;
		templateIdSet(-1774596602) := -1774596602;
		templateIdSet(1530854406) := 1530854406;
		templateIdSet(-255701924) := -255701924;
		templateIdSet(735438027) := 735438027;
		templateIdSet(1658356550) := 1658356550;
		templateIdSet(-1724143534) := -1724143534;
		templateIdSet(-801714209) := -801714209;
		templateIdSet(-181151130) := -181151130;
		templateIdSet(774140657) := 774140657;
		templateIdSet(1730745724) := 1730745724;
		templateIdSet(-1664467352) := -1664467352;
		templateIdSet(431995434) := 431995434;
		templateIdSet(-1029172547) := -1029172547;
		templateIdSet(-1952090832) := -1952090832;
		templateIdSet(1883659812) := 1883659812;
		templateIdSet(-980521968) := -980521968;
		templateIdSet(-306844601) := -306844601;
		templateIdSet(-2039935369) := -2039935369;
		templateIdSet(-23647699) := -23647699;
		templateIdSet(-1933405050) := -1933405050;
		templateIdSet(-1329737896) := -1329737896;
		templateIdSet(144581616) := 144581616;
		templateIdSet(-1122191702) := -1122191702;
		templateIdSet(22306368) := 22306368;
		templateIdSet(281064494) := 281064494;
		templateIdSet(-1320167407) := -1320167407;
		templateIdSet(1061303036) := 1061303036;
		templateIdSet(507210122) := 507210122;
		templateIdSet(-88500234) := -88500234;
		templateIdSet(1472463975) := 1472463975;
		templateIdSet(2080884324) := 2080884324;
		templateIdSet(-2052487231) := -2052487231;
		templateIdSet(-528877582) := -528877582;
		templateIdSet(-135648971) := -135648971;
		templateIdSet(230572551) := 230572551;
		templateIdSet(1057167832) := 1057167832;
		templateIdSet(536694865) := 536694865;
		templateIdSet(437634302) := 437634302;
		templateIdSet(2054517358) := 2054517358;
		templateIdSet(-1337924041) := -1337924041;
		templateIdSet(1353161551) := 1353161551;
		templateIdSet(-2137759113) := -2137759113;
		templateIdSet(179201581) := 179201581;
		templateIdSet(-322799884) := -322799884;
		templateIdSet(1599302627) := 1599302627;
		templateIdSet(-1655466225) := -1655466225;
		templateIdSet(993643963) := 993643963;
		templateIdSet(428493251) := 428493251;
		templateIdSet(1834531933) := 1834531933;
		templateIdSet(1664157751) := 1664157751;
		templateIdSet(-1989431522) := -1989431522;
		templateIdSet(-237557333) := -237557333;
		templateIdSet(-400228634) := -400228634;
		templateIdSet(1550199637) := 1550199637;
		templateIdSet(-1129312673) := -1129312673;
		templateIdSet(1602372839) := 1602372839;
		templateIdSet(2036727535) := 2036727535;
		templateIdSet(-2066158902) := -2066158902;
		templateIdSet(1706352715) := 1706352715;
		templateIdSet(-821627577) := -821627577;
		templateIdSet(1731904862) := 1731904862;
		templateIdSet(1492257317) := 1492257317;
		templateIdSet(1543376981) := 1543376981;
		templateIdSet(-453063519) := -453063519;
		templateIdSet(-1135348098) := -1135348098;
		templateIdSet(-180524642) := -180524642;
		templateIdSet(-1234286053) := -1234286053;
		templateIdSet(-966588406) := -966588406;
		templateIdSet(391653777) := 391653777;
		templateIdSet(1833668435) := 1833668435;
		templateIdSet(-1632342596) := -1632342596;
		templateIdSet(1468700561) := 1468700561;
		templateIdSet(1186404816) := 1186404816;
		templateIdSet(-1236344303) := -1236344303;
		templateIdSet(870515892) := 870515892;
		templateIdSet(-609603936) := -609603936;
		templateIdSet(-108612773) := -108612773;
		templateIdSet(-723081254) := -723081254;
		templateIdSet(-1482529718) := -1482529718;
		templateIdSet(-702596630) := -702596630;
		templateIdSet(-191532605) := -191532605;
		templateIdSet(1368313349) := 1368313349;
		templateIdSet(-1149904183) := -1149904183;
		templateIdSet(-104858180) := -104858180;
		templateIdSet(1901385280) := 1901385280;
		templateIdSet(-519120500) := -519120500;
		templateIdSet(-1030442907) := -1030442907;
		templateIdSet(-272947641) := -272947641;
		templateIdSet(1239922273) := 1239922273;
		templateIdSet(920119228) := 920119228;
		templateIdSet(1460469297) := 1460469297;
		templateIdSet(1098635748) := 1098635748;
		templateIdSet(1461830070) := 1461830070;
		templateIdSet(-1864503130) := -1864503130;
		templateIdSet(1801195805) := 1801195805;
		templateIdSet(-410420377) := -410420377;
		templateIdSet(877344239) := 877344239;
		templateIdSet(1434285776) := 1434285776;
		templateIdSet(558263029) := 558263029;
		templateIdSet(22531863) := 22531863;
		templateIdSet(1828317055) := 1828317055;
		templateIdSet(-1670164093) := -1670164093;
		templateIdSet(284923026) := 284923026;
		templateIdSet(-250957874) := -250957874;
		templateIdSet(1454424858) := 1454424858;
		templateIdSet(51214754) := 51214754;
		templateIdSet(540146307) := 540146307;
		templateIdSet(-635575777) := -635575777;
		templateIdSet(1166887647) := 1166887647;
		templateIdSet(-23601230) := -23601230;
		templateIdSet(-1365202497) := -1365202497;
		templateIdSet(1714967517) := 1714967517;
		templateIdSet(99972710) := 99972710;
		templateIdSet(-1722035741) := -1722035741;
		templateIdSet(-1746165408) := -1746165408;
		templateIdSet(329225748) := 329225748;
		templateIdSet(1301924307) := 1301924307;
		templateIdSet(-846740906) := -846740906;
		templateIdSet(471067493) := 471067493;
		templateIdSet(-1483553925) := -1483553925;
		templateIdSet(-1938284398) := -1938284398;
		templateIdSet(1466927109) := 1466927109;
		templateIdSet(509800328) := 509800328;
		templateIdSet(-444480356) := -444480356;
		templateIdSet(-1400069359) := -1400069359;
		templateIdSet(-589619825) := -589619825;
		templateIdSet(-1781106174) := -1781106174;
		templateIdSet(1848987926) := 1848987926;
		templateIdSet(657995419) := 657995419;
		templateIdSet(1931602976) := 1931602976;
		templateIdSet(-679440489) := -679440489;
		templateIdSet(2059421636) := 2059421636;
		templateIdSet(-737833670) := -737833670;
		templateIdSet(-893992793) := -893992793;
		templateIdSet(-1107035087) := -1107035087;
		templateIdSet(1695831206) := 1695831206;
		templateIdSet(-1266575809) := -1266575809;
		templateIdSet(1872117416) := 1872117416;
		templateIdSet(647715109) := 647715109;
		templateIdSet(-579309007) := -579309007;
		templateIdSet(-1804266052) := -1804266052;
		templateIdSet(527556965) := 527556965;
		templateIdSet(-999929358) := -999929358;
		templateIdSet(2005513882) := 2005513882;
		templateIdSet(-1398923763) := -1398923763;
		templateIdSet(-443319936) := -443319936;
		templateIdSet(510677652) := 510677652;
		templateIdSet(1045341716) := 1045341716;
		templateIdSet(-447148413) := -447148413;
		templateIdSet(-1403754226) := -1403754226;
		templateIdSet(1471627802) := 1471627802;
		templateIdSet(2049606520) := 2049606520;
		templateIdSet(-1589826577) := -1589826577;
		templateIdSet(-399487902) := -399487902;
		templateIdSet(332621686) := 332621686;
		templateIdSet(-106672157) := -106672157;
		templateIdSet(582215540) := 582215540;
		templateIdSet(1807646969) := 1807646969;
		templateIdSet(-231292762) := -231292762;
		templateIdSet(690055217) := 690055217;
		templateIdSet(1613500348) := 1613500348;
		templateIdSet(-1680923480) := -1680923480;
		templateIdSet(-132280259) := -132280259;
		templateIdSet(587872426) := 587872426;
		templateIdSet(1778868007) := 1778868007;
		templateIdSet(-1847299021) := -1847299021;
		templateIdSet(-220731710) := -220731710;
		templateIdSet(700468821) := 700468821;
		templateIdSet(1624058328) := 1624058328;
		templateIdSet(-1634928232) := -1634928232;
		templateIdSet(1167806735) := 1167806735;
		templateIdSet(211170946) := 211170946;
		templateIdSet(-143319658) := -143319658;
		templateIdSet(949453622) := 949453622;
		templateIdSet(-478139487) := -478139487;
		templateIdSet(-1433580500) := -1433580500;
		templateIdSet(1366189880) := 1366189880;
		templateIdSet(1240046010) := 1240046010;
		templateIdSet(-1828835027) := -1828835027;
		templateIdSet(-604809568) := -604809568;
		templateIdSet(537951668) := 537951668;
		templateIdSet(310195605) := 310195605;
		templateIdSet(672875303) := 672875303;
		templateIdSet(-217257040) := -217257040;
		templateIdSet(-1174303683) := -1174303683;
		templateIdSet(-2011612965) := -2011612965;
		templateIdSet(1393463372) := 1393463372;
		templateIdSet(436467649) := 436467649;
		templateIdSet(-505357099) := -505357099;
		templateIdSet(-1460815016) := -1460815016;
		templateIdSet(-97283480) := -97283480;
		templateIdSet(556022527) := 556022527;
		templateIdSet(1415748921) := 1415748921;
		templateIdSet(-1888143954) := -1888143954;
		templateIdSet(-1676060831) := -1676060831;
		templateIdSet(1192165366) := 1192165366;
		templateIdSet(235116667) := 235116667;
		templateIdSet(-169823377) := -169823377;
		templateIdSet(-1125264158) := -1125264158;
		templateIdSet(-328757929) := -328757929;
		templateIdSet(930096576) := 930096576;
		templateIdSet(2122138189) := 2122138189;
		templateIdSet(-2053215911) := -2053215911;
		templateIdSet(-862777644) := -862777644;
		templateIdSet(430317917) := 430317917;
		templateIdSet(-1028552246) := -1028552246;
		templateIdSet(-1950552505) := -1950552505;
		templateIdSet(1885324627) := 1885324627;
		templateIdSet(961716958) := 961716958;
		templateIdSet(-1677860148) := -1677860148;
		templateIdSet(1089096283) := 1089096283;
		templateIdSet(166145494) := 166145494;
		templateIdSet(-234617150) := -234617150;
		templateIdSet(-1157013169) := -1157013169;
		templateIdSet(1184142638) := 1184142638;
		templateIdSet(-1652343367) := -1652343367;
		templateIdSet(-728868300) := -728868300;
		templateIdSet(795701536) := 795701536;
		templateIdSet(1717573293) := 1717573293;
		templateIdSet(-1765458857) := -1765458857;
		templateIdSet(1305646272) := 1305646272;
		templateIdSet(81785677) := 81785677;
		templateIdSet(-12789671) := -12789671;
		templateIdSet(-1238253612) := -1238253612;
		templateIdSet(-1657672270) := -1657672270;
		templateIdSet(1176913189) := 1176913189;
		templateIdSet(254485160) := 254485160;
		templateIdSet(-188216900) := -188216900;
		templateIdSet(-1111134671) := -1111134671;
		templateIdSet(157634799) := 157634799;
		templateIdSet(-1983000237) := -1983000237;
		templateIdSet(1390049732) := 1390049732;
		templateIdSet(467082825) := 467082825;
		templateIdSet(-764217224) := -764217224;
		templateIdSet(-1686134795) := -1686134795;
		templateIdSet(95475824) := 95475824;
		templateIdSet(90200724) := 90200724;
		templateIdSet(-562564605) := -562564605;
		templateIdSet(-1753574002) := -1753574002;
		templateIdSet(-559451929) := -559451929;
		templateIdSet(-1750460566) := -1750460566;
		templateIdSet(1506389102) := 1506389102;
		templateIdSet(-2099378951) := -2099378951;
		templateIdSet(-875338892) := -875338892;
		templateIdSet(53231194) := 53231194;
		templateIdSet(-667183411) := -667183411;
		templateIdSet(-1858670272) := -1858670272;
		templateIdSet(-1166687602) := -1166687602;
		templateIdSet(1633839641) := 1633839641;
		templateIdSet(678366612) := 678366612;
		templateIdSet(-1206446259) := -1206446259;
		templateIdSet(1660993498) := 1660993498;
		templateIdSet(705550423) := 705550423;
		templateIdSet(-2037168026) := -2037168026;
		templateIdSet(1568989425) := 1568989425;
		templateIdSet(344557436) := 344557436;
		templateIdSet(810684832) := 810684832;
		templateIdSet(-347752137) := -347752137;
		templateIdSet(-1572282694) := -1572282694;
		templateIdSet(1934373193) := 1934373193;
		templateIdSet(-1470391842) := -1470391842;
		templateIdSet(-514392493) := -514392493;
		templateIdSet(1856972813) := 1856972813;
		templateIdSet(-1246199654) := -1246199654;
		templateIdSet(-55187689) := -55187689;
		templateIdSet(324224015) := 324224015;
		templateIdSet(-935007080) := -935007080;
		templateIdSet(-2125886699) := -2125886699;
		templateIdSet(-1564805563) := -1564805563;
		templateIdSet(2041405138) := 2041405138;
		templateIdSet(815806815) := 815806815;
		templateIdSet(-1636411245) := -1636411245;
		templateIdSet(1164046340) := 1164046340;
		templateIdSet(208443273) := 208443273;
		templateIdSet(-979596476) := -979596476;
		templateIdSet(512435155) := 512435155;
		templateIdSet(1468417118) := 1468417118;
		templateIdSet(-596190638) := -596190638;
		templateIdSet(123826885) := 123826885;
		templateIdSet(1315706184) := 1315706184;
		templateIdSet(-1062853951) := -1062853951;
		templateIdSet(464620118) := 464620118;
		templateIdSet(1388113371) := 1388113371;
		templateIdSet(2035451832) := 2035451832;
		templateIdSet(-1572524241) := -1572524241;
		templateIdSet(-347584350) := -347584350;
		templateIdSet(-601857572) := -601857572;
		templateIdSet(121096523) := 121096523;
		templateIdSet(1312468678) := 1312468678;
		templateIdSet(681976607) := 681976607;
		templateIdSet(-206468216) := -206468216;
		templateIdSet(-1162023931) := -1162023931;
		templateIdSet(-1465630540) := -1465630540;
		templateIdSet(1941170211) := 1941170211;
		templateIdSet(985567150) := 985567150;
		templateIdSet(-1647095649) := -1647095649;
		templateIdSet(1187313672) := 1187313672;
		templateIdSet(264871813) := 264871813;
		templateIdSet(235278722) := 235278722;
		templateIdSet(-720258795) := -720258795;
		templateIdSet(-1675829608) := -1675829608;
		templateIdSet(-1950783554) := -1950783554;
		templateIdSet(1353633577) := 1353633577;
		templateIdSet(430155940) := 430155940;
		templateIdSet(-636296737) := -636296737;
		templateIdSet(17123656) := 17123656;
		templateIdSet(1208495813) := 1208495813;
		templateIdSet(409740963) := 409740963;
		templateIdSet(-1015312844) := -1015312844;
		templateIdSet(-1971918407) := -1971918407;
		templateIdSet(-1085939198) := -1085939198;
		templateIdSet(1683114645) := 1683114645;
		templateIdSet(761212184) := 761212184;
		templateIdSet(794688942) := 794688942;
		templateIdSet(-2053523137) := -2053523137;
		templateIdSet(-1092500879) := -1092500879;
		templateIdSet(-1730770782) := -1730770782;
		templateIdSet(208928229) := 208928229;
		templateIdSet(807772545) := 807772545;
		templateIdSet(-596014267) := -596014267;
		templateIdSet(-1106535119) := -1106535119;
		templateIdSet(-1599002060) := -1599002060;
		templateIdSet(-1680302670) := -1680302670;
		templateIdSet(144304422) := 144304422;
		templateIdSet(1089638525) := 1089638525;
		templateIdSet(1491099597) := 1491099597;
		templateIdSet(1896411640) := 1896411640;
		templateIdSet(656430760) := 656430760;
		templateIdSet(-637806373) := -637806373;
		templateIdSet(1450479489) := 1450479489;
		templateIdSet(-2028859065) := -2028859065;
		templateIdSet(1338425771) := 1338425771;
		templateIdSet(-508865592) := -508865592;
		templateIdSet(186405659) := 186405659;
		templateIdSet(1367605438) := 1367605438;
		templateIdSet(766575814) := 766575814;
		templateIdSet(-1695066256) := -1695066256;
		templateIdSet(-1026091453) := -1026091453;
		templateIdSet(-2032640577) := -2032640577;
		templateIdSet(1362576287) := 1362576287;
		templateIdSet(430277184) := 430277184;
		templateIdSet(148075865) := 148075865;
		templateIdSet(-1319720093) := -1319720093;
		templateIdSet(-14537666) := -14537666;
		templateIdSet(-1597210008) := -1597210008;
		templateIdSet(389089358) := 389089358;
		templateIdSet(143113263) := 143113263;
		templateIdSet(-1981094972) := -1981094972;
		templateIdSet(787604538) := 787604538;
		templateIdSet(233382897) := 233382897;
		templateIdSet(-820961364) := -820961364;
		templateIdSet(-1929517492) := -1929517492;
		templateIdSet(1825046689) := 1825046689;
		templateIdSet(-497561499) := -497561499;
		templateIdSet(-370688623) := -370688623;
		templateIdSet(1252325702) := 1252325702;
		templateIdSet(1219727576) := 1219727576;
		templateIdSet(-261254912) := -261254912;
		templateIdSet(-826946743) := -826946743;
		templateIdSet(-307530442) := -307530442;
		templateIdSet(-126968979) := -126968979;
		templateIdSet(1352512078) := 1352512078;
		templateIdSet(38436739) := 38436739;
		templateIdSet(-648161516) := -648161516;
		templateIdSet(-1873760103) := -1873760103;
		templateIdSet(1806932877) := 1806932877;
		templateIdSet(-2043897502) := -2043897502;
		templateIdSet(1672986063) := 1672986063;
		templateIdSet(-201478902) := -201478902;
		templateIdSet(-320858653) := -320858653;
		templateIdSet(-428819171) := -428819171;
		templateIdSet(649015964) := 649015964;
		templateIdSet(-1588852672) := -1588852672;
		templateIdSet(1589268411) := 1589268411;
		templateIdSet(-2063383877) := -2063383877;
		templateIdSet(-184506838) := -184506838;
		templateIdSet(205254138) := 205254138;
		templateIdSet(1936734520) := 1936734520;
		templateIdSet(1577598037) := 1577598037;
		templateIdSet(83272939) := 83272939;
		templateIdSet(-383871740) := -383871740;
		templateIdSet(-1338608150) := -1338608150;
		templateIdSet(-132717072) := -132717072;
		templateIdSet(375006515) := 375006515;
		templateIdSet(-144833342) := -144833342; 	-- manufacturing schematic
		templateIdSet(162889564) := 162889564;		-- weapon crate
		templateIdSet(1120303977) := 1120303977;	-- armor crate
		templateIdSet(-200781577) := -200781577;	-- food crate
		templateIdSet(-1966544754) := -1966544754;	-- chemical crate
		templateIdSet(-754298423) := -754298423;	-- electronics crate
		templateIdSet(-1207917085) := -1207917085;	-- clothing crate
		templateIdSet(1717727484) := 1717727484;	-- nyax necklace
		
		templateNameText(2099054288)  := 'armor_bounty_hunter_crafted_belt';
		templateNameText(-852325331)  := 'armor_bounty_hunter_crafted_bicep_l';
		templateNameText(-1108883010) := 'armor_bounty_hunter_crafted_bicep_r';
		templateNameText(315293450)   := 'armor_bounty_hunter_crafted_boots';
		templateNameText(-151793970)  := 'armor_bounty_hunter_crafted_bracer_l';
		templateNameText(-2044294307) := 'armor_bounty_hunter_crafted_bracer_r';
		templateNameText(1649659380)  := 'armor_bounty_hunter_crafted_chest_plate';
		templateNameText(140529204)   := 'armor_bounty_hunter_crafted_gloves';
		templateNameText(-1096765636) := 'armor_bounty_hunter_crafted_helmet';
		templateNameText(2044138449)  := 'armor_bounty_hunter_crafted_leggings';
		templateNameText(-983864428)  := 'armor_layer_kinetic';
		templateNameTable(-983864428) := 'craft_clothing_ingredients_n';
		templateNameText(1594162622)  := 'armor_recon_segment';
		templateNameTable(1594162622) := 'craft_armor_ingredients_n';
		templateNameText(-1438161694) := 'armor_battle_segment';
		templateNameTable(-1438161694):= 'craft_armor_ingredients_n';
		templateNameText(2042493214)  := 'armor_assault_segment';
		templateNameTable(2042493214) := 'craft_armor_ingredients_n';
		templateNameText(-2117547605) := 'weapon_mine_explosive';
		templateNameTable(-2117547605):= 'powerup_n';	
		templateNameText(548609244)   := 'weapon_melee_balancing_weights';
		templateNameTable(548609244)  := 'powerup_n';
		templateNameText(-1004627683) := 'weapon_melee_inertial_fluctuator';
		templateNameTable(-1004627683):= 'powerup_n';
		templateNameText(352903310)   := 'weapon_ranged_barrel';
		templateNameTable(352903310)  := 'powerup_n';
		templateNameText(130303407)   := 'weapon_ranged_grip';
		templateNameTable(130303407)  := 'powerup_n';
		templateNameText(1006954225)  := 'weapon_ranged_muzzle';
		templateNameTable(1006954225) := 'powerup_n';
		templateNameText(200431483)   := 'weapon_ranged_scope';
		templateNameTable(200431483)  := 'powerup_n';
		templateNameText(1219521873)  := 'weapon_ranged_stock';
		templateNameTable(1219521873) := 'powerup_n';
		templateNameText(-551790316)  := 'weapon_melee_surface_serration';
		templateNameTable(-551790316) := 'powerup_n';
		templateNameText(-1042074732) := 'weapon_melee_tactical_grip';
		templateNameTable(-1042074732):= 'powerup_n';
		templateNameText(-356900087)  := 'instant_stimpack_a';
		templateNameTable(-356900087) := 'medicine_name';
		templateNameText(833457054)   := 'instant_stimpack_b';
		templateNameTable(833457054)  := 'medicine_name';
		templateNameText(2023812115)  := 'instant_stimpack_c';
		templateNameTable(2023812115) := 'medicine_name';
		templateNameText(-2092710137) := 'instant_stimpack_d';
		templateNameTable(-2092710137):= 'medicine_name';
		templateNameText(-1674079888) := 'stimpack_pet_a';
		templateNameTable(-1674079888):= 'medicine_name';
		templateNameText(1193294311)  := 'stimpack_pet_b';
		templateNameTable(1193294311) := 'medicine_name';
		templateNameText(237854314)   := 'stimpack_pet_c';
		templateNameTable(237854314)  := 'medicine_name';
		templateNameText(-171018882)  := 'stimpack_pet_d';
		templateNameTable(-171018882) := 'medicine_name';
		templateNameText(-1631144444) := 'rifle_bowcaster_heavy';
		templateNameText(-746051337)  := 'carbine_e11_mark2';
		templateNameText(1893534473)  := 'enhancer_bactajab';
		templateNameTable(1893534473) := 'medicine_name';
		templateNameText(189249679)   := 'enhancer_bactaspray';
		templateNameTable(189249679)  := 'medicine_name';
		templateNameText(-1945173818) := 'enhancer_bactatoss';
		templateNameTable(-1945173818):= 'medicine_name';
		templateNameText(-21620221)   := 'enhancer_deuteriumtoss';
		templateNameTable(-21620221)  := 'medicine_name';
		templateNameText(1004186547)  := 'enhancer_disinfect';
		templateNameTable(1004186547) := 'medicine_name';
		templateNameText(232945406)   := 'enhancer_endorphineinjection';
		templateNameTable(232945406)  := 'medicine_name';
		templateNameText(-2018470158) := 'enhancer_neurotoxin';
		templateNameTable(-2018470158):= 'medicine_name';
		templateNameText(2094584686)  := 'enhancer_nutrientinjection';
		templateNameTable(2094584686) := 'medicine_name';
		templateNameText(-1847366477) := 'enhancer_stabilizer';
		templateNameTable(-1847366477):= 'medicine_name';
		templateNameText(-410958245)  := 'enhancer_thyroidrupture';
		templateNameTable(-410958245) := 'medicine_name';
		templateNameText(-454097742)  := 'enhancer_traumatize';
		templateNameTable(-454097742) := 'medicine_name';
		
		return TRUE;
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error in InitializeData - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in InitializeData');
			return FALSE;
	end;

	procedure AddObjVar (objvar_name varchar2, objvar_type number, objvar_value varchar2)
	is
		n number;
		tempObjVar objVarType;
		oldName varchar2(500);
	begin
		if slot_map.exists(objvar_name) then
			ModifyObjVar(objvar_name, objvar_name, objvar_type, objvar_value);
		else
			if free_slots.count > 0 and length(objvar_name) <= 50 then
				n := free_slots(free_slots.first);
				Obj_ObjVars(n).objvar_name := objvar_name;
				Obj_ObjVars(n).objvar_type := objvar_type;
				Obj_ObjVars(n).objvar_value := objvar_value;
				free_slots.delete(n);
				isObjModified := TRUE;
			else
				if deleted_objvars.count > 0 then
					oldName := deleted_objvars(deleted_objvars.last);
					deleted_objvars.trim(1);
					OV_ObjVars(oldName).objvar_name := objvar_name;
					OV_ObjVars(oldName).objvar_type := objvar_type;
					OV_ObjVars(oldName).objvar_value := objvar_value;
					updated_objvars.extend(1);
					updated_objvars(updated_objvars.last) := oldName;
				else
					tempObjVar.object_id := object_id;
					tempObjVar.objvar_name := objvar_name;
					tempObjVar.objvar_type := objvar_type;
					tempObjVar.objvar_value := objvar_value;
					tempObjVar.objvar_name_id := objvarNameIdMap(objvar_name);
					OV_ObjVars(objvar_name) := tempObjVar;
					added_objvars.extend(1);
					added_objvars(added_objvars.last) := objvar_name;
				end if;
			end if;
		end if;
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error in AddObjVar - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in AddObjVar');
	end;

	procedure DeleteObjVar (objVarName in varchar2)
	is
		n number;
		v varchar2(500);
	begin
		if slot_map.exists(objVarName) then
			if slot_map(objVarName) < 21 then
				n := slot_map(objVarName);
				if Obj_ObjVars(n).objvar_name = objVarName then
					Obj_ObjVars(n).objvar_name := NULL;
					Obj_ObjVars(n).objvar_type := NULL;
					Obj_ObjVars(n).objvar_value := NULL;
					free_slots(n) := n;
					isObjModified := TRUE;
				end if;
			else
				if OV_ObjVars(objVarName).objvar_name  = objVarName then
					OV_ObjVars(objVarName).old_objvar_name_id := OV_ObjVars(objVarName).objvar_name_id;
					OV_ObjVars(objVarName).objvar_name := NULL;
					OV_ObjVars(objVarName).objvar_type := NULL;
					OV_ObjVars(objVarName).objvar_value := NULL;
					deleted_objvars.extend(1);
					deleted_objvars(deleted_objvars.last) := objVarName;
				end if;
			end if;
		elsif instr(objVarName,'*') > 0 then
			n := instr(objVarName,'*') -1;
			v := slot_map.first;
			while v is not null loop
				if substr(v,1,n) = substr(objVarName,1,n) then
					DeleteObjVar(v);
				end if;
				v := slot_map.next(v);
			end loop;
		end if;
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error in DeleteObjVar - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in DeleteObjVar');
	end;

	procedure ModifyObjVar (oldObjVarName varchar2, objvar_name varchar2, objvar_type number, objvar_value varchar2)
	is
		n number;
	begin
		if slot_map.exists(oldObjVarName) then
			if slot_map(oldObjVarName) < 21 then
				n := slot_map(oldObjVarName);
				Obj_ObjVars(n).objvar_name := objvar_name;
				Obj_ObjVars(n).objvar_type := objvar_type;
				Obj_ObjVars(n).objvar_value := objvar_value;
				isObjModified := TRUE;
			else
				OV_ObjVars(oldObjVarName).old_objvar_name_id := OV_ObjVars(oldObjVarName).objvar_name_id;
				OV_ObjVars(oldObjVarName).objvar_name := objvar_name;
				OV_ObjVars(oldObjVarName).objvar_type := objvar_type;
				OV_ObjVars(oldObjVarName).objvar_value := objvar_value;
				updated_objvars.extend(1);
				updated_objvars(updated_objvars.last) := oldObjVarName;
			end if;
		end if;
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error in ModifyObjVar - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in ModifyObjVar');
	end;

	procedure ModifyObjVar (oldObjVarName varchar2, objvar_name varchar2, objvar_type number, rangeTable rangeTableType)
	is
		n number;
	begin
		if slot_map.exists(oldObjVarName) then
			if slot_map(oldObjVarName) < 21 then
				n := slot_map(oldObjVarName);
				Obj_ObjVars(n).objvar_name := objvar_name;
				Obj_ObjVars(n).objvar_type := objvar_type;
				Obj_ObjVars(n).objvar_value := ConvertValue (to_number(Obj_ObjVars(n).objvar_value), rangeTable);
				isObjModified := TRUE;
			else
				if OV_ObjVars(oldObjVarName).old_objvar_name_id = 0 then
					OV_ObjVars(oldObjVarName).old_objvar_name_id := OV_ObjVars(oldObjVarName).objvar_name_id;
				end if;
				OV_ObjVars(oldObjVarName).objvar_name := objvar_name;
				OV_ObjVars(oldObjVarName).objvar_type := objvar_type;
				OV_ObjVars(oldObjVarName).objvar_value := ConvertValue (to_number(OV_ObjVars(oldObjVarName).objvar_value), rangeTable);
				updated_objvars.extend(1);
				updated_objvars(updated_objvars.last) := oldObjVarName;
			end if;
		end if;
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error in ModifyObjVar - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in ModifyObjVar');
	end;

	procedure RenameObjVar (oldObjVarName varchar2, objvar_name varchar2)
	is
		n number;
	begin
		if slot_map.exists(oldObjVarName) then
			if slot_map(oldObjVarName) < 21 then
				n := slot_map(oldObjVarName);
				Obj_ObjVars(n).objvar_name := objvar_name;
				isObjModified := TRUE;
			else
				OV_ObjVars(oldObjVarName).old_objvar_name_id := OV_ObjVars(oldObjVarName).objvar_name_id;
				OV_ObjVars(oldObjVarName).objvar_name := objvar_name;
				updated_objvars.extend(1);
				updated_objvars(updated_objvars.last) := oldObjVarName;
			end if;
		end if;
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error in ModifyObjVar - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in ModifyObjVar');
	end;

	function GetObjVarValue (objvar_name varchar2) return varchar2
	is
	begin
		if slot_map.exists(objvar_name) then
			if slot_map(objvar_name) < 21 then
				return Obj_ObjVars(slot_map(objvar_name)).objvar_value;
			else
				return OV_ObjVars(objvar_name).objvar_value;
			end if;
		else
			return null;
		end if;
	end;

	procedure AddScript (newScript varchar2)
	is
	begin
		script_list := trim(script_list) ||  newScript || ':';
		isObjModified := TRUE;
	end;

	procedure RemoveScript (oldScript varchar2)
	is
	begin
		script_list := replace(script_list, oldScript || ':');
		isObjModified := TRUE;
	end;

	procedure SetNewTemplateId (newTemplateId number)
	is
	begin
		new_template_id := newTemplateId;
		SetTemplateText (newTemplateId);
	end;
	
	procedure SetTemplateText (newTemplateId number)
	is
	begin		
		if templateNameText.exists(newTemplateId) then
			object_name := null;
			name_text  := templateNameText(newTemplateId);
			if templateNameTable.exists(newTemplateId) then
				name_table := templateNameTable(newTemplateId);
			end if;
		end if;
	end;
	
	procedure InvalidateSchematic
	is
	begin
		AddObjVar('ImAnInvalidSchematic',0,1);
		isObjModified := TRUE;
	end;
	
	procedure AddAttrib (attrib_name varchar2, attrib_value float)
	is
		oldName varchar2(500);
		tempAttrib manfAttributeRowType;
	begin
		if manf_attributes.exists(attrib_name) then
			ModifyAttrib(attrib_name, attrib_name, attrib_value);
		else
			if deleted_attribs.count >0 then
				oldName := deleted_attribs(deleted_attribs.last);
				deleted_attribs.trim(1);
				manf_attributes(oldName).attribute := attrib_name;
				manf_attributes(oldName).value := attrib_value;
				updated_attribs.extend(1);
				updated_attribs(updated_attribs.last) := oldName;
			else
				tempAttrib.object_id := object_id;
				tempAttrib.attribute := attrib_name;
				tempAttrib.value := attrib_value;
				manf_attributes(attrib_name) := tempAttrib;
				added_attribs.extend(1);
				added_attribs(added_attribs.last) := attrib_name;
			end if;
		end if;
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error in AddAttrib - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in AddAttrib');
	end;

	procedure DeleteAttrib (attrib_name in varchar2)
	is
		n number;
		v varchar2(500);
	begin
		if manf_attributes.exists(attrib_name) then
			if manf_attributes(attrib_name).attribute  = attrib_name then
				manf_attributes(attrib_name).attribute := NULL;
				manf_attributes(attrib_name).value := NULL;
				deleted_attribs.extend(1);
				deleted_attribs(deleted_attribs.last) := attrib_name;
			end if;
		elsif instr(attrib_name,'*') > 0 then
			n := instr(attrib_name,'*') -1;
			v := manf_attributes.first;
			while v is not null loop
				if substr(v,1,n) = substr(attrib_name,1,n) then
					DeleteAttrib(v);
				end if;
				v := manf_attributes.next(v);
			end loop;
		end if;
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error in DeleteAttrib - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in DeleteAttrib');
	end;

	procedure ModifyAttrib (old_attrib_name varchar2, attrib_name varchar2, attrib_value float)
	is
		n number;
	begin
		if manf_attributes.exists(attrib_name) then
			manf_attributes(old_attrib_name).attribute := attrib_name;
			manf_attributes(old_attrib_name).value := attrib_value;
			updated_attribs.extend(1);
			updated_attribs(updated_attribs.last) := old_attrib_name;
		end if;
	exception
		when others then
			err_num := SQLCODE;
			dbms_output.put_line('Error in ModifyAttribs - time = ' || to_char(sysdate, 'MM/DD/YY HH24:MI:SS'));
			db_error_logger.dblogerror(err_num, 'Error in ModifyAttribs');
	end;
	
	procedure DeleteOldArmorObjvars
	is
	begin
		DeleteObjVar('armor.rating');
		DeleteObjVar('armor.vulnerability');
		DeleteObjVar('armor.encumbrance');
		DeleteObjVar('armor.special*');
		DeleteObjVar('crafting_components.armor*');
		RemoveScript('item.armor.armor');
	end;
	
	procedure SetGenProtection(rangeTable rangeTableType)
	is
	begin
		if isFactory= TRUE then
			ModifyObjVar('crafting_attributes.crafting:armor_effectiveness','crafting_attributes.crafting:general_protection',2,rangeTable);
		else
			ModifyObjVar('armor.effectiveness','armor.general_protection',2,rangeTable);
		end if;
	end;
	
	procedure SetArmorCondLevelCat(condition varchar2, lev varchar2, category varchar2)
	is
	begin
		if isFactory= TRUE then
			AddObjVar('crafting_attributes.crafting:condition',2,condition);
			AddObjVar('armorLevel',0,lev);
			AddObjVar('armorCategory',0,category);
		else
			AddObjVar('armor.condition',2,condition);
			AddObjVar('armor.armorLevel',0,lev);
			AddObjVar('armor.armorCategory',0,category);
		end if;
	end;
	
	procedure SetArmorLevel(lev varchar)
	is
	begin
		if isFactory= TRUE then
			AddObjVar('armorLevel',0,lev);
		else
			AddObjVar('armor.armorLevel',0,lev);
		end if;
	end;
	
	procedure SetArmorCategory(category varchar2)
	is 
	begin
		if isFactory= TRUE then
			AddObjVar('armorCategory',0,category);
		else
			AddObjVar('armor.armorCategory',0,category);
		end if;
	end;

	procedure SetGpAndCond(gp_name varchar2, gp_val varchar2, cond_name varchar2, cond_val varchar2)
	is 
	begin
		AddObjVar(gp_name,2,gp_val);
		AddObjVar(cond_name,2,cond_val);
	end;
	
	procedure DeleteCraftingComponents
	is
	begin
		DeleteObjVar('crafting_components.*');
		DeleteObjVar('item_attrib_keys');
		DeleteObjVar('item_attrib_values');
	end;

	procedure CopySchematicAttributes
	is
		n number;
		v varchar2(500);
	begin
		if (manf_attributes.count = 0) then
			v := slot_map.first;
			while v is not null loop
				if substr(v,1,29) = 'crafting_attributes.crafting:' then
					if slot_map(v) < 21 then
						n := Obj_ObjVars(slot_map(v)).objvar_value;
					else
						n := OV_ObjVars(v).objvar_value;
					end if;
					AddAttrib(substr(v,21), n);
				end if;
				v := slot_map.next(v);
			end loop;
		end if;
		DeleteObjVar('crafting_attributes.crafting:*');
	end;

	procedure HandleFoodSchematic
	is
	begin
		CopySchematicAttributes;
		ConvertAttribValue('flavor', rangeTableType(rangeType(0,200,0.0,200.0)));
		ConvertAttribValue('nutrition', rangeTableType(rangeType(0,200,0.0,200.0)));
		DeleteObjVar('item_attrib_keys');
		DeleteObjVar('item_attrib_values');
	end;
	
	procedure HandleFoodCrate
	is
	begin
		ModifyObjVar('crafting_attributes.crafting:flavor','crafting_attributes.crafting:flavor',2,rangeTableType(rangeType(0,200,0.0,200.0)));
		ModifyObjVar('crafting_attributes.crafting:nutrition','crafting_attributes.crafting:nutrition',2,rangeTableType(rangeType(0,200,0.0,200.0)));
	end;

	procedure ConvertToStimpack(stim_template number,healing_power number, combat_level number)
	is
	begin
		if isSchematic = TRUE then
			CopySchematicAttributes;
			new_schematic_id := stim_template;
			SetTemplateText(SchematicMap(stim_template));
			ModifyAttrib('power','power',healing_power);
			DeleteObjVar('item_attrib_keys');
			DeleteObjVar('item_attrib_values');
			script_list := 'systems.crafting.chemistry.crafted_items.crafting_medpack_stimpack_chemistry:';
			isObjModified := TRUE;
		elsif isFactory = TRUE then	
			SetTemplateText(SchematicMap(stim_template));
			ModifyObjVar('draftSchematic','draftSchematic',0,stim_template);
			ModifyObjVar('crafting_attributes.crafting:power','crafting_attributes.crafting:power',2,healing_power);
			script_list := 'systems.crafting.chemistry.crafted_items.crafting_medpack_stimpack_chemistry:';
			isObjModified := TRUE;
		else
			SetNewTemplateId (SchematicMap(stim_template));
			DeleteObjVar('healing.*');
			DeleteObjVar('consumable.*');
			AddObjVar('healing.power',0,healing_power);
			AddObjVar('healing.combat_level_required',0,combat_level);
			RemoveScript('item.comestible.medicine');
			AddScript('item.medicine.stimpack');
		end if;
	end;
	
	procedure ConvertToPetStimpack(stim_template number,healing_power number)
	is
	begin
		if isSchematic = TRUE then
			CopySchematicAttributes;
			new_schematic_id := stim_template;
			SetTemplateText(SchematicMap(stim_template));
			ModifyAttrib('power','power',healing_power);
			DeleteObjVar('item_attrib_keys');
			DeleteObjVar('item_attrib_values');
			script_list := 'systems.crafting.bio_engineer.instant_stimpack.crafting_stimpack_pet:';
			isObjModified := TRUE;
		elsif isFactory = TRUE then	
			SetTemplateText(SchematicMap(stim_template));
			ModifyObjVar('draftSchematic','draftSchematic',0,stim_template);
			ModifyObjVar('crafting_attributes.crafting:power','crafting_attributes.crafting:power',2,healing_power);
			script_list := 'systems.crafting.bio_engineer.instant_stimpack.crafting_stimpack_pet:';
			isObjModified := TRUE;
		else
			SetNewTemplateId (SchematicMap(stim_template));
			DeleteObjVar('healing.*');
			DeleteObjVar('consumable.*');
			AddObjVar('healing.power',0,healing_power);
			RemoveScript('item.comestible.pet_med');
			AddScript('item.comestible.pet_med');
		end if;
	end;
	
	procedure ConvertToEnhancer(enh_template number, enh_name varchar2, enh_power number)
	is
	begin
		if isSchematic = TRUE then
			CopySchematicAttributes;
			new_schematic_id := enh_template;
			SetTemplateText(SchematicMap(enh_template));
			ModifyAttrib('power','power',enh_power);
			DeleteObjVar('item_attrib_keys');
			DeleteObjVar('item_attrib_values');
			script_list := 'systems.crafting.chemistry.crafting_enhancers:';
			isObjModified := TRUE;
		elsif isFactory = TRUE then	
			SetTemplateText(SchematicMap(enh_template));
			ModifyObjVar('draftSchematic','draftSchematic',0,enh_template);
			ModifyObjVar('crafting_attributes.crafting:power','crafting_attributes.crafting:power',2,enh_power);
			script_list := 'systems.crafting.chemistry.crafting_enhancers:';
			isObjModified := TRUE;
		else
			SetNewTemplateId (SchematicMap(enh_template));
			DeleteObjVar('healing.*');
			DeleteObjVar('consumable.*');
			AddObjVar('commandName',4,enh_name);
			AddObjVar('healing.enhancement',2,enh_power);
			RemoveScript('item.comestible.medicine');
			AddScript('item.medicine.enhancer');
		end if;
	end;
	
	procedure ConvertSpice(spice_name varchar2)
	is
	begin
		if isSchematic = TRUE then
			DeleteObjVar('item_attrib_keys');
			DeleteObjVar('item_attrib_values');
		elsif isFactory = FALSE then
			AddObjVar('spice.name',4,spice_name);
			DeleteObjVar('dur');
			DeleteObjVar('health');
			DeleteObjVar('str');
			DeleteObjVar('con');
			DeleteObjVar('action');
			DeleteObjVar('quick');
			DeleteObjVar('stam');
			DeleteObjVar('mind');
			DeleteObjVar('focus');
			DeleteObjVar('will');
			RemoveScript('item.comestible.spice');
			AddScript('item.spice');
		end if;
	end;
	

	procedure HandleWeaponDots
	is
		a varchar2(500);
		b varchar2(10);
		dmgDelta number;
	begin
		a := dot_list.first;
		while a is not null loop
			if (dot_list(a).exists('type') and dot_list(a).exists('attribute') and dot_list(a).exists('strength')) then
				dmgDelta := ((weapon_attribs.min_damage +weapon_attribs.max_damage) / 2) *
					((to_number(nvl(dot_list(a)('strength').objvar_value,0)) / 1000) *
					(DotsMatrix(dot_list(a)('type').objvar_value, 
						to_number(nvl(dot_list(a)('attribute').objvar_value,-1)) ) / 20));
				weapon_attribs.min_damage := weapon_attribs.min_damage + dmgDelta;	
				weapon_attribs.max_damage := weapon_attribs.max_damage + dmgDelta;	
				
				b := dot_list(a).first;
				while b is not null loop
					DeleteObjVar(dot_list(a)(b).objvar_name);
					b := dot_list(a).next(b);
				end loop;
			end if;	
			a := dot_list.next(a);
		end loop;
		-- also remove powerups
		DeleteObjVar('powerup.*');
		RemoveScript('powerup.weapon');
	end;

	procedure SetWeaponBaseStats(attack number, acc number, minrange number, maxrange number, damagetype number)
	is
	begin
		SetAttackCost(attack);
		SetAccuracy(acc);
		SetMinRange(minrange);
		SetMaxRange(maxrange);
		SetDamageType(damagetype);
	end;

	procedure SetMinDamage (min_damage number) 
	is
	begin
		if isSchematic = TRUE then
			AddAttrib('crafting:minDamage', min_damage);
			DeleteObjVar('crafting_attributes.crafting:minDamage');
		elsif isFactory = TRUE then	
			AddObjVar('crafting_attributes.crafting:minDamage',2,min_damage);
		elsif isWeaponObj = TRUE then
			weapon_attribs.min_damage := min_damage;
		else
			AddObjVar('crafting_components.minDamage',2,min_damage);			
		end if;
	end;
	
	procedure SetMaxDamage (max_damage number)
	is
	begin
		if isSchematic = TRUE then
			AddAttrib('crafting:maxDamage', max_damage);
			DeleteObjVar('crafting_attributes.crafting:maxDamage');
		elsif isFactory = TRUE then	
			AddObjVar('crafting_attributes.crafting:maxDamage',2,max_damage);
		elsif isWeaponObj = TRUE then
			weapon_attribs.max_damage := max_damage;
		else
			AddObjVar('crafting_components.maxDamage',2,max_damage);			
		end if;
	end;
	
	procedure SetAttackSpeed (attack_speed number)
	is
	begin
		if isSchematic = TRUE then
			AddAttrib('crafting:attackSpeed', attack_speed);
			DeleteObjVar('crafting_attributes.crafting:attackSpeed');
		elsif isFactory = TRUE then	
			AddObjVar('crafting_attributes.crafting:attackSpeed',2,attack_speed);
		elsif isWeaponObj = TRUE then
			weapon_attribs.attack_speed := attack_speed;
		else
			AddObjVar('crafting_components.attackSpeed',2,attack_speed);			
		end if;
	end;
	
	procedure SetWoundChance (wound_chance number)
	is
	begin
		if isSchematic = TRUE then
			AddAttrib('crafting:woundChance', wound_chance);
			DeleteObjVar('crafting_attributes.crafting:woundChance');
		elsif isFactory = TRUE then	
			AddObjVar('crafting_attributes.crafting:woundChance',2,wound_chance);
		elsif isWeaponObj = TRUE then
			weapon_attribs.wound_chance := wound_chance;
		else
			AddObjVar('crafting_components.woundChance',2,wound_chance);			
		end if;
	end;
	
	procedure SetAccuracy (accuracy number)
	is
	begin
		if isSchematic = TRUE then
			DeleteAttrib('crafting:zeroRangeMod');
			DeleteAttrib('crafting:minRangeMod');
			DeleteAttrib('crafting:midRangeMod');
			DeleteAttrib('crafting:maxRangeMod');
			AddAttrib('crafting:accuracy', accuracy);
			DeleteObjVar('crafting_attributes.crafting:minRangeMod');
			DeleteObjVar('crafting_attributes.crafting:zeroRangeMod');
			DeleteObjVar('crafting_attributes.crafting:midRangeMod');
			DeleteObjVar('crafting_attributes.crafting:maxRangeMod');
		elsif isFactory = TRUE then	
			AddObjVar('crafting_attributes.crafting:accuracy',2,accuracy);
			DeleteObjVar('crafting_attributes.crafting:zeroRangeMod');
			DeleteObjVar('crafting_attributes.crafting:minRangeMod');
			DeleteObjVar('crafting_attributes.crafting:midRangeMod');
			DeleteObjVar('crafting_attributes.crafting:maxRangeMod');
		elsif isWeaponObj = TRUE then
			weapon_attribs.accuracy := accuracy;
		else
			AddObjVar('crafting_components.accuracy',2,accuracy);			
		end if;
	end;
	
	procedure SetAttackCost (attack_cost number)
	is
	begin
		if isSchematic = TRUE then
			AddAttrib('crafting:attackCost', attack_cost);
			DeleteAttrib('crafting:attackHealthCost');
			DeleteAttrib('crafting:attackActionCost');
			DeleteAttrib('crafting:attackMindCost');
			DeleteObjVar('crafting_attributes.crafting:attackHealthCost');
			DeleteObjVar('crafting_attributes.crafting:attackActionCost');
			DeleteObjVar('crafting_attributes.crafting:attackMindCost');
		elsif isFactory = TRUE then	
			AddObjVar('crafting_attributes.crafting:attackCost',2,attack_cost);
			DeleteObjVar('crafting_attributes.crafting:attackHealthCost');
			DeleteObjVar('crafting_attributes.crafting:attackActionCost');
			DeleteObjVar('crafting_attributes.crafting:attackMindCost');
		elsif isWeaponObj = TRUE then
			weapon_attribs.attack_cost := attack_cost;
		else
			AddObjVar('crafting_components.attackCost',2,attack_cost);			
		end if;
	end;
	
	procedure SetMinRange (min_range number)
	is
	begin
		if isSchematic = TRUE then
			AddAttrib('crafting:minRange', min_range);
			DeleteObjVar('crafting_attributes.crafting:minRange');
			DeleteObjVar('crafting_attributes.crafting:midRange');
		elsif isFactory = TRUE then	
			AddObjVar('crafting_attributes.crafting:minRange',2,min_range);
			DeleteObjVar('crafting_attributes.crafting:midRange');
		elsif isWeaponObj = TRUE then
			weapon_attribs.min_range := min_range;
		else
			AddObjVar('crafting_components.minRange',2,min_range);			
		end if;
	end;
	
	procedure SetMaxRange (max_range number)
	is
	begin
		if isSchematic = TRUE then
			AddAttrib('crafting:maxRange', max_range);
			DeleteObjVar('crafting_attributes.crafting:maxRange');
		elsif isFactory = TRUE then	
			AddObjVar('crafting_attributes.crafting:maxRange',2,max_range);
		elsif isWeaponObj = TRUE then
			weapon_attribs.max_range := max_range;
		else
			AddObjVar('crafting_components.maxRange',2,max_range);			
		end if;
	end;

	procedure SetDamageType (dmg_type number)	
	is
	begin
		if isSchematic = TRUE then
			AddAttrib('crafting:damageType', dmg_type);
			DeleteObjVar('crafting_attributes.crafting:damageType');
		elsif isFactory = TRUE then	
			AddObjVar('crafting_attributes.crafting:damageType',2,dmg_type);
		elsif isWeaponObj = TRUE then
			weapon_attribs.damage_type := dmg_type;
		else
			AddObjVar('crafting_components.damageType',2,dmg_type);			
		end if;
	end;

	procedure SetElementalType (ele_type number)	
	is
	begin
		if isSchematic = TRUE then
			AddAttrib('crafting:elementalType', ele_type);
			DeleteObjVar('crafting_attributes.crafting:elementalType');
		elsif isFactory = TRUE then	
			AddObjVar('crafting_attributes.crafting:elementalType',2,ele_type);
		elsif isWeaponObj = TRUE then
			weapon_attribs.elemental_type := ele_type;
		else
			AddObjVar('crafting_components.elementalType',2,ele_type);			
		end if;
	end;

	procedure SetElementalValue (ele_value number)	
	is
	begin
		if isSchematic = TRUE then
			AddAttrib('crafting:elementalValue', ele_value);
			DeleteObjVar('crafting_attributes.crafting:elementalValue');
		elsif isFactory = TRUE then	
			AddObjVar('crafting_attributes.crafting:elementalValue',2,ele_value);
		elsif isWeaponObj = TRUE then
			weapon_attribs.elemental_value := ele_value;
		else
			AddObjVar('crafting_components.elementalValue',2,ele_value);			
		end if;
	end;

	procedure ConvertAttribValue(attribute varchar2, rangeTable rangeTableType)
	is
		n number;
	begin
		if manf_attributes.exists('crafting:' || attribute) then
			ModifyAttrib('crafting:' || attribute,'crafting:' || attribute,ConvertValue(manf_attributes('crafting:' || attribute).value, rangeTable));
		elsif slot_map.exists('crafting_attributes.crafting:' || attribute) then
			if slot_map('crafting_attributes.crafting:' || attribute) < 21 then
				n := Obj_ObjVars(slot_map('crafting_attributes.crafting:' || attribute)).objvar_value;
			else
				n := OV_ObjVars('crafting_attributes.crafting:' || attribute).objvar_value;
			end if;
			AddAttrib('crafting:' || attribute, ConvertValue(n, rangeTable));
		end if;
	end;

	procedure ConvertMinDamage (rangeTable rangeTableType) 
	is
	begin
		if isSchematic = TRUE then
			ConvertAttribValue('minDamage', rangeTable);
		elsif isFactory = TRUE then	
			ModifyObjVar('crafting_attributes.crafting:minDamage','crafting_attributes.crafting:minDamage',2,rangeTable);
		else
			weapon_attribs.min_damage := ConvertValue(weapon_attribs.min_damage, rangeTable);
		end if;
	end;
	
	procedure ConvertMaxDamage (rangeTable rangeTableType)
	is
	begin
		if isSchematic = TRUE then
			ConvertAttribValue('maxDamage', rangeTable);
		elsif isFactory = TRUE then	
			ModifyObjVar('crafting_attributes.crafting:maxDamage','crafting_attributes.crafting:maxDamage',2,rangeTable);
		else
			weapon_attribs.max_damage := ConvertValue(weapon_attribs.max_damage, rangeTable);
		end if;
	end;
	
	procedure ConvertAttackSpeed (rangeTable rangeTableType)
	is
	begin
		if isSchematic = TRUE then
			ConvertAttribValue('attackSpeed', rangeTable);			
		elsif isFactory = TRUE then	
			ModifyObjVar('crafting_attributes.crafting:attackSpeed','crafting_attributes.crafting:attackSpeed',2,rangeTable);
		else
			weapon_attribs.attack_speed := ConvertValue(weapon_attribs.attack_speed*10, rangeTable)/100;
		end if;
	end;
	
	procedure ConvertWoundChance (rangeTable rangeTableType)
	is
	begin
		if isSchematic = TRUE then
			ConvertAttribValue('woundChance', rangeTable);
		elsif isFactory = TRUE then	
			ModifyObjVar('crafting_attributes.crafting:woundChance','crafting_attributes.crafting:woundChance',2,rangeTable);
		else
			weapon_attribs.wound_chance := ConvertValue(weapon_attribs.wound_chance, rangeTable);
		end if;
	end;
	
	procedure ConvertRangedPowerup
	is
		schematic_id varchar2(50);
		oldval number;
		bonus number;
		penalty number;
		bonusObjVar varchar2(50);
		penaltyObjVar varchar2(50);
	begin
		if slot_map.exists('powerup.max_dam') then
			schematic_id := 996671491; -- ranged_barrel		
			oldval := to_number(GetObjVarValue('powerup.max_dam'));
			if oldval > 0.33 then oldval := 0.33; end if;
			bonus := oldval * 0.9;
			penalty := bonus * 0.67;
			bonusObjVar := 'powerup.damage';
			penaltyObjVar := 'powerup.speed';
		elsif slot_map.exists('powerup.min_dam') then
			schematic_id := 996671491; -- ranged_barrel		
			oldval := to_number(GetObjVarValue('powerup.min_dam'));
			if oldval > 0.33 then oldval := 0.33; end if;
			bonus := oldval * 0.9;
			penalty := bonus * 0.67;
			bonusObjVar := 'powerup.damage';
			penaltyObjVar := 'powerup.speed';
		elsif slot_map.exists('powerup.speed') then
			schematic_id := -671068023; -- ranged_grip
			oldval := to_number(GetObjVarValue('powerup.speed'));
			if oldval > 0.33 then oldval := 0.33; end if;
			bonus := oldval * -0.9;
			penalty := trunc(-134 * bonus);
			bonusObjVar := 'powerup.speed';
			penaltyObjVar := 'powerup.accuracy';
		elsif slot_map.exists('powerup.wound_chance') then
			schematic_id := -1092259441; --ranged_muzzle
			oldval := to_number(GetObjVarValue('powerup.wound'));
			if oldval > 0.33 then oldval := 0.33; end if;
			bonus := oldval * 0.9;
			penalty := bonus * 0.67;
			bonusObjVar := 'powerup.wound';
			penaltyObjVar := 'powerup.actionCost';
		elsif slot_map.exists('powerup.zero_mod') or slot_map.exists('powerup.mid_mod')  or slot_map.exists('powerup.max_mod') then
			schematic_id := 917984840; -- ranged_scope
			oldval := 0.20;
			bonus := 18;
			penalty := 0.09;
			bonusObjVar := 'powerup.accuracy';
			penaltyObjVar := 'powerup.speed';
		else
			schematic_id := 2108030646; -- ranged_stock			
			oldval := to_number(nvl(GetObjVarValue('powerup.health_cost'),'0'));
			if oldval = 0 then
				oldval :=  to_number(nvl(GetObjVarValue('powerup.action_cost'),'0'));
				if oldval = 0 then
					oldval :=  to_number(nvl(GetObjVarValue('powerup.mind_cost'),'0'));
				end if;
			end if;
			if oldval = 0 then 
				oldval := 0.10;
			elsif oldval > 0.33 then 
				oldval := 0.33; 
			end if;
			bonus := oldval * -0.9;
			penalty := trunc(-67 * bonus);
			bonusObjVar := 'powerup.actionCost';
			penaltyObjVar := 'powerup.accuracy';
		end if;	
		if isSchematic = TRUE then
			CopySchematicAttributes;
			new_schematic_id := schematic_id;
			SetTemplateText(SchematicMap(schematic_id));
			DeleteObjVar('powerup.*');
			DeleteObjVar('item_attrib_keys');
			DeleteObjVar('item_attrib_values');
			AddAttrib('crafting:effect',  oldval * 300);
			AddAttrib('crafting:efficiency', 33);
			script_list := 'systems.crafting.item.specific_item.crafting_powerup_item:';
			isObjModified := TRUE;
		elsif isFactory = TRUE then	
			SetTemplateText(SchematicMap(schematic_id));
			DeleteObjVar('powerup.*');
			ModifyObjVar('draftSchematic','draftSchematic',0,schematic_id);
			AddObjVar('crafting_attributes.crafting:effect',2,oldval * 300);
			AddObjVar('crafting_attributes.crafting:efficiency',2,33);
			script_list := 'systems.crafting.item.specific_item.crafting_powerup_item:';
			isObjModified := TRUE;
		else
			SetNewTemplateId(SchematicMap(schematic_id));
			DeleteObjVar('powerup.*');
			AddObjVar('powerup.effect', 2, oldval * 300);
			AddObjVar('powerup.efficiency', 2, 33);
			AddObjVar('powerup.usesLeft', 0, 100);
			AddObjVar(bonusObjVar, 2, bonus);
			AddObjVar(penaltyObjVar, 2, penalty);
		end if;
	end;

	procedure ConvertMeleePowerup
	is
		schematic_id varchar2(50);
		oldval number;
		bonus number;
		penalty number;
		bonusObjVar varchar2(50);
		penaltyObjVar varchar2(50);
	begin
		if slot_map.exists('powerup.max_dam') then
			schematic_id := 1380627363; -- inertial_fluctuator		
			oldval := to_number(GetObjVarValue('powerup.max_dam'));
			if oldval > 0.33 then oldval := 0.33; end if;
			bonus := oldval * 0.9;
			penalty := bonus * 0.67;
			bonusObjVar := 'powerup.damage';
			penaltyObjVar := 'powerup.speed';
		elsif slot_map.exists('powerup.min_dam') then
			schematic_id := 1380627363; -- inertial_fluctuator
			oldval := to_number(GetObjVarValue('powerup.min_dam'));
			if oldval > 0.33 then oldval := 0.33; end if;
			bonus := oldval * 0.9;
			penalty := bonus * 0.67;
			bonusObjVar := 'powerup.damage';
			penaltyObjVar := 'powerup.speed';
		elsif slot_map.exists('powerup.speed') then
			schematic_id := 695657910; -- tactical_grip_modifier
			oldval := to_number(GetObjVarValue('powerup.speed'));
			if oldval > 0.33 then oldval := 0.33; end if;
			bonus := oldval * -0.9;
			penalty := trunc(-134 * bonus);
			bonusObjVar := 'powerup.speed';
			penaltyObjVar := 'powerup.accuracy';
		elsif slot_map.exists('powerup.wound_chance') then
			schematic_id := -862055522; --surface_serration_kit
			oldval := to_number(GetObjVarValue('powerup.wound'));
			if oldval > 0.33 then oldval := 0.33; end if;
			bonus := oldval * 0.9;
			penalty := bonus * 0.67;
			bonusObjVar := 'powerup.wound';
			penaltyObjVar := 'powerup.actionCost';
		elsif slot_map.exists('powerup.zero_mod') or slot_map.exists('powerup.mid_mod')  or slot_map.exists('powerup.max_mod') then
			schematic_id := 1867435580; -- balancing_weights
			oldval := 0.20;
			bonus := 18;
			penalty := 0.09;
			bonusObjVar := 'powerup.accuracy';
			penaltyObjVar := 'powerup.speed';
		else
			schematic_id := -1838768682; -- hilt_reinforcement_kit			
			oldval := to_number(nvl(GetObjVarValue('powerup.health_cost'),'0'));
			if oldval = 0 then
				oldval :=  to_number(nvl(GetObjVarValue('powerup.action_cost'),'0'));
				if oldval = 0 then
					oldval :=  to_number(nvl(GetObjVarValue('powerup.mind_cost'),'0'));
				end if;
			end if;
			if oldval = 0 then 
				oldval := 0.10;
			elsif oldval > 0.33 then 
				oldval := 0.33; 
			end if;
			bonus := oldval * -0.9;
			penalty := trunc(-67 * bonus);
			bonusObjVar := 'powerup.actionCost';
			penaltyObjVar := 'powerup.accuracy';
		end if;	
		if isSchematic = TRUE then
			CopySchematicAttributes;
			new_schematic_id := schematic_id;
			SetTemplateText(SchematicMap(schematic_id));
			DeleteObjVar('powerup.*');
			DeleteObjVar('item_attrib_keys');
			DeleteObjVar('item_attrib_values');
			AddAttrib('crafting:effect',  oldval * 300);
			AddAttrib('crafting:efficiency', 33);
			script_list := 'systems.crafting.item.specific_item.crafting_powerup_item:';
			isObjModified := TRUE;
		elsif isFactory = TRUE then	
			SetTemplateText(SchematicMap(schematic_id));
			ModifyObjVar('draftSchematic','draftSchematic',0,schematic_id);
			AddObjVar('crafting_attributes.crafting:effect',2,oldval * 300);
			AddObjVar('crafting_attributes.crafting:efficiency',2,33);
			script_list := 'systems.crafting.item.specific_item.crafting_powerup_item:';
			isObjModified := TRUE;
		else
			SetNewTemplateId(SchematicMap(schematic_id));
			DeleteObjVar('powerup.*');
			AddObjVar('powerup.effect', 2, oldval * 300);
			AddObjVar('powerup.efficiency', 2, 33);
			AddObjVar('powerup.usesLeft', 0, 100);
			AddObjVar(bonusObjVar, 2, bonus);
			AddObjVar(penaltyObjVar, 2, penalty);
		end if;
	end;

	procedure ConvertMinePowerup
	is
		oldval number;
	begin
		if slot_map.exists('powerup.max_dam') then
			oldval := to_number(GetObjVarValue('powerup.max_dam'));
		elsif slot_map.exists('powerup.min_dam') then
			oldval := to_number(GetObjVarValue('powerup.min_dam'));
		else
			oldval := 0.10;
		end if;	
		if oldval > 0.33 then oldval := 0.33; end if;
		if isSchematic = TRUE then
			CopySchematicAttributes;
			new_schematic_id := -1704666295;
			SetTemplateText(-2117547605);
			DeleteObjVar('powerup.*');
			DeleteObjVar('item_attrib_keys');
			DeleteObjVar('item_attrib_values');
			AddAttrib('crafting:effect',  oldval * 300);
			AddAttrib('crafting:efficiency', 33);
			script_list := 'systems.crafting.item.specific_item.crafting_powerup_item:';
			isObjModified := TRUE;
		elsif isFactory = TRUE then	
			SetTemplateText(-2117547605);
			ModifyObjVar('draftSchematic','draftSchematic',0,-1704666295);
			AddObjVar('crafting_attributes.crafting:effect',2,oldval * 300);
			AddObjVar('crafting_attributes.crafting:efficiency',2,33);
			script_list := 'systems.crafting.item.specific_item.crafting_powerup_item:';
			isObjModified := TRUE;
		else
			SetNewTemplateId(-2117547605);
			DeleteObjVar('powerup.*');
			AddObjVar('powerup.effect', 2, oldval * 300);
			AddObjVar('powerup.efficiency', 2, 33);
			AddObjVar('powerup.usesLeft', 0, 100);
			AddObjVar('powerup.damage', 2, oldval * 0.9);
		end if;
	end;

	function ConvertValue (oldvalue number, rangeTable rangeTableType)
		return number
	is
		retval number;
		i number;
	begin
		retval := rangeTable(rangeTable.first)(3);  -- default to min value
		if oldvalue >= rangeTable(rangeTable.last)(2) then
			retval := rangeTable(rangeTable.last)(4);
		else
			i := rangeTable.first;
			while i is not null loop
				if oldvalue >= rangeTable(i)(1) and oldvalue < rangeTable(i)(2) then
					retval := (( (oldvalue - rangeTable(i)(1)) * 
								 ((rangeTable(i)(4)-rangeTable(i)(3))/(rangeTable(i)(2)-rangeTable(i)(1))) )
								+ rangeTable(i)(3) );
					exit;
				end if;
				i := rangeTable.next(i);
			end loop;
		end if;

		return retval;
	end;

	procedure CheckObjVarForDot (objvar objVarType)
	is
		dotName varchar2(500);
	begin
		if substr(objvar.objvar_name,1,4) = 'dot.' and (instr(objvar.objvar_name,'.',-1) > 4) then
			dotName := substr(objvar.objvar_name,5,instr(objvar.objvar_name,'.',-1)-5);
			dot_list(dotName)(substr(objvar.objvar_name,instr(objvar.objvar_name,'.',1,2)+1,10)) := objvar;
		end if;
	end;
	
	function DotsMatrix(dot_type varchar2, dot_attrib number)
		return number
	is
		retval number := 0;
	begin
		if dots_matrix.exists(dot_type) and dots_matrix(dot_type).exists(dot_attrib) then
			retval := dots_matrix(dot_type)(dot_attrib);
		end if;
		return retval;
	end;

	function SchematicMap(schematic_id number)
		return number
	is
		retval number := 0;
	begin
		if schematic_map.exists(schematic_id) then
			retval := schematic_map(schematic_id);
		end if;
		return retval;
	end;
end;
/
