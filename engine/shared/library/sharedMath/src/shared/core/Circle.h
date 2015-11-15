// ======================================================================
//
// Circle.h
// copyright (c) 2001 Sony Online Entertainment
//
// Simple class to represent a 2D circle in the X-Z plane
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Circle_H
#define INCLUDED_Circle_H

#include "sharedMath/Vector.h"

class Range;
class Plane3d;

// ======================================================================

class Circle
{
public:
	
	Circle ( Vector const & center, float radius );
	
	// ----------
	
	Vector const &  getCenter        ( void ) const;
	void            setCenter        ( Vector const & center );
	
	float           getRadius        ( void ) const;
	void            setRadius        ( float radius );

	float           getRadiusSquared ( void ) const;
	
	Range           getRangeX        ( void ) const;
	Range           getRangeZ        ( void ) const;
	
	Range           getLocalRangeX   ( void ) const;
	
	Plane3d         getPlane         ( void ) const;

protected:

	Vector m_center;
	float  m_radius;
};

// ----------------------------------------------------------------------

inline Circle::Circle ( Vector const & center, float radius ) 
: m_center(center), 
  m_radius(radius)
{

}

// ----------

inline Vector const & Circle::getCenter ( void ) const 
{
	return m_center;
}

inline void Circle::setCenter ( Vector const & center )
{
	m_center = center;
}

// ----------

inline float Circle::getRadius ( void ) const
{
	return m_radius;
}

inline void Circle::setRadius ( float radius )
{
	m_radius = radius;
}

// ----------

inline float Circle::getRadiusSquared ( void ) const
{
	return m_radius * m_radius;
}

// ----------------------------------------------------------------------

#endif

