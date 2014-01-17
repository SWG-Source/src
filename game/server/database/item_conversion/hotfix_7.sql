CREATE OR REPLACE PACKAGE  "HOTFIX_7" as
	procedure StartItemConversion (rbs_to_use varchar2 default null, conversion_id number default 1);
end;
/

CREATE OR REPLACE PACKAGE BODY  "HOTFIX_7" as
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
	
	armor_item_gp   numTableVarchar;
	armor_crate_gp	numTableVarchar;
	
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
		v		   varchar2(500);
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
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_bicep_r.iff
				elsif template_id = 575325453 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_bracer_l.iff
				elsif template_id = 1185807341 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_bracer_r.iff
				elsif template_id = 913882750 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_chest_plate.iff
				elsif template_id = 1994709940 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_helmet.iff
				elsif template_id = 301359835 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/armor_bone_s01_leggings.iff
				elsif template_id = -913758478 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_bicep_l.iff
				elsif template_id = 306929321 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_bicep_r.iff
				elsif template_id = 1654528826 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_bracer_l.iff
				elsif template_id = -1493844838 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_bracer_r.iff
				elsif template_id = -702482167 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_chest_plate.iff
				elsif template_id = 1360182635 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_helmet.iff
				elsif template_id = -1831667261 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/bone/gp_armor_bone_leggings.iff
				elsif template_id = 701587845 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_bicep_l.iff
				elsif template_id = 1026806068 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_bicep_r.iff
				elsif template_id = 1306956967 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_bracer_l.iff
				elsif template_id = 2115381959 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_bracer_r.iff
				elsif template_id = 247689044 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_chest_plate.iff
				elsif template_id = 725211246 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_helmet.iff
				elsif template_id = -59921878 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/armor_chitin_s01_leggings.iff
				elsif template_id = -248317992 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_bicep_l.iff
				elsif template_id = -171534277 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_bicep_r.iff
				elsif template_id = -2062295640 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_bracer_l.iff
				elsif template_id = -253929242 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_bracer_r.iff
				elsif template_id = -2146951819 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_chest_plate.iff
				elsif template_id = 1042717208 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_helmet.iff
				elsif template_id = -175663469 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/chitin/energy_armor_chitin_leggings.iff
				elsif template_id = 2147059193 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_bicep_l.iff
				elsif template_id = 748768172 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_bicep_r.iff
				elsif template_id = 1551109695 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_bracer_l.iff
				elsif template_id = -1537444864 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_bracer_r.iff
				elsif template_id = -729206381 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_chest_plate.iff
				elsif template_id = 1231025524 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_helmet.iff
				elsif template_id = -1779280762 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/composite/armor_composite_leggings.iff
				elsif template_id = 729327903 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_bicep_l.iff
				elsif template_id = -255400918 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_l_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bicep_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_bicep_r.iff
				elsif template_id = -2146197063 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_r_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bicep_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_bracer_l.iff
				elsif template_id = -412867955 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_bracer_r.iff
				elsif template_id = -1749981410 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_chest_plate.iff
				elsif template_id = 1756768751 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_chest_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_helmet.iff
				elsif template_id = -286929665 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_helmet_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_defender/ith_armor_s01_leggings.iff
				elsif template_id = 1749090194 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_leggings_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_bicep_l.iff
				elsif template_id = 183123522 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_l_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_bicep_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_bicep_r.iff
				elsif template_id = 2050980817 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_r_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_bicep_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_bracer_l.iff
				elsif template_id = 1837916498 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_bracer_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_bracer_r.iff
				elsif template_id = 492381377 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_bracer_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_chest_plate.iff
				elsif template_id = 108052567 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_chest_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_helmet.iff
				elsif template_id = 364712723 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_helmet_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_guardian/ith_armor_s02_leggings.iff
				elsif template_id = -492258227 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_leggings_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_bicep_l.iff
				elsif template_id = 1926737359 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_l_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bicep_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_bicep_r.iff
				elsif template_id = 33744988 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_r_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bicep_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_bracer_l.iff
				elsif template_id = -1767695347 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bracer_l_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_bracer_r.iff
				elsif template_id = -428451426 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bracer_r_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_chest_plate.iff
				elsif template_id = -1520174324 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_chest_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_helmet.iff
				elsif template_id = -907953840 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_helmet_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ithorian_sentinel/ith_armor_s03_leggings.iff
				elsif template_id = 428327186 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_ith_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_leggings_ith_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_black_mtn/armor_kashyyykian_black_mtn_bracer_l.iff
				elsif template_id = -1877836182 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_l_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_black_mtn/armor_kashyyykian_black_mtn_bracer_r.iff
				elsif template_id = -523813895 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_r_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_black_mtn/armor_kashyyykian_black_mtn_chest_plate.iff
				elsif template_id = 66083222 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_chest_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_black_mtn/armor_kashyyykian_black_mtn_leggings.iff
				elsif template_id = 523674485 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_leggings_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_ceremonial/armor_kashyyykian_ceremonial_bracer_l.iff
				elsif template_id = 1974187109 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_bracer_l_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_ceremonial/armor_kashyyykian_ceremonial_bracer_r.iff
				elsif template_id = 92205558 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_bracer_r_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_ceremonial/armor_kashyyykian_ceremonial_chest_plate.iff
				elsif template_id = 856491201 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_chest_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_ceremonial/armor_kashyyykian_ceremonial_leggings.iff
				elsif template_id = -91264646 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_leggings_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_hunting/armor_kashyyykian_hunting_bracer_l.iff
				elsif template_id = 811367898 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bracer_l_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_hunting/armor_kashyyykian_hunting_bracer_r.iff
				elsif template_id = 1082734665 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_bracer_r_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_hunting/armor_kashyyykian_hunting_chest_plate.iff
				elsif template_id = 1193819287 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_chest_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_hunting/armor_kashyyykian_hunting_leggings.iff
				elsif template_id = -1082613563 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer15',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_wookie_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer15',2,'0.454545');
							AddScript('item.conversion.armor_leggings_wookie_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_bicep_l.iff
				elsif template_id = 525967921 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_bicep_r.iff
				elsif template_id = 1871505314 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_bracer_l.iff
				elsif template_id = -2050710967 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_bracer_r.iff
				elsif template_id = -183375910 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_chest_plate.iff
				elsif template_id = 689622929 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_helmet.iff
				elsif template_id = -78543095 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/armor_padded_s01_leggings.iff
				elsif template_id = 182484822 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_bicep_l.iff
				elsif template_id = -954052477 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bicep_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_bicep_r.iff
				elsif template_id = -1208543984 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bicep_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bicep_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_bracer_l.iff
				elsif template_id = -911886824 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_bracer_r.iff
				elsif template_id = -1183779957 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_chest_plate.iff
				elsif template_id = -343994769 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_helmet.iff
				elsif template_id = -40666442 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/padded/impact_armor_padded_leggings.iff
				elsif template_id = 1182886663 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_bicep_l.iff
				elsif template_id = 774778471 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.risDeconstruct',0,'1');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_ris');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_bicep_r.iff
				elsif template_id = 1593535476 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.risDeconstruct',0,'1');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_ris');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_bracer_l.iff
				elsif template_id = 582714478 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.risDeconstruct',0,'1');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_ris');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_bracer_r.iff
				elsif template_id = 1382990333 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.risDeconstruct',0,'1');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_ris');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_chest_plate.iff
				elsif template_id = -1832625049 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.risDeconstruct',0,'1');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_ris');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_helmet.iff
				elsif template_id = 1932522632 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.risDeconstruct',0,'1');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_ris');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/ris/armor_ris_leggings.iff
				elsif template_id = -1382065807 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.risDeconstruct',0,'1');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_ris');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.risDeconstruct',0,'1');
							AddScript('item.conversion.armor_ris');
						end if;
					end if;
				--object/tangible/wearables/armor/tantel/armor_tantel_skreej_chest_plate.iff
				elsif template_id = -653428823 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/tantel/armor_tantel_skreej_chest_plate_quest.iff
				elsif template_id = -1993339022 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/tantel/armor_tantel_skreej_helmet.iff
				elsif template_id = 1855229351 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/tantel/armor_tantel_skreej_helmet_quest.iff
				elsif template_id = 275564031 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_chest_plate.iff
				elsif template_id = -420003189 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_chest_plate_quest.iff
				elsif template_id = 309375847 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_helmet.iff
				elsif template_id = 1289593246 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_helmet_quest.iff
				elsif template_id = 103036882 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_pants.iff
				elsif template_id = 416123005 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/zam/armor_zam_wesell_pants_quest.iff
				elsif template_id = -469920034 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_bracer_l.iff
				elsif template_id = -779152232 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_l_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_bracer_l_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_bracer_r.iff
				elsif template_id = -1587916533 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_bracer_r_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_bracer_r_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_helmet.iff
				elsif template_id = 500476724 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_helmet_quest.iff
				elsif template_id = -320086751 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_helmet_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_helmet_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_jacket.iff
				elsif template_id = 678412834 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_jacket_quest.iff
				elsif template_id = 1436153762 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_chest_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_chest_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_pants.iff
				elsif template_id = -177674793 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/ubese/armor_ubese_pants_quest.iff
				elsif template_id = -1886457751 then
					if isFactory = TRUE then
						if armor_item_gp.exists(to_char(object_id + 1)) then
							n := armor_item_gp(to_char(object_id + 1));
							v := GetObjVarValue ('crafting.source_schematic');
							armor_crate_gp(v) := n;
							AddObjVar('crafting_attributes.crafting:general_protection',2,n);
							AddObjVar('crafting_components.scaled.layer14',2,'0.454545');
							AddObjVar('addArmorRefitScript',4,'item.conversion.armor_leggings_conversion');
						end if;
					else
						v := GetObjVarValue ('crafting.source_schematic');
						if v is not null and armor_crate_gp.exists(v) then
							n := armor_crate_gp(v);
							AddObjVar('armor.general_protection',2,n);
							AddObjVar('armor.layer14',2,'0.454545');
							AddScript('item.conversion.armor_leggings_conversion');
						end if;
					end if;
				--object/tangible/wearables/armor/kashyyykian_black_mtn/armor_kashyyykian_black_mtn_chest_plate.iff
				elsif template_id = 66083222 then
					AddObjVar('armor.wookieeDeconstruct',0,1);
					AddScript('item.conversion.armor_wookiee');
				--object/tangible/wearables/armor/kashyyykian_hunting/armor_kashyyykian_hunting_chest_plate.iff
				elsif template_id = 1193819287 then
					AddObjVar('armor.wookieeDeconstruct',0,1);
					AddScript('item.conversion.armor_wookiee');
				--object/tangible/wearables/armor/kashyyykian_ceremonial/armor_kashyyykian_ceremonial_chest_plate.iff
				elsif template_id = 856491201 then
					AddObjVar('armor.wookieeDeconstruct',0,1);
					AddScript('item.conversion.armor_wookiee');
