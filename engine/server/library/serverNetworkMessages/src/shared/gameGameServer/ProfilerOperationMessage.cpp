// ======================================================================
//
// ProfilerOperationMessage.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ProfilerOperationMessage.h"

// ======================================================================

ProfilerOperationMessage::ProfilerOperationMessage(uint32 processId, std::string const &operation) :
	GameNetworkMessage("ProfilerOperationMessage"),
	m_processId(processId),
	m_operation(operation)
{
	addVariable(m_processId);
	addVariable(m_operation);
}

// ----------------------------------------------------------------------

ProfilerOperationMessage::ProfilerOperationMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("ProfilerOperationMessage"),
	m_processId(),
	m_operation()
{
	addVariable(m_processId);
	addVariable(m_operation);
	unpack(source);
}

// ----------------------------------------------------------------------

ProfilerOperationMessage::~ProfilerOperationMessage()
{
}

// ======================================================================

