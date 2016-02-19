// ======================================================================
//
// SpaceAvoidanceManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/SpaceAvoidanceManager.h"

#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/CollisionDetect.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/SpatialDatabase.h"
#include "sharedMath/Range.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Object.h"

#include <vector>

// ======================================================================
//
// SpaceAvoidanceManagerNamespace
//
// ======================================================================

namespace SpaceAvoidanceManagerNamespace
{
	// Lag/Slop factors.
	float const cs_shipBoundSphereRadiusGain = 1.5f;

	// Dynamic and static query flags.
	int const cs_queryDynamicObjectFlags = static_cast<int>(SpatialDatabase::Q_Static) | static_cast<int>(SpatialDatabase::Q_Dynamic);
	int const cs_queryStaticObjectFlags = static_cast<int>(SpatialDatabase::Q_Static);
}

using namespace SpaceAvoidanceManagerNamespace;

// ======================================================================
//
// SpaceAvoidanceManager
//
// ======================================================================

// ----------------------------------------------------------------------

void SpaceAvoidanceManager::install()
{
}

// ----------------------------------------------------------------------
void SpaceAvoidanceManager::remove()
{
}

// ----------------------------------------------------------------------

bool SpaceAvoidanceManager::getAvoidancePosition(Object const & unit, Vector const & velocity, Vector const & targetPosition_w, Vector & avoidancePosition_w, bool const collideWithDynamicObjects)
{
	float const collisionRadius = unit.getCollisionProperty()->getBoundingSphere_w().getRadius();
	return getAvoidancePosition(unit.getTransform_o2w(), collisionRadius, velocity, targetPosition_w, avoidancePosition_w, &unit, collideWithDynamicObjects);
}

// ----------------------------------------------------------------------

