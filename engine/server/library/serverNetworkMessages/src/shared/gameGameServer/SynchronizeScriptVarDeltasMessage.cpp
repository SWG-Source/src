// ======================================================================
//
// SynchronizeScriptVarDeltasMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/SynchronizeScriptVarDeltasMessage.h"

// ======================================================================

SynchronizeScriptVarDeltasMessage::SynchronizeScriptVarDeltasMessage(NetworkId const &networkId, std::vector<int8> const &data) :
	GameNetworkMessage("SynchronizeScriptVarDeltasMessage"),
	m_networkId(networkId),
	m_data(data)
{
	addVariable(m_networkId);
	addVariable(m_data);
}

// ----------------------------------------------------------------------

SynchronizeScriptVarDeltasMessage::SynchronizeScriptVarDeltasMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("SynchronizeScriptVarDeltasMessage"),
	m_networkId(),
	m_data()
{
	addVariable(m_networkId);
	addVariable(m_data);
	unpack(source);
}

// ======================================================================

