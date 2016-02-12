// ======================================================================
//
// CollisionDetect.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CollisionDetect.h"

#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/ComponentExtent.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/DetailExtent.h"
#include "sharedCollision/SimpleExtent.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/Intersect1d.h"
#include "sharedCollision/Intersect3d.h"

#include "sharedObject/Object.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"

#include "sharedMath/Range.h"
#include "sharedMath/ShapeUtils.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/MultiShape.h"

//#include <typeinfo.h>
#include <algorithm>

static int gs_testCounter;

// ----------------------------------------------------------------------

void CollisionDetect::resetTestCounter ( void )
{
	gs_testCounter = 0;
}

int CollisionDetect::getTestCounter ( void )
{
	return gs_testCounter;
}

// ----------------------------------------------------------------------

BaseExtent const * getCollisionExtent_p ( Object const * obj )
{
	if(obj == nullptr) return nullptr;

	CollisionProperty const * collision = obj->getCollisionProperty();

	if(!collision) return nullptr;

	return collision->getExtent_p();
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testObjects ( Object const * objA, Object const * objB )
{
	if(objA == nullptr) return false;
	if(objB == nullptr) return false;

	// ----------

	CollisionProperty const * collA = objA->getCollisionProperty();
	CollisionProperty const * collB = objB->getCollisionProperty();

	if(!collA->isCollidable()) return false;
	if(!collB->isCollidable()) return false;

	if(!collA->canCollideWith(collB)) return false;

	// ----------

	BaseExtent const * extentA = getCollisionExtent_p(objA);
	BaseExtent const * extentB = getCollisionExtent_p(objB);

	return testExtents(extentA,extentB);
}

DetectResult CollisionDetect::testObjects ( Object const * objA, Vector const & velocity, Object const * objB )
{
	if(objA == nullptr) return false;
	if(objB == nullptr) return false;

	// ----------

	CollisionProperty const * collA = objA->getCollisionProperty();
	CollisionProperty const * collB = objB->getCollisionProperty();

	if(!collA->isCollidable()) return false;
	if(!collB->isCollidable()) return false;

	if(!collA->canCollideWith(collB)) return false;

	// ----------

	BaseExtent const * extentA = getCollisionExtent_p(objA);
	BaseExtent const * extentB = getCollisionExtent_p(objB);

	return testExtents(extentA,velocity,extentB);
}

DetectResult CollisionDetect::testObjectsAsCylinders ( Object const * objA, Object const * objB )
{
	if(objA == nullptr) return false;
	if(objB == nullptr) return false;

	// ----------

	CollisionProperty const * collA = objA->getCollisionProperty();
	CollisionProperty const * collB = objB->getCollisionProperty();

	if(!collA->isCollidable()) return false;
	if(!collB->isCollidable()) return false;

	if(!collA->canCollideWith(collB)) return false;

	// ----------

	BaseExtent const * extentA = getCollisionExtent_p(objA);
	BaseExtent const * extentB = getCollisionExtent_p(objB);

	return testExtentsAsCylinders(extentA,extentB);
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testObjectExtent ( Object const * objA, BaseExtent const * extentB )
{
	if(objA == nullptr) return false;

	CollisionProperty const * collA = objA->getCollisionProperty();

	if(!collA || !collA->isCollidable()) return false;

	// ----------

	BaseExtent const * extentA = getCollisionExtent_p(objA);

	return testExtents(extentA,extentB);
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testSphereExtent ( Sphere const & sphereA, BaseExtent const * extentB )
{
	MultiShape shapeA(sphereA);

	SimpleExtent extentA(shapeA);

	DetectResult result = testExtents(&extentA,extentB);

	// don't return a DetectResult that contains a pointer to a temporary
	
	result.extentA = nullptr;

	return result;
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testSphereExtent ( Sphere const & sphereA, Vector const & velocity, BaseExtent const * extentB )
{
	MultiShape shapeA(sphereA);

	SimpleExtent extentA(shapeA);

	DetectResult result = testExtents(&extentA,velocity,extentB);

	// don't return a DetectResult that contains a pointer to a temporary
	
	result.extentA = nullptr;

	return result;
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testSphereObject ( Sphere const & sphereA, Object const * objB )
{
	if(objB == nullptr) return false;

	CollisionProperty const * collB = objB->getCollisionProperty();

	if(!collB || !collB->isCollidable()) return false;

	// ----------

	BaseExtent const * extentB = getCollisionExtent_p(objB);

	return testSphereExtent(sphereA,extentB);
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testSphereObject ( Sphere const & sphereA, Vector const & velocity, Object const * objB )
{
	if(objB == nullptr) return false;

	CollisionProperty const * collB = objB->getCollisionProperty();

	if(!collB || !collB->isCollidable()) return false;

	// ----------

	BaseExtent const * extentB = getCollisionExtent_p(objB);

	return testSphereExtent(sphereA,velocity,extentB);
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testCapsuleExtent ( Capsule const & capsuleA, BaseExtent const * extentB )
{
	DetectResult result = testSphereExtent(capsuleA.getSphereA(),capsuleA.getDelta(),extentB);

	if(result.collided)
	{
		result.collisionTime = Range::inf;
	}

	return result;
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testCapsuleObject ( Capsule const & capsuleA, Object const * objB )
{
	DetectResult result = testSphereObject(capsuleA.getSphereA(),capsuleA.getDelta(),objB);

	if(result.collided)
	{
		result.collisionTime = Range::inf;
	}

	return result;
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testCapsuleObjectAgainstAllTypes(Capsule const & capsule_w, Object const * object)
{
	DetectResult detectResult(false);

	if (object == 0)
		return detectResult;

	BaseExtent const * extentToTest_l = 0;

	{
		// testSphereObject method expanded out
		CollisionProperty const * collisionProperty = object->getCollisionProperty();

		if ((collisionProperty == 0) || (!collisionProperty->isCollidable()))
			return detectResult;

		extentToTest_l = collisionProperty->getExtent_l();
	}
	
	if (extentToTest_l != 0)
	{
		// in the original call to testSphereObject we were getting the world extent
		// instead I'm transforming the capsule into the objects local space which
		// is less work on large extents

		Vector const capsuleBegin_l(object->rotateTranslate_w2o(capsule_w.getPointA()));
		Vector const capsuleEnd_l(object->rotateTranslate_w2o(capsule_w.getPointB()));
		Vector const capsuleVelocity_l(capsuleEnd_l - capsuleBegin_l);
		Sphere const sphere_l(capsuleBegin_l, capsule_w.getRadius());

		// unrolling the original call to CollisionDetect::testSphereExtent
		// return testSphereExtent(sphere_l, capsuleVelocity_l, extentToTest_l);

		MultiShape const sphereMultiShape(sphere_l);
		SimpleExtent const sphereExtent_l(sphereMultiShape);

		// unrolling the original call to CollisionDetect::testExtents below
		// DetectResult result = testExtents(&extentA,velocity,extentB);

		{
			// the extentA and extentB are used here so that we can more
			// closely mirror the body of CollisionDetect::testExtents
			BaseExtent const * const extentA = &sphereExtent_l;
			BaseExtent const * const extentB = extentToTest_l;
			Vector const & velocity = capsuleVelocity_l;

			// typeA is always guaranteed to be ET_Simple;
			ExtentType const typeB = extentB->getType();

			if(typeB == ET_Simple)
			{
				detectResult = testSimpleSimple(extentA, velocity, extentB);
			}
			else if(typeB == ET_Component)
			{
				detectResult = testAnyComponent(extentA, velocity, extentB);
			}
			else if(typeB == ET_Detail)
			{
				detectResult = testAnyDetail(extentA, velocity, extentB);
			}
			else if(typeB == ET_Mesh)
			{
				// TODO:  this actually needs to test against the buckets and triangles
				// of the CollisionMesh.  Until then, AI won't be able to get in close
				// to the larger objects.  This won't allow docking, etc on these objects
				SimpleExtent meshBoundingBoxExtent_l(MultiShape(extentB->getBoundingBox()));
				detectResult = testSimpleSimple(extentA, velocity, &meshBoundingBoxExtent_l);
			}
		}
	}

	// don't return a DetectResult that contains a pointer to a temporary
	// from CollisionDetect::testSphereExtent
	detectResult.extentA = 0;

	if(detectResult.collided)
	{
		detectResult.collisionTime = Range::inf;
	}

	return detectResult;
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testExtents ( BaseExtent const * extentA, BaseExtent const * extentB )
{
	if(extentA == nullptr) return false;
	if(extentB == nullptr) return false;

	ExtentType typeA = extentA->getType();
	ExtentType typeB = extentB->getType();

	// ----------

	if((typeA == ET_Simple) && (typeB == ET_Simple))    { return testSimpleSimple(extentA,extentB); }
	else if(typeA == ET_Component)                      { return testComponentAny(extentA,extentB); }
	else if(typeA == ET_Detail)                         { return testDetailAny   (extentA,extentB); }
	else if(typeB == ET_Component)                      { return testAnyComponent(extentA,extentB); }
	else if(typeB == ET_Detail)                         { return testAnyDetail   (extentA,extentB); }
	else                                                { return false; }
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testExtents ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB )
{
	if(extentA == nullptr) return false;
	if(extentB == nullptr) return false;

	ExtentType typeA = extentA->getType();
	ExtentType typeB = extentB->getType();

	// ----------

	if((typeA == ET_Simple) && (typeB == ET_Simple))    { return testSimpleSimple(extentA,velocity,extentB); }
	else if(typeA == ET_Component)                      { return testComponentAny(extentA,velocity,extentB); }
	else if(typeA == ET_Detail)                         { return testDetailAny   (extentA,velocity,extentB); }
	else if(typeB == ET_Component)                      { return testAnyComponent(extentA,velocity,extentB); }
	else if(typeB == ET_Detail)                         { return testAnyDetail   (extentA,velocity,extentB); }
	else                                                { return false; }
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testExtentsAsCylinders ( BaseExtent const * extentA, BaseExtent const * extentB )
{
	if(extentA == nullptr) return false;
	if(extentB == nullptr) return false;

	SimpleExtent const * simpleExtentA = safe_cast<SimpleExtent const * >(extentA);
	SimpleExtent const * simpleExtentB = safe_cast<SimpleExtent const * >(extentB);

	gs_testCounter++;

	MultiShape shapeA = simpleExtentA->getShape();
	MultiShape shapeB = simpleExtentB->getShape();

	// Stretch the cylinders vertically so that players can't squeeze under creatures

	shapeA.setExtentY( shapeA.getExtentY() * 5.0f );
	shapeB.setExtentY( shapeB.getExtentY() * 5.0f );

	// ----------

	ContainmentResult result = Containment3d::Test(shapeA.getCylinder(),shapeB.getCylinder());

	// ----------

	if(result != CR_Outside)
	{
		return DetectResult(true,extentA,extentB);
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testSimpleSimple ( BaseExtent const * extentA, BaseExtent const * extentB )
{
	SimpleExtent const * simpleExtentA = safe_cast<SimpleExtent const * >(extentA);
	SimpleExtent const * simpleExtentB = safe_cast<SimpleExtent const * >(extentB);

	if(!simpleExtentA) return false;
	if(!simpleExtentB) return false;

	gs_testCounter++;

	MultiShape const & shapeA = simpleExtentA->getShape();
	MultiShape const & shapeB = simpleExtentB->getShape();

	// ----------

	ContainmentResult result = Containment3d::Test(shapeA,shapeB);

	// ----------

	if(result != CR_Outside)
	{
		return DetectResult(true,extentA,extentB);
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testComponentAny ( BaseExtent const * extentA, BaseExtent const * extentB )
{
	ComponentExtent const * component = safe_cast< ComponentExtent const * >(extentA);

	if(!component) return false;

	NOT_NULL(component);

	for(int i = 0; i < component->getExtentCount(); i++)
	{
		BaseExtent const * newExtentA = component->getExtent(i);

		DetectResult result = CollisionDetect::testExtents(newExtentA,extentB);

		if(result.collided)
		{
			return result;
		}
	}

	return false;
}

// ----------

DetectResult CollisionDetect::testAnyComponent ( BaseExtent const * extentA, BaseExtent const * extentB )
{
	// Lint complains if I use safe_cast?

	ComponentExtent const * component = safe_cast< ComponentExtent const * >(extentB);

	if(!component) return false;

	NOT_NULL(component);

	for(int i = 0; i < component->getExtentCount(); i++)
	{
		BaseExtent const * newExtentB = component->getExtent(i);

		DetectResult result = CollisionDetect::testExtents(extentA,newExtentB);

		if(result.collided)
		{
			return result;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testDetailAny ( BaseExtent const * extentA, BaseExtent const * extentB )
{
	DetailExtent const * detail = safe_cast< DetailExtent const * >(extentA);

	if(!detail) return false;

	NOT_NULL(detail);

	DetectResult result;

	for(int i = 0; i < detail->getExtentCount(); i++)
	{
		BaseExtent const * newExtentA = detail->getExtent(i);

		result = CollisionDetect::testExtents(newExtentA,extentB);

		if(!result.collided)
		{
			return false;
		}
	}

	return result;
}

// ----------

DetectResult CollisionDetect::testAnyDetail ( BaseExtent const * extentA, BaseExtent const * extentB )
{
	DetailExtent const * detail = safe_cast< DetailExtent const * >(extentB);

	if(!detail) return false;

	NOT_NULL(detail);

	DetectResult result = false;

	for(int i = 0; i < detail->getExtentCount(); i++)
	{
		BaseExtent const * newExtentB = detail->getExtent(i);

		result = CollisionDetect::testExtents(extentA,newExtentB);

		if(!result.collided)
		{
			return false;
		}
	}

	return result;
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testSimpleSimple ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB )
{
	SimpleExtent const * simpleExtentA = safe_cast<SimpleExtent const * >(extentA);
	SimpleExtent const * simpleExtentB = safe_cast<SimpleExtent const * >(extentB);

	if(!simpleExtentA) return false;
	if(!simpleExtentB) return false;

	gs_testCounter++;

	MultiShape const & shapeA = simpleExtentA->getShape();
	MultiShape const & shapeB = simpleExtentB->getShape();

	// ----------

	Range range = Intersect3d::Intersect(shapeA,velocity,shapeB);

	Range clipRange = Intersect1d::IntersectRanges( range, Range::unit );

	// ----------

	if(!clipRange.isEmpty())
	{

		return DetectResult(true,clipRange,extentA,extentB);
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testComponentAny ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB )
{
	ComponentExtent const * component = safe_cast< ComponentExtent const * >(extentA);

	if(!component) return false;

	NOT_NULL(component);

	DetectResult minResult;

	for(int i = 0; i < component->getExtentCount(); i++)
	{
		BaseExtent const * newExtentA = component->getExtent(i);

		DetectResult result = CollisionDetect::testExtents(newExtentA,velocity,extentB);

		if(result.collisionTime.getMin() < minResult.collisionTime.getMin())
		{
			minResult = result;
		}
	}

	return minResult;
}

// ----------

DetectResult CollisionDetect::testAnyComponent ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB )
{
	// Lint complains if I use safe_cast?

	ComponentExtent const * component = safe_cast< ComponentExtent const * >(extentB);

	if(!component) return false;

	NOT_NULL(component);

	DetectResult minResult;

	for(int i = 0; i < component->getExtentCount(); i++)
	{
		BaseExtent const * newExtentB = component->getExtent(i);

		DetectResult result = CollisionDetect::testExtents(extentA,velocity,newExtentB);

		if(result.collisionTime.getMin() < minResult.collisionTime.getMin() )
		{
			minResult = result;
		}
	}

	return minResult;
}

// ----------------------------------------------------------------------

DetectResult CollisionDetect::testDetailAny ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB )
{
	DetailExtent const * detail = safe_cast< DetailExtent const * >(extentA);

	if(!detail) return false;

	NOT_NULL(detail);

	DetectResult result;

	for(int i = 0; i < detail->getExtentCount(); i++)
	{
		BaseExtent const * newExtentA = detail->getExtent(i);

		result = CollisionDetect::testExtents(newExtentA,velocity,extentB);

		if(!result.collided)
		{
			return false;
		}
	}

	return result;
}

// ----------

DetectResult CollisionDetect::testAnyDetail ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB )
{
	DetailExtent const * detail = safe_cast< DetailExtent const * >(extentB);

	if(!detail) return false;

	NOT_NULL(detail);

	DetectResult result;

	for(int i = 0; i < detail->getExtentCount(); i++)
	{
		BaseExtent const * newExtentB = detail->getExtent(i);

		result = CollisionDetect::testExtents(extentA,velocity,newExtentB);

		if(!result.collided)
		{
			return false;
		}
	}

	return result;
}

// ----------------------------------------------------------------------
