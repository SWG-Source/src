// ======================================================================
//
// Segment3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Segment3d_H
#define INCLUDED_Segment3d_H

#include "sharedMath/Vector.h"

class Line3d;
class Range;

// ----------------------------------------------------------------------

class Segment3d
{
public:

	Segment3d ( Vector const & b, Vector const & e );

	Vector const &  getBegin            ( void ) const;
	Vector const &  getEnd              ( void ) const;

	Vector &        getBegin            ( void );
	Vector &        getEnd              ( void );

	Vector          getDelta            ( void ) const;
	Line3d          getLine             ( void ) const;
	Line3d          getReverseLine      ( void ) const;

	float           getLength           ( void ) const;
	float           getLengthSquared    ( void ) const;

	Range           getRangeX           ( void ) const;
	Range           getRangeY           ( void ) const;
	Range           getRangeZ           ( void ) const;

	Vector          atParam             ( float t ) const;

protected:

	Vector m_begin;
	Vector m_end;
};

// ----------

inline Segment3d::Segment3d ( Vector const & b, Vector const & e ) 
: m_begin(b), m_end(e) 
{
}

inline Vector const & Segment3d::getBegin ( void ) const
{
	return m_begin;
}

inline Vector const & Segment3d::getEnd ( void ) const
{
	return m_end;
}

inline Vector & Segment3d::getBegin ( void )
{
	return m_begin;
}

inline Vector & Segment3d::getEnd ( void )
{
	return m_end;
}

inline Vector	Segment3d::getDelta ( void ) const
{
	return m_end - m_begin;
}

inline float Segment3d::getLength ( void ) const
{
	return getDelta().magnitude();
}

inline float Segment3d::getLengthSquared ( void ) const
{
	return getDelta().magnitudeSquared();
}

inline Vector Segment3d::atParam ( float t ) const
{
	return m_begin + (m_end - m_begin) * t;
}

// ----------------------------------------------------------------------

#endif // #ifndef INCLUDED_Segment3d_H

