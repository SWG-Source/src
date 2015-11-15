//========================================================================
//
// RequestOIDsMessage.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/RequestObjectIdsMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

RequestOIDsMessage::RequestOIDsMessage(uint32 serverId, uint32 howMany, bool logRequest) :
	GameNetworkMessage("RequestOIDsMessage"),
	m_serverId(serverId),
	m_howMany(howMany),
	m_logRequest(logRequest)
{
	addVariable(m_serverId);
	addVariable(m_howMany);
	addVariable(m_logRequest);
}	

//-----------------------------------------------------------------------

RequestOIDsMessage::RequestOIDsMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("RequestOIDsMessage")
{
	addVariable(m_serverId);
	addVariable(m_howMany);
	addVariable(m_logRequest);
	unpack(source);
}

//-----------------------------------------------------------------------

RequestOIDsMessage::~RequestOIDsMessage()
{
}


//-----------------------------------------------------------------------
