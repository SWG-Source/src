// ======================================================================
//
// NetworkStringIds.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NetworkStringIds_H
#define INCLUDED_NetworkStringIds_H

// ======================================================================

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace SharedStringIds
{
	MAKE_STRING_ID(shared,      character_create_failed_bad_location);
}

// ======================================================================

#endif
