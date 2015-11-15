// ======================================================================
//
// AiMovementFollow.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementFollow.h"

#include "serverGame/AiMovementArchive.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/SpatialDatabase.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/AiDebugString.h"

using namespace Scripting;

extern float computeMovementModifier (CreatureObject* const object);

// ======================================================================

AiMovementFollow::AiMovementFollow( AICreatureController * controller )
:	AiMovementTarget(controller),
	m_offsetTarget(),
	m_minDistance(0.0f),
	m_maxDistance(1.0f)
{
	CHANGE_STATE( AiMovementFollow::stateWaiting );
}

AiMovementFollow::AiMovementFollow( AICreatureController * controller, ServerObject const * target, float minDistance, float maxDistance )
:	AiMovementTarget(controller, target),
	m_offsetTarget(),
	m_minDistance(minDistance),
	m_maxDistance(maxDistance)
{
	DEBUG_FATAL(target->getNetworkId() == m_controller->getOwner()->getNetworkId(),("AiMovementFollow - Creature is trying to follow itself\n"));

	// The specified follow distance needs to be the distance between
	// the 2 object's collision spheres

	float const radius = Random::randomReal(minDistance, maxDistance) + target->getRadius() + m_controller->getCreatureRadius();

	m_target.setRadius(radius);

	if(reachedLocation(m_target))
	{
		CHANGE_STATE( AiMovementFollow::stateWaiting );

		triggerWaiting();
	}
}

// ----------

AiMovementFollow::AiMovementFollow( AICreatureController * controller, ServerObject const * target, Vector const & offset, bool relativeOffset, float minDistance, float maxDistance )
:	AiMovementTarget(controller, target),
	m_offsetTarget(target,offset,relativeOffset),
	m_minDistance(minDistance),
	m_maxDistance(maxDistance)
{
	DEBUG_FATAL(target->getNetworkId() == m_controller->getOwner()->getNetworkId(),("AiMovementFollow - Creature is trying to follow itself\n"));

	m_target.setRadius( m_maxDistance );
	m_offsetTarget.setRadius( m_maxDistance );

	if(reachedLocation(m_offsetTarget))
	{
		CHANGE_STATE( AiMovementFollow::stateWaiting  );

		triggerWaiting();
	}
}

// ----------

AiMovementFollow::AiMovementFollow( AICreatureController * controller, Archive::ReadIterator & source )
:	AiMovementTarget(controller, source),
	m_offsetTarget(source),
	m_minDistance(0.0f),
	m_maxDistance(1.0f)
{
	Archive::get(source, m_minDistance);
	Archive::get(source, m_maxDistance);

	SETUP_SYNCRONIZED_STATE( AiMovementFollow::stateWaiting );
	SETUP_SYNCRONIZED_STATE( AiMovementFollow::stateMoving );
}

// ----------

AiMovementFollow::~AiMovementFollow()
{
}

// ----------------------------------------------------------------------

void AiMovementFollow::pack( Archive::ByteStream & target ) const
{
	AiMovementTarget::pack(target);
	m_offsetTarget.pack(target);
	Archive::put(target, m_minDistance);
	Archive::put(target, m_maxDistance);
}

// ----------------------------------------------------------------------

void AiMovementFollow::alter ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementFollow::alter");

	m_target.update();
	m_offsetTarget.update();

	if(!m_target.isValid())
	{
		triggerTargetLost();

		return;
	}

	if(targetTooFar())
	{
		triggerTargetLost();

		return;
	}

	if(m_controller->getStuckCounter() >= 5)
	{
		if(hasWaypoint())
		{
			warpToLocation(getWaypoint());
		}
	}

	if(m_offsetTarget.isValid())
	{
		if(m_offsetTarget.hasChanged())
		{
			addPathNode(m_offsetTarget);

			m_offsetTarget.setChanged(false);
		}
	}
	else
	{
		if(m_target.hasChanged())
		{
			addPathNode(m_target);

			m_target.setChanged(false);
		}
	}

	// For following to work correctly, the path cutting in AiMovementPathFollow::alter needs to
	// not treat the object we're following as an obstacle

	CollisionWorld::getDatabase()->pushIgnoreObject(m_target.getObject());

	AiMovementTarget::alter(time);

	CollisionWorld::getDatabase()->popIgnoreObject();
}

