// ======================================================================
//
// AlterResult.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/AlterResult.h"

#include <algorithm>
#include <limits>

// ======================================================================
/*
	Note these constants are organized so that an object can return the
	minimum value of all of its subcomponents/children and basically do
	the right thing.  If any subcomponent wants to be altered next frame,
	the min of alter return values will be cms_alterNextFrame.  If
	no subcomponents care to be altered but at least one still wants to
	live and specifies cms_keepNoAlter, then the object will return
	cms_keepNoAlter.  Finally, if all subcomponents want to be killed,
	the object will return cms_kill.

	With the above in mind: DON'T CHANGE THESE VALUES (unless you think this
	through and chek all the code that handles alter return values).
*/

float const AlterResult::cms_alterNextFrame = 0.0f;
float const AlterResult::cms_alterQuickly   = 0.5f;
float const AlterResult::cms_keepNoAlter    = 3.3e38f;
float const AlterResult::cms_kill           = 3.4e38f;

// ======================================================================

void AlterResult::formatTime(char *buffer, int bufferLength, float time)
{
	DEBUG_FATAL(bufferLength < 6, ("buffer length must be at least 5"));

	if (time == cms_alterNextFrame)
		strcpy(buffer, "next ");
	else
		if (time == cms_keepNoAlter)
			strcpy(buffer, "none ");
		else
			if (time == cms_kill)
				strcpy(buffer, "kill ");
			else
				snprintf(buffer, bufferLength, "%5.2f", time);
}

// ======================================================================
