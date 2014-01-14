// ======================================================================
//
// Quadratic.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Quadratic.h"


bool Quadratic::solveFor ( float value, float & out1, float & out2 ) const
{
	if((m_A == 0.0f) && (m_B == 0.0f))
	{
		if(m_C == value)
		{
			out1 = -REAL_MAX;
			out2 = REAL_MAX;

			return true;
		}
		else
		{
			return false;
		}
	}

	// ----------

	if(m_A == 0.0f)
	{
		out1 = (value - m_C) / m_B;
		out2 = (value - m_C) / m_B;

		return true;
	}

	// ----------

	float a = m_A;
	float b = m_B;
	float c = m_C - value;

	float det = b*b - 4*a*c;

	if(det < 0) return false;

	float s = sqrt(det);

	float i = 1.0f / (2.0f * a);

	float o1 = (-b + s) * i;
	float o2 = (-b - s) * i;

	if(o1 < o2)
	{
		out1 = o1;
		out2 = o2;
	}
	else
	{
		out1 = o2;
		out2 = o1;
	}
	
	return true;
}
