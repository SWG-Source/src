// ======================================================================
//
// UnloadedPlayerMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/UnloadedPlayerMessage.h"

// ======================================================================

UnloadedPlayerMessage::UnloadedPlayerMessage(const NetworkId &playerId) :
		GameNetworkMessage("UnloadedPlayerMessage"),
		m_playerId(playerId)
{
	addVariable(m_playerId);
}

// ----------------------------------------------------------------------

UnloadedPlayerMessage::UnloadedPlayerMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("UnloadedPlayerMessage"),
		m_playerId()
{
	addVariable(m_playerId);
	unpack(source);
}

// ----------------------------------------------------------------------

UnloadedPlayerMessage::~UnloadedPlayerMessage()
{
}

// ======================================================================
