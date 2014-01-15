// ======================================================================
//
// LocatePlayerResponseMessage.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LocatePlayerResponseMessage.h"

// ======================================================================

LocatePlayerResponseMessage::LocatePlayerResponseMessage(NetworkId const &targetId, NetworkId const &responseId, uint32 responsePid, std::string const &scene, Vector const &position_w, uint32 targetPid) :
	GameNetworkMessage("LocatePlayerResponseMessage"),
	m_targetId(targetId),
	m_responseId(responseId),
	m_responsePid(responsePid),
	m_scene(scene),
	m_position_w(position_w),
	m_targetPid(targetPid)
{
	addVariable(m_targetId);
	addVariable(m_responseId);
	addVariable(m_responsePid);
	addVariable(m_scene);
	addVariable(m_position_w);
	addVariable(m_targetPid);
}

// ----------------------------------------------------------------------

LocatePlayerResponseMessage::LocatePlayerResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("LocatePlayerResponseMessage"),
	m_targetId(),
	m_responseId(),
	m_responsePid(),
	m_scene(),
	m_position_w(),
	m_targetPid()
{
	addVariable(m_targetId);
	addVariable(m_responseId);
	addVariable(m_responsePid);
	addVariable(m_scene);
	addVariable(m_position_w);
	addVariable(m_targetPid);
	unpack(source);
}

// ----------------------------------------------------------------------

LocatePlayerResponseMessage::~LocatePlayerResponseMessage()
{
}

// ----------------------------------------------------------------------

NetworkId const &LocatePlayerResponseMessage::getTargetId() const
{
	return m_targetId.get();
}

// ----------------------------------------------------------------------

NetworkId const &LocatePlayerResponseMessage::getResponseId() const
{
	return m_responseId.get();
}

// ----------------------------------------------------------------------

uint32 LocatePlayerResponseMessage::getResponsePid() const
{
	return m_responsePid.get();
}

// ----------------------------------------------------------------------

Vector const &LocatePlayerResponseMessage::getPosition_w() const
{
	return m_position_w.get();
}

// ----------------------------------------------------------------------

std::string const &LocatePlayerResponseMessage::getScene() const
{
	return m_scene.get();
}

// ----------------------------------------------------------------------

uint32 LocatePlayerResponseMessage::getTargetPid() const
{
	return m_targetPid.get();
}

// ======================================================================

