// ======================================================================
//
// Triangle2d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Triangle2d_H
#define INCLUDED_Triangle2d_H

#include "sharedMath/Vector2d.h"

// ======================================================================
// 2d Triangle

class Triangle2d
{
public:

	Triangle2d( Vector2d const & a, Vector2d const & b, Vector2d const & c );
	Triangle2d();

	// ----------

	Vector2d const &    getCorner   ( int i ) const;
	void				setCorner	( int i, Vector2d const & newCorner );

	Vector2d const &	getCornerA	( void ) const;
	Vector2d const &	getCornerB	( void ) const;
	Vector2d const &	getCornerC	( void ) const;

	void			setCornerA	( Vector2d const & newCorner );
	void			setCornerB	( Vector2d const & newCorner );
	void			setCornerC	( Vector2d const & newCorner );

	// ----------

protected:

	Vector2d m_cornerA;
	Vector2d m_cornerB;
	Vector2d m_cornerC;
};

// ----------

inline Triangle2d::Triangle2d( Vector2d const & a, Vector2d const & b, Vector2d const & c )
{
	m_cornerA = a;
	m_cornerB = b;
	m_cornerC = c;
}

inline Triangle2d::Triangle2d()
{
}

inline Vector2d const & Triangle2d::getCorner ( int i ) const
{
	return (&m_cornerA)[i % 3];
}

inline void Triangle2d::setCorner( int i, Vector2d const & newCorner ) 
{
	(&m_cornerA)[i%3] = newCorner;
}

inline Vector2d const &	Triangle2d::getCornerA	( void ) const
{
	return m_cornerA;
}

inline Vector2d const &	Triangle2d::getCornerB	( void ) const
{
	return m_cornerB;
}

inline Vector2d const &	Triangle2d::getCornerC	( void ) const
{
	return m_cornerC;
}

inline void			Triangle2d::setCornerA	( Vector2d const & newCorner )
{
	m_cornerA = newCorner;
}

inline void			Triangle2d::setCornerB	( Vector2d const & newCorner )
{
	m_cornerB = newCorner;
}

inline void			Triangle2d::setCornerC	( Vector2d const & newCorner )
{
	m_cornerC = newCorner;
}



// ----------------------------------------------------------------------

#endif // #ifdef INCLUDED_Triangle2d_H