// ======================================================================
//
// Contact3d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Contact3d.h"

#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/Intersect2d.h"
#include "sharedCollision/Distance2d.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Overlap3d.h"

#include "sharedMath/Circle.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"
#include "sharedMath/OrientedBox.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Torus.h"
#include "sharedMath/ShapeUtils.h"
#include "sharedMath/OrientedCylinder.h"
#include "sharedMath/MultiShape.h"

Contact Contact::noContact   ( false, true,  REAL_MAX, Vector::zero, Vector::zero );
Contact Contact::cantResolve ( false, false, REAL_MAX, Vector::zero, Vector::zero );

template<class ShapeA, class ShapeB>
void validateIntersection ( ShapeA const & A, Vector const & V, ShapeB const & B, Range hitTime )
{
	ShapeA a = A;
	ShapeA b = ShapeUtils::translate(A,V);
	ShapeA c = ShapeUtils::translate(A,V * hitTime.getMin());
	ShapeA d = ShapeUtils::translate(A,V * hitTime.getMax());

	ContainmentResult hitA = Containment3d::Test(a,B);
	ContainmentResult hitB = Containment3d::Test(b,B);
	ContainmentResult hitC = Containment3d::Test(c,B);
	ContainmentResult hitD = Containment3d::Test(d,B);

	float distA = Distance3d::DistanceBetween(B,a);
	float distB = Distance3d::DistanceBetween(B,b);
	float distC = Distance3d::DistanceBetween(B,c);
	float distD = Distance3d::DistanceBetween(B,d);

	// If the intersection test worked and the hit times are correct, distA should be positive or small,
	// distB should be negative, and distC & distD should be small.

	UNREF(hitA); UNREF(hitB); UNREF(hitC); UNREF(hitD);
	UNREF(distA); UNREF(distB); UNREF(distC); UNREF(distD);
}

// ----------------------------------------------------------------------

