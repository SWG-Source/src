// ======================================================================
//
// Locomotions.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "swgSharedUtility/FirstSwgSharedUtility.h"
#include "swgSharedUtility/Locomotions.h"

#include "StringId.h"
#include "UnicodeUtils.h"

// ======================================================================

namespace
{
	const char *const cs_locomotionNames[] =
		{
		"standing",
		"sneaking",
		"walking",
		"running",
		"kneeling",
		"crouchSneaking",
		"crouchWalking",
		"prone",
		"crawling",
		"climbingStationary",
		"climbing",
		"hovering",
		"flying",
		"lyingDown",
		"sitting",
		"skillAnimating",
		"drivingVehicle",
		"ridingCreature",
		"knockedDown",
		"incapacitated",
		"dead",
		"blocking"
		};

	const int cs_locomotionNameCount = static_cast<int>(sizeof(cs_locomotionNames)/sizeof(cs_locomotionNames[0]));

	StringId cs_locomotionStringIds [cs_locomotionNameCount];

	//----------------------------------------------------------------------

	bool s_installed = false;

	void install ()
	{
		if (s_installed)
			return;

		s_installed = true;

		static const std::string tableName = "locomotion_n";

		for (int i = 0; i < cs_locomotionNameCount; ++i)
		{
			cs_locomotionStringIds [i] = StringId (tableName, Unicode::toLower (cs_locomotionNames [i]));
		}
	}
}

// ======================================================================
/** 
 * Retrieve the display name of a locomotion.
 *
 * This function handles out-of-range locomotion values.
 *
 * @param locomotion  the locomotion for which a display name is desired.
 *
 * @return  the display name of a locomotion.
 */

const char *Locomotions::getLocomotionName(Locomotions::Enumerator locomotion)
{
	DEBUG_FATAL(cs_locomotionNameCount != Locomotions::NumberOfLocomotions, ("Locomotion name table in Locomotions.cpp needs to be updated."));
	if ((locomotion < 0) || (locomotion >= cs_locomotionNameCount))
		return "<locomotion value out of range>";
	else
		return cs_locomotionNames[locomotion];
}

//----------------------------------------------------------------------

const StringId & Locomotions::getLocomotionStringId (int locomotion)
{
	if (!s_installed)
		install ();
	if ((locomotion < 0) || (locomotion >= cs_locomotionNameCount))
	{
		static const StringId nullStringId;
		return nullStringId;
	}	
	else
		return cs_locomotionStringIds[locomotion];
}

// ======================================================================
