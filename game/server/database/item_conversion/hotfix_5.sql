CREATE OR REPLACE PACKAGE  "HOTFIX_5" as
	procedure StartItemConversion (rbs_to_use varchar2 default null, conversion_id number default 1);
	procedure DoSingleObject (objectId number, conversion_id number default 1);
end;
/

CREATE OR REPLACE PACKAGE BODY  "HOTFIX_5" as
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
		endObjectId number;
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
				endObjectId := startObjectId + 19999999999;
				if endObjectId > maxObjectId then
					endObjectId := maxObjectId;
				end if;
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
		and object_template_id in
				(-1264335626,
				-1464343893,
				-1674079888,
				-171018882,
				-1839044042,
				-2022305605,
				-38886533,
				-470906696,
				-589817340,
				-613110139,
				-965865145,
				1193294311,
				162889564,
				1120303977,
				1740191859,
				188520444,
				2144735852,
				237854314,
				344340565,
				810379093,
				981227534,
				-144833342,
				-817576610,
				-200781577,
				-1966544754, 
				-754298423,
				-1207917085)
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
				--object/tangible/component/armor/shield_generator_personal.iff
				if template_id = -817576610 then
					if isSchematic = FALSE then
						if isFactory = TRUE then
							RenameObjVar('crafting_components.layer13','crafting_attributes.crafting:special_protection');
							RenameObjVar('crafting_components.recharge_rate','crafting_attributes.crafting:recharge_rate');
						else
							AddObjVar('armor.layer13',2,'0.4444');
							AddObjVar('armor.recharge_rate',2,'60');
						end if;
					end if;
				--object/tangible/component/armor/shield_generator_personal_b.iff
				elsif template_id = -38886533 then
					if isSchematic = FALSE then
						if isFactory = TRUE then
							RenameObjVar('crafting_components.layer13','crafting_attributes.crafting:special_protection');
							RenameObjVar('crafting_components.recharge_rate','crafting_attributes.crafting:recharge_rate');
						else
							AddObjVar('armor.layer13',2,'0.54545');
							AddObjVar('armor.recharge_rate',2,'60');
						end if;
					end if;
				--object/tangible/component/armor/shield_generator_personal_c.iff
				elsif template_id = -1264335626 then
					if isSchematic = FALSE then
						if isFactory = TRUE then
							RenameObjVar('crafting_components.layer13','crafting_attributes.crafting:special_protection');
							RenameObjVar('crafting_components.recharge_rate','crafting_attributes.crafting:recharge_rate');
						else
							AddObjVar('armor.layer13',2,'0.759167');
							AddObjVar('armor.recharge_rate',2,'60');
						end if;
					end if;
				--object/tangible/component/armor/shield_generator_personal_imperial_test.iff
				elsif template_id = 2144735852 then
					if isSchematic = FALSE then
						if isFactory = TRUE then
							RenameObjVar('crafting_components.layer13','crafting_attributes.crafting:special_protection');
							RenameObjVar('crafting_components.recharge_rate','crafting_attributes.crafting:recharge_rate');
						else
							AddObjVar('armor.layer13',2,'0.759167');
							AddObjVar('armor.recharge_rate',2,'65');
						end if;
					end if;
				--object/tangible/component/armor/armor_segment_enhancement_brackaset.iff
				elsif template_id = 981227534 then
					AddObjVar('attribute.bonus.0',0,'5');
				--object/tangible/component/armor/armor_segment_enhancement_fambaa.iff
				elsif template_id = -613110139 then
					AddObjVar('attribute.bonus.0',0,'5');
				--object/tangible/component/armor/armor_segment_enhancement_janta.iff
				elsif template_id = 188520444 then
					AddObjVar('attribute.bonus.0',0,'10');
				--object/tangible/component/armor/armor_segment_enhancement_kimogila.iff
				elsif template_id = -589817340 then
					AddObjVar('attribute.bonus.0',0,'10');
				--object/tangible/component/armor/armor_segment_enhancement_krayt.iff
				elsif template_id = 1740191859 then
					AddObjVar('attribute.bonus.0',0,'15');
				--object/tangible/component/armor/armor_segment_enhancement_nightsister.iff
				elsif template_id = -1464343893 then
					AddObjVar('attribute.bonus.0',0,'15');
				--object/tangible/component/armor/armor_segment_enhancement_rancor.iff
				elsif template_id = -2022305605 then
					AddObjVar('attribute.bonus.0',0,'15');
				--object/tangible/component/armor/armor_segment_enhancement_sharnaff.iff
				elsif template_id = 810379093 then
					AddObjVar('attribute.bonus.0',0,'5');
				--object/tangible/component/armor/armor_segment_enhancement_voritor_lizard.iff
				elsif template_id = -965865145 then
					AddObjVar('attribute.bonus.0',0,'5');
				--pet_stimpack
				elsif template_id = -171018882 then
					RemoveScript('item.comestible.pet_med');
					AddScript('item.medicine.stimpack_pet');
				--pet_stimpack
				elsif template_id = 237854314 then
					RemoveScript('item.comestible.pet_med');
					AddScript('item.medicine.stimpack_pet');
				--pet_stimpack
				elsif template_id = 1193294311 then
					RemoveScript('item.comestible.pet_med');
					AddScript('item.medicine.stimpack_pet');
				--pet_stimpack
				elsif template_id = -1674079888 then
					RemoveScript('item.comestible.pet_med');
					AddScript('item.medicine.stimpack_pet');
				--object/draft_schematic/weapon/component/blade_vibro_unit.iff
				elsif template_id = -470906696 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(8,15,8,14),rangeType(15,24,14,16)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(11,23,21,26),rangeType(23,54,26,30)));
					SetWoundChance(0);
					SetAttackCost(-3);					
				--object/draft_schematic/weapon/component/blade_vibro_unit_advanced.iff
				elsif template_id = 344340565 then
					ModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,rangeTableType(rangeType(22,28,13,26),rangeType(28,38,26,30)));
					ModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,rangeTableType(rangeType(32,42,42,53),rangeType(42,69,53,60)));
					ModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,rangeTableType(rangeType(-15,0,-20,0)));
					SetWoundChance(0);
					SetAttackCost(-6);					
				--carbine_e11_mk2 from quest
				elsif template_id = -1839044042 then
					HandleWeaponDots;
					ConvertMinDamage(rangeTableType(rangeType(99,189,255,486),rangeType(189,216,486,556)));
					ConvertMaxDamage(rangeTableType(rangeType(297,378,763,971),rangeType(378,432,971,1110)));
					ConvertWoundChance(rangeTableType(rangeType(1,13,2,25),rangeType(13,15.6,25,30)));
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
			'crafting_attributes.crafting:recharge_rate',
			'crafting_attributes.crafting:special_protection',
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
		objvarNameIdMap('crafting_attributes.crafting:recharge_rate') := 0;
		objvarNameIdMap('crafting_attributes.crafting:special_protection') := 0;
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
		templateIdSet(-1674079888) := -1674079888;
		templateIdSet(-171018882) := -171018882;
		templateIdSet(1193294311) := 1193294311;
		templateIdSet(237854314) := 237854314;
		templateIdSet(-470906696) := -470906696;
		templateIdSet(344340565) := 344340565;
		
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
