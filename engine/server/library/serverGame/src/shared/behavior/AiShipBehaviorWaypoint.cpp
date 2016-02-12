// ======================================================================
//
// AiShipBehaviorWaypoint.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorWaypoint.h"

#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpacePath.h"
#include "serverGame/SpacePathManager.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/AiDebugString.h"
#include "sharedLog/Log.h"
#include "sharedTerrain/TerrainObject.h"

// ======================================================================
//
// AiShipBehaviorWaypoint
//
// ======================================================================

// ----------------------------------------------------------------------

AiShipBehaviorWaypoint::AiShipBehaviorWaypoint(AiShipController & aiShipController, bool const cyclic)
 : AiShipBehaviorBase(aiShipController)
 , m_cyclic(cyclic)
 , m_moveToCompleteTriggerSent(false)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorWaypoint::AiShipBehaviorWaypoint() unit(%s) cyclic(%s)", (aiShipController.getOwner() != nullptr) ? aiShipController.getOwner()->getNetworkId().getValueString().c_str() : "nullptr owner", cyclic ? "yes" : "no"));
}

// ----------------------------------------------------------------------

void AiShipBehaviorWaypoint::alter(float deltaSeconds)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiShipBehaviorWaypoint::alter");

	// Get the goal position

	Vector goalPosition_w;
	{
		if (!getNextPosition_w(goalPosition_w))
		{
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorWaypoint::alter() unit(%s) Why do we have a path with no points in it?", m_aiShipController.getOwner()->getNetworkId().getValueString().c_str()));
			goalPosition_w = m_aiShipController.getOwnerPosition_w();
		}
	}

	// Calculate the throttle

	float throttle = 1.0f;
	{
		SpacePath const * const path = m_aiShipController.getPath();

		if (   (path != nullptr)
			&& !m_cyclic
			&& (m_aiShipController.getCurrentPathIndex() == (path->getTransformList().size() - 1)))
		{
			throttle = m_aiShipController.calculateThrottleToPosition_w(goalPosition_w, m_aiShipController.getLargestTurnRadius());

			// See if we need to send the move to complete trigger

			if (   !m_moveToCompleteTriggerSent
				&& m_aiShipController.getOwnerPosition_w().magnitudeBetweenSquared(goalPosition_w) < sqr(m_aiShipController.getLargestTurnRadius()))
			{
				m_moveToCompleteTriggerSent = true;

				GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(m_aiShipController.getOwner());

				if(gameScriptObject != nullptr)
				{
					ScriptParams scriptParams;
					IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SPACE_UNIT_MOVE_TO_COMPLETE, scriptParams));

					LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorWaypoint::alter() unit(%s) Trigger sent: TRIG_SPACE_UNIT_MOVE_TO_COMPLETE", m_aiShipController.getOwner()->getDebugInformation().c_str()));
				}
				else
				{
					DEBUG_WARNING(true, ("debug_ai: Unable to get the ScriptObject for this object(%s)", m_aiShipController.getOwner()->getDebugInformation().c_str()));
				}
			}
		}
	}

	// Move the ship

	m_aiShipController.moveTo(goalPosition_w, throttle, deltaSeconds);
}

// ----------------------------------------------------------------------

bool AiShipBehaviorWaypoint::getNextPosition_w(Vector & position_w)
{
	bool result = false;

	SpacePath * const path = m_aiShipController.getPath();

	if (   (path != nullptr)
	    && !path->isEmpty()
	    && (m_aiShipController.getCurrentPathIndex() < path->getTransformList().size()))
	{
		ShipObject const * const shipObject = m_aiShipController.getShipOwner();

		if (shipObject != nullptr)
		{
			SpacePath::TransformList const & transformList = path->getTransformList();
			Vector const & nextPosition = getGoalPosition_w();
			float const distanceToGoalSquared = shipObject->getPosition_w().magnitudeBetweenSquared(nextPosition);

			// If the distance to the goal position is less than a specified amount,
			// the calculate the next position using a spline to round out the transition
			// to the next point.

			float const turnRadiusSquared = sqr(m_aiShipController.getLargestTurnRadius());

			if (distanceToGoalSquared < turnRadiusSquared)
			{
				// Move to the next path point

				if (m_cyclic)
				{
					if ((m_aiShipController.getCurrentPathIndex() + 1) >= transformList.size())
					{
						m_aiShipController.setCurrentPathIndex(0);
					}
					else
					{
						m_aiShipController.setCurrentPathIndex(m_aiShipController.getCurrentPathIndex() + 1);
					}
				}
				else
				{
					if (m_aiShipController.getCurrentPathIndex() < (transformList.size() - 1))
					{
						m_aiShipController.setCurrentPathIndex(m_aiShipController.getCurrentPathIndex() + 1);
					}
				}
			}

			result = true;
			position_w = getGoalPosition_w();
		}
	}
	else
	{
		m_aiShipController.setCurrentPathIndex(0);
	}

	return result;
}

