//========================================================================
//
// AttribMod.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/AttribMod.h"


bool operator!=(const AttribMod::AttribMod & a, const AttribMod::AttribMod & b)
{
	// we are assuming that two mods with the same tag are equal, if the tags 
	// aren't 0
	if (a.tag != 0 || b.tag != 0)
		return a.tag != b.tag;
	else
		return memcmp(&a, &b, sizeof(AttribMod::AttribMod)) != 0;
}

