// ======================================================================
//
// Containment2d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Containment2d.h"

#include "sharedCollision/Collision2d.h"
#include "sharedCollision/Containment1d.h"
#include "sharedCollision/Distance2d.h"
#include "sharedCollision/Distance3d.h"

#include "sharedMath/Range.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"
#include "sharedMath/Circle.h"
#include "sharedMath/Vector2d.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Segment3d.h"

#include "sharedMath/Transform.h"

using namespace Collision2d;

namespace Containment2d
{

// ----------------------------------------------------------------------
// 2d test - returns true if the test point is on the right side of the
// line

ContainmentResult TestPointLine ( Vector2d const & P, Vector2d const & B, Vector2d const & D )
{
	float d = ((P.x - B.x) * D.y) + ((P.y - B.y) * -D.x);

	return Containment1d::TestFloatGreater(d,0.0f);
}

ContainmentResult TestPointLine ( Vector const & P, Line3d const & L )
{
	Vector const & B = L.getPoint();
	Vector const & N = L.getNormal();

	float d = ((P.x - B.x) * N.z) + ((P.z - B.z) * -N.x);

	return Containment1d::TestFloatGreater(d,0.0f);
}

ContainmentResult TestPointSeg ( Vector const & P, Segment3d const & S )
{
	Vector const & B = S.getBegin();
	Vector const & E = S.getEnd();

	float d = ((P.x - B.x) * (E.z - B.z)) + ((P.z - B.z) * -(E.x - B.x));

	return Containment1d::TestFloatGreater(d,0.0f);
}

ContainmentResult TestPointSeg ( Vector const & P, Vector const & B, Vector const & E )
{
	float d = ((P.x - B.x) * (E.z - B.z)) + ((P.z - B.z) * -(E.x - B.x));

	return Containment1d::TestFloatGreater(d,0.0f);
}

// ----------------------------------------------------------------------

ContainmentResult TestPointABox ( Vector const & V, AxialBox const & B )
{
	ContainmentResult testX = Containment1d::TestFloatRange( V.x, B.getRangeX() );
	ContainmentResult testZ = Containment1d::TestFloatRange( V.z, B.getRangeZ() );

	return Containment::ComposeAxisTests(testX,testZ);
}

ContainmentResult TestPointCircle ( Vector const & V, Circle const & C )
{
	real radiusSquared = C.getRadius() * C.getRadius();

	real distanceSquared = Distance2d::Distance2PointPoint(V,C.getCenter());

	return Containment1d::TestFloatLess(distanceSquared,radiusSquared);
}

// ----------------------------------------------------------------------

ContainmentResult TestCircleABox ( Circle const & C, AxialBox const & B )
{
	ContainmentResult test = TestPointABox( C.getCenter(), B );

	if(test != CR_Outside) 
	{
		// Center point is inside the box 

		ContainmentResult testX = Containment1d::TestRangeRange( C.getRangeX(), B.getRangeX() );
		ContainmentResult testZ = Containment1d::TestRangeRange( C.getRangeZ(), B.getRangeZ() );

		return Containment::ComposeAxisTests(testX,testZ);
	}
	else
	{
		// Center point is outside the box

		Vector V = Distance3d::ClosestPointABox(C.getCenter(),B);

		float dist = Distance2d::DistancePointPoint(V,C.getCenter());

		ContainmentResult result = Containment1d::TestFloatLess( dist, C.getRadius() );

		if(result == CR_Outside)          return CR_Outside;
		else if(result == CR_Boundary)    return CR_TouchingOutside;
		else                            return CR_Overlap;
	}
}

// ----------------------------------------------------------------------
// To test a circle against a yawed box, transform the circle to box-space
// and test against the axial box

ContainmentResult TestCircleYBox ( Circle const & C, YawedBox const & B )
{
	Transform T = B.getTransform_l2p();

	Vector localCenter = T.rotateTranslate_p2l(C.getCenter());

	AxialBox localBox = B.getLocalShape();

	Circle localCircle(localCenter,C.getRadius());

	return TestCircleABox( localCircle, localBox );
};

// ----------------------------------------------------------------------

ContainmentResult TestCircleCircle ( Circle const & A, Circle const & B )
{
	real dist = Distance2d::DistancePointPoint(A.getCenter(),B.getCenter());

	return Containment1d::TestRangeRange( A.getLocalRangeX() + dist, B.getLocalRangeX() );
}

// ----------------------------------------------------------------------

}	// namespace Containment2d

