#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/SpatialDatabase.h"

#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/CollisionDetect.h"
#include "sharedCollision/CollisionProperty.h"

#include "sharedObject/Appearance.h"
#include "sharedObject/Object.h"
#include "sharedObject/World.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/CellProperty.h"

#include "sharedCollision/BarrierObject.h"
#include "sharedCollision/CollisionResolve.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/DoorObject.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/Footprint.h"
#include "sharedCollision/Intersect1d.h"
#include "sharedCollision/MeshExtent.h"
#include "sharedCollision/SimpleExtent.h"

#include "sharedMath/Line3d.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/SphereTree.h"
#include "sharedMath/Range.h"
#include "sharedMath/ShapeUtils.h"

#include <vector>
#include <set>
#include <algorithm>

// Need to disable this warning to make MultiListHandle work

#pragma warning(disable : 4355) // 'this' used in base member initializer list

// ======================================================================

namespace SpatialDatabaseNamespace
{
	SpatialDatabase::ShouldTriggerDoorHitCallback  s_shouldTriggerDoorHitCallback;
}

using namespace SpatialDatabaseNamespace;

// ======================================================================

class CollisionSphereAccessor: public BaseSphereTreeAccessor<CollisionProperty *, CollisionSphereAccessor>
{
public:

	static Sphere getExtent(CollisionProperty const * const collision)
	{
		return collision->getBoundingSphere_w();
	}

	static char const *getDebugName(CollisionProperty const * const collision)
	{
		if (collision)
		{
			Object const &object = collision->getOwner();
			ObjectTemplate const * const objectTemplate = object.getObjectTemplate();
			if (objectTemplate)
				return objectTemplate->getName();
		}

		return 0;
	}

};

// ----------

class FloorSphereAccessor: public BaseSphereTreeAccessor<Floor *, FloorSphereAccessor>
{
public:

	static Sphere getExtent(Floor const * const floor)
	{
		return floor->getBoundingSphere_w();
	}

	static char const *getDebugName(Floor const * const floor)
	{
		if (floor)
		{
			Object const * const object = floor->getOwner();

			if (object)
			{
				ObjectTemplate const * const objectTemplate = object->getObjectTemplate();
				if (objectTemplate)
					return objectTemplate->getName();
			}
		}

		return 0;
	}

};

// ======================================================================
// class SpatialDatabase: PUBLIC STATIC
// ======================================================================
/**
 * This callback allows the user to prevent mobile/door hits from triggering
 * the hit() and hitBy() collision logic on a door based on the return value of
 * the callback.
 *
 * This functionality is used on the client to prevent a door from visually opening for
 * a mounted creature or a rider.  The actual stop logic for movement is on the server.
 *
 * @param callback  If nullptr, no check is made.  If non-nullptr, the callback is called
 *                  any time a mobile is detected to have collided with a DoorObject.
 *                  The return value of the callback dictates whether the hit() and hitBy()
 *                  logic is called on the mobile and door.  If the return value of callback
 *                  is true, then the hit()/hitBy() functions are called; otherwise, hit()/hitBy()
 *                  are not called.
 */

void SpatialDatabase::setShouldTriggerDoorHitCallback (ShouldTriggerDoorHitCallback callback)
{
	s_shouldTriggerDoorHitCallback = callback;
}

// ----------------------------------------------------------------------

SpatialDatabase::ShouldTriggerDoorHitCallback SpatialDatabase::getShouldTriggerDoorHitCallback ()
{
	return s_shouldTriggerDoorHitCallback;
}

// ======================================================================

SpatialDatabase::SpatialDatabase()
: m_staticTree(new CollisionSphereTree())
, m_dynamicTree(new CollisionSphereTree())
, m_doorTree(new CollisionSphereTree())
, m_barrierTree(new CollisionSphereTree())
, m_floorTree(new FloorSphereTree())
, m_ignoreStack(new ObjectConstVec())
{
}

// ----------

SpatialDatabase::~SpatialDatabase ( void )
{
	delete m_staticTree;
	m_staticTree = nullptr;

	delete m_dynamicTree;
	m_dynamicTree = nullptr;

	delete m_doorTree;
	m_doorTree = nullptr;

	delete m_barrierTree;
	m_barrierTree = nullptr;

	delete m_floorTree;
	m_floorTree = nullptr;

	delete m_ignoreStack;
	m_ignoreStack = nullptr;
}

