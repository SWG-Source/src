// ======================================================================
//
// ClusterWideDataGetElementMessage.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ClusterWideDataGetElementMessage.h"

// ======================================================================

std::string const ClusterWideDataGetElementMessage::ms_messageName = "ClusterWideDataGetElementMessage";
unsigned long ClusterWideDataGetElementMessage::ms_requestIdCounter = 0;

// ======================================================================

ClusterWideDataGetElementMessage::ClusterWideDataGetElementMessage(std::string const & managerName, std::string const & elementNameRegex, bool const lockElements) :
	GameNetworkMessage(ms_messageName),
	m_managerName(managerName),
	m_elementNameRegex(elementNameRegex),
	m_lockElements(lockElements),
	m_requestId(static_cast<uint32>(++ms_requestIdCounter))
{
	addVariable(m_managerName);
	addVariable(m_elementNameRegex);
	addVariable(m_lockElements);
	addVariable(m_requestId);
}

// ----------------------------------------------------------------------

ClusterWideDataGetElementMessage::ClusterWideDataGetElementMessage(Archive::ReadIterator & source) :
	GameNetworkMessage(ms_messageName),
	m_managerName(),
	m_elementNameRegex(),
	m_lockElements(false),
	m_requestId(0)
{
	addVariable(m_managerName);
	addVariable(m_elementNameRegex);
	addVariable(m_lockElements);
	addVariable(m_requestId);

	unpack(source);
}

// ----------------------------------------------------------------------

ClusterWideDataGetElementMessage::~ClusterWideDataGetElementMessage()
{
}

// ======================================================================
