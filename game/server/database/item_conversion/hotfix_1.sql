CREATE OR REPLACE PACKAGE  "HOTFIX_1" as
	procedure StartItemConversion (rbs_to_use varchar2 default null, conversion_id number default 1);
	procedure DoSingleObject (objectId number, conversion_id number default 1);
end;
/

CREATE OR REPLACE PACKAGE BODY  "HOTFIX_1" as
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
	
	-- private lookup tables
	templateIdSet   numTableNumber;			-- set of template ids to be converted
	objvarNameIdMap numTableVarchar;		-- map from ObjVar name to OBJECT_VARIABLES_NAMES ID
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

	procedure SetGenProtection(rangeTable rangeTableType);
	procedure SetArmorCondLevelCat(condition varchar2, lev varchar2, category varchar2);
	procedure SetArmorLevel(lev varchar);
	procedure SetArmorCategory(category varchar2);
	procedure SetGpAndCond(gp_name varchar2, gp_val varchar2, cond_name varchar2, cond_val varchar2);
	
	procedure DeleteCraftingComponents;
	
	function ConvertValue (oldvalue number, rangeTable rangeTableType) return number;

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
		and object_template_id in (-448030425, 1838042928, 2082588198, 1741906969, 1120303977, -1207917085)
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

		while objCursor%found loop
			begin
				-- Clear Data
				free_slots.delete;
				free_slots := numTable(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20);
				slot_map.delete;
				for i in 1..20 loop
					Obj_ObjVars(i) := blankObjVar;
				end loop;
				OV_ObjVars.delete;
				updated_objvars.trim(updated_objvars.count);
				deleted_objvars.trim(deleted_objvars.count);
				added_objvars.trim(added_objvars.count);
				isObjModified := FALSE;
				isSchematic := FALSE;
				isFactory := FALSE;
				prefix := 'crafting_components.';
				new_schematic_id := 0;
				
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
					free_slots.delete(1);
				end if;

				Obj_ObjVars(2).object_id := objRow.object_id;
				Obj_ObjVars(2).objvar_name := objRow.objvar_1_name;
				Obj_ObjVars(2).objvar_type := objRow.objvar_1_type;
				Obj_ObjVars(2).objvar_value := objRow.objvar_1_value;
				if ((objRow.objvar_1_name is not null) and (objRow.objvar_1_type != -1))then
					slot_map(objRow.objvar_1_name) := 2;
					free_slots.delete(2);
				end if;

				Obj_ObjVars(3).object_id := objRow.object_id;
				Obj_ObjVars(3).objvar_name := objRow.objvar_2_name;
				Obj_ObjVars(3).objvar_type := objRow.objvar_2_type;
				Obj_ObjVars(3).objvar_value := objRow.objvar_2_value;
				if ((objRow.objvar_2_name is not null) and (objRow.objvar_2_type != -1))then
					slot_map(objRow.objvar_2_name) := 3;
					free_slots.delete(3);
				end if;

				Obj_ObjVars(4).object_id := objRow.object_id;
				Obj_ObjVars(4).objvar_name := objRow.objvar_3_name;
				Obj_ObjVars(4).objvar_type := objRow.objvar_3_type;
				Obj_ObjVars(4).objvar_value := objRow.objvar_3_value;
				if ((objRow.objvar_3_name is not null) and (objRow.objvar_3_type != -1))then
					slot_map(objRow.objvar_3_name) := 4;
					free_slots.delete(4);
				end if;

				Obj_ObjVars(5).object_id := objRow.object_id;
				Obj_ObjVars(5).objvar_name := objRow.objvar_4_name;
				Obj_ObjVars(5).objvar_type := objRow.objvar_4_type;
				Obj_ObjVars(5).objvar_value := objRow.objvar_4_value;
				if ((objRow.objvar_4_name is not null) and (objRow.objvar_4_type != -1))then
					slot_map(objRow.objvar_4_name) := 5;
					free_slots.delete(5);
				end if;

				Obj_ObjVars(6).object_id := objRow.object_id;
				Obj_ObjVars(6).objvar_name := objRow.objvar_5_name;
				Obj_ObjVars(6).objvar_type := objRow.objvar_5_type;
				Obj_ObjVars(6).objvar_value := objRow.objvar_5_value;
				if ((objRow.objvar_5_name is not null) and (objRow.objvar_5_type != -1))then
					slot_map(objRow.objvar_5_name) := 6;
					free_slots.delete(6);
				end if;

				Obj_ObjVars(7).object_id := objRow.object_id;
				Obj_ObjVars(7).objvar_name := objRow.objvar_6_name;
				Obj_ObjVars(7).objvar_type := objRow.objvar_6_type;
				Obj_ObjVars(7).objvar_value := objRow.objvar_6_value;
				if ((objRow.objvar_6_name is not null) and (objRow.objvar_6_type != -1))then
					slot_map(objRow.objvar_6_name) := 7;
					free_slots.delete(7);
				end if;

				Obj_ObjVars(8).object_id := objRow.object_id;
				Obj_ObjVars(8).objvar_name := objRow.objvar_7_name;
				Obj_ObjVars(8).objvar_type := objRow.objvar_7_type;
				Obj_ObjVars(8).objvar_value := objRow.objvar_7_value;
				if ((objRow.objvar_7_name is not null) and (objRow.objvar_7_type != -1))then
					slot_map(objRow.objvar_7_name) := 8;
					free_slots.delete(8);
				end if;

				Obj_ObjVars(9).object_id := objRow.object_id;
				Obj_ObjVars(9).objvar_name := objRow.objvar_8_name;
				Obj_ObjVars(9).objvar_type := objRow.objvar_8_type;
				Obj_ObjVars(9).objvar_value := objRow.objvar_8_value;
				if ((objRow.objvar_8_name is not null) and (objRow.objvar_8_type != -1))then
					slot_map(objRow.objvar_8_name) := 9;
					free_slots.delete(9);
				end if;

				Obj_ObjVars(10).object_id := objRow.object_id;
				Obj_ObjVars(10).objvar_name := objRow.objvar_9_name;
				Obj_ObjVars(10).objvar_type := objRow.objvar_9_type;
				Obj_ObjVars(10).objvar_value := objRow.objvar_9_value;
				if ((objRow.objvar_9_name is not null) and (objRow.objvar_9_type != -1))then
					slot_map(objRow.objvar_9_name) := 10;
					free_slots.delete(10);
				end if;

				Obj_ObjVars(11).object_id := objRow.object_id;
				Obj_ObjVars(11).objvar_name := objRow.objvar_10_name;
				Obj_ObjVars(11).objvar_type := objRow.objvar_10_type;
				Obj_ObjVars(11).objvar_value := objRow.objvar_10_value;
				if ((objRow.objvar_10_name is not null) and (objRow.objvar_10_type != -1))then
					slot_map(objRow.objvar_10_name) := 11;
					free_slots.delete(11);
				end if;

				Obj_ObjVars(12).object_id := objRow.object_id;
				Obj_ObjVars(12).objvar_name := objRow.objvar_11_name;
				Obj_ObjVars(12).objvar_type := objRow.objvar_11_type;
				Obj_ObjVars(12).objvar_value := objRow.objvar_11_value;
				if ((objRow.objvar_11_name is not null) and (objRow.objvar_11_type != -1))then
					slot_map(objRow.objvar_11_name) := 12;
					free_slots.delete(12);
				end if;

				Obj_ObjVars(13).object_id := objRow.object_id;
				Obj_ObjVars(13).objvar_name := objRow.objvar_12_name;
				Obj_ObjVars(13).objvar_type := objRow.objvar_12_type;
				Obj_ObjVars(13).objvar_value := objRow.objvar_12_value;
				if ((objRow.objvar_12_name is not null) and (objRow.objvar_12_type != -1))then
					slot_map(objRow.objvar_12_name) := 13;
					free_slots.delete(13);
				end if;

				Obj_ObjVars(14).object_id := objRow.object_id;
				Obj_ObjVars(14).objvar_name := objRow.objvar_13_name;
				Obj_ObjVars(14).objvar_type := objRow.objvar_13_type;
				Obj_ObjVars(14).objvar_value := objRow.objvar_13_value;
				if ((objRow.objvar_13_name is not null) and (objRow.objvar_13_type != -1))then
					slot_map(objRow.objvar_13_name) := 14;
					free_slots.delete(14);
				end if;

				Obj_ObjVars(15).object_id := objRow.object_id;
				Obj_ObjVars(15).objvar_name := objRow.objvar_14_name;
				Obj_ObjVars(15).objvar_type := objRow.objvar_14_type;
				Obj_ObjVars(15).objvar_value := objRow.objvar_14_value;
				if ((objRow.objvar_14_name is not null) and (objRow.objvar_14_type != -1))then
					slot_map(objRow.objvar_14_name) := 15;
					free_slots.delete(15);
				end if;

				Obj_ObjVars(16).object_id := objRow.object_id;
				Obj_ObjVars(16).objvar_name := objRow.objvar_15_name;
				Obj_ObjVars(16).objvar_type := objRow.objvar_15_type;
				Obj_ObjVars(16).objvar_value := objRow.objvar_15_value;
				if ((objRow.objvar_15_name is not null) and (objRow.objvar_15_type != -1))then
					slot_map(objRow.objvar_15_name) := 16;
					free_slots.delete(16);
				end if;

				Obj_ObjVars(17).object_id := objRow.object_id;
				Obj_ObjVars(17).objvar_name := objRow.objvar_16_name;
				Obj_ObjVars(17).objvar_type := objRow.objvar_16_type;
				Obj_ObjVars(17).objvar_value := objRow.objvar_16_value;
				if ((objRow.objvar_16_name is not null) and (objRow.objvar_16_type != -1))then
					slot_map(objRow.objvar_16_name) := 17;
					free_slots.delete(17);
				end if;

				Obj_ObjVars(18).object_id := objRow.object_id;
				Obj_ObjVars(18).objvar_name := objRow.objvar_17_name;
				Obj_ObjVars(18).objvar_type := objRow.objvar_17_type;
				Obj_ObjVars(18).objvar_value := objRow.objvar_17_value;
				if ((objRow.objvar_17_name is not null) and (objRow.objvar_17_type != -1))then
					slot_map(objRow.objvar_17_name) := 18;
					free_slots.delete(18);
				end if;

				Obj_ObjVars(19).object_id := objRow.object_id;
				Obj_ObjVars(19).objvar_name := objRow.objvar_18_name;
				Obj_ObjVars(19).objvar_type := objRow.objvar_18_type;
				Obj_ObjVars(19).objvar_value := objRow.objvar_18_value;
				if ((objRow.objvar_18_name is not null) and (objRow.objvar_18_type != -1))then
					slot_map(objRow.objvar_18_name) := 19;
					free_slots.delete(19);
				end if;

				Obj_ObjVars(20).object_id := objRow.object_id;
				Obj_ObjVars(20).objvar_name := objRow.objvar_19_name;
				Obj_ObjVars(20).objvar_type := objRow.objvar_19_type;
				Obj_ObjVars(20).objvar_value := objRow.objvar_19_value;
				if ((objRow.objvar_19_name is not null) and (objRow.objvar_19_type != -1))then
					slot_map(objRow.objvar_19_name) := 20;
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
					end if;
					fetch objVarCursor into objVarRow;
				end loop;

				-- check to see if it a weapon (162889564) or armor (1120303977) factory crate
				-- or food (-200781577) or chemicals (-1966544754)
				-- or electronics (-754298423) or clothing (-1207917085)
				if template_id = 162889564 or template_id = 1120303977 
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
   				if template_id = -448030425 then
					SetGpAndCond(prefix || 'general_protection', '0.3', prefix || 'condition', '0.2');
					if isFactory = TRUE then
						ModifyObjVar('draftSchematic','draftSchematic',0,1861772866);
						AddScript('systems.crafting.armor.component.crafting_new_armor_segment');
					else
						SetNewTemplateId(1594162622);		--object/tangible/component/armor/armor_segment_recon.iff
   						RemoveScript('systems.crafting.armor.component.crafting_armor_component_attribute');
   						RemoveScript('item.armor.new_armor');  -- to prevent two copies of same script
   						AddScript('item.armor.new_armor');
 					end if;
   				elsif template_id = 1838042928 then
					RenameObjVar(prefix || 'armor_general_protection',prefix || 'general_protection');
					RenameObjVar(prefix || 'armor_condition',prefix || 'condition');
					if isFactory = FALSE then
						SetNewTemplateId(-1438161694);		--object/tangible/component/armor/armor_segment_battle.iff
   					end if;
   				elsif template_id = 2082588198 then
					RenameObjVar(prefix || 'armor_general_protection',prefix || 'general_protection');
					RenameObjVar(prefix || 'armor_condition',prefix || 'condition');
					if isFactory = FALSE then
						SetNewTemplateId(2042493214);		--object/tangible/component/armor/armor_segment_assault.iff
   					end if;
   				elsif template_id = 1741906969 then
					RenameObjVar(prefix || 'armor_general_protection',prefix || 'general_protection');
					RenameObjVar(prefix || 'armor_condition',prefix || 'condition');
					if isFactory = FALSE then
						SetNewTemplateId(1594162622);		--object/tangible/component/armor/armor_segment_recon.iff
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

				cvt := cvt + 1;
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
	
		
		templateNameText(1594162622)  := 'armor_recon_segment';
		templateNameTable(1594162622) := 'craft_armor_ingredients_n';
		templateNameText(-1438161694) := 'armor_battle_segment';
		templateNameTable(-1438161694):= 'craft_armor_ingredients_n';
		templateNameText(2042493214)  := 'armor_assault_segment';
		templateNameTable(2042493214) := 'craft_armor_ingredients_n';
		
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
