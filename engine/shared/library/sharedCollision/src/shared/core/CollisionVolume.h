// ======================================================================
//
// CollisionVolume.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CollisionVolume_H
#define INCLUDED_CollisionVolume_H

// ======================================================================

class Vector;

// ======================================================================

class CollisionVolume
{
public:

	CollisionVolume();
	virtual ~CollisionVolume();

	// ----------
	// Generic interface

	bool	contains	( Vector const & V );

protected:

	// Disable copying

	CollisionVolume(const CollisionVolume &source);
	CollisionVolume & operator =(const CollisionVolume &source);
};

// ----------------------------------------------------------------------

#endif
