// ======================================================================
//
// CollisionWorld.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CollisionWorld.h"

#include "sharedCollision/BarrierObject.h"
#include "sharedCollision/CollisionDetect.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionResolve.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Distance2d.h"
#include "sharedCollision/DoorObject.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorManager.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedCollision/FloorTri.h"
#include "sharedCollision/Footprint.h"
#include "sharedCollision/FootprintForceReattachManager.h"
#include "sharedCollision/Intersect1d.h"
#include "sharedCollision/SetupSharedCollision.h"
#include "sharedCollision/SimpleExtent.h"
#include "sharedCollision/SpatialDatabase.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedDebug/Report.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FloatingPointUnit.h"
#include "sharedLog/Log.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Circle.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/MultiShape.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedTerrain/TerrainObject.h"
#include <limits>

// ----------

namespace CollisionWorldNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef CollisionWorld::WarpWarningCallback  WarpWarningCallback;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  defaultNearWarpWarning(Object const &object, Vector const &oldPosition_w, Vector const &newPosition_w, int segmentCount);
	void  defaultFarWarpWarning(Object const &object, Vector const &oldPosition_w, Vector const &newPosition_w, int segmentCount);

	bool  testFloorCollision(FloorLocator const &startFloorLocator, Vector const &deltaPosition_w, Vector &collisionLocation_w, Object const *&collisionObject);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int const cs_nearWarpCollisionSegmentCount = 20;
	int const cs_farWarpCollisionSegmentCount = 250;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	SpatialDatabase *  ms_database = nullptr;

	bool               ms_updating = false;
	bool               ms_serverSide = false;

	bool ms_forceReattachContactsPostResolve = false;

	WarpWarningCallback  s_nearWarpWarningCallback;
	WarpWarningCallback  s_farWarpWarningCallback;

	char const * ms_sptatialDatabaseNames[] = {
													"None",
													"Static",
													"Dynamic",
													"Barriers",
													"Doors"
	};

	//----------------------------------------------------------------------

	bool testPassableTerrain(Vector const & startPos, Vector const & delta, float & collisionTime)
	{
		TerrainObject const *const terrainObject = TerrainObject::getConstInstance();
		if (nullptr != terrainObject)
		{
			float totalDistance = delta.magnitude();
			float distanceTraversed = 0.0f;
			Vector deltaNormal = delta;
			deltaNormal.normalize();

			for (; distanceTraversed < totalDistance; )
			{
				float distance = std::min(distanceTraversed + 2.0f, totalDistance);
				Vector const & endPos = startPos + (deltaNormal * distance);

				//-- is the target point in an impassable region?
				if (!terrainObject->isPassableForceChunkCreation(endPos))
				{
					collisionTime = distanceTraversed;
					return false;
				}

				distanceTraversed = distance;
			}
		}
		return true;
	}

	//----------------------------------------------------------------------

	void handleTerrainImpassability(CollisionProperty  & collider, bool const allowSlide)
	{
		PROFILER_AUTO_BLOCK_DEFINE("handleTerrainImpassability");

		Object & object = collider.getOwner();

		CellProperty const * const cellA = collider.getLastCell();
		CellProperty const * const cellB = object.getParentCell();

		if (cellA != CellProperty::getWorldCellProperty() || cellB != CellProperty::getWorldCellProperty())
			return;

		TerrainObject const *const terrainObject = TerrainObject::getConstInstance();
		if (nullptr != terrainObject && terrainObject->hasPassableAffectors())
		{
			// pointA_w is *not* the previous world position, but actually the world
			// position for the previous position in the cell, in respect to the
			// current position of the previous cell.  This has the effect that a
			// stationary object in a moving cell appears to not be moving as far
			// as collision is concerned.

			Vector const & pointA_w = collider.getLastPos_p();
			Vector const & pointB_w = object.getPosition_w();

			Vector const & locomotionTranslation_w = pointB_w - pointA_w;
			Vector targetPosition_w = pointB_w;
			//-- is the target point in an impassable region?
			if (!terrainObject->isPassableForceChunkCreation(targetPosition_w))
			{
				if (allowSlide)
				{
					//-- the passable boundaries are always axis-aligned, so try moving along one or the other world axes
					//-- try moving along the X axis
					Vector const locomotionTranslation_w_x(locomotionTranslation_w.x, locomotionTranslation_w.y, 0.0f);
					targetPosition_w = pointA_w + locomotionTranslation_w_x;
					if (!terrainObject->isPassableForceChunkCreation(targetPosition_w))
					{
						//-- try moving along the Z axis
						Vector const locomotionTranslation_w_z(0.0f, locomotionTranslation_w.y, locomotionTranslation_w.z);
						targetPosition_w = pointA_w + locomotionTranslation_w_z;
						if (!terrainObject->isPassableForceChunkCreation(targetPosition_w))
						{
							targetPosition_w = pointA_w;
						}
					}
				}
				else
				{
					targetPosition_w = pointA_w;
				}

				object.setPosition_w(targetPosition_w);
			}
		}
	}

	//----------------------------------------------------------------------

	CollisionWorld::CanTestCollisionDetectionOnObjectThisFrameFunction ms_canTestCollisionDetectionOnObjectThisFrameFunction = 0;
	CollisionWorld::NoCollisionDetectionThisFrameFunction ms_noCollisionDetectionThisFrameFunction = 0;
	CollisionWorld::CollisionDetectionOnHitFunction ms_collisionDetectionOnHitFunction = 0;
	CollisionWorld::DoCollisionWithTerrainFunction ms_doCollisionWithTerrainFunction = 0;
}

using namespace CollisionWorldNamespace;

// ----------
// Debugging / profiling data

float collisionDetectTime = 0.0f;
float floorUpdateTime = 0.0f;
float footprintUpdateTime = 0.0f;
float terrainHeightTime = 0.0f;
float getCellTime = 0.0f;
float resolveCollisionsTime = 0.0f;
float totalCollisionTime = 0.0f;
float postUpdateTime = 0.0f;
int canMoveCount = 0;
float canMoveTime = 0.0f;
float maxCanMoveTime = 0.0f;
float pathSearchTime = 0.0f;
float pathBuildTime = 0.0f;
float pathfindingTime = 0.0f;
int footprintUpdateCounter = 0;
int pathAllocations = 0;
float pathScrubTime = 0.0f;
int extentUpdateCount = 0;
int behaviorAlterCount = 0;
float behaviorTotalTime = 0.0f;
float behaviorMaxTime = 0.0f;
float ringTotalTime = 0.0f;
int aiControllerAlterCount = 0;
float aiControllerTotalTime = 0.0f;
float aiControllerMaxTime = 0.0f;

std::string ms_reportString;

// ----------

void terrainChangedCallback(Rectangle2d const & rect)
{
	AxialBox box(Vector(rect.x0, -2000.0f, rect.y0), Vector(rect.x1, 2000.0f, rect.y1));

	CollisionWorld::environmentChanged(MultiShape(box));
}

// ======================================================================
// namespace CollisionWorldNamespace
// ======================================================================

void CollisionWorldNamespace::defaultNearWarpWarning(Object const &object, Vector const &oldPosition_w, Vector const &newPosition_w, int segmentCount)
{
	DEBUG_WARNING(ConfigSharedCollision::getReportWarnings(),
		("CollisionWorld::update() had %d segments for object id=[%s], template=[%s], start position=[%.2f,%.2f,%.2f], end position=[%.2f,%.2f,%.2f], object probably should have warped but collision system is not warping it.",
			segmentCount,
			object.getNetworkId().getValueString().c_str(),
			object.getObjectTemplateName(),
			oldPosition_w.x,
			oldPosition_w.y,
			oldPosition_w.z,
			newPosition_w.x,
			newPosition_w.y,
			newPosition_w.z
			));
}

// ----------------------------------------------------------------------

void CollisionWorldNamespace::defaultFarWarpWarning(Object const &object, Vector const &oldPosition_w, Vector const &newPosition_w, int segmentCount)
{
	DEBUG_WARNING(ConfigSharedCollision::getReportWarnings(),
		("CollisionWorld::update() had %d segments for object id=[%s], template=[%s], start position=[%.2f,%.2f,%.2f], end position=[%.2f,%.2f,%.2f], collision system will consider this a warp and adjust accordingly.",
			segmentCount,
			object.getNetworkId().getValueString().c_str(),
			object.getObjectTemplateName(),
			oldPosition_w.x,
			oldPosition_w.y,
			oldPosition_w.z,
			newPosition_w.x,
			newPosition_w.y,
			newPosition_w.z
			));
}

// ----------------------------------------------------------------------
/**
 * Test if a footprint is capable of walking across the floor in a given direction.
 *
 * @param startFloorLocator    the floor locator for the footprint we want to test.
 * @param deltaPosition_w      the world space delta position vector indicating the direction in which
 *                             the footprint will be tested.
 * @param collisionLocation_w  if a collision is detected, this return value will be filled with the
 *                             world space location where the collision occurred.
 * @param collisionObject      if a collision is detected, the object associated with the floor will
 *                             be returned.
 *
 * @return  true if the footprint succeeded in walking across the floor when moving in the specified direction; false otherwise.
 */

