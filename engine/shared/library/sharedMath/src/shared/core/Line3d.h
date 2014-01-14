// ======================================================================
//
// Line3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Line3d_H
#define INCLUDED_Line3d_H

#include "sharedMath/Vector.h"

// ----------------------------------------------------------------------

class Line3d
{
public:

	Line3d ( Vector const & p, Vector const & d );
	
	Vector const & getPoint  ( void ) const;
	Vector const & getNormal ( void ) const;
	
	Vector         atParam   ( float t ) const;
	
	void           flip      ( void );
	Line3d         flipped   ( void ) const;

protected:

	Vector	m_point;
	Vector	m_normal;
};

// ----------

inline Line3d::Line3d ( Vector const & p, Vector const & d ) 
: m_point(p), 
  m_normal(d) 
{
}

inline Vector const & Line3d::getPoint ( void ) const 
{
	return m_point;
}

inline Vector const & Line3d::getNormal ( void ) const
{
	return m_normal;
}

inline Vector Line3d::atParam ( float t ) const
{
	return m_point + m_normal * t;
}

inline void Line3d::flip ( void )
{
	m_normal = -m_normal;
}

inline Line3d Line3d::flipped ( void ) const
{
	return Line3d(m_point,-m_normal);
}

// ----------------------------------------------------------------------

#endif // #ifdef INCLUDED_Line3d_H
