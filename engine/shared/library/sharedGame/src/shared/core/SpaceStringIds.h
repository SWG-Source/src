//======================================================================
//
// SpaceStringIds.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SpaceStringIds_H
#define INCLUDED_SpaceStringIds_H

//======================================================================

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace SpaceStringIds
{
	MAKE_STRING_ID(space/space_interaction,      wpn_fire_fail_disabled);
	MAKE_STRING_ID(space/space_interaction,      wpn_fire_fail_missile_acquire);
	MAKE_STRING_ID(space/space_interaction,      wpn_fire_fail_missile_range);
	MAKE_STRING_ID(space/space_interaction,      wpn_fire_fail_missile_self);
	MAKE_STRING_ID(space/space_interaction,      wpn_fire_fail_missile_target);
	MAKE_STRING_ID(space/space_interaction,      wpn_fire_fail_power);
	MAKE_STRING_ID(space/space_interaction,      wpn_fire_fail_ammo);
	MAKE_STRING_ID(space/space_interaction,      autopilot_engaged);
	MAKE_STRING_ID(space/space_interaction,      autopilot_disengaged);
}

//======================================================================

#endif
