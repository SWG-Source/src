//======================================================================
//
// SharedStringIds.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SharedStringIds_H
#define INCLUDED_SharedStringIds_H

//======================================================================

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace SharedStringIds
{
	MAKE_STRING_ID(shared,        rsp_object_deleted_prose);
	MAKE_STRING_ID(ui,            no);
	MAKE_STRING_ID(ui,            no_sitting_in_water);
	MAKE_STRING_ID(ui,            yes);
	MAKE_STRING_ID(game_language, basic);
	MAKE_STRING_ID(cmd_err,       unstick_in_progress);
	MAKE_STRING_ID(cmd_err,       unstick_request_complete);
	MAKE_STRING_ID(cmd_err,       unstick_request_cancelled);
	MAKE_STRING_ID(shared,        revoke_dependant_skill);
	MAKE_STRING_ID(shared,        no_waypoint_datapad_full);

//	MAKE_STRING_ID(shared,        player_sit_on_chair_prose);
//	MAKE_STRING_ID(shared,        player_sit);
//	MAKE_STRING_ID(shared,        player_stand);
//	MAKE_STRING_ID(shared,        player_prone);
//	MAKE_STRING_ID(shared,        player_kneel);
	
	MAKE_STRING_ID(shared,        harvester_no_power);
	MAKE_STRING_ID(shared,        harvester_no_maint_1);
	MAKE_STRING_ID(shared,        harvester_no_maint_2);
	MAKE_STRING_ID(shared,        harvester_no_maint_3);
	MAKE_STRING_ID(shared,        harvester_no_maint_4);
	MAKE_STRING_ID(shared,        harvester_hopper_full);
	MAKE_STRING_ID(shared,        harvester_resource_depleted);
	MAKE_STRING_ID(shared,        harvester_no_resource);

	MAKE_STRING_ID(shared,        gem_inserted);
	MAKE_STRING_ID(shared,        gem_not_inserted);
	MAKE_STRING_ID(shared,        gem_not_in_inventory);

	MAKE_STRING_ID(shared,        vehicle_disabled_name_prose);
	MAKE_STRING_ID(shared,        vehicle_destroyed_name_prose);

	MAKE_STRING_ID(shared,        imagedesigner_designer_success);
	MAKE_STRING_ID(shared,        imagedesigner_recipient_success);
	MAKE_STRING_ID(shared,        imagedesigner_session_not_valid);
	MAKE_STRING_ID(shared,        imagedesigner_not_enough_time_passed);
	MAKE_STRING_ID(shared,        imagedesigner_failed_to_send_money);
	MAKE_STRING_ID(shared,        imagedesigner_not_paying_enough);
	MAKE_STRING_ID(shared,        imagedesigner_hair_not_valid);
	MAKE_STRING_ID(shared,        imagedesigner_bald_not_valid);
	MAKE_STRING_ID(shared,        imagedesigner_recip_paid);
	MAKE_STRING_ID(shared,        imagedesigner_designer_was_paid);
	MAKE_STRING_ID(shared,        imagedesigner_canceled_by_recip);
	MAKE_STRING_ID(shared,        imagedesigner_wounded_can_not_migrate_stats);
	MAKE_STRING_ID(shared,        waypoint_on_other_planet);
	MAKE_STRING_ID(shared,        waypoint_in_other_space_zone);
	MAKE_STRING_ID(shared,        mission_on_other_planet);

	MAKE_STRING_ID(space/space_interaction, no_destroy_in_flight);
	MAKE_STRING_ID(space/space_interaction, no_booster);
	MAKE_STRING_ID(space/space_interaction, booster_disabled);
	MAKE_STRING_ID(space/space_interaction, booster_low_energy);
	MAKE_STRING_ID(space/space_interaction, booster_energy_depleted);
	MAKE_STRING_ID(space/space_interaction, booster_pilot_only);
	MAKE_STRING_ID(space/space_interaction, booster_not_ready);
	MAKE_STRING_ID(space/space_interaction, not_group_leader_for_formation);

	MAKE_STRING_ID(ui_weapongroupassignment, not_a_pilot);

	MAKE_STRING_ID(shared,        shipcomponentinstall_noinventory);
	MAKE_STRING_ID(shared,        shipcomponentinstall_noinventorycontainer);
	MAKE_STRING_ID(shared,        shipcomponentinstall_inventoryfull);

	MAKE_STRING_ID(base_player,   too_many_waypoints);

	MAKE_STRING_ID(obj_attr_n,    bio_link_pending);
	MAKE_STRING_ID(obj_attr_n,    bio_link_unknown);

	MAKE_STRING_ID(ui,            quest_journal_is_full);

	MAKE_STRING_ID(shared, buffbuilder_canceled_by_recip);
	MAKE_STRING_ID(shared, buffbuilder_canceled_by_buffer);
	MAKE_STRING_ID(shared, buffbuilder_no_target);
	MAKE_STRING_ID(shared, buffbuilder_session_not_valid);
}

//======================================================================

#endif
