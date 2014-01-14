// ======================================================================
//
// Collision2d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Collision2d_H
#define INCLUDED_Collision2d_H

#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"

class Triangle2d;

// ----------------------------------------------------------------------
//@todo - Move this to its own class!

extern const real gkInvertEpsilon;

struct	Matrix2
{
	real m00,m01,m10,m11;

	Matrix2()
	{
	}

	Matrix2( real a, real b, real c, real d )
	: m00(a), m01(b), m10(c), m11(d)
	{
	}

	// ----------

	static bool	Invert	( Matrix2 const & mat, Matrix2 & out )
	{
		real det = mat.m00 * mat.m11 - mat.m01 * mat.m10;

		if(fabs(det) < gkInvertEpsilon)
		{
			return false;
		}

		real invDet = 1.0f / det;

		out.m00 =  mat.m11 * invDet;
		out.m01 = -mat.m01 * invDet;
		out.m10 = -mat.m10 * invDet;
		out.m11 =  mat.m00 * invDet;

		return true;
	}

	static Matrix2 fromColumns( Vector2d const & A, Vector2d const & B )
	{
		return Matrix2(A.x,B.x,A.y,B.y);
	}

	Matrix2 inverted ( void ) 
	{
		// compiler complains if this isn't initialized
		Matrix2 temp(0,0,0,0);

		Invert(*this,temp);

		return temp;
	}

	bool solveFor ( Vector2d const & V, Vector2d & result )
	{
		Matrix2 inv(0,0,0,0);

		if(Invert(*this,inv))
		{
			result = inv * V;

			return true;
		}
		else
		{
			return false;
		}
	}

	Vector2d operator * ( Vector2d const & V )
	{
		Vector2d temp;

		temp.x = m00 * V.x + m01 * V.y;
		temp.y = m10 * V.x + m11 * V.y;

		return temp;
	}
};

// ----------------------------------------------------------------------
// 2d operations - work in the X-Y plane

namespace Collision2d
{

inline Vector		flatten		( Vector const & V )	{ return Vector(V.x,0.0f,V.z); }
inline Vector2d		flatten2d	( Vector const & V )	{ return Vector2d(V.x,V.z); }
inline Vector		expand		( Vector2d const & V )	{ return Vector(V.x, 0.0f, V.y); }

};	// namespace Collision2d

// ----------------------------------------------------------------------

#endif
