#!/usr/bin/perl

&main;

# ======================================================================

sub main
{
	open(INFILE, "<item_conversion.sql.template");
	open(OUTFILE, ">item_conversion.sql");
	print "Generating item_conversion.sql\n";
	while ($line = <INFILE>)
	{
		print OUTFILE $line;
		if ($line eq "-- ****** BEGIN ARMOR CONVERSION RULES ******\n")
		{
			&DoArmorConversion;
		}
		if ($line eq "-- ****** BEGIN ARMOR COMPONENT CONVERSION RULES ******\n")
		{
			&DoArmorComponentConversion;
		}
		if ($line eq "-- ****** BEGIN WEAPON CONVERSION RULES ******\n")
		{
			&DoWeaponConversion;
		}
		if ($line eq "-- ****** BEGIN POWERUP CONVERSION RULES ******\n")
		{
			&DoPowerupConversion;
		}
		if ($line eq "-- ****** BEGIN SABER CONVERSION RULES ******\n")
		{
			&DoSaberConversion;
		}
		if ($line eq "-- ****** BEGIN FOOD CONVERSION RULES ******\n")
		{
			&DoFoodConversion;
		}
		if ($line eq "-- ****** BEGIN MEDICINE CONVERSION RULES ******\n")
		{
			&DoMedicineConversion;
		}
		if ($line eq "-- ****** BEGIN SPICE CONVERSION RULES ******\n")
		{
			&DoSpiceConversion;
		}
		if ($line eq "-- ****** BEGIN TEMPLATE ID SET ******\n")
		{
			&DoTemplateIdSet;
		}
	}
	close(OUTFILE);
	close(INFILE);
}

sub DoArmorConversion
{
	open(ARMOR, "armor_conversion.txt");
	$if = "if";
	<ARMOR>;
	while ($line = <ARMOR>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $script_add, $script_remove, $objvar_remove, $effective, $gen_protect, 
			$cond, $level, $category, $add_objvar,
			$new_temp_id, $new_template, $craft_script, $schematic, $craft_objvar) =
			split /\t/, $line;

		if ($temp_id ne "")
		{		
			print OUTFILE "\t\t\t\t--$template\n";
			print OUTFILE "\t\t\t\t$if template_id = $temp_id then\n";
			$if = "elsif";
			print OUTFILE "\t\t\t\t\tif isSchematic = TRUE then\n";
			print OUTFILE "\t\t\t\t\t\tInvalidateSchematic;\n";
			print OUTFILE "\t\t\t\t\telse\n";

			if ($effective ne "") {
				print OUTFILE "\t\t\t\t\t\tSetGenProtection(rangeTableType(";
				@eff = (split /,/,$effective);
				@gnp = (split /,/,$gen_protect);
				for ($i=0; $i < @eff-1; $i++) {
					print OUTFILE "rangeType(".(split /:/,($eff[$i]))[0].",".(split /:/,($eff[$i]))[1].
						",".(split /:/,($gnp[$i]))[0].",".(split /:/,($gnp[$i]))[1]."),";
				}
				print OUTFILE "rangeType(".(split /:/,($eff[$i]))[0].",".(split /:/,($eff[$i]))[1].
					",".(split /:/,($gnp[$i]))[0].",".(split /:/,($gnp[$i]))[1]."))";
				print OUTFILE ");\n";
			}

			print OUTFILE "\t\t\t\t\t\tDeleteOldArmorObjvars;\n"; 
			print OUTFILE "\t\t\t\t\t\tDeleteObjVar('$_');\n" 
				foreach split /,/, $objvar_remove;	

			if (($cond ne "") && ($level ne "") && ($category ne "")) {
					print OUTFILE "\t\t\t\t\t\tSetArmorCondLevelCat('$cond','$level','$category');\n";
			}
			else {
				if ($cond ne "") {
					print OUTFILE "\t\t\t\t\t\tAddObjVar('armor.condition',2,'$cond');\n";
				}
				if ($level ne "") {
					print OUTFILE "\t\t\t\t\t\tSetArmorLevel('$level');\n";
				}
				if ($category ne "") {
					print OUTFILE "\t\t\t\t\t\tSetArmorCategory('$category');\n";
				}
			}

			print OUTFILE "\t\t\t\t\t\tRemoveScript('$_');\n" 
				foreach split /,/, $script_remove;	

			if ($craft_script ne "") {
				print OUTFILE "\t\t\t\t\t\tif isFactory = TRUE then\n";
					print OUTFILE "\t\t\t\t\t\t\tAddObjVar$_;\n" 
						foreach split /:/, $craft_objvar;	

					if ($schematic ne "") {
						print OUTFILE "\t\t\t\t\t\t\tModifyObjVar('draftSchematic','draftSchematic',0,$schematic);\n";
					}

				print OUTFILE "\t\t\t\t\t\t\tAddScript('$_');\n"
					foreach split /,/, $craft_script;	


				if (($new_temp_id ne "") || ($add_objvar ne "") || ($script_remove ne "") || ($script_add ne "")) {
					print OUTFILE "\t\t\t\t\t\telse\n";

					if ($new_temp_id ne "") {
						print OUTFILE "\t\t\t\t\t\tSetNewTemplateId($new_temp_id);\t\t--$new_template\n";
					}

					print OUTFILE "\t\t\t\t\t\t\tAddObjVar$_;\n" 
						foreach split /:/, $add_objvar;	

					print OUTFILE "\t\t\t\t\t\t\tAddScript('$_');\n"
						foreach split /,/, $script_add;	
				}
				print OUTFILE "\t\t\t\t\t\tend if;\n";
			}
			else
			{
				if ($new_temp_id ne "") {
					print OUTFILE "\t\t\t\t\tSetNewTemplateId($new_temp_id);\t\t--$new_template\n";
				}

				print OUTFILE "\t\t\t\t\t\tAddObjVar$_;\n" 
					foreach split /:/, $add_objvar;	

				print OUTFILE "\t\t\t\t\t\tAddScript('$_');\n"
					foreach split /,/, $script_add;	
			}
			print OUTFILE "\t\t\t\t\tend if;\n";
		}
	}
	close(ARMOR);
}

