// ======================================================================
//
// Postures.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "swgSharedUtility/FirstSwgSharedUtility.h"
#include "swgSharedUtility/Postures.h"

#include "StringId.h"
#include "UnicodeUtils.h"

// ======================================================================

namespace
{
	const char *const cs_postureNames[] =
		{
		"upright",        // Upright
		"crouched",       // Crouched
		"prone",          // Prone
		"sneaking",       // Sneaking
		"blocking",       // Blocking
		"climbing",       // Climbing
		"flying",         // Flying
		"lyingDown",      // LyingDown
		"sitting",        // Sitting
		"skillAnimating", // SkillAnimating
		"drivingVehicle", // DrivingVehicle
		"ridingCreature", // RidingCreature
		"knockedDown",    // KnockedDown
		"incapacitated",  // Incapacitated
		"dead"            // Dead
		};

	const int cs_postureNameCount = static_cast<int>(sizeof(cs_postureNames)/sizeof(cs_postureNames[0]));

	StringId cs_postureStringIds [cs_postureNameCount];

	//----------------------------------------------------------------------

	bool s_installed = false;

	void install ()
	{
		if (s_installed)
			return;

		s_installed = true;

		static const std::string tableName = "posture_n";

		for (int i = 0; i < cs_postureNameCount; ++i)
		{
			cs_postureStringIds [i] = StringId (tableName, Unicode::toLower (cs_postureNames [i]));
		}
	}
}

// ======================================================================
/** 
 * Retrieve the display name of a posture.
 *
 * This function handles out-of-range posture values.
 *
 * @param posture  the posture for which a display name is desired.
 *
 * @return  the display name of a posture.
 */

const char *Postures::getPostureName(Postures::Enumerator posture)
{
	DEBUG_FATAL(cs_postureNameCount != Postures::NumberOfPostures, ("Posture name table in Postures.cpp needs to be updated."));
	if ((posture < 0) || (posture >= cs_postureNameCount))
		return "<posture value out of range>";
	else
		return cs_postureNames[posture];
}

//----------------------------------------------------------------------

const StringId & Postures::getPostureStringId (int posture)
{
	if (!s_installed)
		install ();

	if (posture < 0 || posture >= cs_postureNameCount)
	{
		static StringId nullStringId;
		return nullStringId;
	}
	else
		return cs_postureStringIds [posture];
}

// ======================================================================
