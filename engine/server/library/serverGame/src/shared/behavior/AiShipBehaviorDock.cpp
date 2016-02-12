// ======================================================================
//
// AiShipBehaviorDock.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorDock.h"

#include "serverGame/AiShipController.h"
#include "serverGame/AiShipPilotData.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceDockingManager.h"
#include "serverGame/SpaceSquad.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/SpaceAvoidanceManager.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/AiDebugString.h"
#include "sharedGame/CollisionCallbackManager.h"
#include "sharedLog/Log.h"
#include "sharedMath/AxialBox.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================
//
// AiShipBehaviorDockNamespace
//
// ======================================================================

namespace AiShipBehaviorDockNamespace
{
	float s_tractorBeamErrorDistance = 1.0f;
}

using namespace AiShipBehaviorDockNamespace;

// ======================================================================
//
// AiShipBehaviorDock
//
// ======================================================================

// ----------------------------------------------------------------------
AiShipBehaviorDock::AiShipBehaviorDock(ShipController & shipController, ShipObject & dockTarget, float const secondsAtDock)
 : m_shipController(shipController)
 , m_dockTarget(dockTarget)
 , m_timeAtDockTimer(secondsAtDock)
 , m_approachHardPointList(new HardPointList)
 , m_initialApproachHardPointCount(0)
 , m_exitHardPointList(new HardPointList)
 , m_dockHardPoint()
 , m_landingHardPoint()
 , m_hasLandingHardPoint(shipController.getOwner()->getAppearance()->findHardpoint(TemporaryCrcString("landing1", false), m_landingHardPoint))
 , m_dockingState(DS_approach)
 , m_dockingStateTime(0.0f)
 , m_infiniteDockTime(secondsAtDock < 0.0f)
 , m_dockFinished(false)
 , m_goalPosition_w()
 , m_wingsOpenedBeforeDock(m_shipController.getShipOwner()->hasWings() && m_shipController.getShipOwner()->wingsOpened())
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock: [1/6] unit(%s) dockTarget(%s) secondsAtDock(%.2f) landingHardPoint(%s)", (shipController.getOwner() != nullptr) ? shipController.getOwner()->getNetworkId().getValueString().c_str() : "nullptr owner", dockTarget.getNetworkId().getValueString().c_str(), secondsAtDock, m_hasLandingHardPoint ? "yes" : "no"));

	if (m_shipController.isBeingDocked())
	{
		DEBUG_WARNING(true, ("debug_ai: AiShipBehaviorDock() unit(%s) dockTarget(%s) ERROR: We can't dock with someone if we are currently being docked.", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));

		// Fail the docking

		m_dockTarget = CachedNetworkId::cms_cachedInvalid;
	}

	if (m_shipController.getOwner()->getNetworkId() == dockTarget.getNetworkId())
	{
		DEBUG_WARNING(true, ("debug_ai: AiShipBehaviorDock() unit(%s) dockTarget(%s) ERROR: Why is the ship trying to dock with itself?", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));

		// Fail the docking

		m_dockTarget = CachedNetworkId::cms_cachedInvalid;
	}

	if (m_dockTarget != CachedNetworkId::cms_cachedInvalid)
	{
		SpaceDockingManager::fetchDockingProcedure(*m_shipController.getOwner(), dockTarget, m_dockHardPoint, *m_approachHardPointList, *m_exitHardPointList);
	}

	if (   (m_dockTarget != CachedNetworkId::cms_cachedInvalid)
	    && m_approachHardPointList->empty())
	{
		DEBUG_WARNING(true, ("debug_ai: AiShipBehaviorDock() unit(%s) dockTarget(%s) ERROR Why is the approach hard point list empty?", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));

		// Fail the docking

		SpaceDockingManager::releaseDockingProcedure(m_shipController.getOwner()->getNetworkId(), m_dockTarget);
		m_dockTarget = CachedNetworkId::cms_cachedInvalid;
	}

	if (   (m_dockTarget != CachedNetworkId::cms_cachedInvalid)
	    && m_exitHardPointList->empty())
	{
		DEBUG_WARNING(true, ("debug_ai: AiShipBehaviorDock() unit(%s) dockTarget(%s) ERROR Why is the exit hard point list empty?", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));

		// Fail the docking

		SpaceDockingManager::releaseDockingProcedure(m_shipController.getOwner()->getNetworkId(), m_dockTarget);
		m_dockTarget = CachedNetworkId::cms_cachedInvalid;
	}

	if (m_dockTarget != CachedNetworkId::cms_cachedInvalid)
	{
		// Notify the ship that we want to dock with it

		ShipController * const dockTargetShipController = dockTarget.getController()->asShipController();

		if (dockTargetShipController != nullptr)
		{
			dockTargetShipController->addDockedBy(*shipController.getOwner());
		}
		else
		{
			DEBUG_WARNING(true, ("debug_ai: AiShipBehaviorDock() ERROR: Why does this dockTarget(%s) not have a ship controller?", dockTarget.getDebugInformation().c_str()));

			// Fail the docking

			SpaceDockingManager::releaseDockingProcedure(m_shipController.getOwner()->getNetworkId(), m_dockTarget);
			m_dockTarget = CachedNetworkId::cms_cachedInvalid;
		}
	}

	if (m_dockTarget != CachedNetworkId::cms_cachedInvalid)
	{
		// Turn off collision for the auth-client at this point

		if (m_shipController.getShipOwner()->isPlayerShip())
		{
			m_shipController.appendMessage(CM_addIgnoreIntersect, 0.0f, new MessageQueueGenericValueType<NetworkId>(m_dockTarget), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}

	m_initialApproachHardPointCount = static_cast<int>(m_approachHardPointList->size());

	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock() unit(%s) dockTarget(%s) approachHardPointCount(%u) exitHardPointCount(%u)", (shipController.getOwner() != nullptr) ? shipController.getOwner()->getNetworkId().getValueString().c_str() : "nullptr owner", dockTarget.getNetworkId().getValueString().c_str(), m_approachHardPointList->size(), m_exitHardPointList->size()));
}

// ----------------------------------------------------------------------
AiShipBehaviorDock::~AiShipBehaviorDock()
{
	ShipObject * const ownerShipObject = m_shipController.getShipOwner();

	// Open the wings

	if (   (ownerShipObject != nullptr)
	    && m_wingsOpenedBeforeDock)
	{
		m_shipController.getShipOwner()->openWings();
	}

	// Turn collision back on

	if (m_dockTarget != CachedNetworkId::cms_cachedInvalid)
	{
		CollisionCallbackManager::removeIgnoreIntersect(m_shipController.getOwner()->getNetworkId(), m_dockTarget);
		
		if (    (ownerShipObject != nullptr)
		     && ownerShipObject->isPlayerShip())
		{
			m_shipController.appendMessage(CM_removeIgnoreIntersect, 0.0f, new MessageQueueGenericValueType<NetworkId>(m_dockTarget), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}

	// Notify the dock target that we are done docking with it

	Object * const dockTarget = m_dockTarget.getObject();

	if (dockTarget != nullptr)
	{
		ShipController * const dockTargetShipController = dockTarget->getController()->asShipController();

		if (dockTargetShipController != nullptr)
		{
			dockTargetShipController->removeDockedBy(*m_shipController.getOwner());
		}
	}

	delete m_approachHardPointList;
	delete m_exitHardPointList;
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::alter(float deltaSeconds)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiShipBehaviorDock::alter");

	bool abortDocking = false;

	if (m_dockTarget.getObject() == nullptr)
	{
		// If we lose the dock target, fail the docking procedure.

		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock::alter() unit(%s) dockTarget(%s) DOCK TARGET LOST...UNDOCKING", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));
		abortDocking = true;
	}
	else
	{
		// If the dock target is an AI ship and it begins attacking, stop the docking procedure.

		AiShipController * const dockTargetAiShipController = AiShipController::asAiShipController(m_dockTarget.getObject()->getController());

		if (   (dockTargetAiShipController != nullptr)
			&& dockTargetAiShipController->isAttacking())
		{
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock::alter() unit(%s) dockTarget(%s) DOCK TARGET IS ATTACKING...UNDOCKING", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));
			abortDocking = true;
		}
	}

	if (abortDocking)
	{
		if (m_timeAtDockTimer.isExpired())
		{
			triggerUnDockWithSuccess();
		}
		else
		{
			triggerUnDockWithFailure();
		}
	}
	else
	{
		// Continue docking

		m_dockingStateTime += deltaSeconds;

		switch (m_dockingState)
		{
			case DS_approach: { approach(deltaSeconds); } break;
			case DS_approachTractorBeam: { approachTractorBeam(deltaSeconds); } break;
			case DS_landing: { landing(deltaSeconds); } break;
			case DS_docked: { docked(deltaSeconds); } break;
			case DS_exitTractorBeam: { unDock(deltaSeconds); } break;
			default: { FATAL(true, ("Unexpected docking procedure")); } break;
		}
	}
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::approach(float const deltaSeconds)
{
	setApproachPosition();

	float const distanceToGoalSquared = m_shipController.getOwner()->getPosition_w().magnitudeBetweenSquared(m_goalPosition_w);
	float const turnRadius = m_shipController.getLargestTurnRadius();

	if (distanceToGoalSquared <= sqr(turnRadius))
	{
		// Start the approach tractor beam

		m_dockingState = DS_approachTractorBeam;

		// Turn off collision

		CollisionCallbackManager::addIgnoreIntersect(m_shipController.getOwner()->getNetworkId(), m_dockTarget);
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock: [2/6] unit(%s) dockTarget(%s) TRACTOR BEAM", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));
	}
	else
	{
		// Move the unit towards the first approach position

		m_shipController.setThrottle(1.0f);

		Vector resultAvoidancePosition_w;
		Vector const & shipVelocity = m_shipController.getShipOwner()->getCurrentVelocity_p();
		
		if (SpaceAvoidanceManager::getAvoidancePosition(*m_shipController.getOwner(), shipVelocity, m_goalPosition_w, resultAvoidancePosition_w))
		{
			IGNORE_RETURN(m_shipController.face(resultAvoidancePosition_w, deltaSeconds));
		}
		else
		{
			IGNORE_RETURN(m_shipController.face(m_goalPosition_w, deltaSeconds));
		}
	}

	// Close the wings

	if (m_wingsOpenedBeforeDock)
	{
		m_shipController.getShipOwner()->closeWings();
	}
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::approachTractorBeam(float const deltaSeconds)
{
	m_shipController.setThrottle(0.0f);

	setApproachPosition();

	Vector const & ownerPosition_w = m_shipController.getOwner()->getPosition_w();
	float const distanceToGoalSquared = ownerPosition_w.magnitudeBetweenSquared(m_goalPosition_w);

	if (   m_approachHardPointList->empty()
	    && (distanceToGoalSquared < sqr(s_tractorBeamErrorDistance)))
	{
		if (alignWithDock(deltaSeconds))
		{
			// Start the landing tractor beam

			m_dockingState = DS_landing;

			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock: [3/6] unit(%s) dockTarget(%s) LANDING", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));
		}
	}
	else
	{
		if (distanceToGoalSquared < sqr(s_tractorBeamErrorDistance))
		{
			Vector const forward_w(m_dockTarget.getObject()->rotateTranslate_o2w(m_approachHardPointList->back().rotateTranslate_l2p(Vector(100000.0f, 0.0f, 0.0f))));
			Vector const up_w(m_dockTarget.getObject()->rotateTranslate_o2w(m_approachHardPointList->back().rotateTranslate_l2p(Vector(0.0f, 0.0f, -100000.0f))));

			if (m_shipController.face(forward_w, up_w, deltaSeconds))
			{
				// Move to the next approach point

				if (!m_approachHardPointList->empty())
				{
					m_approachHardPointList->pop_back();
				}
			}
		}
		else
		{
			bool const allowOrientation = (m_initialApproachHardPointCount == static_cast<int>(m_approachHardPointList->size()));
			tractorBeamToGoalPosition(deltaSeconds, allowOrientation);

			if (allowOrientation)
			{
				IGNORE_RETURN(m_shipController.face(m_goalPosition_w, deltaSeconds));
			}
		}
	}
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::landing(float const deltaSeconds)
{
	m_shipController.setThrottle(0.0f);
	setDockPosition();

	Vector const & ownerPosition_w = m_shipController.getOwner()->getPosition_w();
	float const distanceToGoalSquared = ownerPosition_w.magnitudeBetweenSquared(m_goalPosition_w);

	if (distanceToGoalSquared < sqr(s_tractorBeamErrorDistance))
	{
		triggerDocked();
	}

	if (m_dockingState == DS_landing)
	{
		bool const allowOrientation = false;
		tractorBeamToGoalPosition(deltaSeconds, allowOrientation);
	}
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::docked(float const deltaSeconds)
{
	m_shipController.setThrottle(0.0f);

	// Run the dock timer

	if (   !m_infiniteDockTime
	    && m_timeAtDockTimer.updateNoReset(deltaSeconds))
	{
		triggerStartUnDock();
		unDock();
	}
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::unDock(float const deltaSeconds)
{
	if (!m_exitHardPointList->empty())
	{
		setExitPosition();
	}

	// Lerp the ship to the goal position

	Vector const & ownerPosition_w = m_shipController.getOwner()->getPosition_w();
	float const distanceToGoalSquared = ownerPosition_w.magnitudeBetweenSquared(m_goalPosition_w);

	if (distanceToGoalSquared < sqr(s_tractorBeamErrorDistance))
	{
		if (m_exitHardPointList->empty())
		{
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock: [6/6] unit(%s) dockTarget(%s) UNDOCKED", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));

			if (m_timeAtDockTimer.isExpired())
			{
				triggerUnDockWithSuccess();
			}
			else
			{
				// The docking was stopped prematurely

				triggerUnDockWithFailure();
			}

			m_dockFinished = true;
		}
		else
		{
			m_exitHardPointList->pop_front();
		}
	}

	if (m_dockingState == DS_exitTractorBeam)
	{
		bool const allowOrientation = false;
		tractorBeamToGoalPosition(deltaSeconds, allowOrientation);
	}
}

// ----------------------------------------------------------------------
bool AiShipBehaviorDock::alignWithDock(float const deltaSeconds)
{
	CollisionProperty const * const ownerCollisionProperty = NON_NULL(m_shipController.getOwner()->getCollisionProperty());
	AxialBox const ownerAxialBox = ownerCollisionProperty->getExtent_l()->getBoundingBox();
	Vector forward_l;
	Vector up_l;
	float const width = ownerAxialBox.getWidth();
	float const height = ownerAxialBox.getHeight();
	float const depth = ownerAxialBox.getDepth();

	if (   (width < height)
		&& (width < depth))
	{
		forward_l = Vector::unitX;
		up_l = Vector::unitY;
	}
	else if (height <= depth)
	{
		forward_l = Vector::unitX;
		up_l = -Vector::unitZ;
	}
	else
	{
		forward_l = Vector::unitZ;
		up_l = Vector::unitY;
	}

	Vector const forward_w(m_dockTarget.getObject()->rotateTranslate_o2w(m_dockHardPoint.rotateTranslate_l2p(forward_l * 100000.0f)));
	Vector const up_w(m_dockTarget.getObject()->rotateTranslate_o2w(m_dockHardPoint.rotateTranslate_l2p(up_l * 100000.0f)));
	return m_shipController.face(forward_w, up_w, deltaSeconds);
}

// ----------------------------------------------------------------------
bool AiShipBehaviorDock::isDocked() const
{
	return (m_dockingState == DS_docked);
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::triggerDocked()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock: [4/6] unit(%s) dockTarget(%s) timeAtDock(%.2f) DOCKED", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str(), m_timeAtDockTimer.getExpireTime()));

	m_dockingState = DS_docked;
	m_dockingStateTime = 0.0f;

	GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(m_shipController.getOwner());

	if(gameScriptObject != nullptr)
	{
		ScriptParams scriptParams;
		scriptParams.addParam(m_dockTarget);
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SPACE_UNIT_DOCKED, scriptParams));

		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock::triggerDocked() unit(%s) dockTarget(%s)", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));
	}
	else
	{
		DEBUG_WARNING(true, ("debug_ai: AiShipBehaviorDock::triggerDocked() ERROR: Unable to get the ScriptObject for this object(%s)", m_shipController.getOwner()->getDebugInformation().c_str()));
	}
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::triggerStartUnDock()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock: [5/6] unit(%s) dockTarget(%s) START UNDOCK", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));

	GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(m_shipController.getOwner());

	if(gameScriptObject != nullptr)
	{
		ScriptParams scriptParams;
		scriptParams.addParam(m_dockTarget);
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SPACE_UNIT_START_UNDOCK, scriptParams));

		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock::triggerStartUnDock() unit(%s) dockTarget(%s)", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));
	}
	else
	{
		DEBUG_WARNING(true, ("debug_ai: AiShipBehaviorDock::triggerStartUnDock() ERROR: Unable to get the ScriptObject for this object(%s)", m_shipController.getOwner()->getDebugInformation().c_str()));
	}
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::triggerUnDockWithSuccess()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock::triggerUnDockWithSuccess() unit(%s) dockTarget(%s)", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));

	GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(m_shipController.getOwner());

	if(gameScriptObject != nullptr)
	{
		ScriptParams scriptParams;
		scriptParams.addParam(m_dockTarget);
		scriptParams.addParam(true);
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SPACE_UNIT_UNDOCKED, scriptParams));
	}
	else
	{
		DEBUG_WARNING(true, ("debug_ai: AiShipBehaviorDock::triggerUnDockWithSuccess() ERROR: Unable to get the ScriptObject for this object(%s)", m_shipController.getOwner()->getDebugInformation().c_str()));
	}
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::triggerUnDockWithFailure()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock::triggerUnDockWithFailure() unit(%s) dockTarget(%s)", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));

	m_dockFinished = true;

	GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(m_shipController.getOwner());

	if(gameScriptObject != nullptr)
	{
		ScriptParams scriptParams;
		scriptParams.addParam(m_dockTarget);
		scriptParams.addParam(false);
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SPACE_UNIT_UNDOCKED, scriptParams));
	}
	else
	{
		DEBUG_WARNING(true, ("debug_ai: AiShipBehaviorDock::triggerUnDockWithFailure() ERROR: Unable to get the ScriptObject for this object(%s)", m_shipController.getOwner()->getDebugInformation().c_str()));
	}
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::unDock()
{
	// If the ship was still getting to the approach point, immediatelly fail the dock, if the ship was in the middle of the docking procedure, send the ship out the exit path and release it

	if (m_dockingState == DS_approach)
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock::unDock() unit(%s) dockTarget(%s) ABORTING IMMEDIATELLY", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));

		triggerUnDockWithFailure();
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock::unDock() unit(%s) dockTarget(%s) GOING TO EXIT PATH", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));

		m_dockingState = DS_exitTractorBeam;
	}
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::setApproachPosition()
{
	if (!m_approachHardPointList->empty())
	{
		Transform const & transform = m_approachHardPointList->back();

		m_goalPosition_w = m_dockTarget.getObject()->rotateTranslate_o2w(transform.getPosition_p());
	}
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::setDockPosition()
{
	m_goalPosition_w = m_dockTarget.getObject()->rotateTranslate_o2w(m_dockHardPoint.getPosition_p());
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::setExitPosition()
{
	if (!m_exitHardPointList->empty())
	{
		m_goalPosition_w = m_dockTarget.getObject()->rotateTranslate_o2w(m_exitHardPointList->front().getPosition_p());
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipBehaviorDock::setExitPosition() unit(%s) dockTarget(%s) Why is the exit hard point list empty?", m_shipController.getOwner()->getDebugInformation().c_str(), m_dockTarget.getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------
bool AiShipBehaviorDock::isDockFinished() const
{
	return m_dockFinished;
}

// ----------------------------------------------------------------------
CachedNetworkId const & AiShipBehaviorDock::getDockTarget() const
{
	return m_dockTarget;
}

// ----------------------------------------------------------------------
float AiShipBehaviorDock::getMaxTractorBeamSpeed() const
{
	float const shipActualSpeedMaximum = m_shipController.getShipOwner()->getShipActualSpeedMaximum();
	AiShipController * const aiShipController = m_shipController.asAiShipController();

	return (aiShipController != nullptr) ? (shipActualSpeedMaximum * aiShipController->getPilotData()->m_nonCombatMaxSpeedPercent) : shipActualSpeedMaximum;
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::tractorBeamToGoalPosition(float const deltaSeconds, bool const allowOrientation)
{
	// Lerp the ship to the goal position, but only as fast as the ship can actually move

	float const maxTractorBeamSpeed = getMaxTractorBeamSpeed() * deltaSeconds;

	// See if we are close enough to the goal position

	Vector const & ownerPosition_w = m_shipController.getOwner()->getPosition_w();

	if (ownerPosition_w.magnitudeBetween(m_goalPosition_w) <= maxTractorBeamSpeed)
	{
		m_shipController.setPosition_w(m_goalPosition_w, allowOrientation);
	}
	else
	{
		// Still tractor beaming towards the goal position

		Vector directionToGoalPosition(m_goalPosition_w - ownerPosition_w);
		directionToGoalPosition.normalize();
		Vector const newPosition(ownerPosition_w + (directionToGoalPosition * maxTractorBeamSpeed));
	
		m_shipController.setPosition_w(newPosition, allowOrientation);
	}
}

#ifdef _DEBUG
// ----------------------------------------------------------------------
char const * AiShipBehaviorDock::getDockingStateString(DockingState const dockingState)
{
	switch (dockingState)
	{
		case DS_approach: { return "APPROACH"; }
		case DS_approachTractorBeam: { return "APPROACH TRACTOR BEAM"; }
		case DS_landing: { return "LANDING"; }
		case DS_docked: { return "DOCKED"; }
		case DS_exitTractorBeam: { return "EXIT TRACTOR BEAM"; }
		default: break;
	};

	return "";
}

// ----------------------------------------------------------------------
void AiShipBehaviorDock::addDebug(AiDebugString & aiDebugString)
{
	aiDebugString.addText(FormattedString<512>().sprintf("DOCKING[%s]\n", getDockingStateString(m_dockingState)), PackedRgb::solidCyan);

	aiDebugString.addLineToPosition(m_goalPosition_w, PackedRgb::solidCyan);

	if (m_dockTarget.getObject() != nullptr)
	{
		Transform transform;
		transform.multiply(m_dockTarget.getObject()->getTransform_o2w(), m_dockHardPoint);
		aiDebugString.addAxis(transform, 40.0f);
	}
}
#endif // _DEBUG

// ======================================================================
