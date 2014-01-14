// ======================================================================
//
// Containment3d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Containment3d.h"

#include "sharedCollision/Containment2d.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/Containment1d.h"

#include "sharedMath/Range.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Triangle3d.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/Circle.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/OrientedCylinder.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"
#include "sharedMath/OrientedBox.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Torus.h"
#include "sharedMath/MultiShape.h"
#include "sharedMath/Segment3d.h"

#include <vector>

namespace Containment3d
{

// ----------------------------------------------------------------------
// Point-vs-planar

ContainmentResult   TestPointPlane  ( Vector const & A,
                                      Plane3d const & P )
{
	Vector D = A - P.getPoint();

	float t = D.dot(P.getNormal());

	bool bInside = t < 0.0f;

	return bInside ? CR_Inside : CR_Outside;
}

// ----------

ContainmentResult   TestPointTri    ( Vector const & P, Triangle3d const & T )
{
	return TestPointPlane(P, T.getPlane() );
}

// ----------------------------------------------------------------------
// Planar-vs-planar

ContainmentResult   TestTriTri  ( Triangle3d const & A,
                                  Triangle3d const & B )
{
	ContainmentResult sideA = TestPointTri( A.getCornerA(), B );
	ContainmentResult sideB = TestPointTri( A.getCornerB(), B );
	ContainmentResult sideC = TestPointTri( A.getCornerC(), B );

	if((sideA != sideB) || (sideB != sideC)) 
	{
		return CR_Overlap;
	}

	return sideA;
}

// ----------------------------------------------------------------------
// Point-vs-volume

ContainmentResult	TestPointSphere	( Vector const & V,
									  Sphere const & S )
{
	real radiusSquared = S.getRadius() * S.getRadius();

	real distanceSquared = (V-S.getCenter()).magnitudeSquared();

	return Containment1d::TestFloatLess(distanceSquared,radiusSquared);
}

// ----------

ContainmentResult  TestPointCylinder    ( Vector const & V,
                                          Cylinder const & C )
{
	Vector delta = V - C.getBase();
	delta.y = 0;

	real dist = delta.magnitude();
	real radius = C.getRadius();

	ContainmentResult rTest = Containment1d::TestFloatLess(dist,radius);
	ContainmentResult vTest = Containment1d::TestFloatRange(V.y, C.getRangeY());

	// ----------

	return Containment::ComposeAxisTests(rTest,vTest);
}

// ----------

ContainmentResult	TestPointABox	( Vector const & V, AxialBox const & B )
{
	ContainmentResult testX = Containment1d::TestFloatRange( V.x, B.getRangeX() );
	ContainmentResult testY = Containment1d::TestFloatRange( V.y, B.getRangeY() );
	ContainmentResult testZ = Containment1d::TestFloatRange( V.z, B.getRangeZ() );

	return Containment::ComposeAxisTests( testX, testY, testZ );
}

// ----------

ContainmentResult   TestPointYBox  ( Vector const & V, YawedBox const & B )
{
	Vector localV = B.transformToLocal(V);
	
	return Test(localV,B.getLocalShape());
}

// ----------

ContainmentResult   TestPointOBox ( Vector const & V, OrientedBox const & B )
{
	Vector localV = B.transformToLocal(V);

	return Test(localV,B.getLocalShape());
}

// ----------
//@todo - Is there a version of this that doesn't need a square root?

ContainmentResult   TestPointTorus ( Vector const & V, Torus const & T )
{
	Vector L = T.transformToLocal(V);

	float r = sqrt( L.x * L.x + L.z * L.z ) - T.getMajorRadius();

	float d = (r * r) + (L.y * L.y);

	float m = T.getMinorRadius() * T.getMinorRadius();

	return Containment1d::TestFloatLess(d,m);
}

// ----------

ContainmentResult TestPointShape ( Vector const & A, MultiShape const & B )
{
	MultiShape::ShapeType typeB = B.getShapeType();

	// ----------

	if     (typeB == MultiShape::MST_Sphere)        { return Test( A, B.getSphere()   ); }
	else if(typeB == MultiShape::MST_Cylinder)      { return Test( A, B.getCylinder() ); }
	else if(typeB == MultiShape::MST_AxialBox)      { return Test( A, B.getAxialBox() ); }
	else if(typeB == MultiShape::MST_YawedBox)      { return Test( A, B.getYawedBox() ); }
	else if(typeB == MultiShape::MST_OrientedBox)   { return Test( A, B.getOrientedBox() ); }
	else                                            { return CR_Outside; }
}

// ----------------------------------------------------------------------
// Volume-vs-volume, spheres

ContainmentResult   TestSphereSphere    ( Sphere const & A,
                                          Sphere const & B )
{
	// ----------
	// do a quick outside test first

	Vector delta = B.getCenter() - A.getCenter();

	real dist2 = delta.magnitudeSquared();

	real sumRadius = A.getRadius() + B.getRadius();
	real sumRadius2 = sumRadius * sumRadius;

	if(dist2 > sumRadius2)
	{
		return CR_Outside;
	}
	else if(dist2 == sumRadius2) //lint !e777 // testing floats for equality
	{
		return CR_TouchingOutside;
	}
	else
	{
		// ----------
		// and if that fails do the full test

		real dist = sqrt(dist2);

		Range AD( dist - A.getRadius(), dist + A.getRadius() );
		Range BD( -B.getRadius(), B.getRadius() );

		return Containment1d::TestRangeRange( AD, BD );
	}
}

// ----------

ContainmentResult  TestSphereCylinder ( Sphere const & S,
									    Cylinder const & C )
{
	Vector delta = C.getBase() - S.getCenter();
	delta.y = 0;

	real dist = delta.magnitude();

	Range SD( dist - S.getRadius(), dist + S.getRadius() );

	Range CD( -C.getRadius(), C.getRadius() );

	ContainmentResult hTest = Containment1d::TestRangeRange( SD, CD );
	ContainmentResult hTest2 = Containment1d::TestFloatRange( dist, CD );

	ContainmentResult vTest = Containment1d::TestRangeRange( S.getRangeY(), C.getRangeY() );
	ContainmentResult cTest = Containment1d::TestFloatRange( S.getCenter().y, C.getRangeY() );

	// ----------

	if(hTest == CR_Outside)
	{
		// Sphere can't possibly touch the cylinder

		return CR_Outside;
	}
	else if((hTest == CR_TouchingOutside) || (hTest == CR_Boundary))
	{
		// Sphere is touching the outside of the cylinder's tube if its
		// center is inside the vertical range of the cylinder

		if((cTest == CR_Inside) || (cTest == CR_Boundary))
		{
			return CR_TouchingOutside;
		}
		else
		{
			return CR_Outside;
		}
	}
	else if((hTest == CR_Inside) || (hTest == CR_TouchingInside))
	{
		// Sphere is in the tube of the cylinder. It touches the cylinder
		// if its vertical range touches the vertical range of the cylinder

		return Containment::ComposeAxisTests(hTest,vTest);
	}
	else
	{
		// hTest == CR_Overlap

		if(vTest == CR_Outside)
		{
			return CR_Outside;
		}
		else if((vTest == CR_Inside) || (vTest == CR_TouchingInside))
		{
			return CR_Overlap;
		}
		else if (vTest == CR_Boundary)
		{
			// This really shouldn't be happening
			return CR_Boundary;
		}
		else if (vTest == CR_TouchingOutside)
		{
			if(hTest2 == CR_Inside)
			{
				// Sphere is touching a cap of the cylinder
				return CR_TouchingOutside;
			}
			else if(hTest2 == CR_Boundary)
			{
				// Sphere is touching the edge of the cap of the cylinder
				return CR_TouchingOutside;
			}
			else
			{
				// Sphere isn't touching the cap
				return CR_Outside;
			}
		}
		else
		{
			// vTest == CR_Overlap

			if((cTest == CR_Inside) || (cTest == CR_Boundary))
			{
				// The ranges overlap vertically and horizontally, and the center of
				// the sphere is inside the vertical range - the sphere overlaps the
				// cylinder

				return CR_Overlap;
			}
			else
			{
				// The sphere is inside both ranges, but its center is outside both
				// ranges. The sphere overlaps the cylinder if the closest point
				// on the cylinder is inside the sphere.

				Vector closestPoint = Distance3d::ClosestPointCylinder(S.getCenter(),C);

				ContainmentResult result = TestPointSphere(closestPoint,S);

				if(result == CR_Outside)
				{
					return CR_Outside;
				}
				else if(result == CR_Boundary)
				{
					return CR_TouchingOutside;
				}
				else
				{
					return CR_Overlap;
				}
			}
		}
	}
}

ContainmentResult TestSphereOCylinder   ( Sphere const & A, OrientedCylinder const & B )
{
	Vector localCenter = B.transformToLocal(A.getCenter());

	Sphere localSphere(localCenter,A.getRadius());

	return Test( localSphere, B.getLocalShape() );
}

//@todo - HACK - these need to be made correct

ContainmentResult TestSphereABox        ( Sphere const & S, AxialBox const & B )
{
	if(Test(S.getCenter(),B) == CR_Inside)
	{
		return CR_Overlap;
	}
	else
	{
		Vector closest = Distance3d::ClosestPointABox( S.getCenter(), B );

		if(TestPointSphere(closest,S) == CR_Inside)
		{
			return CR_Overlap;
		}
		else
		{
			return CR_Outside;
		}
	}
}

ContainmentResult TestSphereYBox        ( Sphere const & S, YawedBox const & B )
{
	if(Test(S.getCenter(),B) == CR_Inside)
	{
		return CR_Overlap;
	}
	else
	{
		Vector closest = Distance3d::ClosestPointYBox( S.getCenter(), B );

		if(TestPointSphere(closest,S) == CR_Inside)
		{
			return CR_Overlap;
		}
		else
		{
			return CR_Outside;
		}
	}
}

ContainmentResult TestSphereOBox        ( Sphere const & S, OrientedBox const & B )
{
	if(Test(S.getCenter(),B) == CR_Inside)
	{
		return CR_Overlap;
	}
	else
	{
		Vector closest = Distance3d::ClosestPointOBox( S.getCenter(), B );

		if(TestPointSphere(closest,S) == CR_Inside)
		{
			return CR_Overlap;
		}
		else
		{
			return CR_Outside;
		}
	}
}

// ----------------------------------------------------------------------
// Volume-vs-volume, cylinders

ContainmentResult  TestCylinderSphere   ( Cylinder const & A, Sphere const & B )
{
	return TestSphereCylinder(B,A);
}

ContainmentResult  TestCylinderCylinder ( Cylinder const & A,
                                          Cylinder const & B )
{
	Vector delta = B.getBase() - A.getBase();
	delta.y = 0;

	real dist = delta.magnitude();

	Range AD( dist - A.getRadius(), dist + A.getRadius() );
	Range BD( -B.getRadius(), B.getRadius() );

	ContainmentResult hTest = Containment1d::TestRangeRange( AD, BD );
	ContainmentResult vTest = Containment1d::TestRangeRange( A.getRangeY(), B.getRangeY() );

	// ----------

	return Containment::ComposeAxisTests(hTest,vTest);
}

// ----------

ContainmentResult     TestCylinderABox ( Cylinder const & C,
									 	 AxialBox const & B )
{
	ContainmentResult test2d = Containment2d::TestCircleABox(C.getBaseCircle(),B);

	ContainmentResult testY = Containment1d::TestRangeRange( C.getRangeY(), B.getRangeY() );

	return Containment::ComposeAxisTests( test2d, testY );
}

// ----------

ContainmentResult     TestCylinderYBox ( Cylinder const & C,
										 YawedBox const & B )
{
	ContainmentResult test2d = Containment2d::TestCircleYBox(C.getBaseCircle(),B);

	ContainmentResult testY = Containment1d::TestRangeRange( C.getRangeY(), B.getRangeY() );

	return Containment::ComposeAxisTests( test2d, testY );

}

// ----------------------------------------------------------------------

ContainmentResult     TestABoxABox       ( AxialBox const & A,
										 AxialBox const & B )
{
	ContainmentResult testX = Containment1d::TestRangeRange( A.getRangeX(), B.getRangeX() );
	ContainmentResult testY = Containment1d::TestRangeRange( A.getRangeY(), B.getRangeY() );
	ContainmentResult testZ = Containment1d::TestRangeRange( A.getRangeZ(), B.getRangeZ() );

	return Containment::ComposeAxisTests( testX, testY, testZ );
}

// ----------------------------------------------------------------------

ContainmentResult     TestMultiShapes   ( MultiShape const & A,
                                          MultiShape const & B )
{
	MultiShape::ShapeType typeA = A.getShapeType();
	MultiShape::ShapeType typeB = B.getShapeType();

	// ----------

	if(typeA == MultiShape::MST_Cylinder)
	{
		Cylinder cylA = A.getCylinder();

		if     (typeB == MultiShape::MST_Sphere)        { return Test( cylA, B.getSphere()   ); }
		else if(typeB == MultiShape::MST_Cylinder)      { return Test( cylA, B.getCylinder() ); }
		else if(typeB == MultiShape::MST_YawedBox)      { return Test( cylA, B.getYawedBox() ); }
		else if(typeB == MultiShape::MST_AxialBox)      { return Test( cylA, B.getAxialBox() ); }
		else                                            { return CR_Outside; }
	}
	else if(typeB == MultiShape::MST_Cylinder)
	{
		Cylinder cylB = B.getCylinder();

		if     (typeA == MultiShape::MST_Sphere)        { return Test( A.getSphere(),   cylB ); }
		else if(typeA == MultiShape::MST_AxialBox)      { return Test( A.getAxialBox(), cylB ); }
		else if(typeA == MultiShape::MST_YawedBox)      { return Test( A.getYawedBox(), cylB ); }
		else                                            { return CR_Outside; }
	}
	else if(typeA == MultiShape::MST_Sphere)
	{
		Sphere sphereA = A.getSphere();

		if     (typeB == MultiShape::MST_Sphere)			{ return Test( sphereA, B.getSphere()   ); }
		else if(typeB == MultiShape::MST_Cylinder)			{ return Test( sphereA, B.getCylinder() ); }
		else if(typeB == MultiShape::MST_OrientedCylinder)  { return Test( sphereA, B.getOrientedCylinder() ); }
		else if(typeB == MultiShape::MST_AxialBox)			{ return Test( sphereA, B.getAxialBox() ); }
		else if(typeB == MultiShape::MST_YawedBox)			{ return Test( sphereA, B.getYawedBox() ); }
		else if(typeB == MultiShape::MST_OrientedBox)		{ return Test( sphereA, B.getOrientedBox() ); }
		else												{ return CR_Outside; }
	}
	else if(typeB == MultiShape::MST_Sphere)
	{
		Sphere sphereB = B.getSphere();

		if     (typeB == MultiShape::MST_Sphere)			{ return Test( A.getSphere(), sphereB ); }
		else if(typeB == MultiShape::MST_Cylinder)			{ return Test( A.getCylinder(), sphereB ); }
		else if(typeB == MultiShape::MST_OrientedCylinder)	{ return Test( A.getOrientedCylinder(), sphereB ); }
		else if(typeB == MultiShape::MST_AxialBox)			{ return Test( A.getAxialBox(), sphereB ); }
		else if(typeB == MultiShape::MST_YawedBox)			{ return Test( A.getYawedBox(), sphereB ); }
		else if(typeB == MultiShape::MST_OrientedBox)		{ return Test( A.getOrientedBox(), sphereB ); }
		else												{ return CR_Outside; }
	}
	else
	{
		return CR_Outside;
	}
}








// ----------------------------------------------------------------------
// Point-vs-planar with epsilon

ContainmentResult     TestPointPlane   ( Vector const & A,
                                          Plane3d const & P,
                                          real epsilon )
{
	DEBUG_FATAL(!P.isNormalized(),("Containment3d::TestPointPlane (epsilon) - Got a non-normalized plane\n"));

	real dist = (A - P.getPoint()).dot(P.getNormal());

	if(WithinEpsilonInclusive(dist,0.0f,epsilon)) return CR_Boundary;

	return (dist < 0) ? CR_Inside : CR_Outside;
}

// ----------

ContainmentResult     TestPointTri  ( Vector const & P,
                                      Triangle3d const & T,
                                      real epsilon )
{
	return TestPointPlane( P, T.getPlane(), epsilon );
}

// ----------------------------------------------------------------------
// Planar-vs-planar with epsilon

ContainmentResult     TestTriPlane  ( Triangle3d const & T,
                                      Plane3d const & P,
                                      real epsilon )
{
	ContainmentResult testA = TestPointPlane( T.getCornerA(), P, epsilon );
	ContainmentResult testB = TestPointPlane( T.getCornerB(), P, epsilon );
	ContainmentResult testC = TestPointPlane( T.getCornerC(), P, epsilon );

	// If all corners are in the same region, the triangle is in
	// that region also.

	if((testA == testB) && (testB == testC))
	{
		return testA;
	}

	bool hasInside   = (testA == CR_Inside)   || (testB == CR_Inside)   || (testC == CR_Inside);
	bool hasOutside  = (testA == CR_Outside)  || (testB == CR_Outside)  || (testC == CR_Outside);
	bool hasBoundary = (testA == CR_Boundary) || (testB == CR_Boundary) || (testC == CR_Boundary);

	if(hasInside  && hasOutside)  return CR_Overlap;
	if(hasInside  && hasBoundary) return CR_TouchingInside;
	if(hasOutside && hasBoundary) return CR_TouchingOutside;

	DEBUG_FATAL(true,("Containment3d::TestTriPlane (epsilon) - Should never get here\n"));

	return CR_Overlap;
}

// ----------

ContainmentResult     TestTriTri    ( Triangle3d const & A,
                                      Triangle3d const & B,
                                      real epsilon )
{
	return TestTriPlane( A, B.getPlane(), epsilon );
}











// ======================================================================
// Enclosure functions

AxialBox EncloseABox ( VectorVector const & points )
{
	AxialBox temp;

	for(uint i = 0; i < points.size(); i++)
	{
		temp.add( points[i] );
	}

	return temp;
}

AxialBox EncloseABox ( Sphere const & sphere )
{
	float r = sphere.getRadius();
	Vector c = sphere.getCenter();

	Vector d(r,r,r);

	return AxialBox( c + d, c - d );
}

AxialBox EncloseABox ( Segment3d const & S )
{
	return AxialBox( S.getBegin(), S.getEnd() );
}

AxialBox EncloseABox ( Cylinder const & C )
{
	Vector center = C.getCenter();

	Vector extent( C.getExtentX(), C.getExtentY(), C.getExtentZ() );

	return AxialBox(center-extent,center+extent);
}

AxialBox EncloseABox ( AxialBox const & A, AxialBox const & B )
{
	AxialBox temp = A;

	temp.add(B);
	
	return temp;
}

// ----------------------------------------------------------------------

Sphere EncloseSphere ( Sphere const & shape )
{
	return shape;
}

Sphere EncloseSphere ( Cylinder const & shape )
{
	float x = shape.getExtentX();
	float y = shape.getExtentY();

    float radius = sqrt( x*x + y*y );

    return Sphere( shape.getCenter(), radius );
}

Sphere EncloseSphere ( OrientedCylinder const & shape )
{
	float x = shape.getExtentX();
	float y = shape.getExtentY();

	float radius = sqrt( x*x + y*y );

	return Sphere( shape.getCenter(), radius );
}

Sphere EncloseSphere ( AxialBox const & shape )
{
	float x = shape.getExtentX();
	float y = shape.getExtentY();
	float z = shape.getExtentZ();

	float radius = sqrt( x*x + y*y + z*z );

	return Sphere( shape.getCenter(), radius );
}

Sphere EncloseSphere ( YawedBox const & shape )
{
	float x = shape.getExtentX();
	float y = shape.getExtentY();
	float z = shape.getExtentZ();

	float radius = sqrt( x*x + y*y + z*z );

	return Sphere( shape.getCenter(), radius );
}

Sphere EncloseSphere ( OrientedBox const & shape )
{
	float x = shape.getExtentX();
	float y = shape.getExtentY();
	float z = shape.getExtentZ();

	float radius = sqrt( x*x + y*y + z*z );

	return Sphere( shape.getCenter(), radius );
}

// ----------------------------------------------------------------------

Sphere EncloseSphere ( Vector const & A, Vector const & B )
{
	Vector center = (B+A) / 2.0f;

	real radius = (B-A).magnitude() / 2.0f;

	return Sphere(center,radius);
}

// ----------

Sphere EncloseSphere ( Sphere const & S, Vector const & V )
{
	if( Containment::isContainment(TestPointSphere(V,S)) ) return S;

	Vector normal = V - S.getCenter();
	IGNORE_RETURN( normal.normalize() );

	Vector pointS = S.getCenter() - normal * S.getRadius();

	Vector center = Vector::midpoint(V,pointS);

	real radius = (V - pointS).magnitude() / 2.0f;

	return Sphere(center,radius);
}

// ----------

Sphere EncloseSphere ( Sphere const & A, Sphere const & B )
{
	if( Containment::isContainment(TestSphereSphere(A,B)) ) return B;
	if( Containment::isContainment(TestSphereSphere(B,A)) ) return A;

	// ----------
	
	Vector normal = B.getCenter() - A.getCenter();
	
	IGNORE_RETURN( normal.normalize() );

	Vector pointA = A.getCenter() - normal * A.getRadius();
	Vector pointB = B.getCenter() + normal * B.getRadius();

	Vector center = Vector::midpoint(pointA,pointB);

	real radius = (pointB - pointA).magnitude() / 2.0f;

	return Sphere(center,radius);
}

// ----------

Sphere EncloseSphere ( VectorVector const & points )
{
	if(points.size() == 0)
	{
		return Sphere::zero;
	}
	else if(points.size() == 1)
	{
		return Sphere( points[0], 0.0f );
	}

	// ----------

	Sphere sphere = EncloseSphere(points[0],points[1]);

	for(uint i = 2; i < points.size(); i++)
	{
		sphere = EncloseSphere(sphere,points[i]);
	}

	return sphere;
}

// ----------------------------------------------------------------------

bool ClipSphere ( Sphere const & A, Sphere const & B, Sphere & out )
{
	float distance = A.getCenter().magnitudeBetween(B.getCenter());

	float radius = B.getRadius() - distance;

	if(radius < 0.0) 
	{
		return false;
	}
	else
	{
		out = Sphere(A.getCenter(),radius);

		return true;
	}
}

// ----------------------------------------------------------------------

}	// namespace Containment3d

