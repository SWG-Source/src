// ======================================================================
//
// Postures.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Postures_H
#define INCLUDED_Postures_H

// ======================================================================

#include "swgSharedUtility/Postures.def"

class StringId;

// ======================================================================
/**
 * Support utilities for working with postures.
 *
 * Note: -TRF- I think Posture could become a type at this point, convertable
 *       to and from an int8.  These functions would become part of the
 *       type.  Currently I'm following the existing convention of keeping
 *       this in a namespace called Postures.
 */

namespace Postures
{
	const char *     getPostureName     (Postures::Enumerator  posture);
	const StringId & getPostureStringId (int posture);
}

// ======================================================================

#endif
