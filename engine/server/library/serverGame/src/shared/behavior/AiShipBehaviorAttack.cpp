// ======================================================================
//
// AiShipBehaviorAttack.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorAttack.h"

#include "serverGame/AiShipAttackTargetList.h"
#include "serverGame/AiShipController.h"
#include "serverGame/AiShipPilotData.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceSquad.h"
#include "serverGame/SpaceAttackSquad.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/AiDebugString.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

AiShipBehaviorAttack::AiShipBehaviorAttack(AiShipController & aiShipController)
 : AiShipBehaviorBase(aiShipController)
 , m_leashRadius(aiShipController.getPilotData()->m_leashRadius)
 , m_damageTaken(false)
{
}

// ----------------------------------------------------------------------

AiShipBehaviorAttack::AiShipBehaviorAttack(AiShipBehaviorAttack const & rhs)
 : AiShipBehaviorBase(rhs)
 , m_leashRadius(rhs.m_leashRadius)
 , m_damageTaken(rhs.m_damageTaken)
{
}

// ----------------------------------------------------------------------

AiShipBehaviorAttack::~AiShipBehaviorAttack()
{
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttack::alter(float const /*deltaTime*/)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiShipBehaviorAttack::alter");
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttack::setLeashRadius(float const radius)
{
	m_leashRadius = radius;
}

// ----------------------------------------------------------------------

float AiShipBehaviorAttack::getLeashRadius() const
{
	return m_leashRadius;
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttack::isWithinLeashDistance() const
{
	Vector const & ownerPosition_w = getAiShipController().getOwnerPosition_w();
	Vector const & leashAnchorPosition_w = getAiShipController().getSquad().getLeashAnchorPosition_w();
	float const distanceFromLeashAnchorPositionSquared = ownerPosition_w.magnitudeBetweenSquared(leashAnchorPosition_w);

	return (distanceFromLeashAnchorPositionSquared < sqr(getLeashRadius()));
}

// ----------------------------------------------------------------------

float AiShipBehaviorAttack::getPercentWithinLeashDistance() const
{
	DEBUG_WARNING((getLeashRadius() <= 0.0f), ("AiShipBehaviorAttack::getPercentWithinLeashDistance() The leash radius(%f) must be > 0", getLeashRadius()));

	Vector const & ownerPosition_w = getAiShipController().getOwnerPosition_w();
	Vector const & leashAnchorPosition_w = getAiShipController().getSquad().getLeashAnchorPosition_w();

	// == 1 means the unit is on the leash anchor position
	// == 0 means the unit is on the shell of the leash sphere
	// < 0 means the unit is outside the leash sphere

	float const result = (1.0f - (leashAnchorPosition_w.magnitudeBetween(ownerPosition_w) / std::max(1.0f, getLeashRadius())));

	return result;
}

#ifdef _DEBUG
// ----------------------------------------------------------------------

void AiShipBehaviorAttack::addDebug(AiDebugString & aiDebugString)
{
	if (getAiShipController().isAttackSquadLeader())
	{
		// Show the squad leader look at target

		NetworkId const & lookAtTarget = m_aiShipController.getShipOwner()->getPilotLookAtTarget();

		if (lookAtTarget != NetworkId::cms_invalid)
		{
			aiDebugString.addLineToObject(lookAtTarget, PackedRgb::solidRed);
		}

		// Show the squad leader leash position
		
		aiDebugString.addLineToPosition(m_aiShipController.getSquad().getLeashAnchorPosition_w(), PackedRgb::solidGreen);

		// Show the squad leader leash radius

		//aiDebugString.addCircle(m_aiShipController.getLeashAnchorPosition_w(), getLeashRadius(), PackedRgb::solidGreen);
	}
}
#endif // _DEBUG

// ======================================================================
