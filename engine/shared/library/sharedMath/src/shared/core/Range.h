// ======================================================================
//
// Range.h
// copyright (c) 2001 Sony Online Entertainment
//
// Simple class to represent a 1D range (essentially a 1d bounding box)
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Range_H
#define INCLUDED_Range_H

#include "sharedFoundation/Misc.h"

// ======================================================================

class Range
{
public:

	Range();
	Range( float newMin, float newMax );
	
	// ----------
	
	 float const & getMin          ( void ) const;
	 float const & getMax          ( void ) const;
	
	 void          setMin          ( float const & newMin );
	 void          setMax          ( float const & newMax );
	 void set(float const newMin, float const newMax );
	
	// ----------
	
	bool           isBelow         ( Range const & R ) const;
	bool           isAbove         ( Range const & R ) const;
	bool           isTouchingBelow ( Range const & R ) const;
	bool           isTouchingAbove ( Range const & R ) const;
	bool           isEmpty         ( void ) const;
	
	// ----------
	
	bool           contains        ( float V ) const;
	
	Range          operator +      ( float offset ) const;
	
	bool           operator <      ( Range const & R ) const;
	bool           operator ==     ( Range const & R ) const;
	bool           operator !=     ( Range const & R ) const;
	
	float          clamp           ( float V ) const;

	float linearInterpolate(float t) const;
	float cubicInterpolate(float t) const;
	float random() const;
	
	// ----------
	
	static Range empty;
	static Range inf;    // (-inf,inf)
	
	static Range plusInf;   // (0,inf)
	static Range negInf; // (-inf,0)
	static Range unit;      // (0,1)
	
	// ----------
	
	static Range enclose ( Range const & A, float V );
	static Range enclose ( Range const & A, Range const & B );
	static Range enclose ( Range const & A, Range const & B, Range const & C );
	static Range enclose ( Range const & A, Range const & B, Range const & C, Range const & D );

protected:

	float m_min;
	float m_max;
};

// ----------------------------------------------------------------------

inline Range::Range() : m_min(REAL_MAX), m_max(-REAL_MAX)
{
}

inline Range::Range ( float newMin, float newMax ) : m_min(newMin), m_max(newMax)
{
}

// ----------

inline float const & Range::getMin ( void ) const
{
    return m_min;
}

inline float const & Range::getMax ( void ) const
{
    return m_max;
}

inline void Range::setMin ( float const & newMin )
{
    m_min = newMin;
}

inline void Range::setMax ( float const & newMax )
{
    m_max = newMax;
}

inline void Range::set(float const newMin, float const newMax )
{
	setMin(newMin);
	setMax(newMax);
}


// ----------

inline bool Range::isBelow ( Range const & R ) const
{
	return m_max < R.m_min;
}

inline bool Range::isAbove ( Range const & R ) const
{
	return m_min > R.m_max;
}

inline bool Range::isTouchingBelow ( Range const & R ) const
{
	return m_max == R.m_min;
}

inline bool Range::isTouchingAbove ( Range const & R ) const
{
	return m_min == R.m_max;
}

inline bool Range::isEmpty ( void ) const
{
	return m_max < m_min;
}

// ----------

inline bool Range::contains ( float V ) const
{
	if(V < m_min) return false;
	if(V > m_max) return false;

	return true;
}

// ----------

inline Range Range::operator + ( float offset ) const
{
	return Range(m_min + offset, m_max + offset);
}

inline bool Range::operator < ( Range const & R ) const
{
	if( m_min < R.m_min ) 
	{
		return true;
	}
	else if( m_min == R.m_min )
	{
		return m_max < R.m_max;
	}
	else
	{
		return false;
	}
}

inline bool Range::operator == ( Range const & R ) const
{
	if(m_min != R.m_min) return false;
	if(m_max != R.m_max) return false;

	return true;
}

inline bool Range::operator != ( Range const & R ) const
{
	if(m_min != R.m_min) return true;
	if(m_max != R.m_max) return true;

	return false;
}

// ----------

inline float Range::clamp ( float V ) const
{
	if(isEmpty())
	{
		return V;
	}
	else
	{
		return ::clamp(m_min, V, m_max);
	}
}

// ----------

inline float Range::linearInterpolate(float const t) const
{
	return ::linearInterpolate(m_min, m_max, t);
}

// ----------

inline float Range::cubicInterpolate(float const t) const
{
	return ::cubicInterpolate(m_min, m_max, t);
}

// ------------------------------------------------::----------------------

#endif

