// ======================================================================
//
// FileManifest.h
// Copyright 2005 Sony Online Entertainment
// All Rights Reserved.
// Author: Vijay Thakkar
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/VerifyAndLockName.h"

//====================================================================

VerifyAndLockNameRequest::VerifyAndLockNameRequest(uint32 stationId, const NetworkId & characterId, const std::string& templateName, const Unicode::String& characterName, uint32 gameFeatures) :
GameNetworkMessage("VerifyAndLockNameRequest"),
m_stationId(stationId),
m_characterId(characterId),
m_templateName(templateName),
m_characterName(characterName),
m_gameFeatures(gameFeatures)
{
	addVariable(m_stationId);
	addVariable(m_characterId);
	addVariable(m_templateName);
	addVariable(m_characterName);
	addVariable(m_gameFeatures);
}

//-----------------------------------------------------------------------

VerifyAndLockNameRequest::VerifyAndLockNameRequest(Archive::ReadIterator & source) :
GameNetworkMessage("VerifyAndLockNameRequest"),
m_stationId(),
m_characterId(),
m_templateName(),
m_characterName(),
m_gameFeatures()
{
	addVariable(m_stationId);
	addVariable(m_characterId);
	addVariable(m_templateName);
	addVariable(m_characterName);
	addVariable(m_gameFeatures);

	unpack(source);
}

//-----------------------------------------------------------------------

VerifyAndLockNameRequest::~VerifyAndLockNameRequest()
{
}

//-----------------------------------------------------------------------

VerifyAndLockNameResponse::VerifyAndLockNameResponse(uint32 stationId, const Unicode::String& characterName, const StringId &errorMessage) :
GameNetworkMessage("VerifyAndLockNameResponse"),
m_stationId(stationId),
m_characterName(characterName),
m_errorMessage(errorMessage)
{
	addVariable(m_stationId);
	addVariable(m_characterName);
	addVariable(m_errorMessage);
}

//-----------------------------------------------------------------------

VerifyAndLockNameResponse::VerifyAndLockNameResponse(Archive::ReadIterator & source) :
GameNetworkMessage("VerifyAndLockNameResponse"),
m_stationId(),
m_characterName(),
m_errorMessage()
{
	addVariable(m_stationId);
	addVariable(m_characterName);
	addVariable(m_errorMessage);

	unpack(source);
}

//-----------------------------------------------------------------------

VerifyAndLockNameResponse::~VerifyAndLockNameResponse()
{
}

// ======================================================================