sub DoArmorComponentConversion
{
	open(COMPONENT, "armor_component_conversion.txt");
	<COMPONENT>;
	while ($line = <COMPONENT>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $new_template, $new_temp_id, $script_remove, $script_add, 
			$objvar_remove, $gen_protect, $cond, $layer_value, $attrib_bonus, $crate_script, $schematic) =
			split /\t/, $line;

		print OUTFILE "\t\t\t\t--$template\n";
		print OUTFILE "\t\t\t\telsif template_id = $temp_id then\n";
		$if = "elsif";
		print OUTFILE "\t\t\t\t\tif isSchematic = TRUE then\n";
		print OUTFILE "\t\t\t\t\t\tInvalidateSchematic;\n";
		print OUTFILE "\t\t\t\t\telse\n";

		print OUTFILE "\t\t\t\t\t\tDeleteObjVar(prefix || '$_');\n" 
			foreach split /,/, $objvar_remove;	

		if (($gen_protect ne "") && ($cond ne "")) {
			print OUTFILE "\t\t\t\t\t\tSetGpAndCond(prefix || 'general_protection', '$gen_protect', prefix || 'condition', '$cond');\n";
		}
		else {
			if ($gen_protect ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar(prefix || 'general_protection',2,'$gen_protect');\n";
			}
			if ($cond ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar(prefix || 'condition',2,'$cond');\n";
			}
		}
		if ($layer_value ne "") {
			print OUTFILE "\t\t\t\t\t\tAddObjVar(prefix || 'armor_layer".(split /,/,$layer_value)[0]."',2,'".(split /,/,$layer_value)[1]."');\n";
		}
		
		if (($schematic ne "") || ($crate_script ne "")) {
			print OUTFILE "\t\t\t\t\t\tif isFactory = TRUE then\n";

			if ($schematic ne "") {
				print OUTFILE "\t\t\t\t\t\t\tModifyObjVar('draftSchematic','draftSchematic',0,$schematic);\n";
			}

			if ($crate_script ne "") {
				print OUTFILE "\t\t\t\t\t\t\tscript_list := '$crate_script' || ':';\n";
				print OUTFILE "\t\t\t\t\t\t\tisObjModified := TRUE;\n";
			}
				;	
			if (($new_temp_id ne "") || ($attrib_bonus ne "") || ($script_remove ne "") || ($script_add ne "")) {
				print OUTFILE "\t\t\t\t\t\telse\n";
				if ($new_temp_id ne "") {
					print OUTFILE "\t\t\t\t\t\t\tSetNewTemplateId($new_temp_id);\t\t--$new_template\n";
				}
				print OUTFILE "\t\t\t\t\t\t\tAddObjVar('attribute.bonus.".(split /:/,$_)[0]."',2,'".(split /:/,$_)[1]."');\n"
					foreach split /,/, $attrib_bonus;	

				print OUTFILE "\t\t\t\t\t\t\tRemoveScript('$_');\n" 
					foreach split /,/, $script_remove;	

				print OUTFILE "\t\t\t\t\t\t\tAddScript('$_');\n" 
					foreach split /,/, $script_add;	

			}
				print OUTFILE "\t\t\t\t\t\tend if;\n";
		}
		else {
			if (($new_temp_id ne "") || ($attrib_bonus ne "") || ($script_remove ne "") || ($script_add ne "")) {
				if ($new_temp_id ne "") {
					print OUTFILE "\t\t\t\t\t\tSetNewTemplateId($new_temp_id);\t\t--$new_template\n";
				}
				print OUTFILE "\t\t\t\t\t\tAddObjVar('attribute.bonus.".(split /:/,$_)[0]."',2,'".(split /:/,$_)[1]."');\n"
					foreach split /,/, $attrib_bonus;	

				print OUTFILE "\t\t\t\t\t\tRemoveScript('$_');\n" 
					foreach split /,/, $script_remove;	

				print OUTFILE "\t\t\t\t\t\tAddScript('$_');\n" 
					foreach split /,/, $script_add;	

			}
		}

		print OUTFILE "\t\t\t\t\tend if;\n";
	}
	close(COMPONENT);
}

