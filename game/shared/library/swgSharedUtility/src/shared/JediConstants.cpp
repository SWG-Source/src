// ======================================================================
//
// JediConstants.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "swgSharedUtility/FirstSwgSharedUtility.h"
#include "swgSharedUtility/JediConstants.h"


namespace JediStateNamespace
{
	bool isEligible(JediState playerState, JediState skillState)
	{
		if(skillState == 0)
			return true;
		switch(playerState)
		{
		case JS_none:
			return false;
		case JS_forceSensitive:
			return skillState & (JS_forceSensitive);
		case JS_jedi:
			return (skillState & (JS_forceSensitive | JS_jedi)) != 0;
		case JS_forceRankedLight:
			return (skillState & (JS_forceSensitive | JS_jedi | JS_forceRankedLight)) != 0;
		case JS_forceRankedDark:
			return (skillState & (JS_forceSensitive | JS_jedi | JS_forceRankedDark)) != 0;
		default:
			WARNING(true, ("Invalid state %d found when checking skill eligibility", playerState));
			return false;
		}
	}
}

