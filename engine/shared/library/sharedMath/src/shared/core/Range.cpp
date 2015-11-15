// ======================================================================
//
// Range.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Range.h"
#include "sharedRandom/Random.h"

#include <algorithm>

Range Range::empty (  REAL_MAX, -REAL_MAX );
Range Range::inf   ( -REAL_MAX,  REAL_MAX ); 
Range Range::plusInf ( 0.0f, REAL_MAX );
Range Range::negInf (-REAL_MAX,0.0f);
Range Range::unit (0.0f,1.0f);

Range Range::enclose ( Range const & A, float V )
{
	if(A.isEmpty())
	{
		return Range(V,V);
	}
	else
	{
		return Range(std::min(A.getMin(),V),std::max(A.getMax(),V));
	}
}

Range Range::enclose ( Range const & A, Range const & B )
{
	if(A.isEmpty())
	{
		if(B.isEmpty())
		{
			return Range::empty;
		}
		else
		{
			return B;
		}
	}
	else
	{
		if(B.isEmpty())
		{
			return A;
		}
		else
		{
			return Range(std::min(A.getMin(),B.getMin()), std::max(A.getMax(),B.getMax()));
		}
	}
}

Range Range::enclose ( Range const & A, Range const & B, Range const & C )
{
	return enclose(enclose(A,B),C);
}

Range Range::enclose ( Range const & A, Range const & B, Range const & C, Range const & D )
{
	return enclose(enclose(A,B),enclose(C,D));
}

float Range::random() const
{
	return (m_min > m_max) ? Random::randomReal(m_max, m_min) : Random::randomReal(m_min, m_max);
}
