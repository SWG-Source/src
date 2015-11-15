// ======================================================================
//
// Distance2d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Distance2d.h"

#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/Containment2d.h"
#include "sharedCollision/Collision2d.h"

#include "sharedMath/Vector.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"
#include "sharedMath/Quadratic.h"
#include "sharedMath/Circle.h"
#include "sharedMath/Ring.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Range.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Cylinder.h"

namespace Distance2d
{

// ----------------------------------------------------------------------
// Static 2d distance

float DistancePointPoint ( Vector const & A, Vector const & B )
{
	Vector delta(B.x - A.x, 0.0f, B.z - A.z);

	return delta.magnitude();
}

float DistancePointLine ( Vector const & V, Line3d const & L )
{
	Vector C = ClosestPointLine(V,L);

	return Distance2d::DistancePointPoint(V,C);
}

float DistancePointRay ( Vector const & V, Ray3d const & R )
{
	Vector C = ClosestPointRay(V,R);

	return Distance2d::DistancePointPoint(V,C);
}

float DistancePointSeg ( Vector const & V, Segment3d const & S )
{
	Vector C = ClosestPointSeg(V,S);

	return Distance2d::DistancePointPoint(V,C);
}

float DistancePointSeg ( Vector const & V, Vector const & begin, Vector const & end )
{
	return Distance2d::DistancePointSeg( V, Segment3d(begin,end) );
}

float DistancePointSphere ( Vector const & V, Sphere const & S )
{
	float dist = Distance2d::DistancePointPoint(V,S.getCenter());

	return dist - S.getRadius();
}

float DistancePointCylinder ( Vector const & V, Cylinder const & C )
{
	float dist = Distance2d::DistancePointPoint(V,C.getBase());

	return dist - C.getRadius();
}

// ----------
// Static 2d distance squared

float Distance2PointPoint ( Vector const & A, Vector const & B )
{
	Vector delta(B.x - A.x, 0.0f, B.z - A.z);

	return delta.magnitudeSquared();
}

float Distance2PointLine ( Vector const & V, Line3d const & L )
{
	Vector C = ClosestPointLine(V,L);

	return Distance2d::Distance2PointPoint(V,C);
}

float Distance2PointRay ( Vector const & V, Ray3d const & R )
{
	Vector C = ClosestPointRay(V,R);

	return Distance2d::Distance2PointPoint(V,C);
}

float Distance2PointSeg ( Vector const & V, Segment3d const & S )
{
	Vector C = ClosestPointSeg(V,S);

	return Distance2d::Distance2PointPoint(V,C);
}

// ----------------------------------------------------------------------

bool ClosestParamLine ( Vector const & V, Line3d const & L, float & outParam )
{
	Vector A = Collision2d::flatten(V);
	Vector P = Collision2d::flatten(L.getPoint());
	Vector D = Collision2d::flatten(L.getNormal());

	float mag2 = D.magnitudeSquared();

	if(mag2 > 0.0f)
	{
		outParam = D.dot(A-P) / mag2;

		return true;
	}
	else
	{
		return false;
	}
}

Vector ClosestPointLine ( Vector const & V, Line3d const & L )
{
	float t(0);

	if(ClosestParamLine(V,L,t))
	{
		return L.atParam(t);
	}
	else
	{
		return L.getPoint();
	}
}

Vector ClosestPointRay ( Vector const & V, Ray3d const & R )
{
	float t(0);

	if(ClosestParamLine(V,R.getLine(),t))
	{
		return R.atParam(Range::plusInf.clamp(t));
	}
	else
	{
		return R.getPoint();
	}
}

Vector ClosestPointSeg ( Vector const & V, Segment3d const & S )
{
	float t(0);

	if(ClosestParamLine(V,S.getLine(),t))
	{
		return S.atParam(Range::unit.clamp(t));
	}
	else
	{
		return S.getBegin();
	}
}

Vector ClosestPointABox ( Vector const & V, AxialBox const & B )
{
	Vector temp;

	Vector min = B.getMin();
	Vector max = B.getMax();

	temp.x = clamp( min.x, V.x, max.x );
	temp.y = 0.0f;
	temp.z = clamp( min.z, V.z, max.z );

	return temp;
}

Vector ClosestPointYBox ( Vector const & V, YawedBox const & B )
{
	Vector lV = B.transformToLocal(V);

	AxialBox lB = B.getLocalShape();

	Vector lC = ClosestPointABox(lV,lB);

	Vector out = B.transformToWorld(lC);

	return out;
}

Vector ClosestPointCircle ( Vector const & V, Circle const & C )
{
	if(Containment2d::TestPointCircle(V,C) == CR_Inside)
	{
		return V;
	}

	Vector D = V - C.getCenter();

	D.y = 0.0f;

	IGNORE_RETURN( D.normalize() );

	D *= C.getRadius();

	D += C.getCenter();

	return D;
}

Vector ClosestPointRing ( Vector const & V, Ring const & R )
{
	Vector D = V - R.getCenter();

	D.y = 0.0f;

	IGNORE_RETURN( D.normalize() );

	D *= R.getRadius();

	D += R.getCenter();

	return D;
}

// ----------------------------------------------------------------------

Quadratic DistancePointPoint ( Vector const & A2, Vector const & V2, Vector const & B2 )
{
	Vector A(A2.x,0.0f,A2.z);
	Vector V(V2.x,0.0f,V2.z);
	Vector B(B2.x,0.0f,B2.z);

	Vector D = (B-A);

	real a = V.magnitudeSquared();
	real b = -2.0f * D.dot(V);
	real c = D.magnitudeSquared();

	return Quadratic(a,b,c);
}

// ----------------------------------------------------------------------

float ProjectAxis ( Line3d const & L, Vector const & V )
{
	return Collision2d::ComponentAlong( V - L.getPoint(), L.getNormal() );
}

Range ProjectAxis ( Line3d const & L, Circle const & C )
{
	Vector N = L.getNormal();
	N.y = 0.0f;

	real d = C.getRadius() / N.magnitude();

	real c = ProjectAxis( L, C.getCenter() );

	return Range( c - d, c + d );
}

// ----------------------------------------------------------------------

} // namespace Distance2d
