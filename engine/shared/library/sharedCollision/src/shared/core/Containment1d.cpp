// ======================================================================
//
// Containment1d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Containment1d.h"

#include "sharedMath/Range.h"

#include <algorithm>

namespace Containment1d
{

// ----------------------------------------------------------------------

ContainmentResult   TestFloatLess   ( float f, float v )
{
	if(f < v)       { return CR_Inside; }
	else if(f == v) { return CR_Boundary; } //lint !e777 // testing floats for equality
	else            { return CR_Outside; }
}

// ----------

ContainmentResult   TestFloatGreater ( float f, float v )
{
	if(f > v)       { return CR_Inside; }
	else if(f == v) { return CR_Boundary; } //lint !e777 // testing floats for equality
	else            { return CR_Outside; }
}

// ----------

ContainmentResult   TestFloatRange ( float f, Range const & R )
{
	if(f < R.getMax())
	{
		if(f > R.getMin())       { return CR_Inside; }
		else if(f == R.getMin()) { return CR_Boundary; }
		else                     { return CR_Outside; }
	}
	else if (f == R.getMax())
	{
		return CR_Boundary;
	}
	else
	{
		return CR_Outside;
	}
}

// ----------------------------------------------------------------------

ContainmentResult   TestRangeLess   ( Range const & R, float v )
{
	if(R.getMin() > v)
	{
		return CR_Outside;
	}
	else if(R.getMin() == v)
	{
		if(R.getMax() == v) { return CR_Boundary; }
		else                { return CR_TouchingOutside; }
	}
	else
	{
		if(R.getMax() > v)        { return CR_Overlap; }
		else if (R.getMax() == v) { return CR_TouchingInside; }
		else                      { return CR_Inside; }
	}
}

// ----------

ContainmentResult   TestRangeGreater ( Range const & R, float v )
{
	if(R.getMax() < v)
	{
		return CR_Outside;
	}
	else if(R.getMax() == v)
	{
		if(R.getMin() == v) { return CR_Boundary; }
		else                { return CR_TouchingOutside; }
	}
	else
	{
		if(R.getMin() < v)        { return CR_Overlap; }
		else if (R.getMin() == v) { return CR_TouchingInside; }
		else                      { return CR_Inside; }
	}
}

// ----------

ContainmentResult   TestRangeRange ( Range const & A, Range const & B )
{
	real const & aMin = A.getMin();
	real const & aMax = A.getMax();

	real const & bMin = B.getMin();
	real const & bMax = B.getMax();

	if(aMax < bMin)
	{
		return CR_Outside;
	}
	else if(aMax == bMin)
	{
		if(aMin < bMin)       { return CR_TouchingOutside; }
		else if(aMin == bMin) { return CR_Boundary; }
		else                  { DEBUG_FATAL(true,("Containment3d::TestRangeRange - Range A is inverted\n")); return CR_Outside; }
	}
	else
	{
		if(aMax < bMax)
		{
			if(aMin < bMin)       { return CR_Overlap; }
			else if(aMin == bMin) { return CR_TouchingInside; }
			else                  { return CR_Inside; }
		}
		else if(aMax == bMax)
		{
			if(aMin < bMin)       { return CR_Overlap; }
			else if(aMin == bMin) { return CR_TouchingInside; }
			else                  { return CR_TouchingInside; }
		}
		else
		{
			if(aMin < bMax)       { return CR_Overlap; }
			else if(aMin == bMax) { return CR_TouchingOutside; }
			else                  { return CR_Outside; }
		}
	}
}

// ----------------------------------------------------------------------

Range EncloseRanges ( Range const & A, Range const & B )
{
	return Range::enclose(A,B);
}

// ----------

Range EncloseRanges ( Range const & A, Range const & B, Range const & C )
{
	return Range::enclose(A,B,C);
}

// ----------

Range EncloseRanges ( Range const & A, Range const & B, Range const & C, Range const & D )
{
	return Range::enclose(A,B,C,D);
}

// ----------------------------------------------------------------------

}	// namespace Containment1d

