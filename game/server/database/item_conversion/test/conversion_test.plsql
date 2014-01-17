create or replace package body conversion_test
as
	type ObjvarsType is table of cnvtest_objvars%rowtype index by varchar2(500);
	objvars ObjvarsType;
	type RemovedScriptsType is varray(100) of varchar2(102);
	removedScripts RemovedScriptsType;
	maxErrors number := 0;
	numErrors number;
	isInitialized boolean := false;

	procedure setupData;
	procedure checkObjVar (object_id number, name varchar2, value varchar2);
	procedure registerErrorWithTemplateId (p_object_id number, p_template_id number, p_description varchar2);
	procedure registerError (p_object_id number, p_description varchar2);

	procedure testObjects as
	begin
		setupData;

		for objectRow in (select * from objects)
		loop
			if (removedScripts.count <> 0) then
				for i in removedScripts.first .. removedScripts.last
				loop
					if (objectRow.script_list like removedScripts(i)) then
						registerErrorWithTemplateId (objectRow.object_id, objectRow.object_template_id, 'has deprecated scripts (objects table)');
						exit;
					end if;
				end loop;
			end if;

			checkObjVar(objectRow.object_id, objectRow.objvar_0_name, objectRow.objvar_0_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_1_name, objectRow.objvar_1_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_2_name, objectRow.objvar_2_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_3_name, objectRow.objvar_3_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_4_name, objectRow.objvar_4_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_5_name, objectRow.objvar_5_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_6_name, objectRow.objvar_6_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_7_name, objectRow.objvar_7_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_8_name, objectRow.objvar_8_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_9_name, objectRow.objvar_9_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_10_name, objectRow.objvar_10_value); 
			checkObjVar(objectRow.object_id, objectRow.objvar_11_name, objectRow.objvar_11_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_12_name, objectRow.objvar_12_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_13_name, objectRow.objvar_13_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_14_name, objectRow.objvar_14_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_15_name, objectRow.objvar_15_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_16_name, objectRow.objvar_16_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_17_name, objectRow.objvar_17_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_18_name, objectRow.objvar_18_value);
			checkObjVar(objectRow.object_id, objectRow.objvar_19_name, objectRow.objvar_19_value);		
		end loop;
	end;

	procedure testObjvars
	as
	begin
		setupData;

		for objvarRow in (select distinct object_id, co.deprecated
			from object_variables ov, cnvtest_objvars co
			where ov.name_id = co.id
			and (co.deprecated = 1
				or case when translate(ov.value, 'a0123456789','a') is not null then null else to_number(ov.value) end < co.min_value
				or case when translate(ov.value, 'a0123456789','a') is not null then null else to_number(ov.value) end > co.max_value))
		loop
			if (objvarRow.deprecated = 1) then
				registerError(objvarRow.object_id, 'has deprecated objvars (object variables table)');
			else
				registerError(objvarRow.object_id, 'has objvars out of range (object variables table)');
			end if;
		end loop;
	end;

	procedure testScripts
	as
	begin
		setupData;

		for scriptRow in (select distinct object_id
			from scripts s, cnvtest_removed_scripts r
			where s.script=r.name)
		loop
			registerError(scriptRow.object_id, 'has deprecated scripts (scripts table)');
		end loop;
	end;

	procedure testAll
	as
	begin
		testObjvars;
		commit;
		testObjects;
		commit;
		testScripts;
		commit;
	end;

	procedure setupData
	as
	begin
		if isInitialized = false then
			dbms_output.enable(20000);

			delete cnvtest_objvars;

			insert into cnvtest_objvars
			select name, id, deprecated, min_value, max_value
			from object_variable_names ovn, cnvtest_objvar_patterns cop
			where ovn.name like cop.name_pattern
			and ovn.name not in (
				'crafting_components.armor_armorencumbrance',
				'crafting_components.armor_condition',
				'crafting_components.armor_general_protection',
				'crafting_components.armor_layer0',
				'crafting_components.armor_layer1',
				'crafting_components.armor_layer12',
				'crafting_components.armor_layer2',
				'crafting_components.armor_layer4',
				'crafting_components.armor_layer5',
				'crafting_components.armor_layer7'
			);

			commit;

			objvars.delete;
			for objvarRow in (select * from cnvtest_objvars)
			loop
				objvars(objvarRow.name) := objvarRow;
			end loop;

			select * bulk collect into removedScripts from cnvtest_removed_scripts;
			if (removedScripts.count <> 0) then
				for i in removedScripts.first .. removedScripts.last
				loop
					removedScripts(i) := '%' || removedScripts(i) || ':%';
				end loop;
			end if;

			numErrors := 0;

			execute immediate 'analyze table cnvtest_objvars compute statistics';
			execute immediate 'analyze table cnvtest_removed_scripts compute statistics';
		end if;
	end;

	procedure checkObjVar (object_id number, name varchar2, value varchar2)
	as
	begin
		if (objvars.exists(name)) then
			if (objvars(name).deprecated = 1) then
				registerError(object_id, 'has deprecated objvars (objects table)');
			else
				if (value < objvars(name).min_value or value > objvars(name).max_value) then
					registerError(object_id, 'has objvars out of range (objects table)');
				end if; 
			end if;
		end if;

		exception when others then
			registerError(object_id, 'has objvars that were expected to be numeric but were not (objects table)');
	end;

	procedure registerError (p_object_id number, p_description varchar2)
	as
		templateId number;
	begin
		select object_template_id into templateId from objects where object_id = p_object_id;
		
		registerErrorWithTemplateId (p_object_id, templateId, p_description);

	exception when no_data_found then
		registerErrorWithTemplateId (p_object_id, 0, p_description || ', not found in objects table');
	end;

	procedure registerErrorWithTemplateId (p_object_id number, p_template_id number, p_description varchar2)
	as
		templateName varchar2(255);
		previousMessage varchar2(500);
		l_description varchar2(500);
	begin
		l_description := p_description;
		
		if (p_template_id <> 0) then
			begin
				select name into templateName from object_templates where id = p_template_id;

			exception when no_data_found then
				templateName := p_template_id;
				l_description := l_description || ', object template id not found in object_templates';
			end;
		else
			templateName := 'none';
		end if;

		insert into cnvtest_errors
		values (p_object_id, templateName, l_description);

		numErrors := numErrors + 1;
		if (maxErrors<>0 and numErrors >= maxErrors) then
			commit;
			raise_application_error(-20000,'Too many errors found');
		end if;		

	exception when dup_val_on_index then
		update cnvtest_errors set error_description = error_description || ', ' || l_description
		where cnvtest_errors.object_id=p_object_id;
	end;
end;
/
show errors