namespace Contact3d
{

// ----------------------------------------------------------------------

Contact ContactCirclePoint ( Circle const & A, Vector const & V, Vector const & B )
{
	Range hitTime = Intersect2d::IntersectPointCircle(B,-V,A);

	if((hitTime.getMin() < 1.0f) && (hitTime.getMax() > 0.0f))
	{
		Vector N = (A.getCenter() + V * hitTime.getMin()) - B;

		N.y = 0.0f;

		IGNORE_RETURN( N.normalize() );
		
		return Contact( true, true, hitTime.getMin(), B, N );
	}
	else
	{
		return Contact::noContact;
	}
}

Contact ContactCircleCircle ( Circle const & A, Vector const & V, Circle const & B )
{
	Range hitTime = Intersect2d::Intersect(A,V,B);

	if((hitTime.getMin() < 1.0f) && (hitTime.getMax() > 0.0f))
	{
		Vector newCenter = A.getCenter() + V * hitTime.getMin();

		Vector closestPoint = Distance2d::ClosestPointCircle(newCenter,B);

		Vector N = A.getCenter() - closestPoint;

		N.y = 0.0f;

		IGNORE_RETURN( N.normalize() );
	
		return Contact( true, true, hitTime.getMin(), closestPoint, N );
	}
	else
	{
		return Contact::noContact;
	}
}

Contact ContactCircleABox ( Circle const & A, Vector const & V, AxialBox const & B )
{
	Range hitTime = Intersect2d::Intersect(A,V,B);

	if((hitTime.getMin() < 1.0f) && (hitTime.getMax() > 0.0f))
	{
		Vector newCenter = A.getCenter() + V * hitTime.getMin();

		Vector closestPoint = Distance2d::ClosestPointABox(newCenter,B);

		Vector N = A.getCenter() - closestPoint;

		N.y = 0.0f;

		IGNORE_RETURN( N.normalize() );
	
		return Contact( true, true, hitTime.getMin(), closestPoint, N );
	}
	else
	{
		return Contact::noContact;
	}
}

Contact ContactCircleYBox ( Circle const & A, Vector const & V, YawedBox const & B )
{
	Range hitTime = Intersect2d::Intersect(A,V,B);

	if((hitTime.getMin() < 1.0f) && (hitTime.getMax() > 0.0f))
	{
		Vector newCenter = A.getCenter() + V * hitTime.getMin();

		Vector closestPoint = Distance2d::ClosestPointYBox(newCenter,B);

		Vector N = A.getCenter() - closestPoint;

		N.y = 0.0f;

		IGNORE_RETURN( N.normalize() );
	
		return Contact( true, true, hitTime.getMin(), closestPoint, N );
	}
	else
	{
		return Contact::noContact;
	}
}

// ----------------------------------------------------------------------

Contact ContactSphereSphere ( Sphere const & A, Vector const & V, Sphere const & B )
{
	Range hitTime = Intersect3d::Intersect(A,V,B);

	if((hitTime.getMin() < 1.0f) && (hitTime.getMax() > 0.0f))
	{
		Sphere newSphere( A.getCenter() + V * hitTime.getMin(), A.getRadius() );

		Vector closestPoint = Distance3d::ClosestPointSphere(B.getCenter(),newSphere);

		Vector N = closestPoint - B.getCenter();

		IGNORE_RETURN( N.normalize() );
	
		return Contact( true, true, hitTime.getMin(), closestPoint, N );
	}
	else
	{
		return Contact::noContact;
	}
}

// ----------

Contact ContactSphereCylinder ( Sphere const & A, Vector const & V, Cylinder const & B )
{
	Range hitTime = Intersect3d::Intersect(A,V,B);

	if((hitTime.getMin() < 1.0f) && (hitTime.getMax() > 0.0f))
	{
		Sphere newSphere = ShapeUtils::translate(A,V*hitTime.getMin());

		Vector closestPoint = Distance3d::ClosestPointCylinder(newSphere.getCenter(),B);

		Vector N = newSphere.getCenter() - closestPoint;

		IGNORE_RETURN( N.normalize() );
	
		return Contact( true, true, hitTime.getMin(), closestPoint, N );
	}
	else
	{
		return Contact::noContact;
	}
}

// ----------

Contact ContactSphereOCylinder ( Sphere const & A, Vector const & V, OrientedCylinder const & B )
{
	Vector localCenter = B.transformToLocal(A.getCenter());
	Vector localVelocity = B.rotateToLocal(V);

	Contact temp = ContactSphereCylinder( Sphere(localCenter,A.getRadius()), localVelocity, B.getLocalShape() );

	temp.m_point = B.transformToWorld(temp.m_point);
	temp.m_normal = B.rotateToWorld(temp.m_normal);

	return temp;
}

// ----------

Contact ContactSphereABox ( Sphere const & A, Vector const & V, AxialBox const & B )
{
	Range hitTime = Intersect3d::Intersect(A,V,B);

	if((hitTime.getMin() < 1.0f) && (hitTime.getMax() > 0.0f))
	{
		Sphere newSphere = ShapeUtils::translate(A,V*hitTime.getMin());

		Vector closestPoint = Distance3d::ClosestPointABox(newSphere.getCenter(),B);

		Vector N = newSphere.getCenter() - closestPoint;

		IGNORE_RETURN( N.normalize() );
	
		return Contact( true, true, hitTime.getMin(), closestPoint, N );
	}
	else
	{
		return Contact::noContact;
	}
}

Contact ContactSphereYBox ( Sphere const & A, Vector const & V, YawedBox const & B )
{
	Range hitTime = Intersect3d::Intersect(A,V,B);

	if((hitTime.getMin() < 1.0f) && (hitTime.getMax() > 0.0f))
	{
		Sphere newSphere = ShapeUtils::translate(A,V*hitTime.getMin());

		Vector closestPoint = Distance3d::ClosestPointYBox(newSphere.getCenter(),B);

		Vector N = newSphere.getCenter() - closestPoint;

		IGNORE_RETURN( N.normalize() );
	
		return Contact( true, true, hitTime.getMin(), closestPoint, N );
	}
	else
	{
		return Contact::noContact;
	}
}


Contact ContactSphereOBox ( Sphere const & A, Vector const & V, OrientedBox const & B )
{
	Range hitTime = Intersect3d::Intersect(A,V,B);

	if((hitTime.getMin() < 1.0f) && (hitTime.getMax() > 0.0f))
	{
		Sphere newSphere = ShapeUtils::translate(A,V*hitTime.getMin());

		Vector closestPoint = Distance3d::ClosestPointOBox(newSphere.getCenter(),B);

		Vector N = newSphere.getCenter() - closestPoint;

		IGNORE_RETURN( N.normalize() );
	
		return Contact( true, true, hitTime.getMin(), closestPoint, N );
	}
	else
	{
		return Contact::noContact;
	}
}

// ----------------------------------------------------------------------

Contact ContactCylinderSphere ( Cylinder const & A, Vector const & V, Sphere const & B )
{
	Range hitTime = Intersect3d::IntersectSphereCylinder(B,-V,A);

#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG
	validateIntersection(A,V,B,hitTime);
#endif

	// ----------

	if((hitTime.getMin() < 1.0f) && (hitTime.getMax() > 0.0f))
	{
		Cylinder newCylinder( A.getBase() + V * hitTime.getMin(), A.getRadius(), A.getHeight() );

		Vector closestPoint = Distance3d::ClosestPointCylinder(B.getCenter(),newCylinder);

		Vector N = closestPoint - B.getCenter();

		IGNORE_RETURN( N.normalize() );
	
		return Contact( true, true, hitTime.getMin(), closestPoint, N );
	}
	else
	{
		return Contact::noContact;
	}
}

// ----------------------------------------------------------------------

Contact ContactMultiShapes ( MultiShape const & A, Vector const & V, MultiShape const & B )
{
	MultiShape::ShapeType typeA = A.getShapeType();
	MultiShape::ShapeType typeB = B.getShapeType();

	// ----------

	if(typeA == MultiShape::MST_Sphere)
	{
		Sphere sphereA = A.getSphere();

		if     (typeB == MultiShape::MST_Sphere)           { return FindContact( sphereA, V, B.getSphere() ); }
		else if(typeB == MultiShape::MST_Cylinder)         { return FindContact( sphereA, V, B.getCylinder() ); }
		else if(typeB == MultiShape::MST_OrientedCylinder) { return FindContact( sphereA, V, B.getOrientedCylinder() ); }
		else if(typeB == MultiShape::MST_AxialBox)         { return FindContact( sphereA, V, B.getAxialBox() ); }
		else if(typeB == MultiShape::MST_YawedBox)         { return FindContact( sphereA, V, B.getYawedBox() ); }
		else if(typeB == MultiShape::MST_OrientedBox)      { return FindContact( sphereA, V, B.getOrientedBox() ); }
		else                                               { return Contact::cantResolve; }
	}
	else if(typeA == MultiShape::MST_Cylinder)
	{
		Cylinder cylA = A.getCylinder();

		if     (typeB == MultiShape::MST_Sphere)           { return FindContact( cylA, V, B.getSphere() ); }
		else                                               { return Contact::cantResolve; }
	}
	else
	{
		return Contact::cantResolve;
	}
}

// ----------------------------------------------------------------------

} // namespace Contact3d
