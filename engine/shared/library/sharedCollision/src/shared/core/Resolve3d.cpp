// ======================================================================
//
// Resolve3d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Resolve3d.h"

#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/Contact3d.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Distance3d.h"

#include "sharedMath/Vector.h"
#include "sharedMath/MultiShape.h"
#include "sharedMath/ShapeUtils.h"
#include "sharedMath/Circle.h"

#include "sharedMath/Sphere.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/OrientedCylinder.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"
#include "sharedMath/OrientedBox.h"

float maxContactDist = -REAL_MAX;

namespace Resolve3d
{

// ----------------------------------------------------------------------

ResolutionResult ResolveContact_Slide3 ( Contact const & contact, Vector const & V, Vector & outV )
{
	float stepDistance = V.magnitude();

	if(contact.m_resolvable)
	{
		if(contact.m_exists)
		{
			if(contact.m_time > 0.0f)
			{
				if(contact.m_time > 1.0f)
				{
					// Contact occurs too far in the future - nothing to do
					
					outV = V;
					return RR_Future;
				}
				else
				{
					outV = Collision3d::SlideAlong( V, contact.m_normal, contact.m_time );
					return RR_Resolved;
				}
			}
			else
			{
				// Contact occurs in the past

				float contactDistance = stepDistance * -contact.m_time;

				if(contactDistance > maxContactDist) maxContactDist = contactDistance;

				if(contactDistance < 0.05f)
				{
					// but not too far in the past - resolve it
					outV = Collision3d::SlideAlong( V, contact.m_normal, contact.m_time );

					return RR_Resolved;
				}
				else
				{
					return RR_Past;
				}
			}
		}
		else
		{
			// No contact
			return RR_NoCollision;
		}
	}
	else
	{
		return RR_DontKnowHow;
	}
}

// ----------------------------------------------------------------------

template< typename TypeA, typename TypeB >
ResolutionResult GenericSlideResolve ( TypeA const & A, Vector const & V, TypeB const & B, Vector & outV )
{
	Contact contact = Contact3d::FindContact(A,V,B);

	return ResolveContact_Slide3(contact,V,outV);
}

// ----------------------------------------------------------------------

ResolutionResult ResolveCirclePoint ( Circle const & A, Vector const & V, Vector const & B, Vector & outV )
{
	return GenericSlideResolve(A,V,B,outV);
}

ResolutionResult ResolveCircleCircle ( Circle const & A, Vector const & V, Circle const & B, Vector & outV )
{
	return GenericSlideResolve(A,V,B,outV);
}

ResolutionResult ResolveCircleABox ( Circle const & A, Vector const & V, AxialBox const & B, Vector & outV )
{
	return GenericSlideResolve(A,V,B,outV);
}

ResolutionResult ResolveCircleYBox ( Circle const & A, Vector const & V, YawedBox const & B, Vector & outV )
{
	return GenericSlideResolve(A,V,B,outV);
}

// ----------------------------------------------------------------------

ResolutionResult ResolveSphereSphere ( Sphere const & A, Vector const & V, Sphere const& B, Vector & outV )
{
	return GenericSlideResolve(A,V,B,outV);
}

ResolutionResult ResolveSphereCylinder ( Sphere const & A, Vector const & V, Cylinder const & B, Vector & outV )
{
	return GenericSlideResolve(A,V,B,outV);
}

ResolutionResult ResolveSphereOCylinder ( Sphere const & A, Vector const & V, OrientedCylinder const & B, Vector & outV )
{
	return GenericSlideResolve(A,V,B,outV);
}

ResolutionResult ResolveSphereABox ( Sphere const & A, Vector const & V, AxialBox const & B, Vector & outV )
{
	return GenericSlideResolve(A,V,B,outV);
}

ResolutionResult ResolveSphereYBox ( Sphere const & A, Vector const & V, YawedBox const & B, Vector & outV )
{
	return GenericSlideResolve(A,V,B,outV);
}

ResolutionResult ResolveSphereOBox ( Sphere const & A, Vector const & V, OrientedBox const & B, Vector & outV )
{
	return GenericSlideResolve(A,V,B,outV);
}

// ----------------------------------------------------------------------
// Cylinder-vs-abox/ybox collisions are resolved in 2d

ResolutionResult ResolveCylinderCylinder ( Cylinder const & A, Vector const & V, Cylinder const & B, Vector & outV )
{
	return Resolve(A.getBaseCircle(),V,B.getBaseCircle(),outV);
}

ResolutionResult ResolveCylinderABox ( Cylinder const & A, Vector const & V, AxialBox const & B, Vector & outV )
{
	return Resolve(A.getBaseCircle(),V,B,outV);
}

ResolutionResult ResolveCylinderYBox ( Cylinder const & A, Vector const & V, YawedBox const & B, Vector & outV )
{
	return Resolve(A.getBaseCircle(),V,B,outV);
}

ResolutionResult ResolveCylinderSphere ( Cylinder const & A, Vector const & V, Sphere const & B, Vector & outV )
{
	return GenericSlideResolve(A,V,B,outV);
}

// ----------------------------------------------------------------------

ResolutionResult ResolveMultiShapes ( MultiShape const & A, Vector const & V, MultiShape const & B, Vector & outV )
{
	MultiShape::ShapeType typeA = A.getShapeType();
	MultiShape::ShapeType typeB = B.getShapeType();

	// ----------

	if(typeA == MultiShape::MST_Cylinder)
	{
		Cylinder cylA = A.getCylinder();

		if     (typeB == MultiShape::MST_Sphere)           { return Resolve3d::Resolve( cylA, V, B.getSphere(),   outV ); }
		else if(typeB == MultiShape::MST_Cylinder)         { return Resolve3d::Resolve( cylA, V, B.getCylinder(), outV ); }
		else if(typeB == MultiShape::MST_AxialBox)         { return Resolve3d::Resolve( cylA, V, B.getAxialBox(), outV ); }
		else if(typeB == MultiShape::MST_YawedBox)         { return Resolve3d::Resolve( cylA, V, B.getYawedBox(), outV ); }
		else                                               { return RR_DontKnowHow; }
	}
	else if(typeA == MultiShape::MST_Sphere)
	{
		Sphere sphereA = A.getSphere();

		if     (typeB == MultiShape::MST_Sphere)           { return Resolve3d::Resolve( sphereA, V, B.getSphere(),   outV ); }
		else if(typeB == MultiShape::MST_Cylinder)         { return Resolve3d::Resolve( sphereA, V, B.getCylinder(), outV ); }
		else if(typeB == MultiShape::MST_OrientedCylinder) { return Resolve3d::Resolve( sphereA, V, B.getOrientedCylinder(), outV ); }
		else if(typeB == MultiShape::MST_AxialBox)         { return Resolve3d::Resolve( sphereA, V, B.getAxialBox(), outV ); }
		else if(typeB == MultiShape::MST_YawedBox)         { return Resolve3d::Resolve( sphereA, V, B.getYawedBox(), outV ); }
		else if(typeB == MultiShape::MST_OrientedBox)      { return Resolve3d::Resolve( sphereA, V, B.getOrientedBox(), outV ); }
		else                                               { return RR_DontKnowHow; }
	}
	else
	{
		return RR_DontKnowHow;
	}
}

// ----------------------------------------------------------------------

} // namespace Resolve3d
