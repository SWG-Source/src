// ======================================================================
//
// RangeLoop.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/RangeLoop.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedMath/ConfigSharedMath.h"

#include <cmath>

// ----------

float RangeLoop::clip ( float x )
{
	return x - (float)floor(x);
}

// distance you have to go in the positive direction to get from A to B

float RangeLoop::distancePositive ( float A, float B )
{
	A = clip(A);
	B = clip(B);

	return (B > A) ? -A + B : 1 - A + B;
}

// distance you have to go in the negative direction to get from A to B

float RangeLoop::distanceNegative ( float A, float B )
{
	A = clip(A);
	B = clip(B);

	return (B > A) ? 1 + A - B : A - B;
}

bool RangeLoop::containsInclusive ( float min, float max, float x )
{
	min = clip(min);
	max = clip(max);
	x = clip(x);

	if(max > min)
	{
		return ( x >= min ) && ( x <= max );
	}
	else
	{
		return ( x >= min ) || ( x <= max );
	}
}

// distancePositive(A,B) + distanceNegative(A,B) == 1.0

// signed distance from A to B via whichever way's closer

float distance ( float A, float B )
{
	A = RangeLoop::clip(A);
	B = RangeLoop::clip(B);

	float positive = RangeLoop::distancePositive(A,B);

	if(positive <= 0.5)
	{
		return positive;
	}
	else
	{
		return positive-1;
	}
}



// ======================================================================

RangeLoop::RangeLoop ( void )
: m_min( -1.0f ),
  m_max( -1.0f )
{
}

RangeLoop::RangeLoop ( float min, float max )
: m_min( clip(min) ),
  m_max( clip(max) )
{
	if(std::abs(max-min) >= 1.0f)
	{
		m_min = 0.0f;
		m_max = 1.0f;
	}
}

// ----------

RangeLoop RangeLoop::empty;
RangeLoop RangeLoop::full(0.0f,1.0f);

// ----------------------------------------------------------------------

bool RangeLoop::isEmpty ( void ) const
{
	return (m_min == -1.0f) && (m_max == -1.0f);
}

bool RangeLoop::isFull ( void ) const
{
	return (m_min == 0.0f) && (m_max == 1.0f);
}

float RangeLoop::atParam ( float t ) const
{
	if(m_min <= m_max)
	{
		return m_min + (m_max - m_min) * t;
	}
	else
	{
		return clip( m_min + (m_max - m_min + 1.0f) * t );
	}
}

float RangeLoop::getSize ( void ) const
{
	if(m_min <= m_max)
	{
		return m_max - m_min;
	}
	else
	{
		return m_max - m_min + 1.0f;
	}
}

// ----------------------------------------------------------------------
// true if
//         [----R----]
//         xxxxxxxxxxx

bool RangeLoop::containsInclusive ( float x ) const
{
	if(isFull()) return true;
	if(isEmpty()) return false;

	x = clip(x);

	if(m_max == m_min)
	{
		return x == m_min;
	}
	else if(m_max > m_min)
	{
		return ( x >= m_min ) && ( x <= m_max );
	}
	else
	{
		return ( x >= m_min ) || ( x <= m_max );
	}
}

// ----------
// true if
//         (----R----)
//          xxxxxxxxx

bool RangeLoop::containsExclusive ( float x ) const
{
	if(isFull()) return true;
	if(isEmpty()) return false;

	x = clip(x);

	if(m_max == m_min)
	{
		return x == m_min;
	}
	else if(m_max > m_min)
	{
		return ( x > m_min ) && ( x < m_max );
	}
	else
	{
		return ( x > m_min ) || ( x < m_max );
	}
}

// ----------
// true if
//            (----A----)
//                    (----B----)

bool RangeLoop::overlapPositive ( RangeLoop const & R ) const
{
	if(isEmpty() || R.isEmpty()) return false;
	if(isFull() || R.isFull()) return true;

	if(!containsExclusive(R.m_min)) return false;
	if( containsExclusive(R.m_max)) return false;

	if(!R.containsExclusive(m_max)) return false;
	if( R.containsExclusive(m_min)) return false;

	return true;
}

// true if
//            (----B----)
//                    (----A----)

bool RangeLoop::overlapNegative ( RangeLoop const & R ) const
{
	if(isEmpty() || R.isEmpty()) return false;
	if(isFull() || R.isFull()) return true;

	if(!containsExclusive(R.m_max)) return false;
	if( containsExclusive(R.m_min)) return false;

	if(!R.containsExclusive(m_min)) return false;
	if( R.containsExclusive(m_max)) return false;

	return true;
}

// true if
//             [--A--]
//                     [--B--]

bool RangeLoop::disjointInclusive ( RangeLoop const & R ) const
{
	if(isEmpty() || R.isEmpty()) return false;
	if(isFull() || R.isFull()) return true;

	if(containsInclusive(R.m_min)) return false;
	if(containsInclusive(R.m_max)) return false;

	if(R.containsInclusive(m_min)) return false;
	if(R.containsInclusive(m_max)) return false;

	return true;
}

