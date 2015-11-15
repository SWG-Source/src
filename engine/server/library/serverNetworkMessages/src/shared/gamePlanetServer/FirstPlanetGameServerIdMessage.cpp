// ======================================================================
//
// FirstPlanetGameServerIdMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/FirstPlanetGameServerIdMessage.h"

#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

FirstPlanetGameServerIdMessage::FirstPlanetGameServerIdMessage  (uint32 gameServerId) :
		GameNetworkMessage("FirstPlanetGameServerIdMessage"),
		m_gameServerId(gameServerId)
{
	addVariable(m_gameServerId);
}

// ----------------------------------------------------------------------

FirstPlanetGameServerIdMessage::FirstPlanetGameServerIdMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("FirstPlanetGameServerIdMessage"),
		m_gameServerId()
{
	addVariable(m_gameServerId);

	unpack(source);
}

//-----------------------------------------------------------------------

FirstPlanetGameServerIdMessage::~FirstPlanetGameServerIdMessage()
{
}

// ======================================================================
