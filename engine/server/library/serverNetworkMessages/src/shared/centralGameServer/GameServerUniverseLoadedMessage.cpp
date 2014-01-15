// ======================================================================
//
// GameServerUniverseLoadedMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GameServerUniverseLoadedMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

GameServerUniverseLoadedMessage::GameServerUniverseLoadedMessage(uint32 processId, uint32 sourceOfUniverseDataProcessId) :
GameNetworkMessage("GameServerUniverseLoadedMessage"),
m_processId(processId),
m_sourceOfUniverseDataProcessId(sourceOfUniverseDataProcessId)
{
	addVariable(m_processId);
	addVariable(m_sourceOfUniverseDataProcessId);
}

// ----------------------------------------------------------------------

GameServerUniverseLoadedMessage::GameServerUniverseLoadedMessage(Archive::ReadIterator & source) :
GameNetworkMessage("GameServerUniverseLoadedMessage"),
m_processId(),
m_sourceOfUniverseDataProcessId()
{
	addVariable(m_processId);
	addVariable(m_sourceOfUniverseDataProcessId);
	unpack(source);
}

// ----------------------------------------------------------------------

GameServerUniverseLoadedMessage::~GameServerUniverseLoadedMessage()
{
}

// ======================================================================
