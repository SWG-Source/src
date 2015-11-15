// ======================================================================
//
// SynchronizeScriptVarsMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/SynchronizeScriptVarsMessage.h"

// ======================================================================

SynchronizeScriptVarsMessage::SynchronizeScriptVarsMessage(NetworkId const &networkId, std::vector<int8> const &data) :
	GameNetworkMessage("SynchronizeScriptVarsMessage"),
	m_networkId(networkId),
	m_data(data)
{
	addVariable(m_networkId);
	addVariable(m_data);
}

// ----------------------------------------------------------------------

SynchronizeScriptVarsMessage::SynchronizeScriptVarsMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("SynchronizeScriptVarsMessage"),
	m_networkId(),
	m_data()
{
	addVariable(m_networkId);
	addVariable(m_data);
	unpack(source);
}

// ======================================================================

