// ======================================================================
//
// Overlap3d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Overlap3d.h"

#include "sharedCollision/CollisionUtils.h"

#include "sharedCollision/Containment1d.h"

#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Intersect1d.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/Distance2d.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/Overlap2d.h"

#include "sharedMath/Triangle3d.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/Ribbon3d.h"
#include "sharedMath/OrientedBox.h"
#include "sharedMath/Circle.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/Vector.h"
#include "sharedMath/MultiShape.h"


#include <algorithm>	// for swap, max, min

using namespace Collision3d;
using namespace Intersect1d;

// ======================================================================

namespace Overlap3d
{

// ----------------------------------------------------------------------

HitResult TestSegPlane ( Segment3d const & S, Plane3d const & P )
{
	ContainmentResult sideA = Containment3d::TestPointPlane( S.getBegin(), P );
	ContainmentResult sideB = Containment3d::TestPointPlane( S.getEnd(), P );

	if(sideA == sideB) 
	{
		return HR_Miss;	//lint !e731
	}
	else
	{
		return (sideA == CR_Outside) ? HR_HitFront : HR_HitBack;
	}
}

// ----------------------------------------------------------------------

// This table translates the sum of the hit results for each edge into
// a hit result for the triangle

static HitResult returnTable[13] = 
{
	HR_Miss,     // 000 =  0 - degenerate, hits all three edges
	HR_HitFront, // 001 =  1 - hit corner from the front
	HR_HitFront, // 011 =  2 - hit edge from the front
	HR_HitFront, // 111 =  3 - hit front
	HR_HitBack,  // 004 =  4 - hit corner from the back
	HR_Miss,     // 014 =  5 - miss
	HR_Miss,     // 114 =  6 - miss
	HR_Miss,     // XXX =  7 - should never happen
	HR_HitBack,  // 044 =  8 - hit edge from the back
	HR_Miss,     // 144 =  9 - miss 
	HR_Miss,     // XXX = 10 - should never happen
	HR_Miss,     // XXX = 11 - should never happen
	HR_HitBack,  // 444 = 12 - hit the back
};

HitResult TestLineTri ( Line3d const & L, Triangle3d const & T )
{
	int a = TestLineSegTwist( L, T.getEdgeSegment0() );
	int b = TestLineSegTwist( L, T.getEdgeSegment1() );
	int c = TestLineSegTwist( L, T.getEdgeSegment2() );

	if(a == -1) a = 4;
	if(b == -1) b = 4;
	if(c == -1) c = 4;

	return returnTable[a+b+c];
}

// ----------------------------------------------------------------------

HitResult TestRayTri ( Ray3d const & R, Triangle3d const & T )
{
	int whichSideHit = TestLineTri( R.getLine(), T );

	if(whichSideHit == HR_Miss) return HR_Miss;

	// ----------
	
	ContainmentResult sideFlag = Containment3d::TestPointPlane(R.getPoint(),T.getPlane());

	if((whichSideHit == HR_HitFront) && (sideFlag == CR_Outside))
	{
		// Front hit by line and ray origin is in front of the tri - front hit
		return HR_HitFront;
	}
	else if((whichSideHit == HR_HitBack) && (sideFlag == CR_Inside))
	{
		// Back hit by line and ray origin is behind tri - back hit
		return HR_HitBack;
	}
	else
	{
		// No hit
		return HR_Miss;
	}
}

// ----------------------------------------------------------------------

HitResult TestSegTri ( Segment3d const & S, Triangle3d const & T )
{
	Plane3d P = T.getPlane();

	if(TestSegPlane(S,P) == HR_Miss) return HR_Miss;

	return TestLineTri(S.getLine(),T);
}

// ----------------------------------------------------------------------
// A line intersects a ribbon if it twists around the edges of the ribbon
// in different directions


HitResult TestLineRibbon ( Line3d const & L, Ribbon3d const & R )
{
	int A = TestLineLineTwist(L,R.getEdgeA());
	int B = TestLineLineTwist(L,R.getEdgeB());

	HitResult result;

	if(A == 0)
	{
		if(B == 0)		result = HR_Miss;
		else if(B == 1)	result = HR_HitFront;
		else			result = HR_HitBack;
	}
	else if(A == 1)
	{
		if(B == 0)		result = HR_HitBack;
		else if(B == 1)	result = HR_Miss;
		else			result = HR_HitBack;
	}
	else
	{
		if(B == 0)		result = HR_HitFront;
		else if(B == 1)	result = HR_HitFront;
		else			result = HR_Miss;
	}

	return result;
}

// ----------------------------------------------------------------------
// A segment overlaps a ribbon if it overlaps the ribbon's plane and
// the line through it overlaps the ribbon.

HitResult TestSegRibbon ( Segment3d const & S, Ribbon3d const & R )
{
	if(TestLineRibbon(S.getLine(),R) == HR_Miss) return HR_Miss;

	return TestSegPlane(S,R.getPlane());
}

// ----------------------------------------------------------------------

bool TestRibbonRibbon ( Ribbon3d const & A, Ribbon3d const & B )
{
	// Easy test - two ribbons overlap if an edge of either ribbon overlaps
	// the other ribbon

	if(TestLineRibbon(A.getEdgeA(),B) == HR_Hit) return HR_Hit;
	if(TestLineRibbon(A.getEdgeB(),B) == HR_Hit) return HR_Hit;
	if(TestLineRibbon(B.getEdgeA(),A) == HR_Hit) return HR_Hit;
	if(TestLineRibbon(B.getEdgeB(),A) == HR_Hit) return HR_Hit;

	return HR_Miss;
}

// ----------------------------------------------------------------------

// The line only penetrates the triangle if it goes through from front to back

bool    TestSegPlaneSided       ( Segment3d const & S, Plane3d const & P )      { return TestSegPlane(S,P) == HR_HitFront; }
bool    TestSegPlaneUnsided     ( Segment3d const & S, Plane3d const & P )      { return TestSegPlane(S,P) != HR_Miss; }

bool    TestLineTriSided        ( Line3d const & L, Triangle3d const & T )      { return TestLineTri(L,T) == HR_HitFront; }
bool    TestLineTriUnsided      ( Line3d const & L, Triangle3d const & T )      { return TestLineTri(L,T) != HR_Miss; }

bool    TestRayTriSided         ( Ray3d const & R, Triangle3d const & T )       { return TestRayTri(R,T) == HR_HitFront; }
bool    TestRayTriUnsided       ( Ray3d const & R, Triangle3d const & T )       { return TestRayTri(R,T) != HR_Miss; }

bool    TestSegTriSided         ( Segment3d const & S, Triangle3d const & T )   { return TestSegTri(S,T) == HR_HitFront; }
bool    TestSegTriUnsided       ( Segment3d const & S, Triangle3d const & T )   { return TestSegTri(S,T) != HR_Miss; }

bool    TestSegRibbonSided      ( Segment3d const & S, Ribbon3d const & R )     { return TestSegRibbon(S,R) == HR_HitFront; }
bool    TestSegRibbonUnsided    ( Segment3d const & S, Ribbon3d const & R )     { return TestSegRibbon(S,R) != HR_Miss; }

// ----------------------------------------------------------------------

bool TestSegCircle ( Segment3d const & S, Circle const & circle )
{
	Vector A = S.getBegin();
	Vector B = S.getEnd();
	Vector const & C = circle.getCenter();

	if(A.y > B.y) std::swap(A,B);

	if(C.y > B.y) 
	{
		return false;
	}
	else if(C.y < A.y) 
	{
		return false;
	}
	else
	{
		if(A.y == B.y)
		{
			return Overlap2d::TestSegCircle(S,circle);
		}
		else
		{
			float param = (C.y - A.y) / (B.y - A.y);

			Vector D = A + (B - A) * param;

			return Overlap2d::TestPointCircle( D, circle );
		}
	}
}

bool TestSegCircle_Below ( Segment3d const & S, Circle const & circle )
{
	Vector A = S.getBegin();
	Vector B = S.getEnd();
	Vector const & C = circle.getCenter();

	if(A.y > B.y) std::swap(A,B);

	if(C.y > B.y)
	{
		return false;
	}
	else if(C.y < A.y) 
	{
		return Overlap2d::TestSegCircle(S,circle);
	}
	else
	{
		if(A.y == B.y)
		{
			return Overlap2d::TestSegCircle(S,circle);
		}
		else
		{
			float param = (C.y - A.y) / (B.y - A.y);

			Vector D = A + (B - A) * param;

			return Overlap2d::TestSegCircle( Segment3d(D,B), circle );
		}

	}
}

// ----------------------------------------------------------------------

bool TestLineABox ( Line3d const & L, AxialBox const & B )
{
	Vector const & P = L.getPoint();
	Vector const & V = L.getNormal();

	Range timeX = IntersectFloatRange( P.x, V.x, B.getRangeX() );

	if(timeX.isEmpty()) return false;

	Range timeY = IntersectFloatRange( P.y, V.y, B.getRangeY() );

	Range temp = IntersectRanges(timeX,timeY);

	if(temp.isEmpty()) return false;

	Range timeZ = IntersectFloatRange( P.z, V.z, B.getRangeZ() );

	temp = IntersectRanges(temp,timeZ);

	return !temp.isEmpty();
}

// ----------

bool TestRayABox ( Ray3d const & R, AxialBox const & B )
{
	Vector const & P = R.getPoint();
	Vector const & V = R.getNormal();

	Range temp = Range::plusInf;

	Range timeX = IntersectFloatRange( P.x, V.x, B.getRangeX() );

	temp = IntersectRanges(temp,timeX);

	if(temp.isEmpty()) return false;

	Range timeY = IntersectFloatRange( P.y, V.y, B.getRangeY() );

	temp = IntersectRanges(temp,timeY);

	if(temp.isEmpty()) return false;

	Range timeZ = IntersectFloatRange( P.z, V.z, B.getRangeZ() );

	temp = IntersectRanges(temp,timeZ);

	return !temp.isEmpty();
}

// ----------

bool TestSegABox ( Segment3d const & S, AxialBox const & B )
{
	Vector const & P = S.getBegin();
	Vector const & V = S.getDelta();

	Range temp = Range::unit;

	Range timeY = IntersectFloatRange( P.y, V.y, B.getRangeY() );

	temp = IntersectRanges(temp,timeY);

	if(temp.isEmpty()) return false;

	Range timeX = IntersectFloatRange( P.x, V.x, B.getRangeX() );

	temp = IntersectRanges(temp,timeX);

	if(temp.isEmpty()) return false;

	Range timeZ = IntersectFloatRange( P.z, V.z, B.getRangeZ() );

	temp = IntersectRanges(temp,timeZ);

	return !temp.isEmpty();
}

// ----------

bool TestYLineCircle ( Vector const & V, Circle const & C )
{
	return Distance2d::Distance2PointPoint(V,C.getCenter()) < sqr(C.getRadius());
}

bool TestYLineSphere ( Vector const & V, Sphere const & S )
{
	return Distance2d::Distance2PointPoint(V,S.getCenter()) < sqr(S.getRadius());
}

bool TestYLineCylinder ( Vector const & V, Cylinder const & C )
{
	return Distance2d::Distance2PointPoint(V,C.getBase()) < sqr(C.getRadius());
}

bool TestYLineABox ( Vector const & V, AxialBox const & B )
{
	return B.getRangeX().contains(V.x) && B.getRangeZ().contains(V.z);
}

bool TestYLineYBox ( Vector const & V, YawedBox const & B )
{
	return TestYLineABox( B.transformToLocal(V), B.getLocalShape() );
}

bool TestYLineOBox ( Vector const & V, OrientedBox const & B )
{
	Vector point = B.transformToLocal(V);
	Vector normal = B.rotateToLocal(Vector::unitY);

	return TestLineABox( Line3d(point,normal), B.getLocalShape() );
}

bool TestYLineShape ( Vector const & V, MultiShape const & S )
{
	MultiShape::ShapeType type = S.getShapeType();

	// ----------

	if     (type == MultiShape::MST_Sphere)           { return TestYLine( V, S.getSphere() ); }
	else if(type == MultiShape::MST_Cylinder)         { return TestYLine( V, S.getCylinder() ); }
	else if(type == MultiShape::MST_AxialBox)         { return TestYLine( V, S.getAxialBox() ); }
	else if(type == MultiShape::MST_YawedBox)         { return TestYLine( V, S.getYawedBox() ); }
	else if(type == MultiShape::MST_OrientedBox)      { return TestYLine( V, S.getOrientedBox() ); }
	else                                              { return false; }
}

// ----------------------------------------------------------------------
// Moving point vs. volume

bool TestPointABox ( Vector const & P, Vector const & V, AxialBox const & B )
{
	Range timeX = IntersectFloatRange( P.x, V.x, B.getRangeX() );

	if(timeX.isEmpty()) return false;

	Range timeY = IntersectFloatRange( P.y, V.y, B.getRangeY() );

	Range temp = IntersectRanges(timeX,timeY);

	if(temp.isEmpty()) return false;

	Range timeZ = IntersectFloatRange( P.z, V.z, B.getRangeZ() );

	temp = IntersectRanges(temp,timeZ);

	return !temp.isEmpty();
}

// ----------------------------------------------------------------------
// Planar-vs-planar overlap tests

bool TestTriTri ( Triangle3d const & A, Triangle3d const & B )
{
	// Two triangles overlap iff any edge from one triangle overlaps the other triangle

	if(TestSegTriUnsided(A.getEdgeSegment0(),B)) return true;
	if(TestSegTriUnsided(A.getEdgeSegment1(),B)) return true;
	if(TestSegTriUnsided(A.getEdgeSegment2(),B)) return true;

	if(TestSegTriUnsided(B.getEdgeSegment0(),A)) return true;
	if(TestSegTriUnsided(B.getEdgeSegment1(),A)) return true;
	if(TestSegTriUnsided(B.getEdgeSegment2(),A)) return true;

	return false;
}



// ----------------------------------------------------------------------
// Volume-vs-planar overlap tests

bool TestSpherePlane ( Sphere const & A, Plane3d const & B )
{
	float dist = std::abs(Distance3d::DistancePointPlane(A.getCenter(),B));

	return dist <= A.getRadius();
}

bool TestSpherePoly ( Sphere const & A, VertexList const & verts, Plane3d const & P )
{
	if(!TestSpherePlane(A,P)) return false;

	float dist2 = Distance3d::Distance2PointPoly(A.getCenter(),verts);

	return dist2 < (A.getRadius() * A.getRadius());
}

// ----------------------------------------------------------------------
// Volume-vs-volume overlap tests

bool TestABoxABox ( AxialBox const & A, AxialBox const & B )
{
	Vector const & minA = A.getMin();
	Vector const & maxA = A.getMax();

	Vector const & minB = B.getMin();
	Vector const & maxB = B.getMax();

	if(minA.x > maxB.x) return false;
	if(minA.y > maxB.y) return false;
	if(minA.z > maxB.z) return false;

	if(minB.x > maxA.x) return false;
	if(minB.y > maxA.y) return false;
	if(minB.z > maxA.z) return false;

	return true;
}

// ----------------------------------------------------------------------
// Overlap test for two oriented boxes adapted from David Eberly's version
// at www.magic-software.com


bool TestOBoxOBox ( OrientedBox const & boxA, OrientedBox const & boxB )
{
	// convenience variables
	const Vector* akA = boxA.getAxes();
	const Vector* akB = boxB.getAxes();
	const float* afEA = boxA.getExtents();
	const float* afEB = boxB.getExtents();

	// compute difference of box centers, D = C1-C0
	Vector kD = boxB.getCenter() - boxA.getCenter();

	float aafC[3][3];     // matrix C = A^T B, c_{ij} = Dot(A_i,B_j)
	float aafAbsC[3][3];  // |c_{ij}|
	float afAD[3];        // Dot(A_i,D)
	float fR0, fR1, fR;   // interval radii and distance between centers
	float fR01;           // = R0 + R1

	// axis C0+t*A0
	aafC[0][0] = akA[0].dot(akB[0]);
	aafC[0][1] = akA[0].dot(akB[1]);
	aafC[0][2] = akA[0].dot(akB[2]);
	afAD[0] = akA[0].dot(kD);
	aafAbsC[0][0] = static_cast<float>(fabs(aafC[0][0]));
	aafAbsC[0][1] = static_cast<float>(fabs(aafC[0][1]));
	aafAbsC[0][2] = static_cast<float>(fabs(aafC[0][2]));
	fR = static_cast<float>(fabs(afAD[0]));
	fR1 = afEB[0]*aafAbsC[0][0]+afEB[1]*aafAbsC[0][1]+afEB[2]*aafAbsC[0][2];
	fR01 = afEA[0] + fR1;
	if ( fR > fR01 )
		return false;

	// axis C0+t*A1
	aafC[1][0] = akA[1].dot(akB[0]);
	aafC[1][1] = akA[1].dot(akB[1]);
	aafC[1][2] = akA[1].dot(akB[2]);
	afAD[1] = akA[1].dot(kD);
	aafAbsC[1][0] = static_cast<float>(fabs(aafC[1][0]));
	aafAbsC[1][1] = static_cast<float>(fabs(aafC[1][1]));
	aafAbsC[1][2] = static_cast<float>(fabs(aafC[1][2]));
	fR = static_cast<float>(fabs(afAD[1]));
	fR1 = afEB[0]*aafAbsC[1][0]+afEB[1]*aafAbsC[1][1]+afEB[2]*aafAbsC[1][2];
	fR01 = afEA[1] + fR1;
	if ( fR > fR01 )
		return false;

	// axis C0+t*A2
	aafC[2][0] = akA[2].dot(akB[0]);
	aafC[2][1] = akA[2].dot(akB[1]);
	aafC[2][2] = akA[2].dot(akB[2]);
	afAD[2] = akA[2].dot(kD);
	aafAbsC[2][0] = static_cast<float>(fabs(aafC[2][0]));
	aafAbsC[2][1] = static_cast<float>(fabs(aafC[2][1]));
	aafAbsC[2][2] = static_cast<float>(fabs(aafC[2][2]));
	fR = static_cast<float>(fabs(afAD[2]));
	fR1 = afEB[0]*aafAbsC[2][0]+afEB[1]*aafAbsC[2][1]+afEB[2]*aafAbsC[2][2];
	fR01 = afEA[2] + fR1;
	if ( fR > fR01 )
		return false;

	// axis C0+t*B0
	fR = static_cast<float>(fabs(akB[0].dot(kD)));
	fR0 = afEA[0]*aafAbsC[0][0]+afEA[1]*aafAbsC[1][0]+afEA[2]*aafAbsC[2][0];
	fR01 = fR0 + afEB[0];
	if ( fR > fR01 )
		return false;

	// axis C0+t*B1
	fR = static_cast<float>(fabs(akB[1].dot(kD)));
	fR0 = afEA[0]*aafAbsC[0][1]+afEA[1]*aafAbsC[1][1]+afEA[2]*aafAbsC[2][1];
	fR01 = fR0 + afEB[1];
	if ( fR > fR01 )
		return false;

	// axis C0+t*B2
	fR = static_cast<float>(fabs(akB[2].dot(kD)));
	fR0 = afEA[0]*aafAbsC[0][2]+afEA[1]*aafAbsC[1][2]+afEA[2]*aafAbsC[2][2];
	fR01 = fR0 + afEB[2];
	if ( fR > fR01 )
		return false;

	// axis C0+t*A0xB0
	fR = static_cast<float>(fabs(afAD[2]*aafC[1][0]-afAD[1]*aafC[2][0]));
	fR0 = afEA[1]*aafAbsC[2][0] + afEA[2]*aafAbsC[1][0];
	fR1 = afEB[1]*aafAbsC[0][2] + afEB[2]*aafAbsC[0][1];
	fR01 = fR0 + fR1;
	if ( fR > fR01 )
		return false;

	// axis C0+t*A0xB1
	fR = static_cast<float>(fabs(afAD[2]*aafC[1][1]-afAD[1]*aafC[2][1]));
	fR0 = afEA[1]*aafAbsC[2][1] + afEA[2]*aafAbsC[1][1];
	fR1 = afEB[0]*aafAbsC[0][2] + afEB[2]*aafAbsC[0][0];
	fR01 = fR0 + fR1;
	if ( fR > fR01 )
		return false;

	// axis C0+t*A0xB2
	fR = static_cast<float>(fabs(afAD[2]*aafC[1][2]-afAD[1]*aafC[2][2]));
	fR0 = afEA[1]*aafAbsC[2][2] + afEA[2]*aafAbsC[1][2];
	fR1 = afEB[0]*aafAbsC[0][1] + afEB[1]*aafAbsC[0][0];
	fR01 = fR0 + fR1;
	if ( fR > fR01 )
		return false;

	// axis C0+t*A1xB0
	fR = static_cast<float>(fabs(afAD[0]*aafC[2][0]-afAD[2]*aafC[0][0]));
	fR0 = afEA[0]*aafAbsC[2][0] + afEA[2]*aafAbsC[0][0];
	fR1 = afEB[1]*aafAbsC[1][2] + afEB[2]*aafAbsC[1][1];
	fR01 = fR0 + fR1;
	if ( fR > fR01 )
		return false;

	// axis C0+t*A1xB1
	fR = static_cast<float>(fabs(afAD[0]*aafC[2][1]-afAD[2]*aafC[0][1]));
	fR0 = afEA[0]*aafAbsC[2][1] + afEA[2]*aafAbsC[0][1];
	fR1 = afEB[0]*aafAbsC[1][2] + afEB[2]*aafAbsC[1][0];
	fR01 = fR0 + fR1;
	if ( fR > fR01 )
		return false;

	// axis C0+t*A1xB2
	fR = static_cast<float>(fabs(afAD[0]*aafC[2][2]-afAD[2]*aafC[0][2]));
	fR0 = afEA[0]*aafAbsC[2][2] + afEA[2]*aafAbsC[0][2];
	fR1 = afEB[0]*aafAbsC[1][1] + afEB[1]*aafAbsC[1][0];
	fR01 = fR0 + fR1;
	if ( fR > fR01 )
		return false;

	// axis C0+t*A2xB0
	fR = static_cast<float>(fabs(afAD[1]*aafC[0][0]-afAD[0]*aafC[1][0]));
	fR0 = afEA[0]*aafAbsC[1][0] + afEA[1]*aafAbsC[0][0];
	fR1 = afEB[1]*aafAbsC[2][2] + afEB[2]*aafAbsC[2][1];
	fR01 = fR0 + fR1;
	if ( fR > fR01 )
		return false;

	// axis C0+t*A2xB1
	fR = static_cast<float>(fabs(afAD[1]*aafC[0][1]-afAD[0]*aafC[1][1]));
	fR0 = afEA[0]*aafAbsC[1][1] + afEA[1]*aafAbsC[0][1];
	fR1 = afEB[0]*aafAbsC[2][2] + afEB[2]*aafAbsC[2][0];
	fR01 = fR0 + fR1;
	if ( fR > fR01 )
		return false;

	// axis C0+t*A2xB2
	fR = static_cast<float>(fabs(afAD[1]*aafC[0][2]-afAD[0]*aafC[1][2]));
	fR0 = afEA[0]*aafAbsC[1][2] + afEA[1]*aafAbsC[0][2];
	fR1 = afEB[0]*aafAbsC[2][1] + afEB[1]*aafAbsC[2][0];
	fR01 = fR0 + fR1;
	if ( fR > fR01 )
		return false;

	return true;
}

// ----------------------------------------------------------------------

bool TestSphereABox ( Sphere const & A, AxialBox const & B )
{
	Vector close = Distance3d::ClosestPointABox(A.getCenter(),B);

	float dist2 = close.magnitudeBetweenSquared(A.getCenter());

	float radius = A.getRadius();

	return dist2 < (radius * radius);
}

bool TestSphereYBox ( Sphere const & A, YawedBox const & B )
{
	Vector close = Distance3d::ClosestPointYBox(A.getCenter(),B);

	float dist2 = close.magnitudeBetweenSquared(A.getCenter());

	float radius = A.getRadius();

	return dist2 < (radius * radius);
}

bool TestSphereOBox ( Sphere const & A, OrientedBox const & B )
{
	Vector close = Distance3d::ClosestPointOBox(A.getCenter(),B);

	float dist2 = close.magnitudeBetweenSquared(A.getCenter());

	float radius = A.getRadius();

	return dist2 < (radius * radius);
}







// ----------------------------------------------------------------------
// Projected overlap tests

// This test tries to find a separating plane for the two triangles
// 

bool TestTriTri_proj ( Triangle3d const & inTriA,
                       Triangle3d const & inTriB,
                       Vector const & direction,
                       real epsilon )
{
	Triangle3d triA = inTriA;
	Triangle3d triB = inTriB;

	if(!triA.isFacing(direction)) triA.flip();
	if(!triB.isFacing(direction)) triB.flip();

	int i;

	// ----------

	for(i = 0; i < 3; i++)
	{
		Plane3d P = Plane3d( triA.getCorner(i), triA.getEdgeDir(i).cross( direction ) );

		ContainmentResult test = Containment3d::TestTriPlane( triB, P, epsilon );

		if( Containment::isNonContainment(test) ) return false;
	}
	
	// ----------

	for(i = 0; i < 3; i++)
	{
		Plane3d P = Plane3d( triB.getCorner(i), triB.getEdgeDir(i).cross( direction ) );

		ContainmentResult test = Containment3d::TestTriPlane( triA, P, epsilon );

		if( Containment::isNonContainment(test) ) return false;
	}

	// ----------

	return true;
}

// ----------

bool TestSegSeg_proj ( Segment3d const & segA,
                       Segment3d const & segB,
                       Vector const & direction )
{
	Ribbon3d R(segA,direction);

	return TestSegRibbonUnsided(segB,R);
}

// ----------------------------------------------------------------------

}	// namespace Overlap3d

