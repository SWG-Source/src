// ======================================================================
//
// States.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_States_H
#define INCLUDED_States_H

// ======================================================================

#include "swgSharedUtility/States.def"

class StringId;

// ======================================================================
/**
 * Support utilities for working with states.
 */

namespace States
{
	const char *     getStateName       (States::Enumerator  state);
	const StringId & getStateStringId   (int state);

	inline uint64 getStateMask(States::Enumerator state)
	{
		return UINT64_LITERAL(1) << state;
	}
}

// ======================================================================

#endif