// true if
//             (--A--)
//                   (--B--)

bool RangeLoop::disjointExclusive ( RangeLoop const & R ) const
{
	if(isEmpty() || R.isEmpty()) return true;
	if(isFull() || R.isFull()) return false;

	if(containsExclusive(R.m_min)) return false;
	if(containsExclusive(R.m_max)) return false;

	if(R.containsExclusive(m_min)) return false;
	if(R.containsExclusive(m_max)) return false;

	return true;
}

bool RangeLoop::contains ( RangeLoop const & R ) const
{
	if(isEmpty() || R.isEmpty()) return false;
	if(isFull()) return true;
	if(R.isFull()) return false;

	if(!containsInclusive(R.m_min)) return false;
	if(!containsInclusive(R.m_max)) return false;

	return true;
}

bool RangeLoop::operator == ( RangeLoop const & R ) const
{
	if(m_min != R.m_min) return false;
	if(m_max != R.m_max) return false;

	return true;
}

// ----------

RangeLoop RangeLoop::enclose ( float A, float B )
{
	if(distancePositive(A,B) < 0.5f)
	{
		return RangeLoop(A,B);
	}
	else
	{
		return RangeLoop(B,A);
	}
}

RangeLoop RangeLoop::enclose ( RangeLoop const & A, float B )
{
	if(A.isEmpty())
	{
		return RangeLoop(B,B);
	}
	else if(A.isFull())
	{
		return RangeLoop::full;
	}
	else if(A.containsInclusive(B))
	{
		return A;
	}
	else
	{
		float distA = distancePositive(A.getMax(),B);
		float distB = distanceNegative(A.getMin(),B);

		if(distA < distB)
		{
			return RangeLoop(A.getMin(),B);
		}
		else
		{
			return RangeLoop(B,A.getMax());
		}
	}
}

RangeLoop RangeLoop::enclose ( RangeLoop const & A, RangeLoop const & B )
{
#ifdef _DEBUG

	A.validate();
	B.validate();

#endif

	// ----------
	// check full ranges

	bool fullA = A.isFull();
	bool fullB = B.isFull();

	if(fullA || fullB)
	{
		return RangeLoop::full;
	}

	// ----------
	// check empty ranges

	bool emptyA = A.isEmpty();
	bool emptyB = B.isEmpty();

	if(emptyA && emptyB)
	{
		return RangeLoop::empty;
	}
	else if(emptyA)
	{
		return B;
	}
	else if(emptyB)
	{
		return A;
	}

	// ----------
	// check containing/disjoint ranges

	if(A.contains(B))
	{
		return A;
	}
	else if(B.contains(A))
	{
		return B;
	}
	else if (A.disjointExclusive(B))
	{
		float distA = distancePositive(A.getMin(),B.getMax());
		float distB = distanceNegative(A.getMax(),B.getMin());

		if(distA < distB)
		{
			return RangeLoop(A.getMin(),B.getMax());
		}
		else
		{
			return RangeLoop(B.getMin(),A.getMax());
		}
	}

	// ----------
	// check overlapping ranges

	bool overlapAB = A.overlapPositive(B);
	bool overlapBA = B.overlapPositive(A);

	if(overlapAB && overlapBA)
	{
		return RangeLoop(0.0f,1.0f);
	}
	else if(overlapAB)
	{
		return RangeLoop(A.getMin(),B.getMax());
	}
	else if(overlapBA)
	{
		return RangeLoop(B.getMin(),A.getMax());
	}

	// ----------
	// check adjacent ranges

	bool touchAB = (A.getMax() == B.getMin());
	bool touchBA = (B.getMax() == A.getMin());

	if(touchAB && touchBA)
	{
		return RangeLoop(0.0f,1.0f);
	}
	else if(touchAB)
	{
		return RangeLoop(A.getMin(),B.getMax());
	}
	else if(touchBA)
	{
		return RangeLoop(B.getMin(),A.getMax());
	}

	// ----------
	// something's screwed up - we should have caught all the cases by now

	WARNING_STRICT_FATAL(ConfigSharedMath::getReportRangeLoopWarnings(),("RangeLoop::enclose is broken - (%f,%f) (%f,%f)\n",A.getMin(),A.getMax(),B.getMin(),B.getMax()));
	return RangeLoop(0.0f,1.0f);
}

// ----------------------------------------------------------------------

void RangeLoop::validate ( void ) const
{
#ifdef _DEBUG

	if(isFull()) return;

	if(isEmpty()) return;

	if((m_min < 0.0f) || (m_max >= 1.0f))
	{
		DEBUG_WARNING(ConfigSharedMath::getReportRangeLoopWarnings(),("RangeLoop::validate - range (%f,%f) is invalid\n",m_min,m_max));
	}

#endif
}


// ======================================================================
