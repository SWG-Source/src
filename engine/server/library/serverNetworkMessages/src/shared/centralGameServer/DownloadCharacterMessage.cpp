//========================================================================
//
// DownloadCharacterMessage.cpp - tells Centralserver a new object is being created.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/DownloadCharacterMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedMathArchive/VectorArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id		id of object that was created
 */
DownloadCharacterMessage::DownloadCharacterMessage(unsigned int stationId, unsigned int gameServerId, const NetworkId & toCharacterId, bool isAdmin) :
GameNetworkMessage("DownloadCharacterMessage"),
m_stationId(stationId),
m_gameServerId(gameServerId),
m_toCharacterId(toCharacterId),
m_isAdmin(isAdmin)
{
	addVariable(m_stationId);
	addVariable(m_gameServerId);
	addVariable(m_toCharacterId);
	addVariable(m_isAdmin);
}	// DownloadCharacterMessage::DownloadCharacterMessage

//-----------------------------------------------------------------------

DownloadCharacterMessage::DownloadCharacterMessage(Archive::ReadIterator & source) :
GameNetworkMessage("DownloadCharacterMessage"),
m_stationId(),
m_gameServerId(),
m_toCharacterId(),
m_isAdmin()
{
	addVariable(m_stationId);
	addVariable(m_gameServerId);
	addVariable(m_toCharacterId);
	addVariable(m_isAdmin);
	unpack(source);
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
DownloadCharacterMessage::~DownloadCharacterMessage()
{
}	// DownloadCharacterMessage::~DownloadCharacterMessage

//-----------------------------------------------------------------------