-- ****** BEGIN ARMOR COMPONENT CONVERSION RULES ******
				--object/tangible/component/armor/armor_layer_nightsister.iff
				elsif template_id = 2086609620 then
					AddObjVar('attribute.bonus.0',0,'20');
				--object/tangible/component/armor/armor_layer_ris.iff
				elsif template_id = 110363758 then
					AddObjVar('attribute.bonus.0',0,'25');
				--object/tangible/component/armor/feather_peko_albatross.iff
				elsif template_id = -1958737672 then
					AddObjVar('attribute.bonus.0',0,'20');
				--object/tangible/component/armor/armor_segment_enhancement_dragonet.iff
				elsif template_id = 227937004 then
					AddObjVar('attribute.bonus.0',0,'5');
-- ****** BEGIN WEAPON CONVERSION RULES ******
				--component/blaster_power_handler
				elsif template_id = 702649693 then
					DeleteObjVar(prefix || 'woundChance');
				--component/blaster_power_handler_advanced
				elsif template_id = 34919568 then
					DeleteObjVar(prefix || 'woundChance');
				--component/chemical_dispersion_mechanism
				elsif template_id = -1382616459 then
					DeleteObjVar(prefix || 'attackSpeed');
					DeleteObjVar(prefix || 'woundChance');
				--component/chemical_dispersion_mechanism_advanced
				elsif template_id = -900058289 then
					DeleteObjVar(prefix || 'attackSpeed');
					DeleteObjVar(prefix || 'woundChance');
				--component/projectile_feed_mechanism
				elsif template_id = -1474613882 then
					DeleteObjVar(prefix || 'woundChance');
				--component/projectile_feed_mechanism_advanced
				elsif template_id = 590597969 then
					DeleteObjVar(prefix || 'woundChance');
				--component/scope_weapon
				elsif template_id = 403078591 then
					DeleteObjVar(prefix || 'minDamage');
					DeleteObjVar(prefix || 'maxDamage');
				--component/scope_weapon_advanced
				elsif template_id = -269033971 then
					DeleteObjVar(prefix || 'minDamage');
					DeleteObjVar(prefix || 'maxDamage');
				--component/stock
				elsif template_id = -651693115 then
					DeleteObjVar(prefix || 'minDamage');
					DeleteObjVar(prefix || 'maxDamage');
				--component/stock_advanced
				elsif template_id = -1571384798 then
					DeleteObjVar(prefix || 'minDamage');
					DeleteObjVar(prefix || 'maxDamage');
				--quest_rifle_projectile_tusken
				elsif template_id = 981088834 then
					DeleteObjVar('weapon.strCertUsed');
				--battleaxe
				elsif template_id = -637703116 then
					DeleteObjVar('weapon.strCertUsed');
				--battleaxe_quest
				elsif template_id = -1779174489 then
					DeleteObjVar('weapon.strCertUsed');
				--executioners_hack
				elsif template_id = 1840338268 then
					DeleteObjVar('weapon.strCertUsed');
				--cleaver
				elsif template_id = 237038605 then
					DeleteObjVar('weapon.strCertUsed');
				--katana
				elsif template_id = 653612816 then
					DeleteObjVar('weapon.strCertUsed');
				--katana_quest
				elsif template_id = 1608037452 then
					DeleteObjVar('weapon.strCertUsed');
				--maul
				elsif template_id = -613167286 then
					DeleteObjVar('weapon.strCertUsed');
				--2h_sword_scythe
				elsif template_id = -245341807 then
					DeleteObjVar('weapon.strCertUsed');
				--quest_battleaxe
				elsif template_id = -1019635633 then
					DeleteObjVar('weapon.strCertUsed');
				--quest_maul
				elsif template_id = -449112830 then
					DeleteObjVar('weapon.strCertUsed');
				--axe
				elsif template_id = -526113597 then
					DeleteObjVar('weapon.strCertUsed');
				--axe_vibroaxe
				elsif template_id = 589649861 then
					DeleteObjVar('weapon.strCertUsed');
				--baton_gaderiffi
				elsif template_id = -20005656 then
					DeleteObjVar('weapon.strCertUsed');
				--baton_stun
				elsif template_id = 257663710 then
					DeleteObjVar('weapon.strCertUsed');
				--victor_baton_gaderiffi
				elsif template_id = -639753330 then
					DeleteObjVar('weapon.strCertUsed');
				--knife_twilek_dagger
				elsif template_id = -730685311 then
					DeleteObjVar('weapon.strCertUsed');
				--knife_donkuwah
				elsif template_id = -130243512 then
					DeleteObjVar('weapon.strCertUsed');
				--knife_janta
				elsif template_id = 1756821101 then
					DeleteObjVar('weapon.strCertUsed');
				--knife_stone
				elsif template_id = -1501822488 then
					DeleteObjVar('weapon.strCertUsed');
				--knife_survival
				elsif template_id = -132487792 then
					DeleteObjVar('weapon.strCertUsed');
				--knife_vibroblade
				elsif template_id = 1982554682 then
					DeleteObjVar('weapon.strCertUsed');
				--knife_vibroblade_quest
				elsif template_id = -913097330 then
					DeleteObjVar('weapon.strCertUsed');
				--lance_vibro_controller_fp
				elsif template_id = -402333683 then
					DeleteObjVar('weapon.strCertUsed');
				--lance_vibro_nightsister
				elsif template_id = -1811261900 then
					DeleteObjVar('weapon.strCertUsed');
				--lance_nightsister
				elsif template_id = -1178706532 then
					DeleteObjVar('weapon.strCertUsed');
				--staff_janta
				elsif template_id = -1137089652 then
					DeleteObjVar('weapon.strCertUsed');
				--staff_metal
				elsif template_id = -473813792 then
					DeleteObjVar('weapon.strCertUsed');
				--staff
				elsif template_id = -718648728 then
					DeleteObjVar('weapon.strCertUsed');
				--staff_reinforced
				elsif template_id = 238936831 then
					DeleteObjVar('weapon.strCertUsed');
				--lance_vibro
				elsif template_id = 1358850609 then
					DeleteObjVar('weapon.strCertUsed');
				--axe_vibro
				elsif template_id = -1088166271 then
					DeleteObjVar('weapon.strCertUsed');
				--knuckler_vibro
				elsif template_id = 102348899 then
					DeleteObjVar('weapon.strCertUsed');
				--sword
				elsif template_id = -673780945 then
					DeleteObjVar('weapon.strCertUsed');
				--sword_curved
				elsif template_id = 213992376 then
					DeleteObjVar('weapon.strCertUsed');
				--sword_ryyk_blade
				elsif template_id = -1330392160 then
					DeleteObjVar('weapon.strCertUsed');
				--sword_curved_nyax
				elsif template_id = 1834662895 then
					DeleteObjVar('weapon.strCertUsed');
				--sword_nyax
				elsif template_id = -1781845673 then
					DeleteObjVar('weapon.strCertUsed');
				--sword_rantok
				elsif template_id = 1214765971 then
					DeleteObjVar('weapon.strCertUsed');
				--carbine_blaster_cdef
				elsif template_id = -1213235742 then
					DeleteObjVar('weapon.strCertUsed');
				--carbine_blaster_cdef_corsec
				elsif template_id = -1498919413 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_light_blaster_dh17_carbine
				elsif template_id = -1109444980 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_light_blaster_dh17_carbine_black
				elsif template_id = -379196495 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_light_blaster_dh17_carbine_snubnose
				elsif template_id = 648215202 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_disrupter_dxr6
				elsif template_id = -279076105 then
					DeleteObjVar('weapon.strCertUsed');
				--carbine_e11_mk2
				elsif template_id = -2138350593 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_light_blaster_e11_carbine_quest
				elsif template_id = -1839044042 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_light_blaster_e11_carbine_victor
				elsif template_id = -1601393927 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_light_blaster_ee3
				elsif template_id = 418740352 then
					DeleteObjVar('weapon.strCertUsed');
				--carbine_e5
				elsif template_id = 1778522271 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_light_blaster_laser_carbine
				elsif template_id = 764829944 then
					DeleteObjVar('weapon.strCertUsed');
				--carbine_nym_slugthrower
				elsif template_id = -627137144 then
					DeleteObjVar('weapon.strCertUsed');
				--heavy_acid_beam
				elsif template_id = 100102071 then
					DeleteObjVar('weapon.strCertUsed');
				--heavy_lightning_beam
				elsif template_id = 949442296 then
					DeleteObjVar('weapon.strCertUsed');
				--heavy_particle_beam
				elsif template_id = 180006972 then
					DeleteObjVar('weapon.strCertUsed');
				--heavy_rocket_launcher
				elsif template_id = -926822510 then
					DeleteObjVar('weapon.strCertUsed');
				--quest_heavy_acid_beam
				elsif template_id = -633473500 then
					DeleteObjVar('weapon.strCertUsed');
				--quest_heavy_particle_beam
				elsif template_id = 451466626 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_blaster_cdef
				elsif template_id = -82570045 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_blaster_cdef_corsec
				elsif template_id = -860173634 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_blaster_d18
				elsif template_id = 252668529 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_de_10
				elsif template_id = 1866781530 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_blaster_dh17
				elsif template_id = -245080147 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_blaster_dl44
				elsif template_id = 1684501073 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_blaster_dl44_metal
				elsif template_id = -1925049902 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_disrupter_dx2
				elsif template_id = -837691545 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_flechette_fwg5
				elsif template_id = -1792584100 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_flechette_fwg5_quest
				elsif template_id = -1340116130 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_geo_sonic_blaster
				elsif template_id = -1400940656 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_launcher
				elsif template_id = 1980615506 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_blaster_power5
				elsif template_id = 1687234741 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_republic_blaster
				elsif template_id = -2032619870 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_republic_blaster_quest
				elsif template_id = 1070818136 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_scatter
				elsif template_id = 322519616 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_blaster_scout_trooper
				elsif template_id = 129228399 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_blaster_scout_trooper_corsec
				elsif template_id = -1929515589 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_blaster_short_range_combat
				elsif template_id = 1691705957 then
					DeleteObjVar('weapon.strCertUsed');
				--pistol_projectile_striker
				elsif template_id = -1919723538 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_tanlge_gun7
				elsif template_id = -937385248 then
					DeleteObjVar('weapon.strCertUsed');
				--quest_pistol_launcher
				elsif template_id = 685231719 then
					DeleteObjVar('weapon.strCertUsed');
				--quest_pistol_republic_blaster_quest
				elsif template_id = -1340981329 then
					DeleteObjVar('weapon.strCertUsed');
				--quest_rifle_flame_thrower
				elsif template_id = -1238955746 then
					DeleteObjVar('weapon.strCertUsed');
				--quest_rifle_lightning
				elsif template_id = -656409820 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_acid_beam
				elsif template_id = -67541093 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_beam
				elsif template_id = 1223042704 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_berserker
				elsif template_id = 1646698389 then
					DeleteObjVar('weapon.strCertUsed');
				--bowcaster_assault
				elsif template_id = -1930572145 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_blaster_cdef
				elsif template_id = -1437726662 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_blaster_dlt20
				elsif template_id = -2118141076 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_blaster_dlt20a
				elsif template_id = -1354942232 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_blaster_e11
				elsif template_id = 709369610 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_flame_thrower
				elsif template_id = -205520309 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_blaster_ionization_jawa
				elsif template_id = -603889252 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_blaster_laser_rifle
				elsif template_id = -1321212581 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_lightning
				elsif template_id = 397924793 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_sonic_sg82
				elsif template_id = -1148251186 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_spray_stick_stohli
				elsif template_id = 2069010987 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_t21
				elsif template_id = -1374316705 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_disruptor_dxr6
				elsif template_id = -1428709338 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_projectile_tusken
				elsif template_id = -2024883534 then
					DeleteObjVar('weapon.strCertUsed');
				--rifle_victor_projectile_tusken
				elsif template_id = -896708611 then
					DeleteObjVar('weapon.strCertUsed');
				--bug_bomb
				elsif template_id = 1325615183 then
					DeleteObjVar('weapon.strCertUsed');
					ConvertAttackSpeed(rangeTableType(rangeType(100,120,100,120),rangeType(120,1000,100,100),rangeType(1000,1200,100,120)));
				--cryoban
				elsif template_id = 1007504804 then
					DeleteObjVar('weapon.strCertUsed');
					ConvertAttackSpeed(rangeTableType(rangeType(100,120,100,120),rangeType(120,1000,100,100),rangeType(1000,1200,100,120)));
				--cryoban_loot_medium
				elsif template_id = 1775069676 then
					DeleteObjVar('weapon.strCertUsed');
					ConvertAttackSpeed(rangeTableType(rangeType(100,120,100,120),rangeType(120,1000,100,100),rangeType(1000,1200,100,120)));
				--fragmentation
				elsif template_id = -1629773375 then
					DeleteObjVar('weapon.strCertUsed');
					ConvertAttackSpeed(rangeTableType(rangeType(100,120,100,120),rangeType(120,1000,100,100),rangeType(1000,1200,100,120)));
				--fragmentation_light
				elsif template_id = 1333331720 then
					DeleteObjVar('weapon.strCertUsed');
					ConvertAttackSpeed(rangeTableType(rangeType(100,120,100,120),rangeType(120,1000,100,100),rangeType(1000,1200,100,120)));
				--glop
				elsif template_id = 869624722 then
					DeleteObjVar('weapon.strCertUsed');
					ConvertAttackSpeed(rangeTableType(rangeType(100,120,100,120),rangeType(120,1000,100,100),rangeType(1000,1200,100,120)));
				--imperial_detonator
				elsif template_id = 247923643 then
					DeleteObjVar('weapon.strCertUsed');
					ConvertAttackSpeed(rangeTableType(rangeType(100,120,100,120),rangeType(120,1000,100,100),rangeType(1000,1200,100,120)));
				--proton
				elsif template_id = -631441882 then
					DeleteObjVar('weapon.strCertUsed');
					ConvertAttackSpeed(rangeTableType(rangeType(100,120,100,120),rangeType(120,1000,100,100),rangeType(1000,1200,100,120)));
				--thermal_detonator
				elsif template_id = -1328960537 then
					DeleteObjVar('weapon.strCertUsed');
					ConvertAttackSpeed(rangeTableType(rangeType(100,120,100,120),rangeType(120,1000,100,100),rangeType(1000,1200,100,120)));
				--object\weapon\melee\unarmed\unarmed_default_player.iff
				elsif template_id = 2131722719 then
					DeleteObjVar('weapon.strCertUsed');
					SetAttackSpeed(2);
					SetAttackCost(100);
					SetMinRange(0);
					SetMaxRange(5);
