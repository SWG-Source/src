// ======================================================================
//
// Ray3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Ray3d_H
#define INCLUDED_Ray3d_H

#include "sharedMath/Vector.h"

class Line3d;

// ----------------------------------------------------------------------

class Ray3d
{
public:

	Ray3d ( Vector const & p, Vector const & d );

	Vector const & getPoint ( void ) const;
	Vector const & getNormal ( void ) const;

	Line3d	getLine	( void ) const;

	Vector atParam ( float t ) const;

protected:

	Vector	m_point;
	Vector	m_normal;
};

// ----------

inline Ray3d::Ray3d ( Vector const & p, Vector const & d ) 
: m_point(p), m_normal(d) 
{
}

inline Vector const & Ray3d::getPoint ( void ) const
{
	return m_point;
}

inline Vector const & Ray3d::getNormal ( void ) const
{
	return m_normal;
}

inline Vector Ray3d::atParam ( float t ) const
{
	return m_point + m_normal * t;
}

// ----------------------------------------------------------------------

#endif // #ifdef INCLUDED_Ray3d_H

