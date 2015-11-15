//========================================================================
//
// ExcommunicateGameServerMessage.cpp
//
// copyright 2003 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ExcommunicateGameServerMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

ExcommunicateGameServerMessage::ExcommunicateGameServerMessage(uint32 serverId, uint32 pid, const std::string & h) :
		GameNetworkMessage("ExcommunicateGameServerMessage"),
		m_serverId(serverId),
		m_processId(pid),
		m_hostName(h)
{
	addVariable(m_serverId);
	addVariable(m_processId);
	addVariable(m_hostName);
}

//-----------------------------------------------------------------------

ExcommunicateGameServerMessage::ExcommunicateGameServerMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ExcommunicateGameServerMessage"),
		m_serverId()
{
	addVariable(m_serverId);
	addVariable(m_processId);
	addVariable(m_hostName);
	unpack(source);
}

//-----------------------------------------------------------------------

ExcommunicateGameServerMessage::~ExcommunicateGameServerMessage()
{
}

// ======================================================================
