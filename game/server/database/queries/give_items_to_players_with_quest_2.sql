declare
	objvar_id number;
begin
	select id
	into objvar_id
	from object_variable_names
	where name = 'item_reimbursement_list';

	update object_variables
	set value = value || 'object/tangible/furniture/decorative/hologram_xwing.iff:'
	where name_id = objvar_id
	and object_id in (select object_id from temp_players_with_quests p where p.quest_id = 3338276450);

	insert into object_variables
	select object_id, objvar_id, 5, 'object/tangible/furniture/decorative/hologram_xwing.iff:', 0
	from temp_players_with_quests p
	where p.quest_id = 3338276450
	and not exists (select * from object_variables v where v.object_id = p.object_id and v.name_id = objvar_id);
end;
/
