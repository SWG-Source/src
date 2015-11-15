// ======================================================================
//
// Intersect2d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Intersect2d.h"

#include "sharedMath/Line3d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Circle.h"
#include "sharedMath/Range.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Triangle3d.h"

#include "sharedCollision/Intersect1d.h"
#include "sharedCollision/Distance2d.h"
#include "sharedCollision/Containment2d.h"

namespace Overlap2d
{

// ----------------------------------------------------------------------

bool TestPointCircle ( Vector const & V, Circle const & C )
{
	return Distance2d::Distance2PointPoint( C.getCenter(), V ) <= C.getRadiusSquared();
}

bool TestLineCircle ( Line3d const & L, Circle const & C )
{
	return Distance2d::Distance2PointLine( C.getCenter(), L ) <= C.getRadiusSquared();
}

bool TestCircleCircle ( Circle const & A, Circle const & B )
{
	float dist2 = Distance2d::Distance2PointPoint( A.getCenter(), B.getCenter() );

	float rad = A.getRadius() * B.getRadius();

	float rad2 = rad * rad;

	return dist2 <= rad2;
}

bool TestSegCircle ( Segment3d const & S, Circle const & C )
{
	return Distance2d::Distance2PointSeg( C.getCenter(), S ) <= C.getRadiusSquared();
}

// ----------------------------------------------------------------------

bool TestSegABox ( Segment3d const & S, AxialBox const & B )
{
	Vector const & P = S.getBegin();
	Vector const & V = S.getDelta();

	Range temp = Range::unit;

	Range timeX = Intersect1d::IntersectFloatRange( P.x, V.x, B.getRangeX() );

	temp = Intersect1d::IntersectRanges(temp,timeX);

	if(temp.isEmpty()) return false;

	Range timeZ = Intersect1d::IntersectFloatRange( P.z, V.z, B.getRangeZ() );

	temp = Intersect1d::IntersectRanges(temp,timeZ);

	return !temp.isEmpty();
}

// ----------------------------------------------------------------------

bool TestPointTri ( Vector const & P, Triangle3d const & T )
{
	Vector const & A = T.getCornerA();
	Vector const & B = T.getCornerB();
	Vector const & C = T.getCornerC();

	if(Containment2d::TestPointSeg(P,A,B) == CR_Outside) return false;
	if(Containment2d::TestPointSeg(P,B,C) == CR_Outside) return false;
	if(Containment2d::TestPointSeg(P,C,A) == CR_Outside) return false;

	return true;
}

// ----------------------------------------------------------------------

} // namespace Overlap2d
