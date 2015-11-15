//========================================================================
//
// GameServerReadyMessage.cpp - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GameServerReadyMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

GameServerReadyMessage::GameServerReadyMessage(int mapWidth) :
		GameNetworkMessage("GameServerReadyMessage"),
		m_mapWidth(mapWidth)
{
	addVariable(m_mapWidth);
}

//-----------------------------------------------------------------------

GameServerReadyMessage::GameServerReadyMessage(Archive::ReadIterator & source) :
GameNetworkMessage("GameServerReadyMessage")
{
	addVariable(m_mapWidth);
	
	unpack(source);
}

//-----------------------------------------------------------------------

GameServerReadyMessage::~GameServerReadyMessage()
{
}

// ----------------------------------------------------------------------

int GameServerReadyMessage::getMapWidth() const
{
	return m_mapWidth.get();
}

// ======================================================================
