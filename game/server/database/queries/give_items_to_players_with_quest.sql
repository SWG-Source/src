insert into object_variable_names
select (select max(id)+1 from object_variable_names), 'item_reimbursement_list'
from dual
where not exists (select * from object_variable_names where name = 'item_reimbursement_list');

insert into object_variables
select object_id, n.id, 5, 'object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_leggings.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_helmet.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_gloves.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_chest_plate.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_bracer_r.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_bracer_l.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_boots.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_bicep_r.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_bicep_l.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_belt.iff:', 0
from object_variable_names n, temp_players_with_quests p
where n.name='item_reimbursement_list'
and p.quest_id = 24453256030
and not exists (select * from object_variables v where v.object_id = p.object_id and v.name_id = n.id);

insert into object_variables
select object_id, n.id, 5, 'object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_leggings.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_helmet.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_gloves.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_chest_plate.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_bracer_r.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_bracer_l.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_boots.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_bicep_r.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_bicep_l.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_belt.iff:', 0
from object_variable_names n, temp_players_with_quests p
where n.name='item_reimbursement_list'
and p.quest_id = 3338276450
and not exists (select * from object_variables v where v.object_id = p.object_id and v.name_id = n.id);

insert into object_variables
select object_id, n.id, 5, 'object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_leggings.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_helmet.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_gloves.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_chest_plate.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_bracer_r.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_bracer_l.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_boots.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_bicep_r.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_bicep_l.iff:object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_belt.iff:', 0
from object_variable_names n, temp_players_with_quests p
where n.name='item_reimbursement_list'
and p.quest_id = 3008535013
and not exists (select * from object_variables v where v.object_id = p.object_id and v.name_id = n.id);
