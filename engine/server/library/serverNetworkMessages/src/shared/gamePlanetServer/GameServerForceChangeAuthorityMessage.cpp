//========================================================================
//
// GameServerForceChangeAuthorityMessage.cpp - tells the PlanetServer a game 
// server has forced an object to change it's authoritative server.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GameServerForceChangeAuthorityMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "sharedMathArchive/TransformArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id			id of object being made authoritative
 * @param process		process that is authoritative
 */
GameServerForceChangeAuthorityMessage::GameServerForceChangeAuthorityMessage(
	const NetworkId& id, uint32 fromProcess, uint32 toProcess) :
	GameNetworkMessage("GameServerForceChangeAuthorityMessage"),
	m_id(id),
	m_fromProcess(fromProcess),
	m_toProcess(toProcess)
{
	addVariable(m_id);
	addVariable(m_fromProcess);
	addVariable(m_toProcess);
}	// GameServerForceChangeAuthorityMessage::GameServerForceChangeAuthorityMessage

//-----------------------------------------------------------------------

GameServerForceChangeAuthorityMessage::GameServerForceChangeAuthorityMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("GameServerForceChangeAuthorityMessage"),
	m_id(),
	m_fromProcess(),
	m_toProcess()
{
	addVariable(m_id);
	addVariable(m_fromProcess);
	addVariable(m_toProcess);
	unpack(source);
}	// GameServerForceChangeAuthorityMessage::GameServerForceChangeAuthorityMessage

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
GameServerForceChangeAuthorityMessage::~GameServerForceChangeAuthorityMessage()
{
}	// GameServerForceChangeAuthorityMessage::~GameServerForceChangeAuthorityMessage

//-----------------------------------------------------------------------