// ----------------------------------------------------------------------

Vector AiShipBehaviorWaypoint::getGoalPosition_w() const
{
	Vector result;

	SpacePath * const path = m_aiShipController.getPath();

	if (   (path != nullptr)
	    && !path->isEmpty())
	{
		SpacePath::TransformList const & transformList = path->getTransformList();
		result = transformList[m_aiShipController.getCurrentPathIndex()].getPosition_p();

		m_aiShipController.clampPositionToZone(result);
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorWaypoint::getGoalPosition_w() unit(%s) ERROR: Getting a goal position on an empty path.", m_aiShipController.getOwner()->getNetworkId().getValueString().c_str()));
	}

	return result;
}

//// ----------------------------------------------------------------------
//
//Vector AiShipBehaviorWaypoint::getPreviousGoalPosition() const
//{
//	Vector result;
//
//	if (!path.empty())
//	{
//		if ((m_currentPathIndex - 1) >= path.size())
//		{
//			result = path[path.size() - 1].getPosition_p();
//		}
//		else
//		{
//			result = path[m_currentPathIndex - 1].getPosition_p();
//		}
//	}
//
//	return result;
//}
//
//// ----------------------------------------------------------------------
//
//Vector AiShipBehaviorWaypoint::getNextGoalPosition() const
//{
//	Vector result;
//
//	if (!path.empty())
//	{
//		if ((m_currentPathIndex + 1) >= path.size())
//		{
//			result = path[0].getPosition_p();
//		}
//		else
//		{
//			result = path[m_currentPathIndex + 1].getPosition_p();
//		}
//	}
//
//	return result;
//}

// ----------------------------------------------------------------------
AiShipBehaviorType AiShipBehaviorWaypoint::getBehaviorType() const
{
	AiShipBehaviorType result = ASBT_moveTo;

	if (m_cyclic)
	{
		result = ASBT_patrol;
	}

	return result;
}

#ifdef _DEBUG
// ----------------------------------------------------------------------
void AiShipBehaviorWaypoint::addDebug(AiDebugString & aiDebugString)
{
	if (m_aiShipController.isSquadLeader())
	{
		SpacePath const * const path = m_aiShipController.getPath();

		if (path != nullptr)
		{
			aiDebugString.addLineToPosition(m_aiShipController.getMoveToGoalPosition_w(), PackedRgb::solidGreen);
			aiDebugString.addCircle(m_aiShipController.getMoveToGoalPosition_w(), m_aiShipController.getLargestTurnRadius(), PackedRgb::solidGreen);
			aiDebugString.addPath(path->getTransformList(), m_cyclic);

			char const * const text = FormattedString<256>().sprintf("%s [PATH SIZE %u]\n", getBehaviorString(getBehaviorType()), path->getTransformList().size());
			aiDebugString.addText(text, PackedRgb::solidCyan);
		}
		else
		{
			char const * const text = FormattedString<256>().sprintf("%s [ERROR: NO PATH]\n", getBehaviorString(getBehaviorType()));
			aiDebugString.addText(text, PackedRgb::solidCyan);
		}
	}
	else
	{
		AiShipBehaviorBase::addDebug(aiDebugString);
	}
}
#endif // _DEBUG

// ======================================================================
