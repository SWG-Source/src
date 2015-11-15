// ======================================================================
//
// Plane3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Plane3d_H
#define INCLUDED_Plane3d_H

#include "sharedMath/Vector.h"

class Plane;

// ----------------------------------------------------------------------

class Plane3d
{
public:

	Plane3d();

	Plane3d ( Plane const & plane );
	Plane3d ( Vector const & p, Vector const & n );
	Plane3d ( Vector const & A, Vector const & B, Vector const & C );

	// build a plane but don't worry about normalizing the m_normal

	static Plane3d	NoNorm( Vector const & p, Vector const & n );

	Vector const & getPoint ( void ) const;
	Vector const & getNormal ( void ) const;

	bool isNormalized ( void ) const;	//@return True if the plane's normal is unit-length

protected:

	Vector	m_point;
	Vector	m_normal;
};

// ----------

inline Plane3d::Plane3d()
{
}

inline Plane3d::Plane3d ( Vector const & p, Vector const & n ) 
: m_point(p), m_normal(n) 
{
	IGNORE_RETURN( m_normal.normalize() );
}

inline Plane3d::Plane3d ( Vector const & A, Vector const & B, Vector const & C )
: m_point(A), m_normal( (B-A).cross(C-A) )
{
	IGNORE_RETURN( m_normal.normalize() );
}

// build a plane but don't worry about normalizing the m_normal

inline Plane3d	Plane3d::NoNorm( Vector const & p, Vector const & n )
{
	Plane3d temp;

	temp.m_point = p;
	temp.m_normal = n;

	return temp;
}

inline Vector const & Plane3d::getPoint ( void ) const
{
	return m_point;
}

inline Vector const & Plane3d::getNormal ( void ) const
{
	return m_normal;
}

inline bool Plane3d::isNormalized ( void ) const
{
//	return m_normal.isNormalized()
	return true;
}

// ----------------------------------------------------------------------

#endif // #ifdef INCLUDED_Plane3d_H

