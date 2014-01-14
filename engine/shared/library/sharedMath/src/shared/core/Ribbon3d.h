// ======================================================================
//
// Ribbon3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// A ribbon is a segment swept along a line.
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Ribbon3d_H
#define INCLUDED_Ribbon3d_H

#include "sharedMath/Vector.h"

class Segment3d;
class Line3d;
class Plane3d;

// ----------------------------------------------------------------------

class Ribbon3d
{
public:

	Ribbon3d ( Vector const & pointA, Vector const & pointB, Vector const & dir );

	Ribbon3d ( Segment3d const & seg, Vector const & dir );
	Ribbon3d ( Line3d const & line, Vector const & delta );

	Vector const &  getPointA   ( void ) const;
	Vector const &  getPointB   ( void ) const;
	Vector const &  getDir      ( void ) const;
	Vector			getDelta	( void ) const;

	Plane3d         getPlane    ( void ) const;

	Line3d          getEdgeA    ( void ) const;
	Line3d          getEdgeB    ( void ) const;

protected:

	Vector  m_pointA;
	Vector  m_pointB;
	Vector  m_dir;
};

// ----------------------------------------------------------------------

inline Vector const & Ribbon3d::getPointA ( void ) const
{
	return m_pointA;
}

inline Vector const & Ribbon3d::getPointB ( void ) const
{
	return m_pointB;
}

inline Vector const & Ribbon3d::getDir ( void ) const
{
	return m_dir;
}

inline Vector Ribbon3d::getDelta ( void ) const
{
	return m_pointB - m_pointA;
}

// ----------------------------------------------------------------------

#endif // #ifndef INCLUDED_Ribbon3d_H

