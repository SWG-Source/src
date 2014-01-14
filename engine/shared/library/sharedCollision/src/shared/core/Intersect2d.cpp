// ======================================================================
//
// Intersect2d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Intersect2d.h"

#include "sharedCollision/Collision2d.h"
#include "sharedCollision/Intersect1d.h"
#include "sharedCollision/Distance2d.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/Containment1d.h"
#include "sharedCollision/CollisionUtils.h"

#include "sharedMath/Range.h"
#include "sharedMath/Vector2d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/Quadratic.h"
#include "sharedMath/Circle.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"

using namespace Collision2d;

namespace Intersect2d
{
// ----------------------------------------------------------------------
// Find S and T such that 

// lA + dA * s = lB + dB * t

// lAx + dAx * s = lBx + dBx * t
// lAy + dAy * s = lBy + dBy * t

// dAx * s - dBx * t = lBx - lAx
// dAy * s - dBy * t = lBy - lAy

// [  dAx  -dBx  | (lBx - lAx) ]
// [  dAy  -dBy  | (lBy - lAy) ]

bool IntersectLineLine ( Line3d const & A,
                         Line3d const & B,
                         float & outParamA,      // parametric coord of intersection point for line A
                         float & outParamB )     // parametric coord of intersection point for line B
{
	Vector2d lA = flatten2d( A.getPoint() );
	Vector2d dA = flatten2d( A.getNormal() );
	Vector2d lB = flatten2d( B.getPoint() );
	Vector2d dB = flatten2d( B.getNormal() );

	Matrix2 M = Matrix2::fromColumns(dA,-dB);

	Vector2d O(0.0f,0.0f);
	
	if(M.solveFor(lB-lA,O))
	{
		outParamA = O.x;
		outParamB = O.y;

		return true;
	}
	else
	{
		return false;
	}
}

// ----------

Vector IntersectLineLine ( Line3d const & A, Line3d const & B )
{
	float s(0);
	float t(0);

	if(IntersectLineLine(A,B,s,t))
	{
		return A.atParam(s);
	}
	else
	{
		return A.getPoint();
	}
}

// ----------

bool IntersectRayRay ( Ray3d const & A, Ray3d const & B, float & outParamA, float & outParamB )
{
	Vector2d lA = flatten2d( A.getPoint() );
	Vector2d dA = flatten2d( A.getNormal() );
	Vector2d lB = flatten2d( B.getPoint() );
	Vector2d dB = flatten2d( B.getNormal() );

	Matrix2 M = Matrix2::fromColumns(dA,-dB);

	Vector2d O(0.0f,0.0f);
	
	if(M.solveFor(lB-lA,O))
	{
		if(O.x >= 0.0f && O.y >= 0.0f)
		{
			outParamA = O.x;
			outParamB = O.y;

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

// ----------

bool IntersectRayRay ( Ray3d const & A, Ray3d const & B, Vector & out )
{
	float s;
	float t;

	if(IntersectRayRay(A,B,s,t))
	{
		out = A.atParam(s);
		return true;
	}
	else
	{
		return false;
	}
}

// ----------

bool IntersectSegSeg ( Segment3d const & A,
                       Segment3d const & B,
                       float & outParamA,
                       float & outParamB )
{
	float tempA(0);
	float tempB(0);

	IGNORE_RETURN( IntersectLineLine( A.getLine(), B.getLine(), tempA, tempB ) );

	if((tempA < 0.0f) || (tempA > 1.0f))	return false;
	if((tempB < 0.0f) || (tempB > 1.0f))	return false;

	outParamA = tempA;
	outParamB = tempB;

	return true;
}

// ----------

bool IntersectLineSeg ( Line3d const & A, Segment3d const & B, float & outParamA, float & outParamB )
{
	float tempA(0);
	float tempB(0);

	IGNORE_RETURN( IntersectLineLine( A, B.getLine(), tempA, tempB ) );

	if((tempB < 0.0f) || (tempB > 1.0f))	return false;

	outParamA = tempA;
	outParamB = tempB;

	return true;
}

// ----------

bool IntersectRaySeg ( Ray3d const & A, Segment3d const & B, float & outParamA, float & outParamB )
{
	float tempA(0);
	float tempB(0);

	IGNORE_RETURN( IntersectLineLine( A.getLine(), B.getLine(), tempA, tempB ) );

	if(tempA < 0.0f) return false;
	if((tempB < 0.0f) || (tempB > 1.0f))	return false;

	outParamA = tempA;
	outParamB = tempB;

	return true;
}

// ----------------------------------------------------------------------

Range IntersectPointSeg ( Vector const & A, Vector const & V, Segment3d const & B )
{
	float tempA(0);
	float tempB(0);

	Ray3d R(A,V);

	if( IntersectRaySeg( R, B, tempA, tempB ) )
	{
		return Range(tempA,tempA);
	}
	else
	{
		return Range::empty;
	}
}

// ----------

Range IntersectPointCircle ( Vector const & A, Vector const & V, Circle const & B )
{
	Quadratic Q = Distance2d::DistancePointPoint(A,V,B.getCenter());

	float rad2 = B.getRadius() * B.getRadius();

    float newMin(0);
    float newMax(0);

	if(Q.solveFor( rad2, newMin, newMax ))
	{
		return Range(newMin,newMax);
	}
	else
	{
		if(V.magnitudeSquared() == 0.0f)
		{
			if(Q.m_C < rad2)
			{
				return Range::inf;
			}
			else
			{
				return Range::empty;
			}
		}
		else
		{
			return Range::empty;
		}
	}
}

// ----------

Range IntersectPointABox ( Vector const & A, Vector const & V, AxialBox const & B )
{
	Range timeX = Intersect1d::IntersectFloatRange( A.x, V.x, B.getRangeX() );
	Range timeZ = Intersect1d::IntersectFloatRange( A.z, V.z, B.getRangeZ() );
	
	return Intersect1d::IntersectRanges(timeX,timeZ);
}

// ----------------------------------------------------------------------

Range IntersectSegCircle ( Segment3d const & S, Circle const & C )
{
	Quadratic Q = Distance2d::DistancePointPoint( S.getBegin(), S.getDelta(), C.getCenter() );

	float minT(0);
	float maxT(0);

	if(Q.solveFor(C.getRadiusSquared(),minT,maxT))
	{
		Range temp(minT,maxT);

		return Intersect1d::IntersectRanges( temp, Range::unit );
	}
	else
	{
		return Range::empty;
	}
}

// ----------------------------------------------------------------------

Range IntersectCirclePoint ( Circle const & C, Vector const & V, Vector const & P )
{
	return IntersectPointCircle(P,-V,C);
}

// IntersectCircleLine in 2d is a bit easier than in 3d

Range IntersectCircleLine ( Circle const & A, Vector const & V, Line3d const & L )
{
	// create a line perpendicular to L that passes through L's origin

	Vector N = L.getNormal();

	Vector D(-N.z,0.0f,N.x);

	Line3d L2(L.getPoint(),D);

	// and our intersection time with the line is the time for which the projection
	// of the circle onto the perpendicular overlaps its origin.

	Range r = Distance2d::ProjectAxis(L2,A);

	float v = Collision2d::ComponentAlong(V,D);

	Range result = Intersect1d::IntersectRangeFloat(r,v,0.0f);

	return result;
}

// This method needs explanation!

// || -> logical union of time ranges
// && -> logical intersection of time ranges

// Circle vs. segment 
// == point vs. capsule 
// == point vs. sphere A || point vs. cylinder || point vs. sphere B

// point vs. cylinder == point vs. slab && point vs. tube
// point vs. tube == sphere vs. line

// point vs. slab projects to float vs. range

// -> sphere vs. segment = point vs. spheres || (sphere vs. line && float vs. range)

Range IntersectCircleSeg ( Circle const & A, Vector const & V, Segment3d const & S )
{
	Vector N = S.getDelta();
	N.y = 0.0f;

	Line3d L(S.getBegin(),N);

	float c = Distance2d::ProjectAxis(L,A.getCenter());
	float v = Collision2d::ComponentAlong(V,L.getNormal());

	Range slabRange = Intersect1d::IntersectFloatRange( c, v, Range(0.0f,1.0f) );
	Range tubeRange = IntersectCircleLine(A,V,L);

	Range innerRange = Intersect1d::IntersectRanges(tubeRange,slabRange);

	Range beginRange = Intersect2d::IntersectCirclePoint(A,V,S.getBegin());
	Range endRange = Intersect2d::IntersectCirclePoint(A,V,S.getEnd());

	Range outerRange = Containment1d::EncloseRanges(beginRange,innerRange,endRange);

	return outerRange;
}

Range IntersectCircleCircle ( Circle const & A, Vector const & V, Circle const & B )
{
	float sumRadius = A.getRadius() + B.getRadius();

	float sumRad2 = sumRadius * sumRadius;

	if((V.x == 0.0f) && (V.z == 0.0f))
	{
		Vector delta = B.getCenter() - A.getCenter();

		float dist = delta.magnitude();

		if(dist <= sumRadius)
		{
			return Range::inf;
		}
		else
		{
			return Range::empty;
		}
	}

	float newMin(0);
    float newMax(0);

	Quadratic Q = Distance2d::DistancePointPoint(A.getCenter(),V,B.getCenter());

	if(Q.solveFor( sumRad2, newMin, newMax ))
	{
		return Range(newMin,newMax);
	}
	else
	{
		return Range::empty;
	}
}

Range IntersectCirclePlane ( Circle const & C, Vector const & V, Plane3d const & P )
{
	Vector N = P.getNormal();
	N.y = 0.0f;

	Range R = Distance2d::ProjectAxis(Line3d(P.getPoint(),N),C);

	float vel = Collision2d::ComponentAlong(V,N);

	return Intersect1d::IntersectRangeFloat( R, vel, 0.0f );
}

Range IntersectCircleABox ( Circle const & C, Vector const & V, AxialBox const & B )
{
	Range accum = Range::empty;
	Range temp = Range::empty;

	float R = C.getRadius();
	Vector P = C.getCenter();

	// Accumulate collision ranges for the box corners

	for(int i = 0; i < 4; i++)
	{
		// flipping this test to do sphere-vs-point

		temp = IntersectPointCircle( B.getCorner(i), -V, C );

		accum = Containment1d::EncloseRanges(accum,temp);
	}

	// Accumulate collision ranges for the box stretched along each axis

	AxialBox xBox( B.getMin() - Vector(R,0.0f,0.0f), B.getMax() + Vector(R,0.0f,0.0f) );
	AxialBox zBox( B.getMin() - Vector(0.0f,0.0f,R), B.getMax() + Vector(0.0f,0.0f,R) );

	Range hitX = IntersectPointABox(P,V,xBox);
	Range hitZ = IntersectPointABox(P,V,zBox);

	accum = Containment1d::EncloseRanges(accum,hitX);
	accum = Containment1d::EncloseRanges(accum,hitZ);

	// and the cumulative range is our result

	return accum;
}

Range IntersectCircleYBox ( Circle const & C, Vector const & V, YawedBox const & B )
{
	Vector localCenter = B.transformToLocal(C.getCenter());

	Circle localCircle(localCenter,C.getRadius());

	AxialBox localBox = B.getLocalShape();

	Vector localVelocity = B.rotateToLocal(V);

	return IntersectCircleABox( localCircle, localVelocity, localBox );
}

Range Intersect ( Vector const & A, Vector const & V, Circle const & B ) { return IntersectPointCircle(A,V,B); }
Range Intersect ( Circle const & A, Vector const & V, Circle const & B ) { return IntersectCircleCircle(A,V,B); }
Range Intersect ( Circle const & A, Vector const & V, AxialBox const & B ) { return IntersectCircleABox(A,V,B); }
Range Intersect ( Circle const & A, Vector const & V, YawedBox const & B ) { return IntersectCircleYBox(A,V,B); }

// ----------------------------------------------------------------------
} // namespace Intersect2d