-- x****** BEGIN POWERUP CONVERSION RULES ******
-- ****** BEGIN SABER CONVERSION RULES ******
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_training.iff
				elsif template_id = 1530854406 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_one_handed_gen1.iff
				elsif template_id = -255701924 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_one_handed_gen2.iff
				elsif template_id = 735438027 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_one_handed_gen3.iff
				elsif template_id = 1658356550 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_one_handed_gen4.iff
				elsif template_id = -1724143534 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/sword/crafted_saber/sword_lightsaber_one_handed_gen5.iff
				elsif template_id = -801714209 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/polearm/crafted_saber/sword_lightsaber_polearm_gen1.iff
				elsif template_id = -181151130 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/polearm/crafted_saber/sword_lightsaber_polearm_gen2.iff
				elsif template_id = 774140657 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/polearm/crafted_saber/sword_lightsaber_polearm_gen3.iff
				elsif template_id = 1730745724 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/polearm/crafted_saber/sword_lightsaber_polearm_gen4.iff
				elsif template_id = -1664467352 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/2h_sword/crafted_saber/sword_lightsaber_two_handed_gen1.iff
				elsif template_id = 431995434 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/2h_sword/crafted_saber/sword_lightsaber_two_handed_gen2.iff
				elsif template_id = -1029172547 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/2h_sword/crafted_saber/sword_lightsaber_two_handed_gen3.iff
				elsif template_id = -1952090832 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
				--object/weapon/melee/2h_sword/crafted_saber/sword_lightsaber_two_handed_gen4.iff
				elsif template_id = 1883659812 then
					AddObjVar ('jedi.saber.base_stats.attack_cost', 0, 100);
					weapon_attribs.attack_cost := 100;
