// ======================================================================
//
// AxialBox.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_AxialBox_H
#define INCLUDED_AxialBox_H

#include "sharedMath/Vector.h"

class Range;

// ======================================================================

class AxialBox
{
public:

	typedef std::vector<Vector> VertexList;

public:

	AxialBox();
	AxialBox( Vector const & cornerA, Vector const & cornerB );
	AxialBox( Range const & rX, Range const & rY, Range const & rZ );
	AxialBox( AxialBox const & boxA, AxialBox const & boxB );
	
	void            clear       ( void );
	void            add         ( Vector const & V );
	void            addMin      ( Vector const & V );
	void            addMax      ( Vector const & V );
	void            add         ( VertexList const & vertices );
	void            add         ( AxialBox const & A );
	bool            contains    ( Vector const & V ) const;
	bool            contains    ( AxialBox const & A ) const;
	bool            isEmpty     ( void ) const;
	
	// ----------

	Vector const &  getMin      ( void ) const;
	Vector const &  getMax      ( void ) const;
	
	void            setMin      ( Vector const & newMin );
	void            setMax      ( Vector const & newMax );
	
	real            getWidth    ( void ) const;
	real            getHeight   ( void ) const;
	real            getDepth    ( void ) const;
	
	Vector          getSize     ( void ) const;
	Vector          getCenter   ( void ) const;
	Vector          getDelta    ( void ) const;
	
	real            getRadius   ( void ) const;
	float getRadiusSquared() const;
	
	Range           getRangeX   ( void ) const;
	Range           getRangeY   ( void ) const;
	Range           getRangeZ   ( void ) const;
	
	Vector          getBase     ( void ) const;
	
	Vector          getCorner   ( int whichCorner ) const;
	
	real            getVolume   ( void ) const;
	real            getArea     ( void ) const;
	
	// ----------
	
	Vector const &  getAxisX    ( void ) const;
	Vector const &  getAxisY    ( void ) const;
	Vector const &  getAxisZ    ( void ) const;
	
	float           getExtentX  ( void ) const;
	float           getExtentY  ( void ) const;
	float           getExtentZ  ( void ) const;

	bool            intersects  ( AxialBox const & other ) const;

protected:
	
	Vector m_min;
	Vector m_max;
};

// ----------------------------------------------------------------------

inline Vector const & AxialBox::getMin ( void ) const           { return m_min; }
inline Vector const & AxialBox::getMax ( void ) const           { return m_max; }

inline void    AxialBox::setMin    ( Vector const & newMin )    { m_min = newMin; }
inline void    AxialBox::setMax    ( Vector const & newMax )    { m_max = newMax; }

inline real    AxialBox::getWidth  ( void ) const               { return m_max.x - m_min.x; }
inline real    AxialBox::getHeight ( void ) const               { return m_max.y - m_min.y; }
inline real    AxialBox::getDepth  ( void ) const               { return m_max.z - m_min.z; }

inline Vector  AxialBox::getSize   ( void ) const               { return (m_max - m_min); }
inline Vector  AxialBox::getCenter ( void ) const               { return (m_max + m_min) / 2.0f; }
inline Vector  AxialBox::getDelta  ( void ) const               { return (m_max - m_min) / 2.0f; }

inline real    AxialBox::getRadius ( void ) const               { return getDelta().magnitude(); }
inline float AxialBox::getRadiusSquared() const        
{ 
	return getDelta().magnitudeSquared(); 
}

inline Vector  AxialBox::getBase   ( void ) const               { return Vector( (m_min.x + m_max.x) / 2.0f, m_min.y, (m_min.z + m_max.z) / 2.0f ); }

inline Vector  AxialBox::getCorner ( int whichCorner ) const
{
	// These corners are ordered so that the first 4 are on the bottom of the box

	switch(whichCorner)
	{
	case 0: return Vector( m_min.x, m_min.y, m_min.z ); 
	case 1: return Vector( m_max.x, m_min.y, m_min.z ); 
	case 2: return Vector( m_min.x, m_min.y, m_max.z ); 
	case 3: return Vector( m_max.x, m_min.y, m_max.z ); 
	case 4: return Vector( m_min.x, m_max.y, m_min.z ); 
	case 5: return Vector( m_max.x, m_max.y, m_min.z ); 
	case 6: return Vector( m_min.x, m_max.y, m_max.z ); 
	case 7: return Vector( m_max.x, m_max.y, m_max.z ); 

	default: return Vector(0,0,0);
	}
}

inline float AxialBox::getVolume ( void ) const
{
	return (m_max.x - m_min.x) * (m_max.y - m_min.y) * (m_max.z - m_min.z);
}

inline float AxialBox::getArea ( void ) const
{
	Vector V = getSize();

	return ((V.x * V.y) + (V.y * V.z) + (V.z * V.x)) * 2.0f;
}

// ----------------------------------------------------------------------

inline Vector const & AxialBox::getAxisX ( void ) const
{
	return Vector::unitX;
}

inline Vector const & AxialBox::getAxisY ( void ) const
{
	return Vector::unitY;
}

inline Vector const & AxialBox::getAxisZ ( void ) const
{
	return Vector::unitZ;
}

// ----------

inline float AxialBox::getExtentX ( void ) const
{
	return (m_max.x - m_min.x) / 2.0f;
}

inline float AxialBox::getExtentY ( void ) const
{
	return (m_max.y - m_min.y) / 2.0f;
}

inline float AxialBox::getExtentZ ( void ) const
{
	return (m_max.z - m_min.z) / 2.0f;
}

// ----------------------------------------------------------------------

#endif

