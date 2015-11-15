// ======================================================================
//
// UpdateContainmentMessage.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/UpdateContainmentMessage.h"

// ======================================================================

UpdateContainmentMessage::UpdateContainmentMessage(NetworkId const &networkId, NetworkId const &containerId, int slotArrangement) :
	GameNetworkMessage("UpdateContainmentMessage"),
	m_networkId(networkId),
	m_containerId(containerId),
	m_slotArrangement(slotArrangement)
{
	addVariable(m_networkId);
	addVariable(m_containerId);
	addVariable(m_slotArrangement);
}

// ----------------------------------------------------------------------

UpdateContainmentMessage::UpdateContainmentMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("UpdateContainmentMessage"),
	m_networkId(),
	m_containerId(),
	m_slotArrangement()
{
	addVariable(m_networkId);
	addVariable(m_containerId);
	addVariable(m_slotArrangement);
	unpack(source);
}

// ======================================================================

