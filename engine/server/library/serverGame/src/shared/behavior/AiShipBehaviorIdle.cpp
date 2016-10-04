// ======================================================================
//
// AiShipBehaviorIdle.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorIdle.h"

#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "sharedDebug/Profiler.h"
#include "sharedLog/Log.h"

// ======================================================================
//
// AiShipBehaviorIdle
//
// ======================================================================

// ----------------------------------------------------------------------

AiShipBehaviorIdle::AiShipBehaviorIdle(AiShipController & aiShipController)
 : AiShipBehaviorBase(aiShipController)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorIdle() unit(%s)", (m_aiShipController.getOwner() != nullptr) ? m_aiShipController.getOwner()->getNetworkId().getValueString().c_str() : "nullptr owner"));
}

// ----------------------------------------------------------------------

void AiShipBehaviorIdle::alter(float const /*deltaSeconds*/)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiShipBehaviorIdle::alter");
}

// ----------------------------------------------------------------------
AiShipBehaviorType AiShipBehaviorIdle::getBehaviorType() const
{
	return ASBT_idle;
}

// ======================================================================