// ----------------------------------------------------------------------

bool SpatialDatabase::canCollideWithStatics ( CollisionProperty * collision ) const
{
	if(collision == nullptr) return false;

	// On the client, remote creatures and players don't collide with statics

	if( (!collision->isServerSide()) && (!collision->isPlayer()))
	{
		return false;
	}

	if(collision->isPlayerControlled() && collision->isServerSide())
	{
		return false;
	}

	Vector oldPos = CollisionUtils::transformToCell(collision->getLastCell(),collision->getLastPos_p(),collision->getOwner().getParentCell());
	Vector newPos = collision->getOwner().getPosition_p();

	Vector delta(newPos.x - oldPos.x, newPos.y - oldPos.y, newPos.z - oldPos.z);

	// If we didn't hit anything last frame, and we haven't moved, don't do collision.

	if((delta.magnitudeSquared() < 0.0000000001f ) && collision->isIdle() )
	{
		return false;
	}

	return true;
}

// ----------

bool SpatialDatabase::canCollideWithStatic ( Object const * mobObject, Object * staticObject ) const
{
	// Objects can't collide with nullptr

	if(mobObject == nullptr) return false;
	if(staticObject == nullptr) return false;

	// Objects can't collide with themselves

	if(mobObject == staticObject) return false;

	return true;
}

// ----------

bool SpatialDatabase::canCollideWithCreatures ( CollisionProperty const * collision ) const
{
	if(collision == nullptr) return false;

	if(collision->isPlayerControlled() && collision->isServerSide())
	{
		return false;
	}

	return ConfigSharedCollision::getEnableCreatureCollision();
}

bool SpatialDatabase::canCollideWithCreature ( Object * player, Object * creature ) const
{
	if(!player) return false;
	if(!creature) return false;

	if(player->getParentCell() != creature->getParentCell()) return false;

	CollisionProperty * playerCollision = player->getCollisionProperty();
	CollisionProperty * creatureCollision = creature->getCollisionProperty();

	if(!playerCollision) return false;
	if(!creatureCollision) return false;

	return playerCollision->canCollideWith(creatureCollision);
}

// ----------

bool SpatialDatabase::canWalkOnFloor ( CollisionProperty * collision ) const
{
	if(collision == nullptr) return false;

	if(collision->getFootprint() == nullptr) return false;

	return true;
}

// ----------------------------------------------------------------------

void SpatialDatabase::updateStaticCollision(CollisionProperty * mobCollision, ColliderList & colliderList)
{
	NOT_NULL(mobCollision);

	Object * mobObject = &mobCollision->getOwner();

	NOT_NULL(mobObject);

	if(!canCollideWithStatics(mobCollision)) return;

	// ----------

	Capsule queryCapsule_w = mobCollision->getQueryCapsule_w();

	{
		static ColliderList queryResults;
		queryResults.clear();

		m_staticTree->findInRange(queryCapsule_w, queryResults);
		m_barrierTree->findInRange(queryCapsule_w, queryResults);

		int staticCount = static_cast<int>(queryResults.size());

		for(int i = 0; i < staticCount; i++)
		{
			CollisionProperty * staticCollision = queryResults.at(static_cast<unsigned int>(i));

			NOT_NULL(staticCollision);

			Object * staticObject = &staticCollision->getOwner();

			NOT_NULL(staticObject);

			if(!canCollideWithStatic(mobObject,staticObject)) continue;

			if(!staticCollision->canCollideWith(mobCollision)) continue;
			if(!mobCollision->canCollideWith(staticCollision)) continue;

			// ----------

			Capsule queryCapsule_p = CollisionUtils::transformFromWorld(queryCapsule_w,staticObject->getParentCell());

			DetectResult result = CollisionDetect::testCapsuleObject(queryCapsule_p,staticObject);

			if(result.collided)
			{
				colliderList.push_back(staticCollision);
			}
		}
	}

	{
		static ColliderList queryResults;
		queryResults.clear();

		m_doorTree->findInRange(queryCapsule_w, queryResults);

		int const staticCount = static_cast<int>(queryResults.size());
		for(int i = 0; i < staticCount; i++)
		{
			CollisionProperty * const staticCollision = queryResults.at(static_cast<unsigned int>(i));
			NOT_NULL(staticCollision);

			Object * const staticObject = &staticCollision->getOwner();
			NOT_NULL(staticObject);

			if (!canCollideWithStatic(mobObject,staticObject))
				 continue;

			if (!staticCollision->canCollideWith(mobCollision))
				 continue;

			if (!mobCollision->canCollideWith(staticCollision))
				 continue;

			Capsule const queryCapsule_p = CollisionUtils::transformFromWorld(queryCapsule_w,staticObject->getParentCell());

			DetectResult result = CollisionDetect::testCapsuleObject(queryCapsule_p,staticObject);
			if (result.collided)
			{
				DoorObject * const door = safe_cast<DoorObject *>(staticObject);
				if (!s_shouldTriggerDoorHitCallback || (*s_shouldTriggerDoorHitCallback)(mobCollision->getOwner(), *door))
				{
					mobCollision->hit(staticCollision);
					staticCollision->hitBy(mobCollision);
				}
			}
		}
	}
} //lint !e1762 // member function could be made const // Semantically this is non-const.

