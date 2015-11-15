// ======================================================================
//
// ContactPoint.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ContactPoint_H
#define INCLUDED_ContactPoint_H

#include "sharedMath/Vector.h"

class CollisionSurface;
class Transform;
class Iff;

// ======================================================================

class ContactPoint
{
public:

	ContactPoint();

	ContactPoint( CollisionSurface const * surface, Vector const & point, int id, float offset );
	
	ContactPoint( ContactPoint const & locCopy );

	// ----------

	Vector const &  getPosition_l   ( void ) const;
	void            setPosition_l   ( Vector const & newPosition );

	real            getOffset       ( void ) const;
	void            setOffset       ( float offset );

	Vector          getOffsetPosition_l ( void ) const;

	int             getId           ( void ) const;
	void            setId           ( int newId );

	CollisionSurface const * getSurface     ( void ) const;
	void            setSurface      ( CollisionSurface const * pFloor );

	// ----------

	bool            isAttached      ( void ) const;

	// ----------

	void            read_0000       ( Iff & iff );
	void            write           ( Iff & iff ) const;

protected:

	CollisionSurface const * m_pSurface;     // The collision surface this contact point is on
	
	Vector  m_position;     // Position of the contact point in surface space
	real    m_offset;		// Surface offset (need more explanation)
	int     m_hitId;        // Hit identifier, which indicates what part of the collision surface
	                        // this contact point is on. For collision meshes, the hit identifier is
	                        // the index of the hit triangle.
};

// ======================================================================

#endif // #ifndef INCLUDED ContactPoint_H

