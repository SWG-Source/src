// ======================================================================
//
// HardPoint.h
// Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Hardpoint_H
#define INCLUDED_Hardpoint_H

// ======================================================================

#include "sharedFoundation/PersistentCrcString.h"
#include "sharedMath/Transform.h"

// ======================================================================

class Hardpoint
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct LessNameComparator
	{
		bool operator () (Hardpoint const* lhs, Hardpoint const* rhs) const;
		bool operator () (CrcString const& lhs, Hardpoint const* rhs) const;
		bool operator () (Hardpoint const* lhs, CrcString const& rhs) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef const Hardpoint* ConstIterator;

public:

	Hardpoint (const char* newName, const Transform& newTransform);

	CrcString const& getName () const;
	Transform const& getTransform () const;

private:

	// Disabled.
	Hardpoint (void);
	Hardpoint (const Hardpoint&);
	Hardpoint& operator= (const Hardpoint&);

private:

	PersistentCrcString const  m_name;
	Transform const            m_transform;

};

// ======================================================================
// struct LessNameComparator
// ======================================================================

// Define these comparator functions inline so that STL sorts/searches are inlined.

inline bool Hardpoint::LessNameComparator::operator () (Hardpoint const* lhs, Hardpoint const* rhs) const
{
	NOT_NULL (lhs);
	NOT_NULL (rhs);

	return lhs->getName () < rhs->getName ();
}

// ----------------------------------------------------------------------

inline bool Hardpoint::LessNameComparator::operator () (CrcString const& lhs, Hardpoint const* rhs) const
{
	NOT_NULL (rhs);

	return lhs < rhs->getName ();
}

// ----------------------------------------------------------------------

inline bool Hardpoint::LessNameComparator::operator () (Hardpoint const* lhs, CrcString const& rhs) const
{
	NOT_NULL (lhs);

	return lhs->getName () < rhs;
}

// ======================================================================

#endif