sub DoWeaponConversion
{
	open(WEAPON, "weapon_conversion.txt");
	<WEAPON>;
	while ($line = <WEAPON>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $min_dmg_from, $min_dmg_to, $max_dmg_from, $max_dmg_to, 
			$speed_from, $speed_to, $wound_from, $wound_to, 
			$attack_cost, $accuracy,
			$min_range_to, $max_range_to,
			$damage_type, $ele_type, $ele_value, $add_objvar) =
			split /\t/, $line;
			
		if ($temp_id ne "") {
			if ($line =~ m/component/)
			{
				print OUTFILE "\t\t\t\t--$template\n";
				print OUTFILE "\t\t\t\telsif template_id = $temp_id then\n";

				if (($min_dmg_from ne "") && ($min_dmg_to ne "")){
					print OUTFILE "\t\t\t\t\tModifyObjVar(prefix || 'minDamage',prefix || 'minDamage',2,";
					print OUTFILE "rangeTableType(";
					@f = (split /,/,$min_dmg_from);
					@t = (split /,/,$min_dmg_to);
					for ($i=0; $i < @f-1; $i++) {
						if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
							print OUTFILE "\nCONVERSION DATA ERROR!!\n";
						}
						print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
							",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
					}
					if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
						print OUTFILE "\nCONVERSION DATA ERROR!!\n";
					}
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."))";
					print OUTFILE ");\n";
				}
				if (($max_dmg_from ne "") && ($max_dmg_to ne "")){
					print OUTFILE "\t\t\t\t\tModifyObjVar(prefix || 'maxDamage',prefix || 'maxDamage',2,";
					print OUTFILE "rangeTableType(";
					@f = (split /,/,$max_dmg_from);
					@t = (split /,/,$max_dmg_to);
					for ($i=0; $i < @f-1; $i++) {
						if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
							print OUTFILE "\nCONVERSION DATA ERROR!!\n";
						}
						print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
							",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
					}
					if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
						print OUTFILE "\nCONVERSION DATA ERROR!!\n";
					}
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."))";
					print OUTFILE ");\n";
				}
				if ($speed_from ne "") {
					print OUTFILE "\t\t\t\t\tModifyObjVar(prefix || 'attackSpeed',prefix || 'attackSpeed',2,";
					print OUTFILE "rangeTableType(";
					@f = (split /,/,$speed_from);
					@t = (split /,/,$speed_to);
					for ($i=0; $i < @f-1; $i++) {
						if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
							print OUTFILE "\nCONVERSION DATA ERROR!!\n";
						}
						print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
							",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
					}
					if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
						print OUTFILE "\nCONVERSION DATA ERROR!!\n";
					}
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."))";
					print OUTFILE ");\n";
				}
				if ($wound_from ne "") {
					print OUTFILE "\t\t\t\t\tModifyObjVar(prefix || 'woundChance',prefix || 'woundChance',2,";
					print OUTFILE "rangeTableType(";
					@f = (split /,/,$wound_from);
					@t = (split /,/,$wound_to);
					for ($i=0; $i < @f-1; $i++) {
						if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
							print OUTFILE "\nCONVERSION DATA ERROR!!\n";
						}
						print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
							",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
					}
					if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
						print OUTFILE "\nCONVERSION DATA ERROR!!\n";
					}
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."))";
					print OUTFILE ");\n";
				}
				if (($attack_cost ne "") && ($accuracy ne "") && ($min_range_to ne "") && ($max_range_to ne "") &&($damage_type ne "")) {
					print OUTFILE "\t\t\t\t\tSetWeaponBaseStats($attack_cost,$accuracy,$min_range_to,$max_range_to,$damage_type);\n";
				}
				else {
					if ($attack_cost ne "") {
						print OUTFILE "\t\t\t\t\tSetAttackCost($attack_cost);\n";
					}
					if ($accuracy ne "") {
						print OUTFILE "\t\t\t\t\tSetAccuracy($accuracy);\n";
					}
					if ($min_range_to ne "") {
						print OUTFILE "\t\t\t\t\tSetMinRange($min_range_to);\n";
					}
					if ($max_range_to ne "") {
						print OUTFILE "\t\t\t\t\tSetMaxRange($max_range_to);\n";
					}
					if ($damage_type ne "") {
						print OUTFILE "\t\t\t\t\tSetDamageType($damage_type);\n";
					}
				}
				if ($ele_type ne "") {
					print OUTFILE "\t\t\t\t\tSetElementalType($ele_type);\n";
				}
				if ($ele_value ne "") {
					print OUTFILE "\t\t\t\t\tSetElementalValue($ele_value);\n";
				}
				print OUTFILE "\t\t\t\t\tAddObjVar$_;\n" 
					foreach split /:/, $add_objvar;	

				print OUTFILE "\t\t\t\t\tif isSchematic = TRUE or isFactory = TRUE then\n";
				print OUTFILE "\t\t\t\t\t\tDeleteCraftingComponents;\n";
				print OUTFILE "\t\t\t\t\tend if;\n";
			}
			else
			{
				print OUTFILE "\t\t\t\t--$template\n";
				print OUTFILE "\t\t\t\telsif template_id = $temp_id then\n";
				print OUTFILE "\t\t\t\t\tHandleWeaponDots;\n";
				
				if ($min_dmg_from ne "") {
					print OUTFILE "\t\t\t\t\tConvertMinDamage(";
					print OUTFILE "rangeTableType(";
					@f = (split /,/,$min_dmg_from);
					@t = (split /,/,$min_dmg_to);
					for ($i=0; $i < @f-1; $i++) {
						if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
							print OUTFILE "\nCONVERSION DATA ERROR!!\n";
						}
						print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
							",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
					}
					if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
						print OUTFILE "\nCONVERSION DATA ERROR!!\n";
					}
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1].")));\n";
				}
				if ($max_dmg_from ne "") {
					print OUTFILE "\t\t\t\t\tConvertMaxDamage(";
					print OUTFILE "rangeTableType(";
					@f = (split /,/,$max_dmg_from);
					@t = (split /,/,$max_dmg_to);
					for ($i=0; $i < @f-1; $i++) {
						if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
							print OUTFILE "\nCONVERSION DATA ERROR!!\n";
						}
						print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
							",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
					}
					if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
						print OUTFILE "\nCONVERSION DATA ERROR!!\n";
					}
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1].")));\n";
				}
				if ($speed_from ne "") {
					print OUTFILE "\t\t\t\t\tConvertAttackSpeed(";
					print OUTFILE "rangeTableType(";
					@f = (split /,/,$speed_from);
					@t = (split /,/,$speed_to);
					for ($i=0; $i < @f-1; $i++) {
						if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
							print OUTFILE "\nCONVERSION DATA ERROR!!\n";
						}
						print OUTFILE "rangeType(".((split /:/,($f[$i]))[0]*10).",".((split /:/,($f[$i]))[1]*10).
							",".((split /:/,($t[$i]))[0]*100).",".((split /:/,($t[$i]))[1]*100)."),";
					}
					if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
						print OUTFILE "\nCONVERSION DATA ERROR!!\n";
					}
					print OUTFILE "rangeType(".((split /:/,($f[$i]))[0]*10).",".((split /:/,($f[$i]))[1]*10).
						",".((split /:/,($t[$i]))[0]*100).",".((split /:/,($t[$i]))[1]*100).")));\n";
				}
				if ($wound_from ne "") {
					print OUTFILE "\t\t\t\t\tConvertWoundChance(";
					print OUTFILE "rangeTableType(";
					@f = (split /,/,$wound_from);
					@t = (split /,/,$wound_to);
					for ($i=0; $i < @f-1; $i++) {
						if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
							print OUTFILE "\nCONVERSION DATA ERROR!!\n";
						}
						print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
							",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
					}
					if ( ((split /:/,($t[$i]))[0]) > ((split /:/,($t[$i]))[1]) ) {
						print OUTFILE "\nCONVERSION DATA ERROR!!\n";
					}
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1].")));\n";
				}

				if (($attack_cost ne "") && ($accuracy ne "") && ($min_range_to ne "") && ($max_range_to ne "") &&($damage_type ne "")) {
					print OUTFILE "\t\t\t\t\tSetWeaponBaseStats($attack_cost,$accuracy,$min_range_to,$max_range_to,$damage_type);\n";
				}
				else {
					if ($attack_cost ne "") {
						print OUTFILE "\t\t\t\t\tSetAttackCost($attack_cost);\n";
					}
					if ($accuracy ne "") {
						print OUTFILE "\t\t\t\t\tSetAccuracy($accuracy);\n";
					}
					if ($min_range_to ne "") {
						print OUTFILE "\t\t\t\t\tSetMinRange($min_range_to);\n";
					}
					if ($max_range_to ne "") {
						print OUTFILE "\t\t\t\t\tSetMaxRange($max_range_to);\n";
					}
					if ($damage_type ne "") {
						print OUTFILE "\t\t\t\t\tSetDamageType($damage_type);\n";
					}
				}
				if ($ele_type ne "") {
					print OUTFILE "\t\t\t\t\tSetElementalType($ele_type);\n";
				}
				if ($ele_value ne "") {
					print OUTFILE "\t\t\t\t\tSetElementalValue($ele_value);\n";
				}
				print OUTFILE "\t\t\t\t\tAddObjVar$_;\n" 
					foreach split /:/, $add_objvar;	

				if ($temp_id == -1930572145) {
					print OUTFILE "\t\t\t\t\tif isSchematic = TRUE or isFactory = TRUE then\n";
					print OUTFILE "\t\t\t\t\t\tDeleteCraftingComponents;\n";
					print OUTFILE "\t\t\t\t\t\tif isSchematic = TRUE then\n";
					print OUTFILE "\t\t\t\t\t\t\tDeleteObjVar('crafting_attributes.crafting:*');\n";
					print OUTFILE "\t\t\t\t\t\t\tnew_schematic_id := -847903116;\n";
					print OUTFILE "\t\t\t\t\t\telse\n";
					print OUTFILE "\t\t\t\t\t\t\tModifyObjVar('draftSchematic','draftSchematic',0,-847903116);\n";
					print OUTFILE "\t\t\t\t\t\tend if;\n";
					print OUTFILE "\t\t\t\t\t\tSetTemplateText(-1631144444);\n";
					print OUTFILE "\t\t\t\t\telse\n";
					print OUTFILE "\t\t\t\t\t\tSetNewTemplateId(-1631144444);  --convert to heavy bowcaster\n";
					print OUTFILE "\t\t\t\t\t\tAddScript('systems.combat.combat_weapon');\n";
					print OUTFILE "\t\t\t\t\tend if;\n";
				}
				elsif ($temp_id == -2138350593) {
					print OUTFILE "\t\t\t\t\tif isSchematic = TRUE or isFactory = TRUE then\n";
					print OUTFILE "\t\t\t\t\t\tDeleteCraftingComponents;\n";
					print OUTFILE "\t\t\t\t\t\tif isSchematic = TRUE then\n";
					print OUTFILE "\t\t\t\t\t\t\tDeleteObjVar('crafting_attributes.crafting:*');\n";
					print OUTFILE "\t\t\t\t\t\t\tnew_schematic_id := -1108671633;\n";
					print OUTFILE "\t\t\t\t\t\telse\n";
					print OUTFILE "\t\t\t\t\t\t\tModifyObjVar('draftSchematic','draftSchematic',0,-1108671633);\n";
					print OUTFILE "\t\t\t\t\t\tend if;\n";
					print OUTFILE "\t\t\t\t\t\tSetTemplateText(-746051337);\n";
					print OUTFILE "\t\t\t\t\telse\n";
					print OUTFILE "\t\t\t\t\t\tSetNewTemplateId(-746051337);  --convert to E11 Mark II\n";
					print OUTFILE "\t\t\t\t\t\tAddScript('systems.combat.combat_weapon');\n";
					print OUTFILE "\t\t\t\t\tend if;\n";
				}
				else {
					print OUTFILE "\t\t\t\t\tif isSchematic = TRUE or isFactory = TRUE then\n";
					print OUTFILE "\t\t\t\t\t\tDeleteCraftingComponents;\n";
					print OUTFILE "\t\t\t\t\t\tif isSchematic = TRUE then\n";
					print OUTFILE "\t\t\t\t\t\t\tDeleteObjVar('crafting_attributes.crafting:*');\n";
					print OUTFILE "\t\t\t\t\t\tend if;\n";
					print OUTFILE "\t\t\t\t\telse\n";
					print OUTFILE "\t\t\t\t\t\tAddScript('systems.combat.combat_weapon');\n";
					print OUTFILE "\t\t\t\t\tend if;\n";
				}
			}
		}
	}
	close(WEAPON);
}

