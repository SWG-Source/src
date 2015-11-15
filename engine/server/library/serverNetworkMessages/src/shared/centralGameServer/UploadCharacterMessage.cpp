//========================================================================
//
// UploadCharacterMessage.cpp - tells Centralserver a new object is being created.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/UploadCharacterMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedMathArchive/VectorArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id		id of object that was created
 */
UploadCharacterMessage::UploadCharacterMessage(unsigned int stationId, const std::string & packedCharacterData, unsigned int gameServerId, const NetworkId & fromCharacterId, bool isAdmin) :
GameNetworkMessage("UploadCharacterMessage"),
m_stationId(stationId),
m_packedCharacterData(packedCharacterData),
m_fromGameServerId(gameServerId),
m_fromCharacterId(fromCharacterId),
m_isAdmin(isAdmin)
{
	addVariable(m_stationId);
	addVariable(m_packedCharacterData);
	addVariable(m_fromGameServerId);
	addVariable(m_fromCharacterId);
	addVariable(m_isAdmin);
}	// UploadCharacterMessage::UploadCharacterMessage

//-----------------------------------------------------------------------

UploadCharacterMessage::UploadCharacterMessage(Archive::ReadIterator & source) :
GameNetworkMessage("UploadCharacterMessage"),
m_stationId(),
m_packedCharacterData(),
m_fromGameServerId(),
m_fromCharacterId(),
m_isAdmin()
{
	addVariable(m_stationId);
	addVariable(m_packedCharacterData);
	addVariable(m_fromGameServerId);
	addVariable(m_fromCharacterId);
	addVariable(m_isAdmin);
	unpack(source);
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
UploadCharacterMessage::~UploadCharacterMessage()
{
}	// UploadCharacterMessage::~UploadCharacterMessage

//-----------------------------------------------------------------------
