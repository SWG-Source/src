// ======================================================================
//
// RangeLoop.h
// copyright (c) 2001 Sony Online Entertainment
//
// This class requires a bit of explanation - it represents a 1d range
// of numbers defined over the numerical ring [0,1). Because the ring
// is closed, certain operations aren't well-defined - there's no 
// way to express the notions of "greater than" or "less than" nor is
// there a unique way to express the distance between two values.

// Nevertheless, this class is very useful for representing ranges of 
// things that are logically loops, like angles and such.

// Definitions -

// [ 0 ,0.2 ] - all numbers between 0 and 0.2, inclusive

// [ 0.4, 0.2 ] - the union of [ 0.4, 1 ) and [ 0, 0.2 ]


// This class really ought to be called RingRange


// ----------------------------------------------------------------------

#ifndef INCLUDED_RangeLoop_H
#define INCLUDED_RangeLoop_H

// ======================================================================

class RangeLoop
{
public:

	RangeLoop();
	RangeLoop( float newMin, float newMax );
	
	static RangeLoop empty;
	static RangeLoop full;

	// ----------
	
	bool   isEmpty   ( void ) const;
	bool   isFull    ( void ) const;

	float  getMin  ( void ) const;
	float  getMax  ( void ) const;
	
	void   setMin  ( float newMin );
	void   setMax  ( float newMax );

	float  atParam ( float t ) const;

	float  getSize ( void ) const;

	
	// ----------
	
	bool   overlapPositive   ( RangeLoop const & R ) const;       // exclusive test
	bool   overlapNegative   ( RangeLoop const & R ) const;       // exclusive test

	bool   disjointInclusive ( RangeLoop const & R ) const;
	bool   disjointExclusive ( RangeLoop const & R ) const;
	
	bool   containsInclusive ( float V ) const;
	bool   containsExclusive ( float V ) const;

	bool   contains          ( RangeLoop const & R ) const;
	
	bool   operator ==       ( RangeLoop const & R ) const;
	bool   operator !=       ( RangeLoop const & R ) const;
	
	// ----------
	// accessory functions

	static float   clip              ( float V );
	static float   distancePositive  ( float A, float B );
	static float   distanceNegative  ( float A, float B );

	static bool    containsInclusive ( float min, float max, float V );

	static RangeLoop enclose ( float A, float B);
	static RangeLoop enclose ( RangeLoop const & A, float B);
	static RangeLoop enclose ( RangeLoop const & A, RangeLoop const & B);
	
protected:

	void validate ( void ) const;

	float m_min;
	float m_max;
};

// ----------------------------------------------------------------------

inline float RangeLoop::getMin ( void ) const
{
	return m_min;
}

inline float RangeLoop::getMax ( void ) const
{
	return m_max;
}

inline void RangeLoop::setMin ( float min )
{
	m_min = clip(min);
}

inline void RangeLoop::setMax ( float max )
{
	m_max = clip(max);
}

// ======================================================================

#endif

