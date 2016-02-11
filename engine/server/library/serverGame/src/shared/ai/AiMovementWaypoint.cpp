// ======================================================================
//
// AiMovementWaypoint.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementWaypoint.h"

#include "serverGame/AiCreatureController.h"
#include "serverGame/AiMovementArchive.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Footprint.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/AiDebugString.h"
#include "sharedGame/SlopeEffectProperty.h"
#include "sharedLog/Log.h"
#include "sharedMath/Circle.h"
#include "sharedMath/Ray3d.h"
#include "sharedTerrain/TerrainObject.h"

#include <limits>

using namespace Scripting;

float computeMovementModifier (CreatureObject * const object)
{
	if (!object)
	{
		DEBUG_FATAL(true, ("object is nullptr."));
		return 0.0f;
	}

	//-- verify that the object is not on a floor
	const CollisionProperty* const collision = object->getCollisionProperty ();
	const bool isOnSolidFloor = collision && collision->getFootprint() && collision->getFootprint()->isOnSolidFloor ();

	float speedModifier = 1.0f;
	Vector objectFrameK_w(object->getObjectFrameK_w());

	if (object->isInWorldCell () && !isOnSolidFloor)
	{
		const Vector position = object->getPosition_w ();
		const TerrainObject* const terrainObject = TerrainObject::getInstance ();

		if (terrainObject)
		{
			if (terrainObject->isBelowWater (position))
			{
				speedModifier = object->getWaterModPercent();
			}
			else
			{
				// determine the slope effect from the actual terrain
			float terrainHeight;
			Vector normal;
			if (terrainObject->getHeight (position, terrainHeight, normal))
			{
				const float slopeTolerance  = object->getSlopeModAngle();
				const float slopeThreshold  = -cos (convertDegreesToRadians (90.f - slopeTolerance));
				const float speedMultiplier = object->getSlopeModPercent();

					const float slope = normal.dot(objectFrameK_w);
				if (slope < slopeThreshold)
				{
					const float ratio = (slope - slopeThreshold) / (-1.f - slopeThreshold);
						speedModifier = linearInterpolate (1.f, speedMultiplier, ratio);
					}
				}
			}
				}
			}

	// if the creature has a slope effect property, see if it has a greater
	// (more negative) effect on the creature than the terrain
	const Property * property = object->getProperty(SlopeEffectProperty::getClassPropertyId());
	if (property != nullptr)
	{
		// note we use the creature's base speed modifier, not the one modified by skills
		// (although for ai they're probably the same)
		const float baseSpeedMultiplier = object->getBaseSlopeModPercent();
		const SlopeEffectProperty * slopeEffect = safe_cast<const SlopeEffectProperty *>(property);
		const std::vector<Vector> & normals = slopeEffect->getNormals();
		for (std::vector<Vector>::const_iterator i = normals.begin(); i != normals.end(); ++i)
		{
			float testSlope = (*i).dot(objectFrameK_w);
			if (testSlope < 0)
			{
				const float testModifier = linearInterpolate (1.f, baseSpeedMultiplier, -testSlope);
				if (testModifier < speedModifier)
				{
//LOG("slope_test", ("Slope effect setting creature %s speed modifier to %f", object->getNetworkId().getValueString().c_str(), testModifier));
					speedModifier = testModifier;
				}
			}
		}
		object->removeProperty(SlopeEffectProperty::getClassPropertyId());
	}

	return speedModifier;
}

// ======================================================================

AiMovementWaypoint::AiMovementWaypoint( AICreatureController * controller )
 : AiMovementBase(controller)
 , m_avoidanceLocation()
 , m_avoidanceStuckTime(0.0f)
 , m_avoidanceHeadedToWaypointTime(0.0f)
 , m_mostRecentAvoidanceFrameNumber(-1)
 , m_obstacleLocation()
{
	CHANGE_STATE( AiMovementWaypoint::stateWaiting );
}

AiMovementWaypoint::AiMovementWaypoint( AICreatureController * controller, Archive::ReadIterator & source )
: AiMovementBase(controller, source)
 , m_avoidanceLocation(source)
 , m_avoidanceStuckTime(0.0f)
 , m_avoidanceHeadedToWaypointTime(0.0f)
 , m_mostRecentAvoidanceFrameNumber(-1)
 , m_obstacleLocation(source)
{
	SETUP_SYNCRONIZED_STATE( AiMovementWaypoint::stateWaiting );
	SETUP_SYNCRONIZED_STATE( AiMovementWaypoint::stateMoving );
	SETUP_SYNCRONIZED_STATE( AiMovementWaypoint::stateAvoiding );
}

// ----------------------------------------------------------------------

AiMovementWaypoint::~AiMovementWaypoint()
{
}

// ----------------------------------------------------------------------

