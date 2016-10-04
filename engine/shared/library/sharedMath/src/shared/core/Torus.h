// ======================================================================
//
// Torus.h
// copyright (c) 2001 Sony Online Entertainment
//
// Simple class to represent a 2D Torus in the X-Z plane
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Torus_H
#define INCLUDED_Torus_H

#include "sharedMath/Vector.h"

// ======================================================================

class Torus
{
public:

	Torus ( Vector const & center, float majorRadius, float minorRadius );
	Torus ( Vector const & center, Vector const & axis, float majorRadius, float minorRadius );

	// ----------

	Vector const & getCenter ( void ) const;
	Vector const & getAxis   ( void ) const;

	float getMajorRadius ( void ) const;
	float getMinorRadius ( void ) const;

	Vector transformToLocal ( Vector const & V ) const;
	Vector transformToWorld ( Vector const & V ) const;

	Vector rotateToLocal    ( Vector const & V ) const;
	Vector rotateToWorld    ( Vector const & V ) const;

	bool isOriented ( void ) const;

protected:

	Vector  m_center;

	Vector  m_axis;
	Vector  m_tangent;
	Vector  m_binormal;

	float   m_majorRadius;
	float   m_minorRadius;
};

// ----------------------------------------------------------------------

inline Torus::Torus ( Vector const & center, float majorRadius, float minorRadius )
: m_center(center),
  m_axis(0,1,0),
  m_tangent(1,0,0),
  m_binormal(0,0,1),
  m_majorRadius(majorRadius),
  m_minorRadius(minorRadius)
{
}

inline Torus::Torus ( Vector const & center, Vector const & axis, float majorRadius, float minorRadius )
: m_center(center),
  m_axis(axis),

  m_majorRadius(majorRadius),
  m_minorRadius(minorRadius)
{
	m_tangent = m_axis.cross(Vector(0,0,1));
	m_binormal = m_tangent.cross(m_axis);
}

inline Vector const & Torus::getCenter ( void ) const
{
	return m_center;
}

inline Vector const & Torus::getAxis   ( void ) const
{
	return m_axis;
}

inline float Torus::getMajorRadius ( void ) const
{
	return m_majorRadius;
}

inline float Torus::getMinorRadius ( void ) const
{
	return m_minorRadius;
}

inline bool Torus::isOriented ( void ) const
{
	return std::abs(m_axis.y - 1.0f) > 0.00001f;
}

// ----------------------------------------------------------------------

#endif