bool SpaceAvoidanceManager::getAvoidancePosition(Transform const & objectTransform_o2w, float const collisionRadius, Vector const & velocity, Vector const & targetPosition_w, Vector & avoidancePosition_w, const Object * const excludeObject, bool const collideWithDynamicObjects)
{
	bool hasValidCollision = false;

	float const currentSpeed = velocity.approximateMagnitude();

	if (currentSpeed > FLT_MIN)
	{
		// Keep local in case of a recursive call.
		ColliderList colliderList;

		// Sweep the collision representation.
		Vector const & objectPosition_w = objectTransform_o2w.getPosition_p();
		Sphere const querySphere_w(objectPosition_w, collisionRadius * cs_shipBoundSphereRadiusGain);
		Vector const & intersectVector = targetPosition_w - objectPosition_w;
		Capsule const queryCapsule_w(querySphere_w, intersectVector);
		CellProperty const * const queryCell_w = CellProperty::getWorldCellProperty();
		CollisionWorld::getDatabase()->queryFor(collideWithDynamicObjects ? cs_queryDynamicObjectFlags : cs_queryStaticObjectFlags, queryCell_w, false, queryCapsule_w, colliderList);

		if (!colliderList.empty())
		{
			Sphere collisionObjectSphere_w;
			Vector collisionPosition_w;
			float collisionClosestDistance = FLT_MAX;
			Vector collisionNormal_w;

			for(ColliderList::const_iterator iterColliderList = colliderList.begin(); iterColliderList != colliderList.end(); ++iterColliderList)
			{
				CollisionProperty * collisionProperty = NON_NULL(*iterColliderList);
				Object const * const owner = NON_NULL(&collisionProperty->getOwner());

				// Don't collide with the excluded object.
				if (excludeObject != owner)
				{
					Sphere const & collisionSphere_w = collisionProperty->getBoundingSphere_w();
					BaseExtent const * const collisionExtent = NON_NULL(collisionProperty->getExtent_l());
					Vector const & endPoint_l = owner->rotateTranslate_w2o(objectPosition_w);
					Vector const & startPoint_l = owner->rotateTranslate_w2o(targetPosition_w);
					float intersectTime = 0.0f;
					Vector collisionSurfaceNormal_l;

					if (collisionExtent->intersect(startPoint_l, endPoint_l, &collisionSurfaceNormal_l, &intersectTime))
					{
						Vector const & closestCollisionPosition = linearInterpolate(objectPosition_w, targetPosition_w, intersectTime);
						float const collisionDistance = (closestCollisionPosition - objectPosition_w).approximateMagnitude();

						// Grab the closest collision.
						if (collisionDistance < collisionClosestDistance && collisionDistance > collisionRadius)
						{
							collisionClosestDistance = collisionDistance;
							collisionPosition_w = closestCollisionPosition;
							collisionObjectSphere_w = collisionSphere_w;
							collisionNormal_w = owner->rotate_o2w(collisionSurfaceNormal_l);
							hasValidCollision = true;
						}
					}
				}
			}

			if (hasValidCollision)
			{
				if(ConfigSharedCollision::useOriginalAvoidanceAlgorithm())
				{
					Vector const & collisionSphereCenter_w = collisionObjectSphere_w.getCenter();
					Vector perpendiculara(Vector::perpendicular(collisionSphereCenter_w - objectPosition_w));
					Vector perpendicularb = -perpendiculara;
					
					// If normalize fails, do not return the collision.
					hasValidCollision = perpendiculara.approximateNormalize() && perpendicularb.approximateNormalize();
					if(hasValidCollision)
					{
						float const collisionOffset = collisionObjectSphere_w.getRadius() + (collisionRadius * cs_shipBoundSphereRadiusGain * 2.f);
						const Vector & avoidancePositionOption1_w = collisionSphereCenter_w + (perpendiculara * collisionOffset);
						const Vector & avoidancePositionOption2_w = collisionSphereCenter_w + (perpendicularb * collisionOffset);
						
					 	// Make sure the ship takes the shortest path around the sphere
						float const distance1 = avoidancePositionOption1_w.magnitudeBetweenSquared(targetPosition_w);
						float const distance2 = avoidancePositionOption2_w.magnitudeBetweenSquared(targetPosition_w);
						if (distance1 < distance2)
						{
							avoidancePosition_w = avoidancePositionOption1_w;
						}
						else
						{
							avoidancePosition_w = avoidancePositionOption2_w;
						}
					}
				}
				else
				{
					// Add the intersect and collision vectors.
					Vector collisionVector(intersectVector + collisionNormal_w);
					hasValidCollision = collisionVector.normalize();

					// If normalize fails, do not use this collision. 
					if (hasValidCollision)
					{
						// Find a vector perpendicular to collision vector.
						Vector const perpendiculara(Vector::perpendicular(collisionVector));
						Vector const perpendicularb(-perpendiculara);

						// Build a collision offset.
						float const collisionOffset = ((collisionRadius * 2.0f) + collisionClosestDistance) * cs_shipBoundSphereRadiusGain;
						Vector const & avoidancePositionOption1_w = collisionPosition_w + (perpendiculara * collisionOffset);
						Vector const & avoidancePositionOption2_w = collisionPosition_w + (perpendicularb * collisionOffset);
						
						// Ensure the ship takes the shortest path around the sphere.
						float const distance1 = avoidancePositionOption1_w.magnitudeBetweenSquared(targetPosition_w);
						float const distance2 = avoidancePositionOption2_w.magnitudeBetweenSquared(targetPosition_w);
						avoidancePosition_w = (distance1 < distance2) ? avoidancePositionOption1_w : avoidancePositionOption2_w;
					}
					else
					{
						WARNING_STRICT_FATAL(ConfigSharedCollision::isSpaceAiLoggingEnabled(), ("SpaceAvoidanceManager::getAvoidancePosition() - perpendicular.approximateNormalize() failed."));
					}
				}
			}
		}
	}
	
	return hasValidCollision;
}

// ======================================================================