sub DoPowerupConversion
{
	open(POWERUP, "powerup_conversion.txt");
	<POWERUP>;
	while ($line = <POWERUP>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $conversion) =
			split /\t/, $line;
		if ($conversion ne "")
		{		
			print OUTFILE "\t\t\t\t--$template\n";
			print OUTFILE "\t\t\t\telsif template_id = $temp_id then\n";
			print OUTFILE "\t\t\t\t\t$conversion;\n";
		}
	}
	close(POWERUP);
}

sub DoSaberConversion
{
	open(SABER, "saber_conversion.txt");
	<SABER>;
	while ($line = <SABER>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $min_dmg_from, $min_dmg_to, $max_dmg_from, $max_dmg_to, 
			$speed_from, $speed_to, $wound_from, $wound_to, 
			$attack_cost, $accuracy,
			$min_range_to, $max_range_to,
			$damage_type, $ele_type, $ele_value, $force) =
			split /\t/, $line;
			
		if ($line =~ m/object\/tangible\/component/)
		{
			print OUTFILE "\t\t\t\t--$template\n";
			print OUTFILE "\t\t\t\telsif template_id = $temp_id then\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.crystal.stats.mid_rng');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.crystal.stats.zero_mod');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.crystal.stats.min_mod');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.crystal.stats.mid_mod');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.crystal.stats.max_mod');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.crystal.stats.action');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.crystal.stats.health');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.crystal.stats.mind');\n";

			print OUTFILE "\t\t\t\t\tif not(slot_map.exists('jedi.crystal.stats.color')) and slot_map.exists('jedi.crystal.owner.name') then\n";

			if ($min_dmg_from ne "") {
				print OUTFILE "\t\t\t\t\t\tn := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.min_dmg'),0)),";
				print OUTFILE "rangeTableType(";
				@f = (split /,/,$min_dmg_from);
				@t = (split /,/,$min_dmg_to);
				for ($i=0; $i < @f-1; $i++) {
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
				}
				print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
					",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1].")));\n";
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.min_dmg', 0, n);\n";
			}

			if ($max_dmg_from ne "") {
				print OUTFILE "\t\t\t\t\t\tn := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.max_dmg'),0)),";
				print OUTFILE "rangeTableType(";
				@f = (split /,/,$max_dmg_from);
				@t = (split /,/,$max_dmg_to);
				for ($i=0; $i < @f-1; $i++) {
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
				}
				print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
					",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1].")));\n";
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.max_dmg', 0, n);\n";
			}

			if ($speed_from ne "") {
				print OUTFILE "\t\t\t\t\t\tn := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.speed'),100)),";
				print OUTFILE "rangeTableType(";
				@f = (split /,/,$speed_from);
				@t = (split /,/,$speed_to);
				for ($i=0; $i < @f-1; $i++) {
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
				}
				print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
					",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1].")));\n";
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.speed', 2, n);\n";
			}

			if ($wound_from ne "") {
				print OUTFILE "\t\t\t\t\t\tn := ConvertValue(to_number(nvl(GetObjVarValue('jedi.crystal.stats.wound'),0)),";
				print OUTFILE "rangeTableType(";
				@f = (split /,/,$wound_from);
				@t = (split /,/,$wound_to);
				for ($i=0; $i < @f-1; $i++) {
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
				}
				print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
					",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1].")));\n";
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.wound', 2, n);\n";
			}

			if ($attack_cost ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.attack_cost', 2, $attack_cost);\n";
			}
			if ($force ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.force', 2, $force);\n";
			}
			if ($accuracy ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.accuracy', 2, $accuracy);\n";
			}
			if ($min_range_to ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.min_rng', 2, $min_range_to);\n";
			}
			if ($max_range_to ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.max_rng', 2, $max_range_to);\n";
			}
			if ($damage_type ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.damage_type', 0, $damage_type);\n";
			}
			if ($ele_type ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.elemental_type', 0, $ele_type);\n";
			}
			if ($ele_value ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar ('jedi.crystal.stats.elemental_value', 0, $ele_value);\n";
			}
			print OUTFILE "\t\t\t\t\tend if;\n";
		}
		else
		{
			print OUTFILE "\t\t\t\t--$template\n";
			print OUTFILE "\t\t\t\telsif template_id = $temp_id then\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.saber.base_stats.mid_rng');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.saber.base_stats.zero_mod');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.saber.base_stats.min_mod');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.saber.base_stats.mid_mod');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.saber.base_stats.max_mod');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.saber.base_stats.action');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.saber.base_stats.health');\n";
			print OUTFILE "\t\t\t\t\tDeleteObjVar('jedi.saber.base_stats.mind');\n";

			if ($min_dmg_from ne "") {
				print OUTFILE "\t\t\t\t\tn := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.min_dmg'),0)),";
				print OUTFILE "rangeTableType(";
				@f = (split /,/,$min_dmg_from);
				@t = (split /,/,$min_dmg_to);
				for ($i=0; $i < @f-1; $i++) {
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
				}
				print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
					",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1].")));\n";
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.min_dmg', 0, n);\n";
				print OUTFILE "\t\t\t\t\tweapon_attribs.min_damage := n;\n";
			}

			if ($max_dmg_from ne "") {
				print OUTFILE "\t\t\t\t\tn := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.max_dmg'),0)),";
				print OUTFILE "rangeTableType(";
				@f = (split /,/,$max_dmg_from);
				@t = (split /,/,$max_dmg_to);
				for ($i=0; $i < @f-1; $i++) {
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
				}
				print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
					",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1].")));\n";
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.max_dmg', 0, n);\n";
				print OUTFILE "\t\t\t\t\tweapon_attribs.max_damage := n;\n";
			}

			if ($speed_from ne "") {
				print OUTFILE "\t\t\t\t\tn := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.speed'),100)),";
				print OUTFILE "rangeTableType(";
				@f = (split /,/,$speed_from);
				@t = (split /,/,$speed_to);
				for ($i=0; $i < @f-1; $i++) {
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
				}
				print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
					",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1].")));\n";
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.speed', 2, n);\n";
				print OUTFILE "\t\t\t\t\tweapon_attribs.attack_speed := n;\n";
			}

			if ($wound_from ne "") {
				print OUTFILE "\t\t\t\t\tn := ConvertValue(to_number(nvl(GetObjVarValue('jedi.saber.base_stats.wound'),0)),";
				print OUTFILE "rangeTableType(";
				@f = (split /,/,$wound_from);
				@t = (split /,/,$wound_to);
				for ($i=0; $i < @f-1; $i++) {
					print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
						",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1]."),";
				}
				print OUTFILE "rangeType(".(split /:/,($f[$i]))[0].",".(split /:/,($f[$i]))[1].
					",".(split /:/,($t[$i]))[0].",".(split /:/,($t[$i]))[1].")));\n";
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.wound', 2, n);\n";
				print OUTFILE "\t\t\t\t\tweapon_attribs.wound_chance := n;\n";
			}

			if ($attack_cost ne "") {
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.attack_cost', 2, $attack_cost);\n";
				print OUTFILE "\t\t\t\t\tweapon_attribs.attack_cost := $attack_cost;\n";
			}
			if ($force ne "") {
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.force', 2, $force);\n";
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.force', 2, $force);\n";
			}
			if ($accuracy ne "") {
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.accuracy', 2, $accuracy);\n";
				print OUTFILE "\t\t\t\t\tweapon_attribs.accuracy := $accuracy;\n";
			}
			if ($min_range_to ne "") {
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.min_rng', 2, $min_range_to);\n";
				print OUTFILE "\t\t\t\t\tweapon_attribs.min_range := $min_range_to;\n";
			}
			if ($max_range_to ne "") {
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.max_rng', 2, $max_range_to);\n";
				print OUTFILE "\t\t\t\t\tweapon_attribs.max_range := $max_range_to;\n";
			}
			if ($damage_type ne "") {
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.damage_type', 0, $damage_type);\n";
				print OUTFILE "\t\t\t\t\tweapon_attribs.damage_type := $damage_type;\n";
			}
			if ($ele_type ne "") {
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.elemental_type', 0, $ele_type);\n";
				print OUTFILE "\t\t\t\t\tweapon_attribs.elemental_type := $ele_type;\n";
			}
			if ($ele_value ne "") {
				print OUTFILE "\t\t\t\t\tAddObjVar ('jedi.saber.base_stats.elemental_value', 0, $ele_value);\n";
				print OUTFILE "\t\t\t\t\tweapon_attribs.elemental_value := $ele_value;\n";
			}
			print OUTFILE "\t\t\t\t\tAddScript('systems.combat.combat_weapon');\n";
			
		}
	}
	close(SABER);
}

