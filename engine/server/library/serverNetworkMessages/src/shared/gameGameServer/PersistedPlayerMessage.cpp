// ======================================================================
//
// PersistedPlayerMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/PersistedPlayerMessage.h"

// ======================================================================

PersistedPlayerMessage::PersistedPlayerMessage(const NetworkId &playerId) :
		GameNetworkMessage("PersistedPlayerMessage"),
		m_playerId(playerId)
{
	addVariable(m_playerId);
}

// ----------------------------------------------------------------------

PersistedPlayerMessage::PersistedPlayerMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("PersistedPlayerMessage"),
		m_playerId()
{
	addVariable(m_playerId);
	unpack(source);
}

// ----------------------------------------------------------------------

PersistedPlayerMessage::~PersistedPlayerMessage()
{
}

// ======================================================================