bool CollisionWorldNamespace::testFloorCollision(FloorLocator const &startFloorLocator, Vector const &deltaPosition_w, Vector &collisionLocation_w, Object const *&collisionObject)
{
	// Get the floor from the floor locator.
	Floor const *const floor = startFloorLocator.getFloor();
	if (!floor)
	{
		// For some reason this solid floor does not have an attached floor.  No collision.
		WARNING(true, ("testFloorCollision(): attempted to test floor with a starting floor locator that had a nullptr floor, unexpected.  Check calling code's assumptions."));
		return true;
	}

	// Test if we can walk across the floor.
	FloorLocator  destinationLocator;

	// @todo verify that we want deltaPosition in world space here.  Look at other floor collision resolution
	// code.  The parameter name suggests this but the usage suggests it should be in some kind of "floor space".
	PathWalkResult const pathWalkResult = floor->canMove(startFloorLocator, deltaPosition_w, -1, -1, destinationLocator);

	if ((pathWalkResult == PWR_DoesntEnter)
		|| (pathWalkResult == PWR_WalkOk)
		|| (pathWalkResult == PWR_ExitedMesh))
	{
		// In these cases, indicate there is no collision with the floor.
		return false;
	}

	// A collision occurred with the floor.
	collisionLocation_w = destinationLocator.getPosition_w();

	Floor const *const destinationFloor = destinationLocator.getFloor();
	if (!destinationFloor)
	{
		// Hmm, we think we collided with a floor but the destination collision floor location does not have an associated floor.
		// This most likely means that the pathWalkResult that was issued indicates a non-collision and should be caught in the
		// if statement above.
		WARNING(true,
			("testFloorCollision(): floor collision occurred while checking for floor collisions for floor of object id [%s] but collision floor location has no associated floor.  Calling this a non-collision.  PathWalkResult was [%d].",
				floor->getOwner() ? floor->getOwner()->getNetworkId().getValueString().c_str() : "<nullptr floor owner>",
				static_cast<int>(pathWalkResult)
				));
		return false;
	}

	collisionObject = destinationFloor->getOwner();
	if (!collisionObject)
	{
		// We had a collision on a floor, but the floor had a nullptr owner.  Consider this a non-collision.
		DEBUG_WARNING(true, ("testFloorCollision(): floor collision occurred, destination collision floor location had a floor but floor reported a nullptr owner.  Calling this a non-collision."));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

void CollisionWorld::registerCanTestCollisionDetectionOnObjectThisFrame(CanTestCollisionDetectionOnObjectThisFrameFunction function)
{
	ms_canTestCollisionDetectionOnObjectThisFrameFunction = function;
}

// ----------------------------------------------------------------------

bool CollisionWorld::canTestCollisionDetectionOnObjectThisFrame(Object * const object)
{
	if (ms_canTestCollisionDetectionOnObjectThisFrameFunction != 0)
	{
		return (*ms_canTestCollisionDetectionOnObjectThisFrameFunction)(object);
	}
	return true;
}

// ----------------------------------------------------------------------

void CollisionWorld::registerNoCollisionDetectionThisFrame(NoCollisionDetectionThisFrameFunction function)
{
	ms_noCollisionDetectionThisFrameFunction = function;
}

// ----------------------------------------------------------------------

void CollisionWorld::noCollisionDetectionThisFrame(Object * const object)
{
	if (ms_noCollisionDetectionThisFrameFunction != 0)
	{
		(*ms_noCollisionDetectionThisFrameFunction)(object);
	}
}

// ----------------------------------------------------------------------

void CollisionWorld::registerDoCollisionDetectionOnHit(CollisionDetectionOnHitFunction function)
{
	ms_collisionDetectionOnHitFunction = function;
}

// ----------------------------------------------------------------------

bool CollisionWorld::doCollisionDetectionOnHit(Object * const object, Object * const wasHitByThisObject)
{
	if (ms_collisionDetectionOnHitFunction != 0)
	{
		return(*ms_collisionDetectionOnHitFunction)(object, wasHitByThisObject);
	}

	return false;
}

// ----------------------------------------------------------------------

void CollisionWorld::registerDoCollisionWithTerrain(DoCollisionWithTerrainFunction function)
{
	ms_doCollisionWithTerrainFunction = function;
}

// ----------------------------------------------------------------------

bool CollisionWorld::doCollisionWithTerrain(Object * const object)
{
	if (ms_doCollisionWithTerrainFunction != 0)
	{
		return(*ms_doCollisionWithTerrainFunction)(object);
	}

	return false;
}

// ======================================================================
// class CollisionWorld: PUBLIC STATIC
// ======================================================================

void CollisionWorld::install(bool serverSide)
{
	DEBUG_REPORT_LOG(true, ("CollisionWorld::install()\n"));

	s_nearWarpWarningCallback = defaultNearWarpWarning;
	s_farWarpWarningCallback = defaultFarWarpWarning;

	ConfigSharedCollision::install();
	FloorMesh::install();
	CollisionResolve::install();
	PerformanceTimer::install();

	ms_serverSide = serverSide;

	if (!serverSide)
	{
		TerrainObject::addTerrainChangedFunction(terrainChangedCallback);
	}

	ms_database = new SpatialDatabase();

	ExitChain::add(CollisionWorld::remove, "CollisionWorld::remove()");
}

// ----------

void CollisionWorld::remove(void)
{
	DEBUG_REPORT_LOG(true, ("CollisionWorld::remove()\n"));

	CollisionResolve::remove();
	FloorMesh::remove();

	s_nearWarpWarningCallback = nullptr;
	s_farWarpWarningCallback = nullptr;

	delete ms_database;
	ms_database = nullptr;
}

// ----------------------------------------------------------------------

void CollisionWorld::update(float time)
{
	CollisionNotification::purgeQueue();

	// ----------

	ms_updating = true;

	floorUpdateTime = 0.0f;
	footprintUpdateTime = 0.0f;
	collisionDetectTime = 0.0f;
	resolveCollisionsTime = 0.0f;
	postUpdateTime = 0.0f;
	terrainHeightTime = 0.0f;

	PerformanceTimer timer;
	timer.start();

#ifdef _MSC_VER

	FloatingPointUnit::Precision oldPrecision = FloatingPointUnit::getPrecision();
	FloatingPointUnit::setPrecision(FloatingPointUnit::P_64);

#endif

	// ----------

	CollisionResolve::resetCounters();

	typedef std::vector<CollisionProperty*>  CollisionPropertyVector;
	static CollisionPropertyVector           active;

	active.clear();

	CollisionProperty * cursor = CollisionProperty::getActiveHead();

	while (cursor)
	{
		active.push_back(cursor);

		cursor = cursor->getNext();
	}

	CollisionPropertyVector::size_type const count = active.size();

	if (count > 3000)
	{
		WARNING(true, ("CollisionWorld::update - Updating more than 3000 objects this frame - something's probably wrong\n"));
	}

	for (CollisionPropertyVector::size_type i = 0; i < count; i++)
	{
		update(active[i], time);
	}

	footprintUpdateCounter = static_cast<int>(count);

	// ----------

#ifdef _MSC_VER

	FloatingPointUnit::setPrecision(oldPrecision);

#endif

	timer.stop();
	totalCollisionTime = timer.getElapsedTime();

	// ----------
	// Post-update, where test code goes

	updateReportString();

	if (totalCollisionTime >= 0.1f && ConfigSharedCollision::getLogLongFrames())
	{
		// Collision took more than 100 milliseconds - log the report

		LOG("Collision", (ms_reportString.c_str()));
	}

	ms_updating = false;
}

// ----------------------------------------------------------------------

bool CollisionWorld::spatialSweepAndResolve(CollisionProperty * collider)
{
	Object * object = &collider->getOwner();
	NOT_NULL(object);

	bool hadACollisionThisFrame = false;

	if (canTestCollisionDetectionOnObjectThisFrame(object))
	{
		CellProperty const * const cell_w = CellProperty::getWorldCellProperty();
		Capsule const capsule_w(collider->getQueryCapsule_w());
		ColliderList collidedWith;

		int const queryMask = ConfigSharedCollision::getSpatialSweepAndResolveDefaultMask();

		ms_database->queryFor(queryMask, cell_w, true, capsule_w, collidedWith);

		int const potentialCount = static_cast<int>(collidedWith.size());

		for (int i = 0; i < potentialCount; ++i)
		{
			CollisionProperty * const toTestCollision = collidedWith.at(static_cast<unsigned int>(i));

			if ((toTestCollision) && (toTestCollision != collider))
			{
				CollisionProperty * const wasHitByThisCollision = collidedWith.at(i);
				Object * const wasHitByThisObject = &wasHitByThisCollision->getOwner();

				if (doCollisionDetectionOnHit(object, wasHitByThisObject))
				{
					hadACollisionThisFrame = true;
				}
			}
		}

		if (doCollisionWithTerrain(object))
		{
			hadACollisionThisFrame = true;
		}

		if (!hadACollisionThisFrame)
		{
			noCollisionDetectionThisFrame(object);
		}
	}

	return(hadACollisionThisFrame);
}

// ----------------------------------------------------------------------

void CollisionWorld::update(CollisionProperty * collider, float time)
{
	FATAL(!collider, ("CollisionWorld::update(): collider is nullptr."));

	PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::update");

	Object * object = &collider->getOwner();

	Footprint * foot = collider->getFootprint();

	if (foot == nullptr)
	{
		PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::update: foot == nullptr");

		IGNORE_RETURN(CollisionWorld::spatialSweepAndResolve(collider));
		collider->storePosition();
		collider->updateIdle();

		return;
	}

	// ----------

	CellProperty * cellA = collider->getLastCell();
	CellProperty * cellB = object->getParentCell();

	// pointA_w is *not* the previous world position, but actually the world
	// position for the previous position in the cell, in respect to the
	// current position of the previous cell.  This has the effect that a
	// stationary object in a moving cell appears to not be moving as far
	// as collision is concerned.

	Vector pointA_w = CollisionUtils::transformToWorld(cellA, collider->getLastPos_p());
	Vector pointB_w = object->getPosition_w();

	Vector delta = pointB_w - pointA_w;

	delta.y = 0.0f;

	float moveLength = delta.magnitude();

	// 2 meters per move segment

	int segments = static_cast<int>(ceil(moveLength / 2.0f));

	if (segments <= 1)
	{
		PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::update: segments <= 1");

		updateSegment(collider, time);
	}
	else
	{
		PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::update: segments > 1");

		// Break the move into pieces and resolve each separately

		if (segments > cs_nearWarpCollisionSegmentCount)
		{
			PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::update: segments > cs_nearWarpCollisionSegmentCount");

			// we can't actually warp the player, because that creats an exploit where they can cause themselves to get warped into bad places
			if (s_nearWarpWarningCallback)
				(*s_nearWarpWarningCallback)(collider->getOwner(), pointA_w, pointB_w, segments);
		}

		if (segments > cs_farWarpCollisionSegmentCount)
		{
			PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::update: segments > cs_farWarpCollisionSegmentCount");

			if (s_farWarpWarningCallback)
				(*s_farWarpWarningCallback)(collider->getOwner(), pointA_w, pointB_w, segments);

			CollisionWorld::objectWarped(&collider->getOwner());

			segments = 1;
		}

		// Move the player back to where they were at the start of the last frame,
		// in the context of parent transforms this frame, so we can start resolving collisions

		// This version just moves them, & hopefully they'll end up in the same cell as they were last frame

		Vector pointA_B = CollisionUtils::transformFromWorld(pointA_w, cellB);

		NAN_CHECK(pointA_B);

		object->setPosition_p(pointA_B);

		if (object->getParentCell() != cellA)
		{
			REPORT_LOG(ConfigSharedCollision::getReportMessages(), ("CollisionWorld::update - Tried to move the object back to where it was last frame, but it ended up in a different cell\n"));
		}

		// ----------

		float segmentTime = time / static_cast<float>(segments);
		Vector segmentDelta_w = (pointB_w - pointA_w) / static_cast<float>(segments);

		for (int i = 0; i < segments; i++)
		{
			Vector segmentDelta_p = CollisionUtils::rotateToCell(CellProperty::getWorldCellProperty(), segmentDelta_w, object->getParentCell());

			Vector goal = object->getPosition_p() + segmentDelta_p;

			NAN_CHECK(goal);

			object->setPosition_p(goal);

			updateSegment(collider, segmentTime);
		}
	}

	collider->updateIdle();

	CellProperty const * finalCell = object->getParentCell();

	if (finalCell && (finalCell != CellProperty::getWorldCellProperty()))
	{
		if (!foot->isAttached())
		{
			PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::update: !foot->isAttached()");

			IGNORE_RETURN(foot->snapToCellFloor());
		}
	}
}

// ----------------------------------------------------------------------

void CollisionWorld::updateSegment(CollisionProperty * collider, float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::updateSegment");

	Object * object = &collider->getOwner();

	Footprint * foot = collider->getFootprint();

	// This is done in the parent space of the destination point, because the cell we were in
	// may have moved, and we need to do collision relative to the potentially moving cells.
	Vector pointA_p = CollisionUtils::transformToCell(collider->getLastCell(), collider->getLastPos_p(), object->getParentCell());
	Vector pointB_p = object->getPosition_p();

	Vector delta = pointB_p - pointA_p;

	if ((delta.x != 0.0f) || (delta.y != 0.0f))
	{
		Vector footPos_p = foot->getPosition_p();
		Vector groundNormal_p = foot->getGroundNormal_p();

		if (groundNormal_p.y != 0.0f)
		{
			float dx = pointB_p.x - footPos_p.x;
			float dz = pointB_p.z - footPos_p.z;

			float dydx = -(groundNormal_p.x / groundNormal_p.y);
			float dydz = -(groundNormal_p.z / groundNormal_p.y);

			float groundOffset = (dx * dydx) + (dz * dydz);

			pointB_p.y += groundOffset;

			NAN_CHECK(pointB_p);

			object->setPosition_p(pointB_p);
		}
	}

	// ----------
	// do the actual collision resolution step

	IGNORE_RETURN(updateStep(collider, time));

	// and now repeatedly snap the object and re-run collision resolution until
	// there's no collision or we give up

	int j;

	for (j = 0; j < 10; j++)
	{
		IGNORE_RETURN(foot->snapObjectToGround());

		ResolutionResult result = updateStep(collider, 0.0f);

		if (result == RR_NoCollision)
		{
			break;
		}
	}
}

// ----------------------------------------------------------------------
// push the object away from anything it's colliding with

void CollisionWorld::shoveAway(CollisionProperty * const collisionProperty, ColliderList const & colliderList, float time)
{
	Object * obj = &collisionProperty->getOwner();

	Vector accum;

	Vector objectPos_p = obj->getPosition_p();

	ColliderList::const_iterator ii = colliderList.begin();
	ColliderList::const_iterator iiEnd = colliderList.end();

	for (; ii != iiEnd; ++ii)
	{
		CollisionProperty * const collisionPropertyToTest = *ii;
		Vector obstaclePos_p = collisionPropertyToTest->getOwner().getPosition_p();

		Vector delta = objectPos_p - obstaclePos_p;

		delta.y = 0.0f;

		IGNORE_RETURN(delta.normalize());

		accum += delta;
	}

	IGNORE_RETURN(accum.normalize());

	accum *= 8.0f * time;

	Vector newPos = objectPos_p + accum;

	NAN_CHECK(newPos);

	obj->setPosition_p(newPos);
}

// ----------------------------------------------------------------------
// returns true if any collisions were resolved

ResolutionResult CollisionWorld::updateStep(CollisionProperty * collider, float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::updateStep");

	NOT_NULL(collider);

	Footprint * foot = collider->getFootprint();

	NOT_NULL(foot);

	if (foot->isFloating())
	{
		Vector newPos = foot->m_addToWorldPos;

		NAN_CHECK(newPos);

		collider->getOwner().setPosition_p(newPos);

		foot->updateFloating();

		return RR_NoCollision;
	}

	//----------------------------------------------------------------------

	handleTerrainImpassability(*collider, true);

	// ----------

	ResolutionResult result;

	PerformanceTimer timer;

	static ColliderList colliderList;

	{
		PROFILER_AUTO_BLOCK_DEFINE("update floor database");
		timer.start();

		ms_database->updateFloorCollision(collider, false);

		timer.stop();
		floorUpdateTime += timer.getElapsedTime();
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("update collisions");
		timer.start();

		ms_database->updateStaticCollision(collider, colliderList);

		timer.stop();
		collisionDetectTime += timer.getElapsedTime();
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("update footprints");
		timer.start();

		foot->updatePreResolve(time);

		timer.stop();
		footprintUpdateTime += timer.getElapsedTime();
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("resolve collisions");
		timer.start();

		result = CollisionResolve::resolveCollisions(collider, colliderList);

		timer.stop();
		resolveCollisionsTime += timer.getElapsedTime();
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("post update");
		timer.start();

		foot->updatePostResolve(time);

		if (ms_forceReattachContactsPostResolve)
			foot->forceReattachContacts();

		timer.stop();
		postUpdateTime += timer.getElapsedTime();
	}

	handleTerrainImpassability(*collider, false);

	collider->storePosition();
	colliderList.clear();

	return result;
}

// ----------------------------------------------------------------------

bool CollisionWorld::isUpdating(void)
{
	return ms_updating;
}

// ----------------------------------------------------------------------

void CollisionWorld::updateReportString(void)
{
	ms_reportString.clear();

	char buffer[256];

	sprintf(buffer, "CollsionWorld::update -\n");
	ms_reportString += buffer;

	const int staticObjectCount = ms_database->getObjectCount(SpatialDatabase::Q_Static);
	const int dynamicObjectCount = ms_database->getObjectCount(SpatialDatabase::Q_Dynamic);
	const int floorCount = ms_database->getFloorCount();

	sprintf(buffer, "Database : %d S / %d D / %d F\n", staticObjectCount, dynamicObjectCount, floorCount);

	ms_reportString += buffer;

	int colliderCount = CollisionResolve::getColliderCount();
	int obstacleCount = CollisionResolve::getObstacleCount();
	int collisionCount = CollisionResolve::getCollisionCount();
	int bounceCount = CollisionResolve::getBounceCount();

	sprintf(buffer, "Resolution : %d colliders, %d obstacles, %d collisions, %d bounces\n", colliderCount, obstacleCount, collisionCount, bounceCount);
	ms_reportString += buffer;

	ms_reportString += "C     E     F     P     L     T     O\n";

	ms_reportString += buffer;

	float behaviorAverage = behaviorTotalTime / static_cast<float>(behaviorAlterCount);

	sprintf(buffer, "Behavior             : %d, %1.3f total, %1.3f avg, %1.3f max\n", behaviorAlterCount, behaviorTotalTime * 1000.0f, behaviorAverage * 1000.0f, behaviorMaxTime * 1000.0f);
	ms_reportString += buffer;

	float aiControllerAverage = aiControllerTotalTime / static_cast<float>(aiControllerAlterCount);

	sprintf(buffer, "AiController         : %d, %1.3f total, %1.3f avg, %1.3f max\n", aiControllerAlterCount, aiControllerTotalTime * 1000.0f, aiControllerAverage * 1000.0f, aiControllerMaxTime * 1000.0f);
	ms_reportString += buffer;

	float canMoveAverage = canMoveTime / static_cast<float>(canMoveCount);

	sprintf(buffer, "Can move             : %d, %1.3f total, %1.3f avg, %1.3f max\n", canMoveCount, canMoveTime * 1000.0f, canMoveAverage * 1000.0f, maxCanMoveTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Extent updates       : %d\n", extentUpdateCount);
	ms_reportString += buffer;

	sprintf(buffer, "Path scrub time      : %1.3f msec\n", pathScrubTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Pathfinding time     : %1.3f msec\n", pathfindingTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Path build time      : %1.3f msec\n", pathBuildTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Path search time     : %1.3f msec\n", pathSearchTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Path search alloc    : %d allocs\n", pathAllocations);
	ms_reportString += buffer;

	sprintf(buffer, "Footprint updates    : %d updates\n", footprintUpdateCounter);
	ms_reportString += buffer;

	sprintf(buffer, "All collision        : %1.3f msec\n", totalCollisionTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Floor update         : %1.3f msec\n", floorUpdateTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Footprint update     : %1.3f msec\n", footprintUpdateTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Collision detection  : %1.3f msec\n", collisionDetectTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Collision resolution : %1.3f msec\n", resolveCollisionsTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Terrain height       : %1.3f msec\n", terrainHeightTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Get cell             : %1.3f msec\n", getCellTime * 1000.0f);
	ms_reportString += buffer;

	sprintf(buffer, "Post-update          : %1.3f msec\n", postUpdateTime * 1000.0f);
	ms_reportString += buffer;

	extentUpdateCount = 0;

	behaviorAlterCount = 0;
	behaviorTotalTime = 0.0f;
	behaviorMaxTime = 0.0f;

	ringTotalTime = 0.0f;

	canMoveCount = 0;
	canMoveTime = 0.0f;
	maxCanMoveTime = 0.0f;

	aiControllerAlterCount = 0;
	aiControllerTotalTime = 0.0f;
	aiControllerMaxTime = 0.0f;
}

// ----------------------------------------------------------------------

void CollisionWorld::reportCallback(void)
{
#if _DEBUG

	if (!ConfigSharedCollision::getReportStatus()) return;

	DEBUG_REPORT_PRINT(true, (ms_reportString.c_str()));

#endif
}

// ----------------------------------------------------------------------

void CollisionWorld::setNearWarpWarningCallback(WarpWarningCallback callback)
{
	if (callback)
		s_nearWarpWarningCallback = callback;
	else
		s_nearWarpWarningCallback = defaultNearWarpWarning;
}

// ----------------------------------------------------------------------

void CollisionWorld::setFarWarpWarningCallback(WarpWarningCallback callback)
{
	if (callback)
		s_farWarpWarningCallback = callback;
	else
		s_farWarpWarningCallback = defaultFarWarpWarning;
}

// ----------------------------------------------------------------------

void CollisionWorld::addObject(Object * object)
{
	if (object == nullptr) return;

	CollisionProperty * collision = object->getCollisionProperty();

	if ((collision == nullptr) || collision->getDisableCollisionWorldAddRemove()) return;

	if (collision->isInCollisionWorld()) return;

	// ----------

	collision->addToCollisionWorld();

	SpatialDatabase::Query query = collision->getSpatialDatabaseStorageType();
	bool added = false;

	if (collision->isMobile())
	{
		added = ms_database->addObject(query, object);
	}
	else
	{
		Floor * pFloor = collision->getFloor();

		if (pFloor && !pFloor->isCellFloor())
		{
			added = ms_database->addFloor(pFloor);
		}

		if (collision->getExtent_l())
		{
			if (dynamic_cast<DoorObject const * const>(object) != 0)
			{
				query = SpatialDatabase::Q_Doors;
			}
			else if (dynamic_cast<BarrierObject const * const>(object) != 0)
			{
				query = SpatialDatabase::Q_Barriers;
			}

			added = ms_database->addObject(query, object) || added;
		}

		environmentChanged(MultiShape(collision->getBoundingSphere_w()));
	}

	if (added)
	{
		char const * name = object->getObjectTemplateName();

		if (name == nullptr)
		{
			Appearance const * appearance = object->getAppearance();

			if (appearance)
			{
				name = appearance->getAppearanceTemplateName();
			}
		}

		int index = 0;
		switch (query)
		{
		case SpatialDatabase::Q_Static:
			index = 1;
			break;
		case SpatialDatabase::Q_Dynamic:
			index = 2;
			break;
		case SpatialDatabase::Q_Barriers:
			index = 3;
			break;
		case SpatialDatabase::Q_Doors:
			index = 4;
			break;
		default:
			break;
		}

		DEBUG_REPORT_LOG(ConfigSharedCollision::getReportChanges(), ("Added object %s to collision world [%s] at (%f,%f,%f)\n", name, ms_sptatialDatabaseNames[index], object->getPosition_w().x, object->getPosition_w().y, object->getPosition_w().z));
	}
}

// ----------

void CollisionWorld::removeObject(Object * object)
{
	if (object == nullptr) return;

	CollisionProperty * collision = object->getCollisionProperty();

	if ((collision == nullptr) || collision->getDisableCollisionWorldAddRemove()) return;

	if (!collision->isInCollisionWorld()) return;

	// ----------

	bool removed = false;

	if (collision->isMobile())
	{
		removed = ms_database->removeObject(SpatialDatabase::Q_Dynamic, object);
	}
	else
	{
		Floor * pFloor = collision->getFloor();

		if (pFloor && !pFloor->isCellFloor())
		{
			removed = ms_database->removeFloor(pFloor);
		}

		if (collision->getExtent_l())
		{
			SpatialDatabase::Query query = SpatialDatabase::Q_Static;

			if (dynamic_cast<DoorObject const * const>(object) != 0)
			{
				query = SpatialDatabase::Q_Doors;
			}
			else if (dynamic_cast<BarrierObject const * const>(object) != 0)
			{
				query = SpatialDatabase::Q_Barriers;
			}

			removed = ms_database->removeObject(query, object) || removed;
		}

		environmentChanged(MultiShape(collision->getBoundingSphere_w()));
	}

	if (removed)
	{
		DEBUG_REPORT_LOG(ConfigSharedCollision::getReportChanges(), ("Removed object %s from collision world\n", object->getObjectTemplateName()));
	}

	collision->removeFromCollisionWorld();
}

// ----------

void CollisionWorld::moveObject(Object * object)
{
	if (object == nullptr) return;

	CollisionProperty * collision = object->getCollisionProperty();

	if (collision == nullptr) return;

	if (!collision->isInCollisionWorld()) return;

	// ----------

	if (object->getTransform_o2w() == collision->getLastTransform_w())
	{
		return;
	}

	collision->setIdle(false);
	collision->setExtentsDirty(true);

	IGNORE_RETURN(ms_database->moveObject(collision));

	if (!collision->isMobile())
	{
		environmentChanged(MultiShape(collision->getBoundingSphere_w()));
	}
}

// ----------

void CollisionWorld::cellChanged(Object * object)
{
	if (object == nullptr) return;

	CollisionProperty * collision = object->getCollisionProperty();

	if (collision == nullptr) return;

	if (!collision->isInCollisionWorld()) return;

	// ----------

	collision->setIdle(false);
	collision->cellChanged();
}

// ----------

void CollisionWorld::appearanceChanged(Object * object)
{
	if (object == nullptr) return;

	CollisionProperty * collision = object->getCollisionProperty();

	if (collision == nullptr) return;

	if (!collision->isInCollisionWorld()) return;

	// ----------

	if (!collision->isMobile())
	{
		IGNORE_RETURN(ms_database->moveObject(collision));

		collision->initFloor();
	}
}

// ----------------------------------------------------------------------
// Something about the environment (terrain, static objects) has changed.
// Wake up all collision properties in the changed area.

void CollisionWorld::environmentChanged(MultiShape const & shape)
{
	static ObjectVec mobs;

	mobs.clear();

	IGNORE_RETURN(ms_database->queryDynamics(shape, &mobs));

	ObjectVec::size_type const count = mobs.size();

	if (count > 500)
	{
		Vector c = shape.getCenter();
		float x = shape.getExtentX() * 2.0f;
		float y = shape.getExtentY() * 2.0f;
		float z = shape.getExtentZ() * 2.0f;

		WARNING(true, ("CollisionWorld::environmentChanged - Changing environment wakes up over 500 AIs. This is probably bad. (%f,%f,%f), (%f,%f,%f)\n", c.x, c.y, c.z, x, y, z));
	}

	for (ObjectVec::size_type i = 0; i < count; i++)
	{
		Object * mob = mobs[i];

		NOT_NULL(mob);

		CollisionProperty * mobCollision = mob->getCollisionProperty();

		NOT_NULL(mobCollision);

		mobCollision->setIdle(false);
	}
}

// ----------------------------------------------------------------------

SpatialDatabase * CollisionWorld::getDatabase(void)
{
	return ms_database;
}

// ----------------------------------------------------------------------

void CollisionWorld::objectWarped(Object * object)
{
	if (object == nullptr) return;

	CollisionProperty * collision = object->getCollisionProperty();

	// object does not have a CollisionProperty
	if (collision == nullptr) return;

	// object is not in CollisionWorld
	if (!collision->isInCollisionWorld()) return;

	collision->objectWarped();
}

// ----------------------------------------------------------------------
// useTerrainLos indicates whether to check for terrain LOS
//
// generateTerrainLos indicates whether to generate missing terrain for
// terrain LOS (only valid if useTerrainLos is true)
//
// terrainLosMinDistance and terrainLosMaxDistance specifies the required
// range of the 2 objects before terrain LOS is done (only valid
// if useTerrainLos is true)

QueryInteractionResult CollisionWorld::queryInteraction(CellProperty const * cellA, Vector const & pointA,
	CellProperty const * cellB, Vector const & pointB,
	Object const * ignoreObject,
	bool const useTerrainLos,
	bool const generateTerrainLos,
	float const terrainLosMinDistance,
	float const terrainLosMaxDistance,
	float & outHitTime, Object const * & outHitObject)
{
	PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::queryInteraction");
	Vector localB = CollisionUtils::transformToCell(cellB, pointB, cellA);

	Segment3d inSeg(pointA, localB);

	// ----------
	// Test portals first. If the query line hits a portal, we'll ignore any collision after that hit.

	float hitPortalTime = REAL_MAX;

	CellProperty const * nextCell = cellA->getDestinationCell(pointA, localB, hitPortalTime);

	// ----------
	// Test all extents first

	BaseExtent const * cellExtent = cellA->getCollisionExtent();

	if (cellExtent)
	{
		Range hitRange = cellExtent->rangedIntersect(inSeg);

		if (!hitRange.isEmpty() && (hitRange.getMin() < hitPortalTime))
		{
			outHitTime = hitRange.getMin();
			outHitObject = &cellA->getOwner();

			return QIR_HitCellExtent;
		}
	}

	// ----------
	// Test the extents of all objects in the cell

	Object const * hitObject;
	float hitTime = 0.0f;

	if (ms_database->queryInteraction(cellA, inSeg, ignoreObject, hitObject, hitTime))
	{
		if (hitTime < hitPortalTime)
		{
			outHitTime = hitTime;
			outHitObject = hitObject;

			return QIR_HitObjectExtent;
		}
	}

	// ----------
	// If we're in the world cell, test terrain.

	if (useTerrainLos)
	{
		if ((inSeg.getLength() >= terrainLosMinDistance) && (inSeg.getLength() <= terrainLosMaxDistance))
		{
			if (cellA == CellProperty::getWorldCellProperty())
			{
				PROFILER_AUTO_BLOCK_DEFINE("CollisionWorld::queryInteraction::Terrain");

				TerrainObject * terrain = TerrainObject::getInstance();

				CollisionInfo info;

				bool terrainCollide = false;

				if (terrain)
				{
					if (generateTerrainLos)
					{
						// do terrain LOS check, generating any missing terrain between the 2 points
						terrainCollide = terrain->collideForceChunkCreation(pointA, localB, info);
					}
					else
					{
						// do terrain LOS check, using only the generated terrain between the 2 points
						terrainCollide = terrain->collide(pointA, localB, info);
					}
				}

				if (terrainCollide)
				{
					// info doesn't have the correct hit time, so compute it using the hit point

					Vector hitPoint = info.getPoint();

					Vector hitDelta = hitPoint - pointA;

					Vector delta = localB - pointA;

					float hitTime = Collision3d::ComponentAlong(hitDelta, delta);

					if (hitTime < hitPortalTime)
					{
						outHitTime = hitTime;
						outHitObject = nullptr;

						return QIR_HitTerrain;
					}
				}
			}
		}
	}

	// ----------
	// If we did not hit a portal, the query ends in this cell.
	// If this cell is not the goal cell, then the query fails.

	if (nextCell == nullptr)
	{
		if (cellA != cellB)
		{
			outHitTime = 1.0f;
			outHitObject = nullptr;

			return QIR_MissedTarget;
		}
		else
		{
			return QIR_None;
		}
	}

	// ----------
	// If we hit a portal, recurse through it

	else
	{
		Vector clipPoint = pointA + (localB - pointA) * hitPortalTime;

		// Don't bother recursing if the segment we'd test in the next cell is tiny

		if (clipPoint.magnitudeBetween(localB) <= 0.01f)
		{
			return QIR_None;
		}

		Vector nextPoint = CollisionUtils::transformToCell(cellA, clipPoint, nextCell);

		// ----------
		// infinite recursion sentinel

		static int recursed = 0;

		if (recursed >= 10)
		{
			return QIR_None;
		}

		// ----------

		recursed++;

		QueryInteractionResult recurseResult = queryInteraction(nextCell,
			nextPoint,
			cellB,
			pointB,
			ignoreObject,
			useTerrainLos,
			generateTerrainLos,
			terrainLosMinDistance,
			terrainLosMaxDistance,
			outHitTime,
			outHitObject);

		recursed--;

		float lastLength = pointA.magnitudeBetween(localB);
		float nextLength = clipPoint.magnitudeBetween(localB);

		float timeScale = nextLength / lastLength;

		outHitTime = hitPortalTime + outHitTime * timeScale;

		return recurseResult;
	}
}

// ----------------------------------------------------------------------

bool CollisionWorld::query(Line3d const & line, ObjectVec * outList)
{
	return ms_database->queryStatics(line, outList);
}

// ----------

bool CollisionWorld::query(Sphere const & sphere, ObjectVec * outList)
{
	return ms_database->queryStatics(sphere, outList);
}

// ----------------------------------------------------------------------
// Given startLoc and goalLoc in the same cell, determine if startLoc
// can reach goalLoc

CanMoveResult CollisionWorld::canMoveInCell(Object const * object,
	CellProperty const * startCell,
	FloorLocator const & startLoc,
	Vector const & goalPos,
	bool checkY,
	bool checkFlora,
	bool checkFauna,
	FloorLocator & outLoc,
	int & outHitPortalId)
{
	FloorLocator goalLoc;

	if (checkY)
	{
		makeLocator(startCell, goalPos, goalLoc);
	}
	else
	{
		goalLoc.setFloor(startCell->getFloor());
		goalLoc.setPosition_p(goalPos);
	}

	return canMoveInCell(object, startCell, startLoc, goalLoc, checkY, checkFlora, checkFauna, outLoc, outHitPortalId);
}

CanMoveResult CollisionWorld::canMoveInCell(Object const * object,
	CellProperty const * startCell,
	FloorLocator const & startLoc,
	FloorLocator const & goalLoc,
	bool checkY,
	bool checkFlora,
	bool checkFauna,
	FloorLocator & outLoc,
	int & outHitPortalId)
{
	// ----------
	// Test collision extents

	Vector startPos = startLoc.getPosition_p();
	Vector goalPos = goalLoc.getPosition_p();

	Segment3d querySeg(startPos, goalPos);

	if (startCell == CellProperty::getWorldCellProperty())
	{
		float radius = startLoc.getRadius();

		Sphere sphere(startPos + Vector(0.0f, radius, 0.0f), radius);
		Vector delta = goalPos - startPos;
		float collisionTime = 0.0f;

		if (!testPassableTerrain(sphere.getCenter(), delta, collisionTime))
		{
			//-- hit impassable terrain
			return CMR_HitObstacle;
		}

		if (ms_database && ms_database->queryMaterial(startCell, sphere, delta, MT_Solid, checkFlora, checkFauna, object, collisionTime))
		{
			float epsilon = 0.05f / delta.magnitude();

			if (collisionTime > -epsilon)
			{
				return CMR_HitObstacle;
			}
		}
	}

	// ----------
	// The move didn't hit an edge on the current floor or any collision extents.

	// Test for collisions on other floors. These will be expensive, since we have to search
	// for a start point on each floor (which is slow).

	// ----------
	// Test object floors

	bool hasOutLoc = false;

	if (startCell == CellProperty::getWorldCellProperty())
	{
		if (ms_database)
		{
			FloorVec floors;

			IGNORE_RETURN(ms_database->queryFloors(startCell, querySeg, &floors));

			for (uint i = 0; i < floors.size(); i++)
			{
				Floor const * floor = floors[i];

				CanMoveResult result = canMoveOnFloor(object, startLoc, floor, goalLoc, checkY, checkFlora, checkFauna, outLoc, outHitPortalId);

				if (result == CMR_MoveOK)
				{
					if (outHitPortalId != -1)
					{
						hasOutLoc = true;
					}
				}
				else
				{
					return result;
				}
			}
		}
	}

	// Last but not least, test the cell floor (if there is one)
	// Test cell floor

	Floor const * cellFloor = startCell->getFloor();

	if (cellFloor)
	{
		return canMoveOnFloor(object, startLoc, cellFloor, goalLoc, checkY, checkFlora, checkFauna, outLoc, outHitPortalId);
	}
	else
	{
		if (!hasOutLoc)	outLoc = goalLoc;

		return CMR_MoveOK;
	}
}

// ----------------------------------------------------------------------

CanMoveResult CollisionWorld::canMove(CellProperty const * startCell,
	Vector const & startPos,
	Vector const & goalPos,
	float moveRadius,
	bool checkY,
	bool checkFlora,
	bool checkFauna)
{
	canMoveCount++;

	FloorLocator startLoc(startPos, moveRadius);

	FloorLocator dummy;

	return canMove(nullptr, startCell, startLoc, goalPos, checkY, checkFlora, checkFauna, dummy);
}

// ----------

CanMoveResult CollisionWorld::canMove(Object const * object,
	Vector const & goalPos,
	float moveRadius,
	bool checkY,
	bool checkFlora,
	bool checkFauna)
{
	if (object == nullptr)
	{
		return CMR_Error;
	}
	else
	{
		CollisionProperty const * collision = object->getCollisionProperty();

		if (collision)
		{
			Footprint const * foot = collision->getFootprint();

			if (foot && foot->isObjectInSync())
			{
				FloorLocator const * footLoc = foot->getAnyContact();

				if (footLoc && footLoc->isAttached())
				{
					FloorLocator objectLoc = *footLoc;

					objectLoc.setPosition_p(object->getPosition_p());

					objectLoc.setRadius(moveRadius);

					CellProperty const * startCell = object->getParentCell();

					FloorLocator dummy;

					return canMove(object, startCell, objectLoc, goalPos, checkY, checkFlora, checkFauna, dummy);
				}
			}
		}
	}

	FloorLocator startLoc(object->getPosition_p(), moveRadius);

	FloorLocator dummy;

	return canMove(object, object->getParentCell(), startLoc, goalPos, checkY, checkFlora, checkFauna, dummy);
}

// ----------------------------------------------------------------------

CanMoveResult CollisionWorld::canMove(Object const * object,
	CellProperty const * startCell,
	FloorLocator const & startLoc,
	Vector const & goalPos,
	bool checkY,
	bool checkFlora,
	bool checkFauna,
	FloorLocator & endLoc)
{
	if (startCell == nullptr)
	{
		startCell = CellProperty::getWorldCellProperty();
	}

	// ----------

	int hitPortalId = -1;

	CanMoveResult cellResult = canMoveInCell(object, startCell, startLoc, goalPos, checkY, checkFlora, checkFauna, endLoc, hitPortalId);

	if (cellResult != CMR_MoveOK)
	{
		// We hit something in this cell

		return cellResult;
	}

	if (hitPortalId == -1)
	{
		// We made it to the goal without hitting a portal - the move
		// succeeds

		return CMR_MoveOK;
	}

	// ----------
	// We crossed a portal-adjacent floor edge on our way to the goal

	Floor const * floor = endLoc.getFloor();

	if (!floor)
	{
		return CMR_Error;
	}

	Object const * owner = floor->getOwner();

	if (!owner)
	{
		return CMR_Error;
	}

	CellProperty * newCell = startCell->getDestinationCell(owner, hitPortalId);

	if (!newCell)
	{
		return CMR_Error;
	}

	// so restart the move in the new cell at the point where we crossed
	// the portal-adjacent edge

	Vector hitPortalPos = endLoc.getPosition_p();

	Vector newStartPos = CollisionUtils::transformToCell(startCell, hitPortalPos, newCell);

	FloorLocator newStartLoc(newStartPos, startLoc.getRadius());

	Vector newGoalPos = CollisionUtils::transformToCell(startCell, goalPos, newCell);

	float dist2 = newStartPos.magnitudeBetweenSquared(newGoalPos);

	if (dist2 < 0.0000000001f)
	{
		return CMR_MoveOK;
	}
	else
	{
		static int recursionCounter = 0;

		if (recursionCounter == 10)
		{
			Vector s = startLoc.getPosition_w();

			WARNING_STRICT_FATAL(ConfigSharedCollision::getReportWarnings(), ("CollisionWorld::canMove - infinite recursion detected (%f,%f,%f)", s.x, s.y, s.z));

			endLoc = FloorLocator::invalid;
			return CMR_Error;
		}

		recursionCounter++;

		CanMoveResult result = canMove(object, newCell, newStartLoc, newGoalPos, checkY, checkFlora, checkFauna, endLoc);

		recursionCounter--;

		return result;
	}
}

// ----------
// A nullptr goal cell means we don't care what cell we end up in as long as we make it to the goal

CanMoveResult CollisionWorld::canMove(Object const * object,
	CellProperty const * startCell,
	FloorLocator const & startLoc,
	CellProperty const * goalCell,
	FloorLocator const & goalLoc,
	bool checkY,
	bool checkFlora,
	bool checkFauna)
{
	// ----------

	if (startCell == nullptr)
	{
		startCell = CellProperty::getWorldCellProperty();
	}

	if (goalCell == nullptr)
	{
		FloorLocator dummy;

		return canMove(object, startCell, startLoc, goalLoc.getPosition_p(), checkY, checkFlora, checkFauna, dummy);
	}

	// ----------

	int hitPortalId = -1;
	FloorLocator endLoc;

	if (startCell == goalCell)
	{
		// We're in the right cell. The move succeeds if it makes it to the goal
		// without hitting a portal on the way there.

		CanMoveResult cellResult = canMoveInCell(object, startCell, startLoc, goalLoc, checkY, checkFlora, checkFauna, endLoc, hitPortalId);

		if ((cellResult == CMR_MoveOK) && (hitPortalId != -1))
		{
			return CMR_MissedGoal;
		}
		else
		{
			return cellResult;
		}
	}

	// ----------

	// We're not in our goal cell. Try to move as far as we can towards the goal
	// in our current cell

	Vector const & goalPos = goalLoc.getPosition_p();

	Vector localGoalPos = CollisionUtils::transformToCell(goalCell, goalPos, startCell);

	CanMoveResult cellResult = canMoveInCell(object, startCell, startLoc, localGoalPos, checkY, checkFlora, checkFauna, endLoc, hitPortalId);

	if (cellResult != CMR_MoveOK)
	{
		// We hit something while trying to get out of this cell - the move fails.

		return cellResult;
	}

	if (hitPortalId == -1)
	{
		// We can move towards the goal from the current cell, but we don't exit the cell via a portal-adjacent
		// floor edge and we're not in the right goal cell - we can never get to the goal cell,
		// so the move fails

		return CMR_MissedGoal;
	}

	// Otherwise, restart the query in the cell on the other side of the portal-adjacent edge

	Floor const * floor = endLoc.getFloor();

	if (!floor)
	{
		return CMR_Error;
	}

	Object const * owner = floor->getOwner();

	if (!owner)
	{
		return CMR_Error;
	}

	CellProperty * newCell = startCell->getDestinationCell(owner, hitPortalId);

	if (!newCell)
	{
		return CMR_Error;
	}

	Vector hitPortalPos = endLoc.getPosition_p();

	Vector newStartPos = CollisionUtils::transformToCell(startCell, hitPortalPos, newCell);

	FloorLocator newStartLoc(newStartPos, startLoc.getRadius());

	newStartLoc.setRadius(startLoc.getRadius());

	return canMove(object, newCell, newStartLoc, goalCell, goalLoc, checkY, checkFlora, checkFauna);
}

// ----------------------------------------------------------------------

CanMoveResult CollisionWorld::canMoveOnFloor(Object const * object,
	FloorLocator const & startLoc,
	Floor const * floor,
	FloorLocator const & goalLoc,
	bool checkY,
	bool checkFlora,
	bool checkFauna,
	FloorLocator & outLoc,
	int & outHitPortalId)
{
	FloorMesh const * floorMesh = floor->getFloorMesh();

	Vector const & startPos = startLoc.getPosition_p() + Vector(0.0f, startLoc.getOffset(), 0.0f);
	Vector const & goalPos = goalLoc.getPosition_p() + Vector(0.0f, goalLoc.getOffset(), 0.0f);

	Vector localGoalPos = goalPos;

	float radius = startLoc.getRadius();

	Vector delta = localGoalPos - startPos;

	delta.y = 0.0f;

	if (!startLoc.isAttachedTo(floor))
	{
		// Starting location isn't on this floor, run the canMoveOnFloor version that does an
		// attachment test instead

		return canMoveOnFloor(object, startPos, radius, floor, goalLoc, checkY, checkFlora, checkFauna, outLoc, outHitPortalId);
	}

	// ----------

	FloorLocator result;

	PathWalkResult walkResult = floor->canMove(startLoc, delta, -1, -1, result);

	outLoc = result;

	outHitPortalId = -1;

	if ((result.getTriId() >= 0) && (result.getEdgeId() >= 0))
	{
		int const exitTriId = result.getTriId();

		FloorTri const & exitTri = floorMesh->getFloorTri(exitTriId);

		outHitPortalId = exitTri.getPortalId(result.getEdgeId());
	}

	// ----------
	// The move has clipped to a crossable edge of the floor.
	// Check to see if the move re-enters this floor, and if it does
	// test the move there.

	// Since this is a recursive call, it'll catch an arbitrary number of reentrances

	if (walkResult == PWR_HitPortalEdge)
	{
		// Exiting the mesh along a portal-adjacent edge is always a successful move.

		return CMR_MoveOK;
	}
	else if (walkResult == PWR_ExitedMesh)
	{
		if (floor->isCellFloor())
		{
			// Trying to walk off a ledge inside a cell isn't allowed at the moment.

			return CMR_HitFloorEdge;
		}

		Vector reenterDelta = delta - (delta * result.getTime());

		FloorLocator reenterLoc;

		if (floor->findEntryPoint(result, reenterDelta, false, reenterLoc))
		{
			if (reenterLoc.getTime() > 0.0f)
			{
				Vector reenterPos = reenterLoc.getPosition_p();

				UNREF(reenterPos);

				reenterLoc.setRadius(radius);

				static int recursionCounter = 0;

				if (recursionCounter == 5)
				{
					Vector s = startLoc.getPosition_w();

					WARNING_STRICT_FATAL(ConfigSharedCollision::getReportWarnings(), ("CollisionWorld::canMoveOnFloor - infinite recursion detected (%f,%f,%f)", s.x, s.y, s.z));
					outLoc = FloorLocator::invalid;
					outHitPortalId = -1;
					return CMR_Error;
				}

				recursionCounter++;
				CanMoveResult floorResult = canMoveOnFloor(object, reenterLoc, floor, goalLoc, checkY, checkFlora, checkFauna, outLoc, outHitPortalId);

				recursionCounter--;

				return floorResult;
			}
		}

		// Failure to reenter a floor in the world cell isn't a collision because
		// it means you're back on the terrain.

		return CMR_MoveOK;
	}
	else if (walkResult == PWR_WalkOk)
	{
		if ((floor->getCell() == CellProperty::getWorldCellProperty()))
		{
			if (result.isFallthrough())
			{
				return CMR_MoveOK;
			}
		}

		outLoc = result;
		outHitPortalId = -1;

		if ((floor == startLoc.getFloor()) && (floor == goalLoc.getFloor()))
		{
			// The start and goal locs are on this floor - if the move didn't get to
			// the goal tri, the move failed.

			if (checkY && (goalLoc.getId() != -1) && (result.getId() != goalLoc.getId()))
			{
				return CMR_MissedGoal;
			}
			else
			{
				return CMR_MoveOK;
			}
		}
		else
		{
			if (result.getFloor() == floor)
			{
				Vector resultPos = result.getPosition_p();

				if ((resultPos.y - goalPos.y) > 3.0f)
				{
					return CMR_MissedGoal;
				}
				else
				{
					return CMR_MoveOK;
				}
			}
			else
			{
				return CMR_MoveOK;
			}
		}
	}
	else
	{
		return CMR_HitFloorEdge;
	}
}

// ----------------------------------------------------------------------

CanMoveResult CollisionWorld::canMoveOnFloor(Object const * object,
	Vector const & inPos,
	float moveRadius,
	Floor const * floor,
	FloorLocator const & inGoalLoc,
	bool checkY,
	bool checkFlora,
	bool checkFauna,
	FloorLocator & outLoc,
	int & outHitPortalId)
{
	bool hasStartLoc = false;
	FloorLocator startLoc;

	FloorLocator goalLoc = inGoalLoc;

	if (goalLoc.getFloor() != floor)
	{
		Vector goalPos_p = goalLoc.getPosition_p(floor->getCell());

		FloorLocator tempLoc;

		if (floor->dropTest(goalPos_p, tempLoc))
		{
			goalLoc = tempLoc;
		}
	}

	// ----------
	// We have a starting position, but we don't know where the path first hits
	// the floor, so try to find a starting point for the move.

	// If the start point is already above the floor, the floor move will begin
	// at the floor point under the start point

	if (floor->dropTest(inPos, startLoc))
	{
		Vector startPos_w = startLoc.getPosition_w();

		UNREF(startPos_w);

		hasStartLoc = true;
	}
	else
	{
		// Otherwise we have to search the floor for the first boundary
		// crossing and start there.

		Vector delta = goalLoc.getPosition_p() - inPos;

		FloorLocator tempLoc(inPos, moveRadius);

		if (floor->findEntryPoint(tempLoc, delta, false, startLoc))
		{
			bool edgeCrossable = floor->getFloorMesh()->getFloorTri(startLoc.getId()).isCrossable(startLoc.getEdgeId());

			if (!edgeCrossable)
			{
				return CMR_HitFloorEdge;
			}

			hasStartLoc = true;
		}
	}

	// ----------
	// If we found a starting point for the floor move, run the floor move

	if (hasStartLoc)
	{
		startLoc.setRadius(moveRadius);

		return canMoveOnFloor(object, startLoc, floor, goalLoc, checkY, checkFlora, checkFauna, outLoc, outHitPortalId);
	}
	else
	{
		// Otherwise the move succeeds - failing to enter a floor is not a collision.

		return CMR_MoveOK;
	}
}

// ----------------------------------------------------------------------
// Find all obstacles in a capsule containing the start and end extents,
// do a temporal collision test against each of them, return true if
// the creature hit one and a pointer to the first one hit.

bool CollisionWorld::findFirstObstacle(Object const * creature, float creatureRadius, Vector const & testPos, Vector const & goalPos, bool testStatics, bool testCreatures, Object const * & outObject)
{
	UNREF(goalPos);
	UNREF(creatureRadius);

	Vector startPos = creature->getPosition_p();

	Vector delta = testPos - startPos;

	if (creature == nullptr) return false;

	CollisionProperty const * collision = creature->getCollisionProperty();

	if (collision == nullptr) return false;

	BaseExtent const * baseExtent = collision->getExtent_p();

	SimpleExtent const * simpleExtent = dynamic_cast<SimpleExtent const *>(baseExtent);

	if (simpleExtent == nullptr) return false;

	MultiShape const & shape = simpleExtent->getShape();

	if (shape.getShapeType() != MultiShape::MST_Sphere) return false;

	// ----------
	// Test collision extents

	DetectResult minResult;
	Object const * minObject = nullptr;

	typedef std::vector<Object*> ObjectVector;
	static ObjectVector  statics;
	static ObjectVector  creatures;

	statics.clear();
	creatures.clear();

	if (ms_database->queryObjects(creature->getParentCell(), shape, delta, (testStatics ? &statics : nullptr), (testCreatures ? &creatures : nullptr)))
	{
		ObjectVector::size_type const nStatics = statics.size();
		ObjectVector::size_type i;

		for (i = 0; i < nStatics; i++)
		{
			Object const * obstacle = statics[i];

			if (obstacle == nullptr) continue;

			if (obstacle == creature) continue;

			if (Distance2d::Distance2(obstacle->getPosition_w(), goalPos) < 0.000001) continue;

			DetectResult result = CollisionDetect::testObjects(creature, delta, obstacle);

			if (result.collided && (result.collisionTime < minResult.collisionTime))
			{
				minResult = result;
				minObject = obstacle;
			}
		}

		ObjectVector::size_type const nCreatures = creatures.size();

		for (i = 0; i < nCreatures; i++)
		{
			Object const * obstacle = creatures[i];

			if (obstacle == nullptr) continue;

			if (obstacle == creature) continue;

			if (Distance2d::Distance2(obstacle->getPosition_w(), goalPos) < 0.000001) continue;

			DetectResult result = CollisionDetect::testObjects(creature, delta, obstacle);

			if (result.collided && (result.collisionTime < minResult.collisionTime))
			{
				minResult = result;
				minObject = obstacle;
			}
		}
	}

	//-- Check for floor collisions.
	Footprint const *const footprint = collision->getFootprint();
	FloorLocator const *const floorLocator = (footprint ? footprint->getAnyContact() : nullptr);
	if (floorLocator)
	{
		Object const *floorCollisionObject = nullptr;
		Vector        collisionLocation_w;

		// Do the floor check.
		bool const floorCollisionOccurred = testFloorCollision(*floorLocator, creature->getTransform_p2w().rotate_l2p(delta), collisionLocation_w, floorCollisionObject);
		if (floorCollisionOccurred && floorCollisionObject)
		{
			// We collided with a floor.  Check if we collide with the floor sooner than anything else.
			bool useFloorCollision;

			if (!minResult.collided) // don't return a floor as a collision object if nothing was collided with earlier
			{
				useFloorCollision = false;
			}
			else
			{
				// Figure out if collided object is closer than floor.   Collided object looks like its
				// extent is assumed to be in the same parent space as creature.  That might be a bug.
				float const minResultMagnitudeSquared = (creature->getPosition_p() - minResult.extentB->getCenter()).magnitudeSquared();
				float const floorMagnitudeSquared = (creature->getPosition_w() - collisionLocation_w).magnitudeSquared();

				useFloorCollision = floorMagnitudeSquared < minResultMagnitudeSquared;
			}

			if (useFloorCollision)
			{
				// We will use the floor object as the collision object.
				outObject = floorCollisionObject;
				return true;
			}
		}
	}

	if (minResult.collided)
	{
		outObject = minObject;

		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

bool CollisionWorld::findFirstObstacle(Sphere const & sphere, Vector const & goalPos, bool testStatics, bool testCreatures, Object const * & outObject)
{
	Vector startPos = sphere.getCenter();

	Vector delta = goalPos - startPos;

	MultiShape shape(sphere);

	// ----------
	// Test collision extents

	DetectResult minResult;
	Object const * minObject = nullptr;

	static std::vector<Object*> statics;
	static std::vector<Object*> creatures;

	statics.clear();
	creatures.clear();

	if (ms_database->queryObjects(CellProperty::getWorldCellProperty(), shape, delta, (testStatics ? &statics : nullptr), (testCreatures ? &creatures : nullptr)))
	{
		size_t const nStatics = statics.size();
		size_t i;

		for (i = 0; i < nStatics; i++)
		{
			Object const * obstacle = statics[i];

			if (obstacle == nullptr) continue;

			DetectResult result = CollisionDetect::testSphereObject(sphere, delta, obstacle);

			if (result.collided && (result.collisionTime < minResult.collisionTime))
			{
				minResult = result;
				minObject = obstacle;
			}
		}

		size_t const nCreatures = creatures.size();

		for (i = 0; i < nCreatures; i++)
		{
			Object const * obstacle = creatures[i];

			if (obstacle == nullptr) continue;

			if (Distance2d::Distance2(obstacle->getPosition_w(), goalPos) < 0.000001) continue;

			DetectResult result = CollisionDetect::testSphereObject(sphere, delta, obstacle);

			if (result.collided && (result.collisionTime < minResult.collisionTime))
			{
				minResult = result;
				minObject = obstacle;
			}
		}
	}

	if (minResult.collided)
	{
		outObject = minObject;

		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------
// Compute the radius of the largest sphere that can fit around the given point without hitting anything

bool CollisionWorld::calcBubble(CellProperty const * cell, Vector const & point_p, float maxDistance, float & outRadius)
{
	return calcBubble(cell, point_p, nullptr, maxDistance, outRadius);
}

bool CollisionWorld::calcBubble(CellProperty const * cell, Vector const & point_p, Object const * ignoreObject, float maxDistance, float & outRadius)
{
	Vector point_w = CollisionUtils::transformToWorld(cell, point_p);

	float minDistance = maxDistance;

	{
		static ObjectVec results;

		results.clear();

		if (ms_database->queryCloseStatics(point_w, minDistance, &results))
		{
			ObjectVec::size_type const count = results.size();

			for (ObjectVec::size_type i = 0; i < count; i++)
			{
				Object const * object = results[i];

				NOT_NULL(object);

				if (object == ignoreObject) continue;

				CollisionProperty const * collision = object->getCollisionProperty();

				NOT_NULL(collision);

				float distance = REAL_MAX;

				if (collision->getDistance(point_w, minDistance, distance))
				{
					if (distance < minDistance) minDistance = distance;
				}
			}
		}
	}

	// ----------

	{
		static FloorVec results;

		results.clear();

		if (ms_database->queryCloseFloors(point_w, minDistance, &results))
		{
			FloorVec::size_type const count = results.size();

			for (FloorVec::size_type i = 0; i < count; i++)
			{
				Floor const * floor = results[i];

				NOT_NULL(floor);

				if (floor->getOwner() == ignoreObject) continue;

				float distance = REAL_MAX;

				FloorEdgeId dummyId;

				if (floor->getDistanceUncrossable2d(point_w, minDistance, distance, dummyId))
				{
					if (distance < minDistance) minDistance = distance;
				}
			}
		}
	}

	// ----------

	if (cell && (cell != CellProperty::getWorldCellProperty()))
	{
		Floor const * floor = cell->getFloor();

		if (floor)
		{
			float distance = REAL_MAX;

			FloorEdgeId dummyId;

			if (floor->getDistanceUncrossable2d(point_w, minDistance, distance, dummyId))
			{
				if (distance < minDistance) minDistance = distance;
			}
		}
	}

	// ----------

	if (minDistance < maxDistance)
	{
		outRadius = minDistance;
		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

bool CollisionWorld::testClear(CellProperty const * cell, Sphere const & sphere_p)
{
	bool hitStatic = ms_database->queryMaterial(cell, sphere_p, MT_Solid);

	if (hitStatic) return false;

	return true;
}

// ----------------------------------------------------------------------

bool CollisionWorld::canMoveInSpace(Object const * object, Sphere const & sphere, Vector const & goalPos)
{
	Vector delta = goalPos - sphere.getCenter();
	float collisionTime = 0.0f;

	if (ms_database->queryMaterial(CellProperty::getWorldCellProperty(), sphere, delta, MT_Solid, false, false, object, collisionTime))
	{
		float epsilon = 0.05f / delta.magnitude();

		if (collisionTime > -epsilon)
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool CollisionWorld::findLocators(CellProperty const * cell, Vector const & point, std::vector<FloorLocator> & outLocs)
{
	outLocs.clear();

	// ----------
	// If the point is in a cell, its only locator is the one on the cell floor

	if (cell && (cell != CellProperty::getWorldCellProperty()))
	{
		Floor const * floor = cell->getFloor();

		FloorLocator tempLoc;

		if (floor && floor->dropTest(point, tempLoc))
		{
			outLocs.push_back(tempLoc);

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		// ----------
		// Otherwise we need to find all object floors and see if the point falls on any of them

		if (ms_database)
		{
			FloorVec floors;

			Segment3d querySeg(point, point - (Vector::unitY * 10.0f));

			IGNORE_RETURN(ms_database->queryFloors(cell, querySeg, &floors));

			for (uint i = 0; i < floors.size(); i++)
			{
				Floor * floor = floors[i];

				FloorLocator tempLoc;

				if (floor && floor->dropTest(point, tempLoc))
				{
					outLocs.push_back(tempLoc);
				}
			}
		}

		return outLocs.size() != 0.0f;
	}
}

// ----------------------------------------------------------------------
// This should work OK, but if the point drops onto multiple floors it will make a locator for only
// the topmost floor

bool CollisionWorld::makeLocator(CellProperty const * cell, Vector const & point, FloorLocator & outLoc)
{
	static FloorLocatorVec tempLocs;

	tempLocs.clear();

	if (findLocators(cell, point, tempLocs))
	{
		float maxHeight = -REAL_MAX;
		int highestLoc = -1;

		for (uint i = 0; i < tempLocs.size(); i++)
		{
			float height = tempLocs[i].getPosition_p().y;

			if (height > maxHeight)
			{
				maxHeight = height;
				highestLoc = static_cast<int>(i);
			}
		}

		if (highestLoc != -1)
		{
			outLoc = tempLocs[static_cast<uint>(highestLoc)];
			return true;
		}
		else
		{
			outLoc = FloorLocator(point, 0.0f);
			return 	false;
		}
	}
	else
	{
		outLoc = FloorLocator(point, 0.0f);
		return false;
	}
}

// ----------------------------------------------------------------------

bool CollisionWorld::isServerSide(void)
{
	return ms_serverSide;
}

void CollisionWorld::setServerSide(bool serverSide)
{
	ms_serverSide = serverSide;
}

//----------------------------------------------------------------------

void CollisionWorld::handleSceneChange(std::string const & sceneId)
{
	ms_forceReattachContactsPostResolve = FootprintForceReattachManager::isSceneFootprintForceReattach(sceneId);
}

//----------------------------------------------------------------------

/** @pre the object is "on" all floors in floorList
 *  @return the floor that is least below the player
 */
Floor const * CollisionWorld::getFloorStandingOn(Object const & object)
{
	CollisionProperty const * const objectCollisionProperty = object.getCollisionProperty();
	Footprint const * const objectFootprint = (objectCollisionProperty != nullptr) ? objectCollisionProperty->getFootprint() : nullptr;

	if (objectFootprint == nullptr)
	{
		return nullptr;
	}

	MultiListHandle const & floorList = objectFootprint->getFloorList();
	float const objectHeight = object.getPosition_w().y;
	Floor const * resultFloor = nullptr;
	float resultDistanceBelowObject = 0.0f;
	float dropTestHeight = std::numeric_limits<float>::min();

	for (MultiListConstDataIterator<FloorContactShape> it(floorList); it; ++it)
	{
		FloorContactShape const * const contact = *it;
		FloorLocator const & loc = contact->m_contact;
		Floor const * const f = loc.getFloor();

		FloorLocator tempLoc;
		if (f->dropTest(object.getPosition_p(), tempLoc))
		{
			//if we might be standing on it (it's below us), give 1/2 meter buffer space
			if ((tempLoc.getPosition_w().y - 0.5) <= objectHeight)
			{
				float const distanceBelowObject = std::max(0.0f, objectHeight - tempLoc.getPosition_w().y);

				//if none found yet, use it
				if (!resultFloor)
				{
					resultFloor = f;
					resultDistanceBelowObject = distanceBelowObject;
					dropTestHeight = tempLoc.getPosition_w().y;
				}
				else
				{
					//if it's less below us than the current, use it
					if (distanceBelowObject < resultDistanceBelowObject)
					{
						resultFloor = f;
						resultDistanceBelowObject = distanceBelowObject;
						dropTestHeight = tempLoc.getPosition_w().y;
					}
				}
			}
		}
	}

	if (dropTestHeight > std::numeric_limits<float>::min())
	{
		TerrainObject const * const terrainObject = TerrainObject::getConstInstance();

		if (terrainObject != nullptr)
		{
			CellProperty const * const parentCell = object.getParentCell();

			if (parentCell->isWorldCell())
			{
				float terrainHeight;
				bool result = false;

				// This has to be split client/server because on the server forcing chunk
				// creation is thread safe, but on the client it is not thread safe.

				if (isServerSide())
				{
					result = terrainObject->getHeightForceChunkCreation(object.getPosition_w(), terrainHeight);
				}
				else
				{
					result = terrainObject->getHeight(object.getPosition_w(), terrainHeight);
				}

				if (result)
				{
					if (dropTestHeight < terrainHeight)
					{
						resultFloor = nullptr;
					}
				}
			}
		}
	}

	return resultFloor;
}

// ----------------------------------------------------------------------
