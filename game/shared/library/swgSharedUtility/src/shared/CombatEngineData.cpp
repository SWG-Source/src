// ======================================================================
//
// CombatEngineData.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "swgSharedUtility/FirstSwgSharedUtility.h"
#include "swgSharedUtility/CombatEngineData.h"

#include "sharedObject/Object.h"

// ======================================================================

namespace
{
	const char *const cs_combatDefenseNames[CombatEngineData::CD_numCombatDefense] =
		{
			"miss",                   // CD_miss
			"hit",                    // CD_hit
			"block",                  // CD_block
			"evade",                  // CD_evade
			"redirect",               // CD_redirect
			"counterAttack",          // CD_counterAttack
			"lightsaberBlock"         // CD_lightsaberBlock
			"lightsaberCounter"       // CD_lightsaberCounter
			"lightsaberCounterTarget" // CD_lightsaberCounterTarget
		};

	const int cs_combatDefenseNameCount = CombatEngineData::CD_numCombatDefense;
}

// ======================================================================

const char *const CombatEngineData::getCombatDefenseName(CombatEngineData::CombatDefense combatDefense)
{
	if ((combatDefense < 0) || (combatDefense >= cs_combatDefenseNameCount))
		return "<CombatDefense value out of range>";
	else
		return cs_combatDefenseNames[combatDefense];
}

// ======================================================================
