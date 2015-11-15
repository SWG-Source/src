// ======================================================================
//
// AiShipBehaviorBase.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorBase.h"

#include "serverGame/AiShipController.h"
#include "serverGame/SpaceSquad.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/AiDebugString.h"
#include "sharedObject/Object.h"

// ======================================================================
//
// AiShipBehaviorBase
//
// ======================================================================

// ----------------------------------------------------------------------

AiShipBehaviorBase::AiShipBehaviorBase(AiShipController & aiShipController)
 : m_aiShipController(aiShipController)
{
}

// ----------------------------------------------------------------------

AiShipBehaviorBase::AiShipBehaviorBase(AiShipBehaviorBase const & rhs)
 : m_aiShipController(rhs.m_aiShipController)
{
}

// ----------------------------------------------------------------------

AiShipBehaviorBase::~AiShipBehaviorBase()
{
}

// ----------------------------------------------------------------------

AiShipBehaviorType AiShipBehaviorBase::getBehaviorType() const
{
	return ASBT_invalid;
}

// ----------------------------------------------------------------------

char const * AiShipBehaviorBase::getBehaviorString(AiShipBehaviorType const behavior)
{
	switch (behavior)
	{
		case ASBT_idle: { return "IDLE"; }
		case ASBT_track: { return "TRACK"; }
		case ASBT_moveTo: { return "MOVETO"; }
		case ASBT_patrol: { return "PATROL"; }
		case ASBT_follow: { return "FOLLOW"; }
		case ASBT_dock: { return "DOCK"; }
		case ASBT_invalid:
		default: break;
	}

	return "INVALID";
}

// ----------------------------------------------------------------------

AiShipController & AiShipBehaviorBase::getAiShipController()
{
	return m_aiShipController;
}

// ----------------------------------------------------------------------

AiShipController const & AiShipBehaviorBase::getAiShipController() const
{
	return m_aiShipController;
}

#ifdef _DEBUG
// ----------------------------------------------------------------------

void AiShipBehaviorBase::addDebug(AiDebugString & aiDebugString)
{
	aiDebugString.addText(FormattedString<512>().sprintf("%s\n", getBehaviorString(getBehaviorType())), PackedRgb::solidCyan);
}
#endif // _DEBUG

// ======================================================================
