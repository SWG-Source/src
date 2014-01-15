//========================================================================
//
// CharacterTransferStatusMessage.cpp - tells Centralserver a new object is being created.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CharacterTransferStatusMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedMathArchive/VectorArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id		id of object that was created
 */
CharacterTransferStatusMessage::CharacterTransferStatusMessage(unsigned int gameServerId, const NetworkId & toCharacterId, const std::string & statusMessage) :
GameNetworkMessage("CharacterTransferStatusMessage"),
m_gameServerId(gameServerId),
m_toCharacterId(toCharacterId),
m_statusMessage(statusMessage)
{
	addVariable(m_gameServerId);
	addVariable(m_toCharacterId);
	addVariable(m_statusMessage);
}	// CharacterTransferStatusMessage::CharacterTransferStatusMessage

//-----------------------------------------------------------------------

CharacterTransferStatusMessage::CharacterTransferStatusMessage(Archive::ReadIterator & source) :
GameNetworkMessage("CharacterTransferStatusMessage"),
m_gameServerId(),
m_toCharacterId(),
m_statusMessage()
{
	addVariable(m_gameServerId);
	addVariable(m_toCharacterId);
	addVariable(m_statusMessage);
	unpack(source);
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
CharacterTransferStatusMessage::~CharacterTransferStatusMessage()
{
}	// CharacterTransferStatusMessage::~CharacterTransferStatusMessage

//-----------------------------------------------------------------------
