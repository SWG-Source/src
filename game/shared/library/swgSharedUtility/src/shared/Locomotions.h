// ======================================================================
//
// Locomotions.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Locomotions_H
#define INCLUDED_Locomotions_H

// ======================================================================

#include "swgSharedUtility/Locomotions.def"

class StringId;

// ======================================================================
/**
 * Support utilities for working with locomotions.
 */

namespace Locomotions
{
	const char *     getLocomotionName     (Locomotions::Enumerator  locomotion);
	const StringId & getLocomotionStringId (int locomotion);
}

// ======================================================================

#endif
