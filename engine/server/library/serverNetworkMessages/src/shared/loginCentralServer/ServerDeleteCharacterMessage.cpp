// ======================================================================
//
// ServerDeleteCharacterMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ServerDeleteCharacterMessage.h"

// ======================================================================

ServerDeleteCharacterMessage::ServerDeleteCharacterMessage (StationId stationId, const NetworkId &characterId, uint32 loginServerId) :
		GameNetworkMessage("ServerDeleteCharacterMessage"),
		m_stationId(stationId),
		m_characterId(characterId),
		m_loginServerId(loginServerId)
{
	addVariable(m_stationId);
	addVariable(m_characterId);
	addVariable(m_loginServerId);
}

// ----------------------------------------------------------------------

ServerDeleteCharacterMessage::ServerDeleteCharacterMessage (Archive::ReadIterator & source) :
		GameNetworkMessage("ServerDeleteCharacterMessage"),
		m_stationId(),
		m_characterId(),
		m_loginServerId()
{
	addVariable(m_stationId);
	addVariable(m_characterId);
	addVariable(m_loginServerId);
	unpack(source);
}

// ----------------------------------------------------------------------

 ServerDeleteCharacterMessage::~ServerDeleteCharacterMessage ()
{
}

// ----------------------------------------------------------------------

void ServerDeleteCharacterMessage::setLoginServerId (uint32 loginServerId) 
{
	m_loginServerId.set(loginServerId);
}

// ======================================================================
