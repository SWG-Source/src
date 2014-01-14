//======================================================================
//
// SharedHyperspaceStringIds.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SharedHyperspaceStringIds_H
#define INCLUDED_SharedHyperspaceStringIds_H

//======================================================================

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace SharedHyperspaceStringIds
{
	MAKE_STRING_ID(shared_hyperspace, not_cleared_for_point);
	MAKE_STRING_ID(shared_hyperspace, zone_too_full);
	MAKE_STRING_ID(shared_hyperspace, already_at_point);
}

//======================================================================

#endif

