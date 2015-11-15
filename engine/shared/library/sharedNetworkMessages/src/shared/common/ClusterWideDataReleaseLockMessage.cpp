// ======================================================================
//
// ClusterWideDataReleaseLockMessage.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ClusterWideDataReleaseLockMessage.h"

// ======================================================================

std::string const ClusterWideDataReleaseLockMessage::ms_messageName = "ClusterWideDataReleaseLockMessage";

// ======================================================================

ClusterWideDataReleaseLockMessage::ClusterWideDataReleaseLockMessage(std::string const & managerName, unsigned long const lockKey) :
	GameNetworkMessage(ms_messageName),
	m_managerName(managerName),
	m_lockKey(static_cast<uint32>(lockKey))
{
	addVariable(m_managerName);
	addVariable(m_lockKey);
}

// ----------------------------------------------------------------------

ClusterWideDataReleaseLockMessage::ClusterWideDataReleaseLockMessage(Archive::ReadIterator & source) :
	GameNetworkMessage(ms_messageName),
	m_managerName(),
	m_lockKey(0)
{
	addVariable(m_managerName);
	addVariable(m_lockKey);

	unpack(source);
}

// ----------------------------------------------------------------------

ClusterWideDataReleaseLockMessage::~ClusterWideDataReleaseLockMessage()
{
}

// ======================================================================
