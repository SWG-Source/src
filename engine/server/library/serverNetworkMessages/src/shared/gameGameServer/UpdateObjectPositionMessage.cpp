// ======================================================================
//
// UpdateObjectPositionMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/UpdateObjectPositionMessage.h"

// ======================================================================



UpdateObjectPositionMessage::UpdateObjectPositionMessage(NetworkId const &networkId, Transform const &transform, Transform const &worldspaceTransform, NetworkId const &containerId, int slotArrangement, NetworkId const &loadWith, bool playerControlled, bool isCreatureObject) :
	GameNetworkMessage("UpdateObjectPositionMessage"),
	m_networkId(networkId),
	m_transform(transform),
	m_worldspaceTransform(worldspaceTransform),
	m_containerId(containerId),
	m_slotArrangement(slotArrangement),
	m_loadWith(loadWith),
	m_playerControlled(playerControlled),
	m_creatureObject(isCreatureObject)
{
	addVariable(m_networkId);
	addVariable(m_transform);
	addVariable(m_worldspaceTransform);
	addVariable(m_containerId);
	addVariable(m_slotArrangement);
	addVariable(m_loadWith);
	addVariable(m_playerControlled);
	addVariable(m_creatureObject);
}

// ----------------------------------------------------------------------

UpdateObjectPositionMessage::UpdateObjectPositionMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("UpdateObjectPositionMessage"),
	m_networkId(),
	m_transform(),
	m_worldspaceTransform(),
	m_containerId(),
	m_slotArrangement(),
	m_loadWith(),
	m_playerControlled(),
	m_creatureObject()
{
	addVariable(m_networkId);
	addVariable(m_transform);
	addVariable(m_worldspaceTransform);
	addVariable(m_containerId);
	addVariable(m_slotArrangement);
	addVariable(m_loadWith);
	addVariable(m_playerControlled);
	addVariable(m_creatureObject);
	unpack(source);
}

// ======================================================================

