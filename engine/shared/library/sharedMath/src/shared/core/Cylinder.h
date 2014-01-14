// ======================================================================
//
// Cylinder.h
// copyright (c) 2001 Sony Online Entertainment
//
// Very, very simple Y-axis aligned cylinder used for simple collision
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Cylinder_H
#define INCLUDED_Cylinder_H

#include "sharedMath/Vector.h"

class Range;
class Circle;
class Ring;

// ======================================================================

class Cylinder
{
public:

	Cylinder();
	Cylinder( Vector const & base, real radius, real height );
	
	Vector const &  getBase         ( void ) const;
	float           getRadius       ( void ) const;
	float           getHeight       ( void ) const;
	
	void            setBase         ( Vector const & newBase );
	void            setRadius       ( real newRadius );
	void            setHeight       ( real newHeight );
	
	// ----------
	// Helper methods for MultiShape
	
	Vector          getCenter       ( void ) const;
	
	Vector const &  getAxisX        ( void ) const;
	Vector const &  getAxisY        ( void ) const;
	Vector const &  getAxisZ        ( void ) const;
	
	float           getExtentX      ( void ) const;
	float           getExtentY      ( void ) const;
	float           getExtentZ      ( void ) const;
	
	// ----------
	
	Range           getRangeY       ( void ) const;
	
	Circle          getTopCircle    ( void ) const;
	Circle          getBaseCircle   ( void ) const;
	
	Ring            getTopRing      ( void ) const;
	Ring            getBaseRing     ( void ) const;

protected:

	Vector  m_base;
	float   m_radius;
	float   m_height;
};

// ----------------------------------------------------------------------

inline Cylinder::Cylinder()
: m_base( Vector::zero ),
  m_radius( 1.0f ),
  m_height( 1.0f )
{
}

inline Cylinder::Cylinder ( Vector const & base, real radius, real height )
: m_base(base), 
  m_radius(radius), 
  m_height(height)
{
}

// ----------------------------------------------------------------------

inline Vector const & Cylinder::getBase ( void ) const
{
	return m_base;
}

inline float Cylinder::getRadius ( void ) const
{
	return m_radius;
}

inline float Cylinder::getHeight ( void ) const
{
	return m_height;
}

// ----------------------------------------------------------------------

inline void Cylinder::setBase ( Vector const & newBase )
{
	m_base = newBase;
}

inline void Cylinder::setRadius ( real newRadius )
{
	m_radius = newRadius;
}

inline void Cylinder::setHeight ( real newHeight )
{
	m_height = newHeight;
}

// ----------------------------------------------------------------------

inline Vector Cylinder::getCenter ( void ) const
{
	return m_base + Vector( 0.0f, m_height / 2.0f, 0.0f );
}

// ----------

inline Vector const & Cylinder::getAxisX ( void ) const
{
	return Vector::unitX;
}

inline Vector const & Cylinder::getAxisY ( void ) const
{
	return Vector::unitY;
}

inline Vector const & Cylinder::getAxisZ ( void ) const
{
	return Vector::unitZ;
}

// ----------

inline float Cylinder::getExtentX ( void ) const
{
	return getRadius();
}

inline float Cylinder::getExtentY ( void ) const
{
	return getHeight() / 2.0f;
}

inline float Cylinder::getExtentZ ( void ) const
{
	return getRadius();
}

// ----------------------------------------------------------------------

#endif // #ifdef INCLUDED_Cylinder_H