-- x****** BEGIN FOOD CONVERSION RULES ******
-- x****** BEGIN MEDICINE CONVERSION RULES ******
-- x****** BEGIN SPICE CONVERSION RULES ******
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
			'addArmorRefitScript',
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
			'armor.wookieeDeconstruct',
			'armorCategory',
			'armorLevel',
			'attribute.bonus.0',
			'biolink.faction_points',
			'blindChance',
			'blindDuration',
			'buff_name',
			'burnDuration',
			'burnIntensity',
			'commandName',
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
			'crafting_components.scaled.layer14',
			'crafting_components.scaled.layer15',
			'crafting_components.scaled.risDeconstruct',
			'crafting_components.woundChance',
			'crafting_components.zeroRangeMod',
			'draftSchematic',
			'duration',
			'effect_class',
			'effectiveness',
			'filling',
			'healing.combat_level_required',
			'healing.enhancement',
			'healing.power',
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
		
		cursor armorCrateCursor is
		select o.object_id, 
		decode (objvar_0_name, 'armor.general_protection',to_number(objvar_0_value),
		decode (objvar_1_name, 'armor.general_protection',to_number(objvar_1_value),
		decode (objvar_2_name, 'armor.general_protection',to_number(objvar_2_value),
		decode (objvar_3_name, 'armor.general_protection',to_number(objvar_3_value),
		decode (objvar_4_name, 'armor.general_protection',to_number(objvar_4_value),
		decode (objvar_5_name, 'armor.general_protection',to_number(objvar_5_value),
		decode (objvar_6_name, 'armor.general_protection',to_number(objvar_6_value),
		decode (objvar_7_name, 'armor.general_protection',to_number(objvar_7_value),
		decode (objvar_8_name, 'armor.general_protection',to_number(objvar_8_value),
		decode (objvar_9_name, 'armor.general_protection',to_number(objvar_9_value),
		decode (objvar_10_name, 'armor.general_protection',to_number(objvar_10_value),
		decode (objvar_11_name, 'armor.general_protection',to_number(objvar_11_value),
		decode (objvar_12_name, 'armor.general_protection',to_number(objvar_12_value),
		decode (objvar_13_name, 'armor.general_protection',to_number(objvar_13_value),
		decode (objvar_14_name, 'armor.general_protection',to_number(objvar_14_value),
		decode (objvar_15_name, 'armor.general_protection',to_number(objvar_15_value),
		decode (objvar_16_name, 'armor.general_protection',to_number(objvar_16_value),
		decode (objvar_17_name, 'armor.general_protection',to_number(objvar_17_value),
		decode (objvar_18_name, 'armor.general_protection',to_number(objvar_18_value),
		decode (objvar_19_name, 'armor.general_protection',to_number(objvar_19_value),
		v.value ))))) ))))) )))))  ))))) as gp_val
		from objects o, 
		(select object_id, max(value) as value from object_variables v 
				where v.name_id in (select id from object_variable_names where name = 'armor.general_protection')
				group by object_id) v 
		where contained_by = o.object_id - 1 
		and object_template_id in 
		(1386184862,575325453,1185807341,913882750,1994709940,301359835,-913758478,306929321,1654528826,-1493844838,-702482167,1360182635,-1831667261,701587845,1026806068,1306956967,2115381959,247689044,725211246,-59921878,-248317992,-171534277,-2062295640,-253929242,-2146951819,1042717208,-175663469,2147059193,748768172,1551109695,-1537444864,-729206381,1231025524,-1779280762,729327903,-255400918,-2146197063,-412867955,-1749981410,1756768751,-286929665,1749090194,183123522,2050980817,1837916498,492381377,108052567,364712723,-492258227,1926737359,33744988,-1767695347,-428451426,-1520174324,-907953840,428327186,-1877836182,-523813895,66083222,523674485,1974187109,92205558,856491201,-91264646,811367898,1082734665,1193819287,-1082613563,525967921,1871505314,-2050710967,-183375910,689622929,-78543095,182484822,-954052477,-1208543984,-911886824,-1183779957,-343994769,-40666442,1182886663,774778471,1593535476,582714478,1382990333,-1832625049,1932522632,-1382065807,-653428823,-1993339022,1855229351,275564031,-420003189,309375847,1289593246,103036882,416123005,-469920034,-779152232,-1587916533,500476724,-320086751,678412834,1436153762,-177674793,-1886457751)
		and conversion_id is not null
		and o.object_id = v.object_id(+);
		
		
		v varchar2(500);
		stmt varchar2(4000);
		objid number;
		gpval number;
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

		objvarNameIdMap('addArmorRefitScript') := 0;
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
		objvarNameIdMap('armor.wookieeDeconstruct') := 0;
		objvarNameIdMap('armorCategory') := 0;
		objvarNameIdMap('armorLevel') := 0;
		objvarNameIdMap('attribute.bonus.0') := 0;
		objvarNameIdMap('biolink.faction_points') := 0;
		objvarNameIdMap('blindChance') := 0;
		objvarNameIdMap('blindDuration') := 0;
		objvarNameIdMap('buff_name') := 0;
		objvarNameIdMap('burnDuration') := 0;
		objvarNameIdMap('burnIntensity') := 0;
		objvarNameIdMap('commandName') := 0;
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
		objvarNameIdMap('crafting_components.scaled.layer14') := 0;
		objvarNameIdMap('crafting_components.scaled.layer15') := 0;
		objvarNameIdMap('crafting_components.scaled.risDeconstruct') := 0;
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
		objvarNameIdMap('healing.combat_level_required') := 0;
		objvarNameIdMap('healing.enhancement') := 0;
		objvarNameIdMap('healing.power') := 0;
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
		templateIdSet(1185807341) := 1185807341;
		templateIdSet(913882750) := 913882750;
		templateIdSet(1994709940) := 1994709940;
		templateIdSet(301359835) := 301359835;
		templateIdSet(-913758478) := -913758478;
		templateIdSet(306929321) := 306929321;
		templateIdSet(1654528826) := 1654528826;
		templateIdSet(-1493844838) := -1493844838;
		templateIdSet(-702482167) := -702482167;
		templateIdSet(1360182635) := 1360182635;
		templateIdSet(-1831667261) := -1831667261;
		templateIdSet(701587845) := 701587845;
		templateIdSet(1026806068) := 1026806068;
		templateIdSet(1306956967) := 1306956967;
		templateIdSet(2115381959) := 2115381959;
		templateIdSet(247689044) := 247689044;
		templateIdSet(725211246) := 725211246;
		templateIdSet(-59921878) := -59921878;
		templateIdSet(-248317992) := -248317992;
		templateIdSet(-171534277) := -171534277;
		templateIdSet(-2062295640) := -2062295640;
		templateIdSet(-253929242) := -253929242;
		templateIdSet(-2146951819) := -2146951819;
		templateIdSet(1042717208) := 1042717208;
		templateIdSet(-175663469) := -175663469;
		templateIdSet(2147059193) := 2147059193;
		templateIdSet(748768172) := 748768172;
		templateIdSet(1551109695) := 1551109695;
		templateIdSet(-1537444864) := -1537444864;
		templateIdSet(-729206381) := -729206381;
		templateIdSet(1231025524) := 1231025524;
		templateIdSet(-1779280762) := -1779280762;
		templateIdSet(729327903) := 729327903;
		templateIdSet(-255400918) := -255400918;
		templateIdSet(-2146197063) := -2146197063;
		templateIdSet(-412867955) := -412867955;
		templateIdSet(-1749981410) := -1749981410;
		templateIdSet(1756768751) := 1756768751;
		templateIdSet(-286929665) := -286929665;
		templateIdSet(1749090194) := 1749090194;
		templateIdSet(183123522) := 183123522;
		templateIdSet(2050980817) := 2050980817;
		templateIdSet(1837916498) := 1837916498;
		templateIdSet(492381377) := 492381377;
		templateIdSet(108052567) := 108052567;
		templateIdSet(364712723) := 364712723;
		templateIdSet(-492258227) := -492258227;
		templateIdSet(1926737359) := 1926737359;
		templateIdSet(33744988) := 33744988;
		templateIdSet(-1767695347) := -1767695347;
		templateIdSet(-428451426) := -428451426;
		templateIdSet(-1520174324) := -1520174324;
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
		templateIdSet(525967921) := 525967921;
		templateIdSet(1871505314) := 1871505314;
		templateIdSet(-2050710967) := -2050710967;
		templateIdSet(-183375910) := -183375910;
		templateIdSet(689622929) := 689622929;
		templateIdSet(-78543095) := -78543095;
		templateIdSet(182484822) := 182484822;
		templateIdSet(-954052477) := -954052477;
		templateIdSet(-1208543984) := -1208543984;
		templateIdSet(-911886824) := -911886824;
		templateIdSet(-1183779957) := -1183779957;
		templateIdSet(-343994769) := -343994769;
		templateIdSet(-40666442) := -40666442;
		templateIdSet(1182886663) := 1182886663;
		templateIdSet(774778471) := 774778471;
		templateIdSet(1593535476) := 1593535476;
		templateIdSet(582714478) := 582714478;
		templateIdSet(1382990333) := 1382990333;
		templateIdSet(-1832625049) := -1832625049;
		templateIdSet(1932522632) := 1932522632;
		templateIdSet(-1382065807) := -1382065807;
		templateIdSet(-653428823) := -653428823;
		templateIdSet(-1993339022) := -1993339022;
		templateIdSet(1855229351) := 1855229351;
		templateIdSet(275564031) := 275564031;
		templateIdSet(-420003189) := -420003189;
		templateIdSet(309375847) := 309375847;
		templateIdSet(1289593246) := 1289593246;
		templateIdSet(103036882) := 103036882;
		templateIdSet(416123005) := 416123005;
		templateIdSet(-469920034) := -469920034;
		templateIdSet(-779152232) := -779152232;
		templateIdSet(-1587916533) := -1587916533;
		templateIdSet(500476724) := 500476724;
		templateIdSet(-320086751) := -320086751;
		templateIdSet(678412834) := 678412834;
		templateIdSet(1436153762) := 1436153762;
		templateIdSet(-177674793) := -177674793;
		templateIdSet(-1886457751) := -1886457751;
		templateIdSet(2086609620) := 2086609620;
		templateIdSet(110363758) := 110363758;
		templateIdSet(-1958737672) := -1958737672;
		templateIdSet(227937004) := 227937004;
		templateIdSet(702649693) := 702649693;
		templateIdSet(34919568) := 34919568;
		templateIdSet(-1382616459) := -1382616459;
		templateIdSet(-900058289) := -900058289;
		templateIdSet(-1474613882) := -1474613882;
		templateIdSet(590597969) := 590597969;
		templateIdSet(403078591) := 403078591;
		templateIdSet(-269033971) := -269033971;
		templateIdSet(-651693115) := -651693115;
		templateIdSet(-1571384798) := -1571384798;
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
		templateIdSet(2131722719) := 2131722719;
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
		

				
		open armorCrateCursor;
		fetch armorCrateCursor into  objid, gpval;
		while armorCrateCursor%found loop
			if gpval is not null then
				armor_item_gp(to_char(objid)) := gpval;			  	
			end if;
			fetch armorCrateCursor into objid, gpval;
		end loop;
		close armorCrateCursor;

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
		if instr(script_list,newScript) = 0 then
			script_list := trim(script_list) ||  newScript || ':';
			isObjModified := TRUE;
		end if;
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
			weapon_attribs.attack_speed := ConvertValue(weapon_attribs.attack_speed*10, rangeTable)/10;
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
