// ======================================================================
//
// Capsule.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Capsule_H
#define INCLUDED_Capsule_H

#include "sharedMath/Vector.h"
#include "sharedMath/Sphere.h"

// ----------------------------------------------------------------------

class Capsule
{
public:

	Capsule ( Vector const & A, Vector const & B, float radius );

	Capsule ( Sphere const & A, Vector const & delta );

	Vector const &  getPointA           ( void ) const;
	Vector const &  getPointB           ( void ) const;

	float const &   getRadius           ( void ) const;

	// ----------

	Vector          getCenter           ( void ) const;
	float           getTotalRadius      ( void ) const;

	Sphere          getBoundingSphere   ( void ) const;

	Sphere          getSphereA          ( void ) const;
	Sphere          getSphereB          ( void ) const;

	Vector          getDelta            ( void ) const;

	// ----------
	
	bool            contains            ( Sphere const & S ) const;
	bool            intersectsSphere    ( Sphere const & S ) const;

protected:

	Vector m_pointA;
	Vector m_pointB;
	float  m_radius;

	// These two values help accelerate sphere tree queries

	Vector m_normal;
	float  m_segmentLength;
};

// ----------

inline Capsule::Capsule ( Vector const & A, Vector const & B, float radius ) 
: m_pointA(A), m_pointB(B), m_radius(radius)
{
	Vector delta = B - A;

	m_segmentLength = delta.magnitude();

	if(m_segmentLength > 0.0f)
	{
		m_normal = delta / m_segmentLength;
	}
	else
	{
		m_normal = Vector::zero;
	}
}

inline Capsule::Capsule ( Sphere const & S, Vector const & delta )
: m_pointA(S.getCenter()),
  m_pointB(S.getCenter() + delta),
  m_radius(S.getRadius())
{
	m_segmentLength = delta.magnitude();

	if(m_segmentLength > 0.0f)
	{
		m_normal = delta / m_segmentLength;
	}
	else
	{
		m_normal = Vector::zero;
	}
}

inline Vector const & Capsule::getPointA ( void ) const
{
	return m_pointA;
}

inline Vector const & Capsule::getPointB ( void ) const
{
	return m_pointB;
}

inline float const & Capsule::getRadius ( void ) const
{
	return m_radius;
}

inline Vector Capsule::getCenter ( void ) const
{
	return (m_pointA + m_pointB) / 2.0f;
}

inline float Capsule::getTotalRadius ( void ) const
{
	return m_radius + (m_segmentLength / 2.0f);
}

inline Sphere Capsule::getBoundingSphere ( void ) const
{
	return Sphere(getCenter(),getTotalRadius());
}

// ----------

inline Sphere Capsule::getSphereA ( void ) const
{
	return Sphere(m_pointA,m_radius);
}

inline Sphere Capsule::getSphereB ( void ) const
{
	return Sphere(m_pointB,m_radius);
}

inline Vector Capsule::getDelta ( void ) const
{
	return m_pointB - m_pointA;
}

// ----------------------------------------------------------------------

inline bool Capsule::contains ( Sphere const & S ) const
{
	if(m_radius < S.getRadius()) return false; 

	Vector D = S.getCenter() - m_pointA;

	float t = clamp(0.0f, D.dot(m_normal), m_segmentLength);

	float diff2 = sqr(m_radius - S.getRadius());

	float dist2 = D.magnitudeBetweenSquared(m_normal * t);

	return dist2 < diff2;
}

inline bool Capsule::intersectsSphere ( Sphere const & S ) const
{
	Vector D = S.getCenter() - m_pointA;

	float t = clamp(0.0f, D.dot(m_normal), m_segmentLength);

	float sum2 = sqr(m_radius + S.getRadius());

	float dist2 = D.magnitudeBetweenSquared(m_normal * t);

	return dist2 < sum2;
}

// ----------------------------------------------------------------------

#endif // #ifndef INCLUDED_Capsule_H

