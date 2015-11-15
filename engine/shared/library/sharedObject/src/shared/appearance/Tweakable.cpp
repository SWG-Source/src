// ======================================================================
//
// Tweakable.cpp
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/Tweakable.h"

// ======================================================================

Tweakable::~Tweakable()
{
}

// ----------------------------------------------------------------------

float Tweakable::getTweakValue ( int whichValue ) const
{
	UNREF(whichValue);

	return 0.0f;
}

// ----------------------------------------------------------------------

void Tweakable::setTweakValue ( int whichValue, float newValue )
{
	UNREF(whichValue);
	UNREF(newValue);
}

// ======================================================================
