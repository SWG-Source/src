// ======================================================================
//
// ClusterWideDataRemoveElementMessage.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ClusterWideDataRemoveElementMessage.h"

// ======================================================================

std::string const ClusterWideDataRemoveElementMessage::ms_messageName = "ClusterWideDataRemoveElementMessage";

// ======================================================================

ClusterWideDataRemoveElementMessage::ClusterWideDataRemoveElementMessage(std::string const & managerName, std::string const & elementNameRegex, unsigned long const lockKey) :
	GameNetworkMessage(ms_messageName),
	m_managerName(managerName),
	m_elementNameRegex(elementNameRegex),
	m_lockKey(static_cast<uint32>(lockKey))
{
	addVariable(m_managerName);
	addVariable(m_elementNameRegex);
	addVariable(m_lockKey);
}

// ----------------------------------------------------------------------

ClusterWideDataRemoveElementMessage::ClusterWideDataRemoveElementMessage(Archive::ReadIterator & source) :
	GameNetworkMessage(ms_messageName),
	m_managerName(),
	m_elementNameRegex(),
	m_lockKey(0)
{
	addVariable(m_managerName);
	addVariable(m_elementNameRegex);
	addVariable(m_lockKey);

	unpack(source);
}

// ----------------------------------------------------------------------

ClusterWideDataRemoveElementMessage::~ClusterWideDataRemoveElementMessage()
{
}

// ======================================================================
