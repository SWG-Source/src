// ======================================================================
//
// OrientedCircle.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_OrientedCircle_H
#define INCLUDED_OrientedCircle_H

#include "sharedMath/Vector.h"

class Circle;

// ======================================================================

class OrientedCircle
{
public:

	OrientedCircle ( Vector const & center, Vector const & axis, float radius );
	OrientedCircle ( Circle const & circle );

	Vector const & getAxis ( void ) const;
	float getRadius ( void ) const;
	Vector const & getCenter ( void ) const;

protected:

	Vector m_center;
	Vector m_axis;

	float  m_radius;
};


// ----------------------------------------------------------------------

inline Vector const & OrientedCircle::getAxis ( void ) const
{
	return m_axis;
}

inline float OrientedCircle::getRadius ( void ) const
{
	return m_radius;
}

inline Vector const & OrientedCircle::getCenter ( void ) const
{
	return m_center;
}

// ======================================================================

#endif

