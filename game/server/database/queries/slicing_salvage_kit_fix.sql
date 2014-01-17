begin

	update objects
	set script_list = trim(script_list) || 'item.slicing.salvage_kit:'
	where object_template_id in (
		  -14212001, -- object/tangible/slicing/slicing_armor_upgrade_kit.iff
		  96542655	 -- object/tangible/slicing/slicing_weapon_upgrade_kit.iff
	)
	and script_list not like '%item.slicing.salvage_kit%';

	commit;
end;
/