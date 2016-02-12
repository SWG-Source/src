// ======================================================================
//
// AiShipBehaviorTrack.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorTrack.h"

#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "sharedDebug/Profiler.h"
#include "sharedLog/Log.h"
#include "sharedObject/Object.h"

// ======================================================================
//
// AiShipBehaviorTrack
//
// ======================================================================

// ----------------------------------------------------------------------

AiShipBehaviorTrack::AiShipBehaviorTrack(AiShipController & aiShipController, Object const & target)
 : AiShipBehaviorBase(aiShipController)
 , m_target(&target)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorTrack() unit(%s) target(%s)", (m_aiShipController.getOwner() != nullptr) ? m_aiShipController.getOwner()->getNetworkId().getValueString().c_str() : "nullptr owner", target.getNetworkId().getValueString().c_str()));
}

// ----------------------------------------------------------------------

void AiShipBehaviorTrack::alter(float deltaSeconds)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiShipBehaviorTrack::alter");

	if (m_target != nullptr)
	{
		IGNORE_RETURN(m_aiShipController.face(m_target->getPosition_w(), deltaSeconds));
	}
}

// ----------------------------------------------------------------------
AiShipBehaviorType AiShipBehaviorTrack::getBehaviorType() const
{
	return ASBT_track;
}

// ======================================================================
