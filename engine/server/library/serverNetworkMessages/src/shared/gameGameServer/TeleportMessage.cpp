// ======================================================================
//
// TeleportMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/TeleportMessage.h"

// ======================================================================

TeleportMessage::TeleportMessage(NetworkId const &actorId, std::string const &sceneId, Vector const &position_w, NetworkId const &containerId, Vector const &position_p) :
	GameNetworkMessage("TeleportMessage"),
	m_actorId(actorId),
	m_sceneId(sceneId),
	m_position_w(position_w),
	m_containerId(containerId),
	m_position_p(position_p)
{
	addVariable(m_actorId);
	addVariable(m_sceneId);
	addVariable(m_position_w);
	addVariable(m_containerId);
	addVariable(m_position_p);
}

// ----------------------------------------------------------------------

TeleportMessage::TeleportMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("TeleportMessage"),
	m_actorId(),
	m_sceneId(),
	m_position_w(),
	m_containerId(),
	m_position_p()
{
	addVariable(m_actorId);
	addVariable(m_sceneId);
	addVariable(m_position_w);
	addVariable(m_containerId);
	addVariable(m_position_p);
	unpack(source);
}

// ----------------------------------------------------------------------

TeleportMessage::~TeleportMessage()
{
}

// ======================================================================

