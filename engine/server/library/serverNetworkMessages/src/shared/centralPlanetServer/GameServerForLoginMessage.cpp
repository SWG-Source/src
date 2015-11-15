// ======================================================================
//
// GameServerForLoginMessage.cpp
//
// Copyright 2001-2003 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GameServerForLoginMessage.h"

#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

GameServerForLoginMessage::GameServerForLoginMessage(uint32 stationId, uint32 server, const NetworkId & characterId) :
	GameNetworkMessage("GameServerForLoginMessage"),
	m_stationId(stationId),
	m_server(server),
	m_characterId(characterId)
{
	addVariable(m_stationId);
	addVariable(m_server);
	addVariable(m_characterId);
}

// ----------------------------------------------------------------------

GameServerForLoginMessage::GameServerForLoginMessage(Archive::ReadIterator &source) :
		GameNetworkMessage("GameServerForLoginMessage"),
		m_stationId(),
		m_server(),
		m_characterId()
{
	addVariable(m_stationId);
	addVariable(m_server);
	addVariable(m_characterId);

	unpack(source);
}

//-----------------------------------------------------------------------

GameServerForLoginMessage::~GameServerForLoginMessage()
{
}

// ======================================================================

