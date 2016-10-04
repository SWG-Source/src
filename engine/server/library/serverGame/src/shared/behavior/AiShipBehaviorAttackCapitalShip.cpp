// ======================================================================
//
// AiShipBehaviorAttackCapitalShip.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorAttackCapitalShip.h"

#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "sharedDebug/Profiler.h"
#include "sharedLog/Log.h"

// ======================================================================

AiShipBehaviorAttackCapitalShip::AiShipBehaviorAttackCapitalShip(AiShipController & aiShipController)
		: AiShipBehaviorAttack(aiShipController)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorAttackCapitalShip::AiShipBehaviorAttackCapitalShip() unit(%s)", (m_aiShipController.getOwner() != nullptr) ? m_aiShipController.getOwner()->getNetworkId().getValueString().c_str() : "nullptr owner"));
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackCapitalShip::~AiShipBehaviorAttackCapitalShip()
{
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackCapitalShip::alter(float const /*deltaSeconds*/)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiShipBehaviorAttackCapitalShip::alter");
}

// ======================================================================
