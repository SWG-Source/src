// ======================================================================
//
// PolySolver.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_PolySolver_H
#define INCLUDED_PolySolver_H

// ----------------------------------------------------------------------

class PolySolver
{
public:

	static int solveQuadratic ( double const c[3], double r[2] );
	static int solveCubic     ( double const c[4], double r[3] );
	static int solveQuartic   ( double const c[5], double r[4] );

};

// ----------------------------------------------------------------------

#endif // #ifndef INCLUDED_PolySolver_H

