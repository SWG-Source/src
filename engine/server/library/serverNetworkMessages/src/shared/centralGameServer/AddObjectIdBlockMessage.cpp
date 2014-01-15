//========================================================================
//
// AddOIDBlockMessage.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/AddObjectIdBlockMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

AddOIDBlockMessage::AddOIDBlockMessage(uint32 serverId, const NetworkId &start, const NetworkId &end, bool logRequest) :
	GameNetworkMessage("AddOIDBlockMessage"),
	m_serverId(serverId),
	m_start(start),
	m_end(end),
	m_logRequest(logRequest)
{
	addVariable(m_serverId);
	addVariable(m_start);
	addVariable(m_end);
	addVariable(m_logRequest);
}	

//-----------------------------------------------------------------------

AddOIDBlockMessage::AddOIDBlockMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("AddOIDBlockMessage")
{
	addVariable(m_serverId);
	addVariable(m_start);
	addVariable(m_end);
	addVariable(m_logRequest);
	unpack(source);
}

//-----------------------------------------------------------------------

AddOIDBlockMessage::~AddOIDBlockMessage()
{
}


//-----------------------------------------------------------------------