void AiMovementWaypoint::pack( Archive::ByteStream & target ) const
{
	AiMovementBase::pack(target);
	m_avoidanceLocation.pack(target);
	m_obstacleLocation.pack(target);
}

// ----------------------------------------------------------------------

void AiMovementWaypoint::alter ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementWaypoint::alter");

	AiMovementBase::alter(time);
}

// ----------------------------------------------------------------------

void AiMovementWaypoint::refresh( void )
{
	AiMovementBase::refresh();
	CHANGE_STATE( AiMovementWaypoint::stateWaiting );
}

// ----------------------------------------------------------------------

bool AiMovementWaypoint::getHibernateOk ( void ) const
{
	return m_stateFunction == CAST_STATE(AiMovementWaypoint::stateWaiting);
}

// ----------------------------------------------------------------------

void AiMovementWaypoint::getDebugInfo ( std::string & outString ) const
{
	AiMovementBase::getDebugInfo(outString);

	outString += "\n";

	outString += "AiMovementWaypoint:\n";

	char buffer[256];

	if(hasWaypoint())
	{
		Vector goal = getWaypoint().getPosition_w();
		sprintf(buffer,"Waypoint : %s at (%f,%f,%f)\n",getWaypoint().getObjectId().getValueString().c_str(),goal.x,goal.y,goal.z);
		outString += buffer;
	}
	else
	{
		outString += "Waypoint : none\n";
	}

	if(m_obstacleLocation.isValid() && m_avoidanceLocation.isValid())
	{
		Vector obstacle = m_obstacleLocation.getPosition_w();
		Vector avoid = m_avoidanceLocation.getPosition_w();

		sprintf(buffer,"Obstacle Avoidance : avoiding %s at (%f,%f,%f)\n",m_obstacleLocation.getObjectId().getValueString().c_str(),obstacle.x,obstacle.y,obstacle.z);
		outString += buffer;
		sprintf(buffer,"Obstacle Avoidance : going to (%f,%f,%f)\n",avoid.x,avoid.y,avoid.z);
		outString += buffer;

		IGNORE_RETURN(snprintf(buffer, sizeof(buffer), "Obstacle Avoidance: accumulated stuck time (%.2f) seconds\n", m_avoidanceStuckTime));
		buffer[sizeof(buffer) - 1] = '\0';
		outString += buffer;
	}
	else
	{
		outString += "Obstacle Avoidance : none\n";
	}

	sprintf(buffer,"Speed : %f current, %f desired (modifier %f)\n",m_controller->getSpeed(),getDesiredSpeed(),computeMovementModifier(m_controller->getCreature()));
	outString += buffer;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementWaypoint::stateWaiting ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementWaypoint::stateWaiting");

	UNREF (time);

	if (hasWaypoint() && getWaypoint().isValid())
	{
		return triggerMoving();
	}
	else
	{
		return ASR_Done;
	}
}

// ----------------------------------------------------------------------

