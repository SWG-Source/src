// ======================================================================
//
// AiShipBehaviorFollow.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorFollow.h"

#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/Formation.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceSquad.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedLog/Log.h"

// ======================================================================
//
// AiShipBehaviorFollowNamespace
//
// ======================================================================

// ----------------------------------------------------------------------
namespace AiShipBehaviorFollowNamespace
{
	float slowDownRequestRadiusGain = 0.9f;
}

using namespace AiShipBehaviorFollowNamespace;

// ======================================================================
//
// AiShipBehaviorFollow
//
// ======================================================================

// ----------------------------------------------------------------------
AiShipBehaviorFollow::AiShipBehaviorFollow(AiShipController & aiShipController, NetworkId const & followedUnit)
 : AiShipBehaviorBase(aiShipController)
 , m_followedUnit(followedUnit)
 , m_followedUnitLost(false)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorFollow::AiShipBehaviorFollow() unit(%s) followedUnit(%s)", (m_aiShipController.getOwner() != nullptr) ? m_aiShipController.getOwner()->getNetworkId().getValueString().c_str() : "nullptr owner", followedUnit.getValueString().c_str()));

	DEBUG_WARNING((m_followedUnit.getObject() == nullptr), ("Trying to follow a nullptr object."));
}

// ----------------------------------------------------------------------
void AiShipBehaviorFollow::alter(float const deltaSeconds)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiShipBehaviorFollow::alter");

	// Get the goal position

	Object * const followedUnitObject = m_followedUnit.getObject();
	Vector goalPosition_w;

	if (followedUnitObject != nullptr)
	{
		goalPosition_w = Formation::getPosition_w(followedUnitObject->getTransform_o2w(), m_aiShipController.getFormationPosition_l());

		// Determine if the unit being followed is far enough away to request it to slow down

		if (m_aiShipController.getOwner()->getPosition_w().magnitudeBetweenSquared(goalPosition_w) > sqr(m_aiShipController.getLargestTurnRadius() * slowDownRequestRadiusGain))
		{
			ShipController * const shipController = followedUnitObject->getController()->asShipController();
			AiShipController * const aiShipController = (shipController != nullptr) ? shipController->asAiShipController() : nullptr;

			if (aiShipController != nullptr)
			{
				aiShipController->requestSlowDown();
			}
		}
	}
	else
	{
		if (!m_followedUnitLost)
		{
			m_followedUnitLost = true;

			triggerFollowedUnitLost();
		}

		// Handle the case where we could not resolve the goal position

		Object const * const object = m_aiShipController.getOwner();

		if (object != nullptr)
		{
			goalPosition_w = object->getPosition_w();
		}
	}

	// Calculate the throttle

	float const throttle = m_aiShipController.calculateThrottleToPosition_w(goalPosition_w, m_aiShipController.getLargestTurnRadius());

	// Move the ship

	m_aiShipController.moveTo(goalPosition_w, throttle, deltaSeconds);
}

// ----------------------------------------------------------------------
void AiShipBehaviorFollow::triggerFollowedUnitLost()
{
	Object * object = m_aiShipController.getOwner();
	ServerObject *serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
	GameScriptObject * gameScriptObject = (serverObject != nullptr) ? serverObject->getScriptObject() : nullptr;

	if(gameScriptObject != nullptr)
	{
		ScriptParams scriptParams;
		scriptParams.addParam(m_followedUnit);
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SPACE_UNIT_FOLLOW_LOST, scriptParams));
	}
	else
	{
		WARNING(true, ("Unable to get the ScriptObject for this object."));
	}
}

// ----------------------------------------------------------------------
AiShipBehaviorType AiShipBehaviorFollow::getBehaviorType() const
{
	return ASBT_follow;
}

// ======================================================================
