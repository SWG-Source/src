// ======================================================================
//
// Intersect1d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Intersect1d_H
#define INCLUDED_Intersect1d_H

#include "sharedMath/Range.h"

#include <algorithm>

class Range;

namespace Intersect1d
{

// ----------------------------------------------------------------------
// Static intersection

inline Range IntersectRanges ( Range const & A, Range const & B )
{
	return Range( std::max(A.getMin(),B.getMin()), 
	              std::min(A.getMax(),B.getMax()) );
}

inline Range IntersectRanges ( Range const & A, Range const & B, Range const & C )
{
	return Range( std::max(std::max(A.getMin(),B.getMin()),C.getMin()),
	              std::min(std::min(A.getMax(),B.getMax()),C.getMax()) );

}

// ----------------------------------------------------------------------
// Temporal intersect tests

// A value P is changing with velocity V
// Find the range of time for which P is less/greater than R

inline Range IntersectFloatLess ( float P, float V, float R )
{
	if(V == 0)
	{
		if(P < R) return Range::inf;
		else      return Range::empty;
	}

	float t = (R - P) / V;

	if(V < 0) return Range(t,REAL_MAX);
	else      return Range(-REAL_MAX,t);
}

inline Range IntersectFloatGreater ( float P, float V, float R )
{
	if(V == 0)
	{
		if(P > R) return Range::inf;
		else      return Range::empty;
	}

	float t = (R - P) / V;

	if(V > 0) return Range(t,REAL_MAX);
	else      return Range(-REAL_MAX,t);
}

// ----------

inline Range IntersectFloatFloat ( real A, real V, real B )
{
	if(V == 0)
	{
		if(A == B)	return Range::inf;      //lint !e777 // testing floats for equality
		else		return Range::empty;
	}

	float t = (B - A) / V;

	return Range(t,t);
}

// A value P is moving with velocity V.
// Find the range of time for which P overlaps the range R.

inline Range IntersectFloatRange ( float A, real V, Range const & R )
{
	Range timeMin = IntersectFloatGreater(A,V,R.getMin());
	Range timeMax = IntersectFloatLess(A,V,R.getMax());

	return IntersectRanges(timeMin,timeMax);
}

inline Range IntersectRangeFloat ( Range const & A, float V, float B )
{ 
	return IntersectFloatRange(B,-V,A); 
}

// A range P is moving with velocity V.
// Find the range of time for which P overlaps the range R.

inline Range IntersectRangeRange ( Range const & A, real V, Range const & B )
{
	Range timeMin = IntersectFloatGreater(A.getMax(),V,B.getMin());
	Range timeMax = IntersectFloatLess(A.getMin(),V,B.getMax());

	return IntersectRanges(timeMin,timeMax);
}

// ----------------------------------------------------------------------

} // namespace Intersect1d

#endif

