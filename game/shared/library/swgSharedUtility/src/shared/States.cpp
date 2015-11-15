// ======================================================================
//
// States.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "swgSharedUtility/FirstSwgSharedUtility.h"
#include "swgSharedUtility/States.h"

#include "StringId.h"
#include "UnicodeUtils.h"

// ======================================================================

namespace
{
	const char *const cs_stateNames[] =
		{
		"cover",
		"combat",
		"peace",
		"aiming",
		"alert",
		"berserk",
		"feignDeath",
		"combatAttitudeEvasive",
		"combatAttitudeNormal",
		"combatAttitudeAggressive",
		"tumbling",
		"rallied",
		"stunned",
		"blinded",
		"dizzy",
		"intimidated",
		"immobilized",
		"frozen",
		"swimming",
		"sittingOnChair",
		"crafting",
		"glowingJedi",
		"maskScent",
		"poisoned",
		"bleeding",
		"diseased",
		"onFire",
		"ridingMount",
		"mountedCreature",
		"pilotingShip",
		"shipOperations",
		"shipGunner",
		"shipInterior",
		"pilotingPobShip",
		"performingDeathBlow",
		"disguised",
		"electricBurned",
		"coldBurned",
		"acidBurned",
		"energyBurned",
		"kineticBurned"
		};

	const int cs_stateNameCount = static_cast<int>(sizeof(cs_stateNames)/sizeof(cs_stateNames[0]));

	StringId cs_stateStringIds [cs_stateNameCount];

	//----------------------------------------------------------------------

	bool s_installed = false;

	void install ()
	{
		if (s_installed)
			return;

		s_installed = true;

		static const std::string tableName = "state_n";

		for (int i = 0; i < cs_stateNameCount; ++i)
		{
			cs_stateStringIds [i] = StringId (tableName, Unicode::toLower (cs_stateNames [i]));
		}
	}
}

// ======================================================================
/** 
 * Retrieve the display name of a state.
 *
 * This function handles out-of-range state values.
 *
 * @param state  the state for which a display name is desired.
 *
 * @return  the display name of a state.
 */

const char *States::getStateName(States::Enumerator state)
{
	DEBUG_FATAL(cs_stateNameCount != States::NumberOfStates, ("State name table in States.cpp needs to be updated."));
	if ((state < 0) || (state >= cs_stateNameCount))
		return "<state value out of range>";
	else
		return cs_stateNames[state];
}

//----------------------------------------------------------------------

const StringId & States::getStateStringId (int state)
{
	if (!s_installed)
		install ();

	if (state < 0 || state >= cs_stateNameCount)
	{
		static StringId nullStringId;
		return nullStringId;
	}
	else
		return cs_stateStringIds [state];
}

// ======================================================================