sub DoFoodConversion
{
	open(FOOD, "food_conversion.txt");
	<FOOD>;
	while ($line = <FOOD>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $buff_name, $eff_objvar, $effectiveness, $dur_objvar, $duration, 
			$objvar_remove, $script_remove, $script_add) =
			split /\t/, $line;
		if ($effectiveness ne "")
		{		
			print OUTFILE "\t\t\t\t--$template\n";
			print OUTFILE "\t\t\t\telsif template_id = $temp_id then\n";
			print OUTFILE "\t\t\t\t\tif isSchematic = TRUE then\n";
			print OUTFILE "\t\t\t\t\t\tHandleFoodSchematic;\n";
			print OUTFILE "\t\t\t\t\telsif isFactory = TRUE then\n";
			print OUTFILE "\t\t\t\t\t\tHandleFoodCrate;\n";
			print OUTFILE "\t\t\t\t\telse\n";

			if ($buff_name ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar('buff_name',4,'$buff_name');\n";
			}

			if ($eff_objvar ne "") {
				print OUTFILE "\t\t\t\t\t\tif slot_map.exists('$eff_objvar') then\n";
				print OUTFILE "\t\t\t\t\t\t\tModifyObjVar('$eff_objvar','effectiveness',2,rangeTableType(rangeType(0,".($effectiveness*2).",0.0,2.0)));\n";
				print OUTFILE "\t\t\t\t\t\telse\n";
				print OUTFILE "\t\t\t\t\t\t\tAddObjVar('effectiveness',2,'0.98');\n";
				print OUTFILE "\t\t\t\t\t\tend if;\n";
			}
			elsif ($effectiveness ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar('effectiveness',2,'$effectiveness');\n";
			}

			if ($dur_objvar ne "") {
				print OUTFILE "\t\t\t\t\t\tif slot_map.exists('$dur_objvar') then\n";
				print OUTFILE "\t\t\t\t\t\t\tModifyObjVar('$dur_objvar','duration',2,rangeTableType(rangeType(0,".($duration*2).",0.0,2.0)));\n";
				print OUTFILE "\t\t\t\t\t\telse\n";
				print OUTFILE "\t\t\t\t\t\t\tAddObjVar('duration',2,'0.9');\n";
				print OUTFILE "\t\t\t\t\t\tend if;\n";
			}
			elsif ($duration ne "") {
				print OUTFILE "\t\t\t\t\t\tAddObjVar('duration',2,'$duration');\n";
			}

			print OUTFILE "\t\t\t\t\t\tDeleteObjVar('$_');\n" 
				foreach split /,/, $objvar_remove;	

			print OUTFILE "\t\t\t\t\t\tRemoveScript('$_');\n" 
				foreach split /,/, $script_remove;	

			print OUTFILE "\t\t\t\t\t\tAddScript('$_');\n" 
				foreach split /,/, $script_add;	

			print OUTFILE "\t\t\t\t\t\tRenameObjVar('consumable.stomachValues','filling');\n";
			print OUTFILE "\t\t\t\t\tend if;\n";
		}
	}
	close(FOOD);
}

