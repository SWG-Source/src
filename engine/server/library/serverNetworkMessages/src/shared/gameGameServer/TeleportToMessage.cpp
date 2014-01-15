// ======================================================================
//
// TeleportToMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/TeleportToMessage.h"

// ======================================================================

TeleportToMessage::TeleportToMessage(NetworkId const &actorId, NetworkId const &targetId, uint32 processId) :
	GameNetworkMessage("TeleportToMessage"),
	m_actorId(actorId),
	m_targetId(targetId),
	m_processId(processId)
{
	addVariable(m_actorId);
	addVariable(m_targetId);
	addVariable(m_processId);
}

// ----------------------------------------------------------------------

TeleportToMessage::TeleportToMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("TeleportToMessage"),
	m_actorId(),
	m_targetId(),
	m_processId()
{
	addVariable(m_actorId);
	addVariable(m_targetId);
	addVariable(m_processId);
	unpack(source);
}

// ----------------------------------------------------------------------

TeleportToMessage::~TeleportToMessage()
{
}

// ======================================================================