// ----------------------------------------------------------------------

void SpatialDatabase::updateFloorCollision ( CollisionProperty * mobCollision, bool attachFromObject )
{
	NOT_NULL(mobCollision);

	Object * mobObject = &mobCollision->getOwner();

	NOT_NULL(mobObject);

	if(!canWalkOnFloor(mobCollision)) return;

	Footprint const * foot = mobCollision->getFootprint();

	NOT_NULL(foot);

	NOT_NULL(mobCollision);

	// ----------
	// Test for collisions with floors

	Capsule queryCapsule_w = mobCollision->getQueryCapsule_w();

	static FloorVec queryResults;

	queryResults.clear();

	m_floorTree->findInRange( queryCapsule_w, queryResults );

	for(uint i = 0; i < queryResults.size(); i++)
	{
		Floor * floor = queryResults.at(i);

		if(!floor) continue;

		Capsule queryCapsule_p = CollisionUtils::transformFromWorld(queryCapsule_w,floor->getCell());

		BaseExtent const * floorExtent_p = floor->getExtent_p();

		DetectResult result = CollisionDetect::testCapsuleExtent(queryCapsule_p,floorExtent_p);

		if(result.collided)
		{
			Object const * owner = floor->getOwner();

			if(owner)
			{
				if(mobObject->getParentCell() != owner->getParentCell())
				{
					continue;
				}
			}

			Footprint * pFoot = mobCollision->getFootprint();

			if(pFoot)
			{
				IGNORE_RETURN( pFoot->attachTo(floor,attachFromObject) );
			}
		}
	}
} //lint !e1762 // member function could be made const // Semantically this is non-const.

// ----------------------------------------------------------------------

void SpatialDatabase::updateCreatureCollision(CollisionProperty * playerCollision, ColliderList & colliderList)
{
	NOT_NULL(playerCollision);

	Object * playerObject = &playerCollision->getOwner();

	NOT_NULL(playerObject);

	if(!canCollideWithCreatures(playerCollision)) return;

	// ----------

	Capsule queryCapsule_w = playerCollision->getQueryCapsule_w();

	static ColliderList queryResults;

	queryResults.clear();

	m_dynamicTree->findInRange( queryCapsule_w, queryResults );

	int creatureCount = static_cast<int>(queryResults.size());

	for(int j = 0; j < creatureCount; j++)
	{
		CollisionProperty * creatureCollision = queryResults.at(static_cast<unsigned int>(j));

		NOT_NULL(creatureCollision);

		Object * creatureObject = &creatureCollision->getOwner();

		NOT_NULL(creatureObject);

		if(!canCollideWithCreature(playerObject,creatureObject)) continue;

		// ----------

		Capsule queryCapsule_p = CollisionUtils::transformFromWorld(queryCapsule_w,creatureObject->getParentCell());

		DetectResult result = CollisionDetect::testCapsuleObject(queryCapsule_p,creatureObject);

		if(result.collided)
		{
			colliderList.push_back(creatureCollision);
		}

	}
} //lint !e1762 // member function could be made const // Semantically this is non-const.