sub DoMedicineConversion
{
	open(MEDS, "medicine_conversion.txt");
	<MEDS>;
	while ($line = <MEDS>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $stim_pack) =
			split /\t/, $line;
		if ($stim_pack ne "")
		{		
			print OUTFILE "\t\t\t\t--$template\n";
			print OUTFILE "\t\t\t\telsif template_id = $temp_id then\n";
			print OUTFILE "\t\t\t\t\t$stim_pack;\n";
		}
	}
	close(MEDS);
}

sub DoSpiceConversion
{
	open(SPICE, "spice_conversion.txt");
	<SPICE>;
	while ($line = <SPICE>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $spice_name) =
			split /\t/, $line;
		if ($spice_name ne "")
		{		
			print OUTFILE "\t\t\t\t--$template\n";
			print OUTFILE "\t\t\t\telsif template_id = $temp_id then\n";
			print OUTFILE "\t\t\t\t\tConvertSpice('$spice_name');\n";
		}
	}
	close(SPICE);
}

sub DoTemplateIdSet
{
	open(ARMOR, "armor_conversion.txt");
	<ARMOR>;
	while ($line = <ARMOR>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $script_add, $objvar_remove, $effective, $gen_protect, 
			$encumb, $encumb_split, $cond, $level, $category, $special, $layer) =
			split /\t/, $line;
		if ($temp_id ne "")
		{		
			print OUTFILE "\t\ttemplateIdSet($temp_id) := $temp_id;\n";
		}
	}
	close(ARMOR);

	open(COMPONENT, "armor_component_conversion.txt");
	<COMPONENT>;
	while ($line = <COMPONENT>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $new_template, $new_temp_id, $script_remove, $script_add, 
			$objvar_remove, $gen_protect, $encumb, $cond, $layer_value, $attrib_bonus, $cnt) =
			split /\t/, $line;
		if ($temp_id ne "")
		{		
			print OUTFILE "\t\ttemplateIdSet($temp_id) := $temp_id;\n";
		}
	}
	close(COMPONENT);

	open(WEAPON, "weapon_conversion.txt");
	<WEAPON>;
	while ($line = <WEAPON>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $min_dmg_from, $min_dmg_to, $max_dmg_from, $max_dmg_to, 
			$speed_from, $speed_to, $wound_from, $wound_to, 
			$attack_cost, $accuracy,
			$min_range_to, $max_range_to,
			$damage_type, $ele_type, $ele_value) =
			split /\t/, $line;
			
		if ($temp_id ne "")
		{		
			print OUTFILE "\t\ttemplateIdSet($temp_id) := $temp_id;\n";
		}
	}
	close(WEAPON);

	open(POWERUP, "powerup_conversion.txt");
	<POWERUP>;
	while ($line = <POWERUP>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $conversion) =
			split /\t/, $line;
		if ($conversion ne "")
		{		
			print OUTFILE "\t\ttemplateIdSet($temp_id) := $temp_id;\n";
		}
	}
	close(POWERUP);

	open(SABER, "saber_conversion.txt");
	<SABER>;
	while ($line = <SABER>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $min_dmg_from, $min_dmg_to, $max_dmg_from, $max_dmg_to, 
			$speed_from, $speed_to, $wound_from, $wound_to, 
			$attack_cost, $accuracy,
			$min_range_to, $max_range_to,
			$damage_type, $ele_type, $ele_value, $force) =
			split /\t/, $line;
		print OUTFILE "\t\ttemplateIdSet($temp_id) := $temp_id;\n";
	}
	close(SABER);

	open(FOOD, "food_conversion.txt");
	<FOOD>;
	while ($line = <FOOD>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $buff_name, $eff_objvar, $effectiveness, $dur_objvar, $duration, 
			$objvar_remove, $script_remove, $script_add) =
			split /\t/, $line;
		if ($effectiveness ne "")
		{		
			print OUTFILE "\t\ttemplateIdSet($temp_id) := $temp_id;\n";
		}
	}
	close(FOOD);

	open(MEDS, "medicine_conversion.txt");
	<MEDS>;
	while ($line = <MEDS>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $stim_pack) =
			split /\t/, $line;
		if ($stim_pack ne "")
		{		
			print OUTFILE "\t\ttemplateIdSet($temp_id) := $temp_id;\n";
		}
	}
	close(MEDS);

	open(SPICE, "spice_conversion.txt");
	<SPICE>;
	while ($line = <SPICE>)
	{
		$line =~ s/\"|\n|\[|\]//g;
		($temp_id, $template, $spice_name) =
			split /\t/, $line;
		if ($spice_name ne "")
		{		
			print OUTFILE "\t\ttemplateIdSet($temp_id) := $temp_id;\n";
		}
	}
	close(SPICE);
}
