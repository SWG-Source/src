// ======================================================================
//
// Ring.h
// copyright (c) 2001 Sony Online Entertainment
//
// Simple class to represent a 2D Ring in the X-Z plane
//
// This class is only semantically different from a Circle - Circles are
// 2-dimensional entities (i.e. a circle cut from paper) whereas a Ring
// is a 1-dimensional entity (a piece of wire bent into a circle)
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Ring_H
#define INCLUDED_Ring_H

#include "sharedMath/Vector.h"

class Range;
class Plane3d;

// ======================================================================

class Ring
{
public:

	Ring ( Vector const & center, float radius );

	// ----------

	Vector const & getCenter ( void ) const;

	float getRadius ( void ) const;
	float getRadiusSquared ( void ) const;

	Range getRangeX ( void ) const;
	Range getRangeZ ( void ) const;

	Range getLocalRangeX ( void ) const;

	Plane3d getPlane ( void ) const;

protected:

	Vector m_center;
	float  m_radius;
};

// ----------------------------------------------------------------------

inline Ring::Ring ( Vector const & center, float radius ) 
: m_center(center), 
  m_radius(radius)
{

}

inline Vector const & Ring::getCenter ( void ) const 
{
	return m_center;
}

inline float Ring::getRadius ( void ) const
{
	return m_radius;
}

inline float Ring::getRadiusSquared ( void ) const
{
	return m_radius * m_radius;
}


// ----------------------------------------------------------------------

#endif