// ----------------------------------------------------------------------

bool SpatialDatabase::addObject(Query const query, Object * const object)
{
	if(object == nullptr)
		return false;

	CollisionProperty * collision = object->getCollisionProperty();

	if(collision == nullptr)
		return false;

	if(collision->getSpatialSubdivisionHandle())
	{
		WARNING(true, ("SpatialDatabase:: %s has already been added and has a SpatialSubdivisionHandle", object->getObjectTemplate()->getName()));
		return false;
	}

	switch (query)
	{
		case Q_Static:
			collision->setSpatialSubdivisionHandle(m_staticTree->addObject(collision));
			break;
		case Q_Dynamic:
			collision->setSpatialSubdivisionHandle(m_dynamicTree->addObject(collision));
			break;
		case Q_Barriers:
			collision->setSpatialSubdivisionHandle(m_barrierTree->addObject(collision));
			break;
		case Q_Doors:
			collision->setSpatialSubdivisionHandle(m_doorTree->addObject(collision));
			break;
		case Q_All:
			FATAL(true, ("SpatialDatabase::addObject: Q_All is not a valid parameter for add"));
		default:
			break;
	}

	return true;
}

// ----------------------------------------------------------------------

bool SpatialDatabase::removeObject(Query const query, Object * const object)
{
	if(object == nullptr)
		return false;

	CollisionProperty * collision = object->getCollisionProperty();

	if(collision == nullptr)
		 return false;

	if(collision->getSpatialSubdivisionHandle() == nullptr)
		return false;

	switch (query)
	{
		case Q_Static:
			m_staticTree->removeObject(collision->getSpatialSubdivisionHandle());
			break;
		case Q_Dynamic:
			m_dynamicTree->removeObject(collision->getSpatialSubdivisionHandle());
			break;
		case Q_Barriers:
			m_barrierTree->removeObject(collision->getSpatialSubdivisionHandle());
			break;
		case Q_Doors:
			m_doorTree->removeObject(collision->getSpatialSubdivisionHandle());
			break;
		case Q_All:
			WARNING(true, ("SpatialDatabase::removeObject: is being called with Q_All"));
			m_staticTree->removeObject(collision->getSpatialSubdivisionHandle());
			m_dynamicTree->removeObject(collision->getSpatialSubdivisionHandle());
			m_barrierTree->removeObject(collision->getSpatialSubdivisionHandle());
			m_doorTree->removeObject(collision->getSpatialSubdivisionHandle());
			break;

		default:
			break;
	}

	collision->setSpatialSubdivisionHandle(nullptr);

	return true;
}

// ----------------------------------------------------------------------

int SpatialDatabase::getObjectCount(int queryMask) const
{
	int count = 0;

	if (queryMask & Q_Static)
		count += m_staticTree->getObjectCount();

	if (queryMask & Q_Dynamic)
		count += m_dynamicTree->getObjectCount();

	return count;
}

// ----------------------------------------------------------------------

void SpatialDatabase::queryFor(int queryMask, CellProperty const * const cell_p, bool restrictToSameCell, Capsule const & capsule_p, ColliderList & collidedWith) const
{
	Capsule const capsule_w(CollisionUtils::transformToWorld(cell_p, capsule_p));

	ColliderList temporaryCollidedWith;

	if (queryMask & Q_Static)
		m_staticTree->findInRange(capsule_w, temporaryCollidedWith);

	if (queryMask & Q_Dynamic)
		m_dynamicTree->findInRange(capsule_w, temporaryCollidedWith);

	if (queryMask & Q_Barriers)
		m_barrierTree->findInRange(capsule_w, temporaryCollidedWith);

	if (queryMask & Q_Doors)
		m_doorTree->findInRange(capsule_w, temporaryCollidedWith);

	int const potentialCount = static_cast<int>(temporaryCollidedWith.size());

	for(int i = 0; i < potentialCount; ++i)
	{
		CollisionProperty * const toTestCollision = temporaryCollidedWith.at(static_cast<unsigned int>(i));

		Object const * const toTestObject = &toTestCollision->getOwner();

		if ((restrictToSameCell) && (cell_p != toTestObject->getParentCell()))
		{
			continue;
		}

		Capsule const toTestCapsule_p(CollisionUtils::transformFromWorld(capsule_w,toTestObject->getParentCell()));

		DetectResult const result(CollisionDetect::testCapsuleObjectAgainstAllTypes(toTestCapsule_p, toTestObject));

		if(result.collided)
		{
			collidedWith.push_back(toTestCollision);
		}
	}
}

