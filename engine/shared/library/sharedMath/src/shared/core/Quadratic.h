// ======================================================================
//
// Quadratic.h
// copyright (c) 2001 Sony Online Entertainment
//
// Simple class to represent a quadratic equation
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Quadratic_H
#define INCLUDED_Quadratic_H

// ======================================================================

class Quadratic
{
public:

	Quadratic ( float a, float b, float c );

	// ----------

	bool solveFor( float value, float & out1, float & out2 ) const;

	// ----------
	
	float m_A;
	float m_B;
	float m_C;
};

// ----------------------------------------------------------------------

inline Quadratic::Quadratic ( float a, float b, float c )
: m_A(a), m_B(b), m_C(c)
{
}

// ----------------------------------------------------------------------

#endif