AiStateResult AiMovementWaypoint::stateMoving ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementWaypoint::stateMoving");

	if(!m_controller->getCreature()->canMove())
	{
		return ASR_Done;
	}

	if (!hasWaypoint())
	{
		return triggerWaiting();
	}

	if (!getWaypoint().isValid()) 
	{
		return triggerTargetLost();
	}

	if (!updateWaypoint())
	{
		return triggerTargetLost();
	}

	bool const reachedGoal = moveTowards(getWaypoint(), getFinalWaypoint(), time);

	if (reachedGoal)
	{
		return triggerWaypoint();
	}

	if (findObstacle())
	{
		return triggerAvoiding();
	}

 	return ASR_Done;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementWaypoint::stateAvoiding ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementWaypoint::stateAvoiding");

	//LOG("avoidance_log", ("In avoid state, time: %1.2f\n", m_avoidanceStuckTime));

	//-- Clear the avoidance stuck timer if we haven't avoided in the last x number of frames.
	int const frameNumber = Os::getNumberOfUpdates();
	int const deltaFramesSinceLastAvoidance = frameNumber - m_mostRecentAvoidanceFrameNumber;

	if (deltaFramesSinceLastAvoidance > ConfigServerGame::getBehaviorMaxAvoidancePersistenceFrameCount())
		m_avoidanceStuckTime = 0.0f;

	//-- Remember that we did need to avoid this frame.
	m_mostRecentAvoidanceFrameNumber = frameNumber;

	//-- Handle avoiding.
	if (!m_controller->getCreature()->canMove())
		return ASR_Done;

	if(!hasWaypoint() || !getWaypoint().isValid())
		return triggerTargetLost();
	
	if(!updateWaypoint())
		return triggerTargetLost();

	if(!updateAvoidancePoint(time))
		return triggerDoneAvoiding();

	// ----------

	//-- Accumulate the stuck time for this frame.  If we exceeded the max stuck time,
	//   warp the AI to its target location.
	m_avoidanceStuckTime += time;

	if(m_avoidanceStuckTime > ConfigServerGame::getBehaviorMaxAvoidanceStuckTime())
	{
		warpToLocation(getWaypoint());
		m_avoidanceStuckTime = 0.0f;
		return triggerDoneAvoiding();
	}

	bool const reachedGoal = moveTowards(m_avoidanceLocation, getFinalWaypoint(), time);

	if (reachedGoal)
	{
		return triggerDoneAvoiding();
	}

	return ASR_Done;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementWaypoint::triggerWaiting ( void )
{
	CHANGE_STATE( AiMovementWaypoint::stateWaiting );

	return ASR_Done;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementWaypoint::triggerMoving ( void )
{
	m_avoidanceLocation = AiLocation();

	CHANGE_STATE( AiMovementWaypoint::stateMoving );

	return ASR_Continue;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementWaypoint::triggerAvoiding ( void )
{
	m_avoidanceHeadedToWaypointTime = 0.0f;

	CHANGE_STATE( AiMovementWaypoint::stateAvoiding );

	return ASR_Continue;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementWaypoint::triggerDoneAvoiding ( void )
{
	m_obstacleLocation = AiLocation();
	m_avoidanceLocation = AiLocation();

	CHANGE_STATE( AiMovementWaypoint::stateMoving );

	return ASR_Continue;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementWaypoint::triggerWaypoint ( void )
{
	clearWaypoint();

	return triggerWaiting();
}

// ----------------------------------------------------------------------

AiStateResult AiMovementWaypoint::triggerTargetLost ( void )
{
	clearWaypoint();
	
	return triggerWaiting();
}

// ----------------------------------------------------------------------

bool AiMovementWaypoint::hasWaypoint ( void ) const
{
	return false;
}

// ----------------------------------------------------------------------

AiLocation const & AiMovementWaypoint::getWaypoint ( void ) const
{
	return AiLocation::invalid;
}

// ----------------------------------------------------------------------

AiLocation const & AiMovementWaypoint::getFinalWaypoint() const
{
	return AiLocation::invalid;
}

// ----------------------------------------------------------------------

bool AiMovementWaypoint::updateWaypoint ( void )
{
	return true;
}

// ----------------------------------------------------------------------

void AiMovementWaypoint::clearWaypoint ( void )
{
}

// ----------------------------------------------------------------------

bool AiMovementWaypoint::moveTowards(AiLocation const & nextLocation, AiLocation const & finalLocation, float const time)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementWaypoint::moveTowards");

	bool reachedGoal = false;
	bool finalLocationAdjustment = false;

	float const desiredSpeed = getDesiredSpeed();
	m_controller->setSpeed(desiredSpeed);

	AiLocation goalLocation;

	if (finalLocation.isValid())
	{
		Sphere finalLocationSphere(finalLocation.getPosition_p(), finalLocation.getRadius());

		if (finalLocationSphere.contains(nextLocation.getPosition_p()))
		{
			finalLocationAdjustment = true;

			// We know we are attempting to get too close to this final
			// position so find a better position that does not get too
			// close

			Vector const & creaturePosition_p = m_controller->getCreaturePosition_p();
			Vector direction(nextLocation.getPosition_p() - creaturePosition_p);
			direction.normalize();
			Ray3d const ray(creaturePosition_p, direction);
			Intersect3d::ResultData result;
			
			if (Intersect3d::intersectRaySphereWithData(ray, finalLocationSphere, &result))
			{
				goalLocation = AiLocation(nextLocation.getCell(), creaturePosition_p + direction * result.m_length);
			}
		}
	}
	
	if (!goalLocation.isValid())
	{
		goalLocation = nextLocation;
	}

	if (goalLocation.isValid())
	{
		m_controller->moveTowards(goalLocation.getCell(), goalLocation.getPosition_p(), time);

		if (   finalLocation.isValid()
		    && finalLocationAdjustment)
		{
			if (reachedLocation(finalLocation))
			{
				reachedGoal = true;
			}
		}
		else
		{
			if (reachedLocation(nextLocation))
			{
				reachedGoal = true;
			}
		}
	}

	return reachedGoal;
}

// ----------------------------------------------------------------------

float AiMovementWaypoint::getDesiredSpeed ( void ) const
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementWaypoint::getDesiredSpeed");

	float result = 0.0f;

	if (hasWaypoint())
	{
		CreatureObject * const creatureOwner = m_controller->getCreature();

		if(m_controller->isRunning())
		{
			result = creatureOwner->getRunSpeed() * computeMovementModifier(creatureOwner);
		}
		else
		{
			result = creatureOwner->getWalkSpeed() * computeMovementModifier(creatureOwner);
		}
	}

	return result;
}

// ----------------------------------------------------------------------

bool AiMovementWaypoint::getDecelerate ( void ) const
{
	return !(m_controller->getCreature()->isInCombat());
}

// ----------------------------------------------------------------------

bool AiMovementWaypoint::findObstacle()
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementWaypoint::findObstacle");

	CreatureObject * const creatureOwner = m_controller->getCreature();
	Vector const & creaturePosition_p = creatureOwner->getPosition_p();
	Vector const & goalPosition_p = getWaypoint().getPosition_p();
	Vector delta = goalPosition_p - creaturePosition_p;

	// Clamp the obstacle lookahead distance to 5 meters

	if (delta.magnitudeSquared() > 25.0f)
	{
		IGNORE_RETURN( delta.normalize() );

		delta *= 5.0f;
	}

	ColliderList collidedWith;

	// Collide with the statics in the world
	{
		bool const restrictToSameCell = true;
		Capsule const capsule(creaturePosition_p, creaturePosition_p + delta, m_controller->getCreatureRadius());
		
		CollisionWorld::getDatabase()->queryFor(SpatialDatabase::Q_Static, m_controller->getCreatureCell(), restrictToSameCell, capsule, collidedWith);
	}

	// Check out the collision results
	// Note: this needs to check for the closest collision object from the owner's position
	{
		m_obstacleLocation = AiLocation();
		ColliderList::const_iterator iterCollider = collidedWith.begin();

		for (; iterCollider != collidedWith.end(); ++iterCollider)
		{
			CollisionProperty * const collisionProperty = *iterCollider;
			Object const & object = collisionProperty->getOwner();

			if (object.getNetworkId() != creatureOwner->getNetworkId())
			{
				// We have collided with something
	
				m_obstacleLocation = AiLocation(&object);
				break;
			}
		}
	}

	if (!m_obstacleLocation.isValid())
	{
		return false;
	}

	return updateAvoidancePoint(0.0f);
}

// ----------------------------------------------------------------------

bool AiMovementWaypoint::updateAvoidancePoint ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementWaypoint::updateAvoidancePoint");

	m_obstacleLocation.update();

	if(!m_obstacleLocation.isValid()) return false;

	Vector avoidancePoint;

	CreatureObject const * creature = m_controller->getCreature();

	Vector creaturePos = creature->getPosition_p();

	Vector waypoint = getWaypoint().getPosition_p();

	Vector delta = waypoint - creaturePos;

	Object const * obstacle = m_obstacleLocation.getObject();

	bool avoidOK = Collision3d::CalcAvoidancePoint( creature, delta, obstacle, avoidancePoint );

	if (!avoidOK)
	{
		// when avoidOk is false, the collision system could not find a valid move point
		// (AI moving into a horseshoe, etc.)
		avoidancePoint = waypoint;
	}
	else if (fabs(avoidancePoint.x - waypoint.x) < 0.01f && fabs(avoidancePoint.z - waypoint.z) < 0.01f)
	{
		// when avoidOk is true, the collision system has a valid point
		// this can happen when we are moving around an obstacle, or if we did not run into an obstacle at all
		// if we did not run into an obstacle, we increase the not stuck counter to get out of the avoid state

		if (m_avoidanceHeadedToWaypointTime > (ConfigServerGame::getBehaviorMaxAvoidanceStuckTime() * 0.1f))
		{
			//LOG("avoidance_log", ("Getting out of avoid state\n"));
			m_avoidanceHeadedToWaypointTime = 0.0f;
			// returning false will send us to the movement state if we are called from stateAvoiding
			return false;
		}

		m_avoidanceHeadedToWaypointTime += time;
	}

	//LOG("avoidance_log", ("AvoidancePoint is %1.2f %1.2f %1.2f, Waypoint is %1.2f %1.2f %1.2f\n", avoidancePoint.x, avoidancePoint.y, avoidancePoint.z, waypoint.x, waypoint.y, waypoint.z));

	avoidancePoint.y = creaturePos.y;

	m_avoidanceLocation = AiLocation(getWaypoint().getCell(),avoidancePoint);

	// Return true even if we couldn't calculate an avoidance point earlier.
	// The rest of the code needs to continue attempting to avoid the obstacle
	// so that the avoidance stuck timer can kick in.
	return true;
}

#ifdef _DEBUG
// ----------------------------------------------------------------------
void AiMovementWaypoint::addDebug(AiDebugString & aiDebugString)
{
	AiMovementBase::addDebug(aiDebugString);

	if (hasWaypoint())
	{
		aiDebugString.addLineToPosition(getWaypoint().getPosition_w(), PackedRgb::solidCyan);
	}
}
#endif // _DEBUG

// ======================================================================