// ----------------------------------------------------------------------
// The ignore stack is used for temporarily "hiding" objects during
// a queryInteraction or queryMaterial test

void SpatialDatabase::pushIgnoreObject ( Object const * object )
{
	m_ignoreStack->push_back(object);
}

void SpatialDatabase::popIgnoreObject ( void )
{
	m_ignoreStack->pop_back();
}

bool SpatialDatabase::checkIgnoreObject ( Object const * object ) const
{
	unsigned int ignoreCount = m_ignoreStack->size();

	for(unsigned int i = 0; i < ignoreCount; i++)
	{
		if(object == m_ignoreStack->at(i))
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool SpatialDatabase::addFloor ( Floor * floor )
{
	if(floor == nullptr) return false;

	if(floor->getSpatialSubdivisionHandle()) return false;

	// ----------

	floor->setSpatialSubdivisionHandle( m_floorTree->addObject(floor) );

	return true;
}

// ----------

bool SpatialDatabase::removeFloor ( Floor * floor )
{
	if(floor == nullptr) return false;

	if(floor->getSpatialSubdivisionHandle() == nullptr) return false;

	// ----------

	m_floorTree->removeObject( floor->getSpatialSubdivisionHandle() );

	floor->setSpatialSubdivisionHandle(nullptr);

	return true;
}

// ----------

int SpatialDatabase::getFloorCount ( void ) const
{
	return m_floorTree->getObjectCount();
}

// ----------------------------------------------------------------------

bool SpatialDatabase::moveObject ( CollisionProperty * collision )
{
	if(collision == nullptr) return false;

	SpatialSubdivisionHandle * handle = collision->getSpatialSubdivisionHandle();

	if(handle == nullptr) return false;

	// ----------

	if (collision->isMobile())
	{
		m_dynamicTree->move(handle);
	}
	else
	{
		m_staticTree->move(handle);
	}

	Floor * floor = collision->getFloor();

	if (floor)
	{
		m_floorTree->move(floor->getSpatialSubdivisionHandle());
	}

	return true;
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryStatics ( Sphere const & sphere, ObjectVec * outList ) const
{
	return queryStatics(MultiShape(sphere),outList);
}

// ----------

bool SpatialDatabase::queryStatics ( AxialBox const & box, ObjectVec * outList ) const
{
	return queryStatics(MultiShape(box),outList);
}

// ----------

bool SpatialDatabase::queryStatics ( MultiShape const & shape, ObjectVec * outList ) const
{
	return queryStatics( nullptr, shape, outList );
}

// ----------

bool SpatialDatabase::queryStatics ( Line3d const & line, ObjectVec * outList ) const
{
	bool result = false;

	Vector normDir = line.getNormal();
	IGNORE_RETURN(normDir.normalize());

	static ColliderList results;

	results.clear();

	m_staticTree->findOnRay(line.getPoint(),normDir,results);
	m_barrierTree->findOnRay(line.getPoint(),normDir,results);

	uint const resultSize = results.size();
	for(uint i = 0; i < resultSize; ++i)
	{
		CollisionProperty * collision = results[i];

		BaseExtent const * extent = collision->getExtent_p();

		Range hitRange = extent->rangedIntersect(line);

		if(!hitRange.isEmpty())
		{
			result = true;

			if(outList)
			{
				outList->push_back( &collision->getOwner() );
			}
			else
			{
				break;
			}
		}
	}

	return result;
}

// ----------

bool SpatialDatabase::queryStatics ( CellProperty const * cell, MultiShape const & shape, ObjectVec * outList ) const
{
	return queryObjects(cell,shape,outList,nullptr);
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryDynamics ( Sphere const & sphere, ObjectVec * outList ) const
{
	return queryObjects( nullptr, MultiShape(sphere), nullptr, outList );
}

// ----------

bool SpatialDatabase::queryDynamics ( MultiShape const & shape, ObjectVec * outList ) const
{
	return queryObjects( nullptr, shape, nullptr, outList );
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryObjects ( CellProperty const * cell, MultiShape const & shape, ObjectVec * staticList, ObjectVec * creatureList ) const
{
	return queryObjects(cell,shape,Vector::zero,staticList,creatureList);
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryObjects ( CellProperty const * cell, MultiShape const & shape, Vector const & velocity,
                                     ObjectVec * staticList, ObjectVec * creatureList ) const
{
	SimpleExtent tempExtent(shape);

	bool result = false;

	static ColliderList tempResults;

	Sphere sphere(shape.getBoundingSphere());

	Capsule capsule(sphere,velocity);

	// ----------

	if(staticList)
	{
		tempResults.clear();

		m_staticTree->findInRange( capsule, tempResults );
		m_dynamicTree->findInRange( capsule, tempResults );
		m_barrierTree->findInRange( capsule, tempResults );

		for(uint i = 0; i < tempResults.size(); i++)
		{
			CollisionProperty * staticCollision = tempResults[i];

			NOT_NULL(staticCollision);

			Object * owner = &(staticCollision->getOwner());

			if(cell && (owner->getParentCell() != cell)) continue;

			DetectResult detectResult = CollisionDetect::testExtents(&tempExtent,velocity,staticCollision->getExtent_p());

			if(detectResult.collided)
			{
				result = true;

				staticList->push_back(owner);
			}
		}
	}

	// ----------

	if(creatureList)
	{
		tempResults.clear();

		m_dynamicTree->findInRange( capsule, tempResults );

		for(uint i = 0; i < tempResults.size(); i++)
		{
			CollisionProperty * mobCollision = tempResults[i];

			NOT_NULL(mobCollision);

			Object * owner = &(mobCollision->getOwner());

			if(cell && (owner->getParentCell() != cell)) continue;

			DetectResult detectResult = CollisionDetect::testExtents(&tempExtent,velocity,mobCollision->getExtent_p());

			if(detectResult.collided)
			{
				result = true;

				creatureList->push_back(&mobCollision->getOwner());
			}
		}
	}

	// ----------

	return result;
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryFloors ( Sphere const & sphere_w, FloorVec * outList ) const
{
	return queryFloors(sphere_w,Vector::zero,outList);
}

bool SpatialDatabase::queryFloors ( Capsule const & capsule, FloorVec * outList ) const
{
	return queryFloors( capsule.getSphereA(), capsule.getDelta(), outList );
}

// ----------

bool SpatialDatabase::queryFloors ( Sphere const & sphere_w, Vector const & delta, FloorVec * outList ) const
{
	if(outList) outList->clear();

	static FloorVec tempResults;

	tempResults.clear();

	m_floorTree->findInRange( Capsule(sphere_w,delta), tempResults );

	bool result = false;

	for(uint i = 0; i < tempResults.size(); i++)
	{
		Floor * floor = tempResults[i];

		// ----------

		if(!floor) continue;

		if(floor->getCell() != CellProperty::getWorldCellProperty() ) continue;

		// ----------
		// testing world-space sphere with parent-space extent is OK because we know the extent is in the world cell

		DetectResult detectResult = CollisionDetect::testSphereExtent(sphere_w,delta,floor->getExtent_p());

		if(detectResult.collided)
		{
			result = true;

			if(outList) outList->push_back(floor);
		}
	}

	// ----------

	return result;
}

// ----------

bool SpatialDatabase::queryFloors ( CellProperty const * cell, Segment3d const & seg, FloorVec * outList ) const
{
	bool result = false;

	// ----------

	FloorVec tempResults;

	m_floorTree->findOnSegment( seg.getBegin(), seg.getEnd(), tempResults );

	for(uint i = 0; i < tempResults.size(); i++)
	{
		Floor * floor = tempResults[i];

		if(!floor) continue;

		if(cell)
		{
			if(floor->getCell() != cell) continue;
		}

		BaseExtent const * floorExtent = floor->getExtent_p();

		if(!floorExtent->rangedIntersect(seg).isEmpty())
		{
			result = true;

			if(outList) outList->push_back(floor);
		}
	}

	// ----------

	return result;
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryCloseStatics ( Vector const & point_w, float maxDistance, ObjectVec * outList ) const
{
	float minClose;
	float maxClose;

	CollisionProperty * dummy = nullptr;

	if(m_staticTree->findClosest(point_w,maxDistance,dummy,minClose,maxClose))
	{
		return queryStatics(Sphere(point_w,maxClose),outList);
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryCloseFloors ( Vector const & point_w, float maxDistance, FloorVec * outList ) const
{
	float minClose;
	float maxClose;

	Floor * dummy = nullptr;

	if(m_floorTree->findClosest(point_w,maxDistance,dummy,minClose,maxClose))
	{
		return queryFloors(Sphere(point_w,maxClose),outList);
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryInteraction ( CellProperty const * cell,
										 Segment3d const & seg_p,
										 Object const * ignoreObject,
										 Object const * & outHitObject,
										 float & outHitTime) const
{
	// NOTE -- this returns the first object tested against and not
	// the closest as is implied;

	static ColliderList results;
	static ColliderList barrierResults;

	results.clear();
	barrierResults.clear();

	Vector begin_w = CollisionUtils::transformToWorld(cell,seg_p.getBegin());
	Vector end_w = CollisionUtils::transformToWorld(cell,seg_p.getEnd());

	m_staticTree->findOnSegment(begin_w,end_w,results);
	m_barrierTree->findOnSegment(begin_w,end_w,barrierResults);

	ColliderList::const_iterator ii = barrierResults.begin();
	ColliderList::const_iterator iiEnd = barrierResults.end();

	for(; ii != iiEnd; ++ii)
	{
		CollisionProperty * const collision = *ii;
		BarrierObject const * const barrier = safe_cast<BarrierObject const * const>(&collision->getOwner());

		if (barrier && barrier->isActive())
			results.push_back(collision);
	}

	ii = results.begin();
	iiEnd = results.end();

	for(; ii != iiEnd; ++ii)
	{
		CollisionProperty * const collision = *ii;

		if (&collision->getOwner() == ignoreObject)
			 continue;

		if (checkIgnoreObject(&collision->getOwner()))
			 continue;

		if (cell && (collision->getOwner().getParentCell() != cell))
			continue;

		// _must_ do this test - lairs use the flags to keep from blocking LOS

		if( !collision->blocksInteraction(IT_See) ) continue;

		// ----------

		BaseExtent const * extent = collision->getExtent_p();

		float t = 0.0f;
		if (extent->intersect(seg_p.getBegin(), seg_p.getEnd(), &t))
		{
			outHitObject = &collision->getOwner();
			outHitTime = t * seg_p.getBegin().magnitudeBetween(seg_p.getEnd());
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryMaterial ( CellProperty const * cell,
									  Vector const & point_p,
									  MaterialType /*material*/ ) const
{
	static ColliderList results;

	results.clear();

	Vector point_w = CollisionUtils::transformToWorld(cell,point_p);

	m_staticTree->findAtPoint(point_w,results);
	m_barrierTree->findAtPoint(point_w,results);

	for(uint i = 0; i < results.size(); i++)
	{
		CollisionProperty * collision = results[i];

		if(checkIgnoreObject(&collision->getOwner())) continue;

		if(cell && (collision->getOwner().getParentCell() != cell))
		{
			continue;
		}

		if( !collision->blocksMovement() ) continue;

		// ----------

		BaseExtent const * extent = collision->getExtent_p();

		if(extent->contains(point_p))
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryMaterial ( CellProperty const * cell,
									  Segment3d const & seg_p,
									  MaterialType /*material*/ ) const
{
	static ColliderList results;

	results.clear();

	Vector begin_w = CollisionUtils::transformToWorld(cell,seg_p.getBegin());
	Vector end_w = CollisionUtils::transformToWorld(cell,seg_p.getEnd());

	m_staticTree->findOnSegment(begin_w,end_w,results);
	m_barrierTree->findOnSegment(begin_w,end_w,results);

	for(uint i = 0; i < results.size(); i++)
	{
		CollisionProperty * collision = results[i];

		if(checkIgnoreObject(&collision->getOwner())) continue;

		if(cell && (collision->getOwner().getParentCell() != cell))
		{
			continue;
		}

		if( !collision->blocksMovement() ) continue;

		// ----------

		BaseExtent const * extent = collision->getExtent_p();

		// don't do material queries on mesh extents

		// TODO: why are we skipping out on the tests when the extent is a MeshExtent???
		MeshExtent const * mesh = dynamic_cast<MeshExtent const *>(extent);

		if(mesh != nullptr) continue;

		Range hitRange = extent->rangedIntersect(seg_p);

		if(!hitRange.isEmpty())
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryMaterial ( CellProperty const * cellA,
									  Sphere const & sphereA_p,
									  Vector const & velocityA_p,
									  MaterialType /*materialA*/,
									  bool checkFlora,
									  bool checkFauna,
									  Object const * ignoreObject,
									  float & outCollisionTime ) const
{
	Capsule queryCapsule_w = CollisionUtils::transformToWorld(cellA, Capsule(sphereA_p,velocityA_p));

	static ColliderList results;

	results.clear();

	m_staticTree->findInRange(queryCapsule_w,results);
	m_barrierTree->findInRange(queryCapsule_w,results);

	{
		for(uint i = 0; i < results.size(); i++)
		{
			CollisionProperty * collisionB = results[i];

			NOT_NULL(collisionB);

			if(&collisionB->getOwner() == ignoreObject) continue;

			if(checkIgnoreObject(&collisionB->getOwner())) continue;

			if(!checkFlora && collisionB->isFlora())
			{
				continue;
			}

			if( !collisionB->blocksMovement() ) continue;

			// ----------

			CellProperty const * cellB = collisionB->getCell();

			Sphere sphereA_B = CollisionUtils::transformToCell(cellA,sphereA_p,cellB);

			Vector velocityA_B = CollisionUtils::rotateToCell(cellA,velocityA_p,cellB);

			DetectResult result = CollisionDetect::testSphereObject(sphereA_B,velocityA_B,&collisionB->getOwner());

			if(result.collided)
			{
				outCollisionTime = result.collisionTime.getMin();

				return true;
			}
		}
	}

	if(checkFauna)
	{
		results.clear();

		m_dynamicTree->findInRange(queryCapsule_w,results);

		for(uint i = 0; i < results.size(); i++)
		{
			CollisionProperty * collisionB = results[i];

			NOT_NULL(collisionB);
			if (collisionB == nullptr)
				continue;

			// ----------

			if(&collisionB->getOwner() == ignoreObject) continue;

			if(checkIgnoreObject(&collisionB->getOwner())) continue;

			CellProperty const * cellB = collisionB->getCell();

			Sphere sphereA_B = CollisionUtils::transformToCell(cellA,sphereA_p,cellB);

			Vector velocityA_B = CollisionUtils::rotateToCell(cellA,velocityA_p,cellB);

			DetectResult result = CollisionDetect::testSphereObject(sphereA_B,velocityA_B,&collisionB->getOwner());

			if(result.collided)
			{
				outCollisionTime = result.collisionTime.getMin();

				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool SpatialDatabase::queryMaterial ( CellProperty const * cellA,
									  Sphere const & sphereA_p,
									  MaterialType /*materialA*/ ) const
{
	Sphere sphereA_w = CollisionUtils::transformToWorld(cellA,sphereA_p);

	static ColliderList results;

	results.clear();

	m_staticTree->findInRange(sphereA_w.getCenter(),sphereA_w.getRadius(),results);
	m_barrierTree->findInRange(sphereA_w.getCenter(),sphereA_w.getRadius(),results);

	for(uint i = 0; i < results.size(); i++)
	{
		CollisionProperty * collisionB = results[i];

		if(checkIgnoreObject(&collisionB->getOwner())) continue;

		if( !collisionB->blocksMovement() ) continue;

		// ----------

		CellProperty const * cellB = collisionB->getCell();

		Sphere sphereA_B = CollisionUtils::transformToCell(cellA,sphereA_p,cellB);

		DetectResult result = CollisionDetect::testSphereObject(sphereA_B,&collisionB->getOwner());

		if(result.collided)
		{
			return true;
		}
	}

	return false;
}
