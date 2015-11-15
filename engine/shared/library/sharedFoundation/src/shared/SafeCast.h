// ======================================================================
//
// SafeCast.h
// Copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SafeCast_H
#define INCLUDED_SafeCast_H

// ======================================================================

#if DEBUG_LEVEL == DEBUG_LEVEL_RELEASE

	#define safe_cast static_cast

#else

	template <typename T, typename U>
	T safe_cast(U u)
	{
		if (!u)
			return 0;

		T t = dynamic_cast<T>(u);
		NOT_NULL(t);
		return t;
	}

#endif

// ======================================================================

#endif