// ----------------------------------------------------------------------

void AiMovementFollow::refresh( void )
{
	AiMovementTarget::refresh();
	CHANGE_STATE( AiMovementFollow::stateWaiting );
}

// ----------------------------------------------------------------------

void AiMovementFollow::getDebugInfo ( std::string & outString ) const
{
	AiMovementTarget::getDebugInfo(outString);

	Vector const & goalPosition_w = m_target.getPosition_w();
	outString += FormattedString<1024>().sprintf("AiMovementFollow: following(%d) position(%.2f,%.2f,%.2f)", m_target.getObjectId().getHashValue(), goalPosition_w.x, goalPosition_w.y, goalPosition_w.z);

	if (m_offsetTarget.isValid())
	{
		Vector const & offset = m_offsetTarget.getOffset();

		outString += FormattedString<1024>().sprintf(" offset(%.2f,%.2f,%.2f)", offset.x, offset.y, offset.z);
	}

	outString += FormattedString<1024>().sprintf(" distance[%.2f...%.2f]", m_minDistance, m_maxDistance);
}

// ----------------------------------------------------------------------
	
AiStateResult AiMovementFollow::stateWaiting ( float time )
{
	m_target.update();
	m_offsetTarget.update();

	if (!m_target.isValid())
	{
		return triggerTargetLost();
	}

	if (getPathLength() > 1)
	{
		return triggerMoving();
	}

	// Follow behavior doesn't transition from waiting to moving until the AI
	// is no longer in range of its target
	{
		if (m_offsetTarget.isValid())
		{
			if (!reachedLocation(m_offsetTarget))
			{
				return triggerMoving();
			}
			else
			{
				return ASR_Done;
			}
		}
		else
		{
			// mbogue: I am trying to understand this legacy code and I have
			// run into an interesting problem here. These 2 if checks are redundant.
			// If the first check is true, the second check should be true.
			// Somehow the second check is not true and I need to track down why
			// the m_target is able to move without adding a new path location.
			// Without the reachedLocation() check it causes the AI to toggle
			// between Moving and Waiting states for no useful reason.



			if (getPathLength() > 0)
			{
				if (!reachedLocation(m_target))
				{
					return triggerMoving();
				}
				else
				{
					return ASR_Done;
				}
			}
			else
			{
				// It is possible that something has moved us away from our target.
				// If that is the case, we need to re-path to the target and resume moving.
				if (!reachedLocation(m_target))
				{
					if (!targetTooFar())
					{
						addPathNode(m_target);
						return triggerMoving();
					}
					else
					{
						//whatever it is has moved us too far away from our target
						return triggerTargetLost();
					}
				}
			}
		}
	}

	return ASR_Done;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementFollow::stateMoving ( float time )
{
	if (m_offsetTarget.isValid())
	{
		// If there's only one node in the path, and we're within range of the target,
		// go back to waiting state (we don't need to walk to the node).

		if(getPathLength() <= 1)
		{
			AiLocation temp = m_offsetTarget;

			temp.setRadius(m_minDistance);

			if(reachedLocation(temp))
			{
				return triggerWaiting();
			}
		}
	}

	return AiMovementTarget::stateMoving(time);
}

// ----------------------------------------------------------------------

AiStateResult AiMovementFollow::triggerWaiting ( void )
{
	m_controller->triggerScriptsWithTarget(TRIG_FOLLOW_WAITING,m_target.getObjectId());

	return AiMovementTarget::triggerWaiting();
}

// ----------------------------------------------------------------------

AiStateResult AiMovementFollow::triggerMoving ( void )
{
	m_controller->triggerScriptsWithTarget(TRIG_FOLLOW_MOVING,m_target.getObjectId());

	return AiMovementTarget::triggerMoving();
}

// ----------------------------------------------------------------------

AiStateResult AiMovementFollow::triggerTargetLost ( void )
{
	m_controller->triggerScriptsWithTarget(TRIG_FOLLOW_TARGET_LOST,m_target.getObjectId());

	endBehavior();
	
	return AiMovementTarget::triggerTargetLost();
}

// ----------------------------------------------------------------------

bool AiMovementFollow::updateWaypoint ( void )
{
	return true;
}

// ----------------------------------------------------------------------

bool AiMovementFollow::targetTooFar ( void ) const
{
	Vector const & creaturePos_w = m_controller->getOwner()->getPosition_w();
	Vector const & targetPos_w = m_target.getPosition_w();

	return creaturePos_w.magnitudeBetweenSquared(targetPos_w) > sqr(ConfigServerGame::getMaxCombatRange() * 2.0f);
}

// ----------------------------------------------------------------------

AiMovementType AiMovementFollow::getType() const
{
	return AMT_follow;
}

// ----------------------------------------------------------------------

AiLocation const & AiMovementFollow::getOffsetTarget() const
{
	return m_offsetTarget;
}

// ----------------------------------------------------------------------

float AiMovementFollow::getMinDistance() const
{
	return m_minDistance;
}

// ----------------------------------------------------------------------

float AiMovementFollow::getMaxDistance() const
{
	return m_maxDistance;
}

// ----------------------------------------------------------------------

AiMovementFollow * AiMovementFollow::asAiMovementFollow()
{
	return this;
}

// ----------------------------------------------------------------------

AiLocation const & AiMovementFollow::getFinalWaypoint() const
{
	if (m_offsetTarget.isValid())
	{
		return AiLocation::invalid;
	}

	return m_target;
}

#ifdef _DEBUG
// ----------------------------------------------------------------------
void AiMovementFollow::addDebug(AiDebugString & aiDebugString)
{
	FormattedString<512> fs;
	AiPath const * const path = getPath();

	if (m_offsetTarget.isValid())
	{
		aiDebugString.addText(fs.sprintf("FOLLOW OFFSET\n"), PackedRgb::solidCyan);
		Vector const & offsetPosition_p = m_offsetTarget.getOffset_p();
		aiDebugString.addText(fs.sprintf("offset(%.1f, %.1f, %.1f)\n", offsetPosition_p.x, offsetPosition_p.y, offsetPosition_p.z), PackedRgb::solidCyan);
		aiDebugString.addText(fs.sprintf("offsetTarget(%s)\n", m_offsetTarget.getObjectId().getValueString().c_str()), PackedRgb::solidCyan);
	}
	else
	{
		aiDebugString.addText(fs.sprintf("FOLLOW\n"), PackedRgb::solidCyan);
	}
	
	aiDebugString.addText(fs.sprintf("%s\n", m_stateName.c_str()), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("target(%s)\n", m_target.getObjectId().getValueString().c_str()), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("distance[%.1f...%.1f]\n", m_minDistance, m_maxDistance), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("pathSize(%u)\n", path->size()), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("stuckCounter(%i)\n", m_controller->getStuckCounter()), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("avoidanceStuckTime(%.0f)\n", m_avoidanceStuckTime), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("obstacle(%s)\n", m_obstacleLocation.getObjectId().getValueString().c_str()), PackedRgb::solidCyan);

	if (!path->empty())
	{
		AiDebugString::TransformList transformList;
		AiPath::const_iterator iterPath = path->begin();

		Transform transform(m_controller->getOwner()->getTransform_o2p());
		transform.move_l(Vector(0.0f, 0.1f, 0.0f));
		transformList.push_back(transform);

		for (; iterPath != path->end(); ++iterPath)
		{
			transform.setPosition_p(iterPath->getPosition_p());
			transform.move_l(Vector(0.0f, 0.1f, 0.0f));
			transformList.push_back(transform);
		}

		bool const cyclic = false;
		aiDebugString.addPath(transformList, cyclic);
	}
}
#endif // _DEBUG

// ======================================================================
