// ======================================================================
//
// LocateObjectResponseMessage.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LocateObjectResponseMessage.h"

// ======================================================================

LocateObjectResponseMessage::LocateObjectResponseMessage(NetworkId const &targetId, NetworkId const &responseId, uint32 responsePid, Vector const &position_w, std::string const &scene, std::string const &sharedTemplateName, uint32 targetPid, std::vector<NetworkId> const &containers, bool isAuthoritative, NetworkId const &residenceOf) :
	GameNetworkMessage("LocateObjectResponseMessage"),
	m_targetId(targetId),
	m_responseId(responseId),
	m_responsePid(responsePid),
	m_position_w(position_w),
	m_scene(scene),
	m_sharedTemplateName(sharedTemplateName),
	m_targetPid(targetPid),
	m_containers(containers),
	m_isAthoritative(isAuthoritative),
	m_residenceOf(residenceOf)
{
	addVariable(m_targetId);
	addVariable(m_responseId);
	addVariable(m_responsePid);
	addVariable(m_position_w);
	addVariable(m_scene);
	addVariable(m_sharedTemplateName);
	addVariable(m_targetPid);
	addVariable(m_containers);
	addVariable(m_isAthoritative);
	addVariable(m_residenceOf);
}

// ----------------------------------------------------------------------

LocateObjectResponseMessage::LocateObjectResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("LocateObjectResponseMessage"),
	m_targetId(),
	m_responseId(),
	m_responsePid(),
	m_position_w(),
	m_scene(),
	m_sharedTemplateName(),
	m_targetPid(),
	m_containers(),
	m_isAthoritative(false),
	m_residenceOf()
{
	addVariable(m_targetId);
	addVariable(m_responseId);
	addVariable(m_responsePid);
	addVariable(m_position_w);
	addVariable(m_scene);
	addVariable(m_sharedTemplateName);
	addVariable(m_targetPid);
	addVariable(m_containers);
	addVariable(m_isAthoritative);
	addVariable(m_residenceOf);
	unpack(source);
}

// ----------------------------------------------------------------------

LocateObjectResponseMessage::~LocateObjectResponseMessage()
{
}

// ----------------------------------------------------------------------

NetworkId const &LocateObjectResponseMessage::getTargetId() const
{
	return m_targetId.get();
}

// ----------------------------------------------------------------------

NetworkId const &LocateObjectResponseMessage::getResponseId() const
{
	return m_responseId.get();
}

// ----------------------------------------------------------------------

uint32 LocateObjectResponseMessage::getResponsePid() const
{
	return m_responsePid.get();
}

// ----------------------------------------------------------------------

Vector const &LocateObjectResponseMessage::getPosition_w() const
{
	return m_position_w.get();
}

// ----------------------------------------------------------------------

std::string const &LocateObjectResponseMessage::getScene() const
{
	return m_scene.get();
}

// ----------------------------------------------------------------------

std::string const &LocateObjectResponseMessage::getSharedTemplateName() const
{
	return m_sharedTemplateName.get();
}

// ----------------------------------------------------------------------

uint32 LocateObjectResponseMessage::getTargetPid() const
{
	return m_targetPid.get();
}

// ----------------------------------------------------------------------

std::vector<NetworkId> const &LocateObjectResponseMessage::getContainers() const
{
	return m_containers.get();
}

// ----------------------------------------------------------------------

bool LocateObjectResponseMessage::getIsAuthoritative() const
{
	return m_isAthoritative.get();
}

// ----------------------------------------------------------------------

NetworkId const &LocateObjectResponseMessage::getResidenceOf() const
{
	return m_residenceOf.get();
}

// ======================================================================

