// ======================================================================
//
// LoginCreateCharacterAckMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoginCreateCharacterAckMessage.h"

// ======================================================================

LoginCreateCharacterAckMessage::LoginCreateCharacterAckMessage (const StationId &stationId, const NetworkId &characterNetworkId) :
		GameNetworkMessage("LoginCreateCharacterAckMessage"),
		m_stationId(stationId),
		m_characterNetworkId(characterNetworkId)
{
	addVariable(m_stationId);
	addVariable(m_characterNetworkId);
}

// ----------------------------------------------------------------------

LoginCreateCharacterAckMessage::LoginCreateCharacterAckMessage (Archive::ReadIterator & source) :
		GameNetworkMessage("LoginCreateCharacterAckMessage"),
		m_stationId(),
		m_characterNetworkId()
{
	addVariable(m_stationId);
	addVariable(m_characterNetworkId);
	
	unpack(source);
}

// ----------------------------------------------------------------------

LoginCreateCharacterAckMessage::~LoginCreateCharacterAckMessage ()
{
}

// ======================================================================
