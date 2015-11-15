// ======================================================================
//
// FrameEndMessage.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/FrameEndMessage.h"

// ======================================================================

FrameEndMessage::FrameEndMessage(unsigned int pid, unsigned long frameTime, char const *profilerData) :
	GameNetworkMessage("FrameEndMessage"),
	m_processId(pid),
	m_frameTime(frameTime),
	m_profilerData(profilerData)
{
	addVariable(m_processId);
	addVariable(m_frameTime);
	addVariable(m_profilerData);
}

// ----------------------------------------------------------------------

FrameEndMessage::FrameEndMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("FrameEndMessage"),
	m_processId(),
	m_frameTime(),
	m_profilerData()
{
	addVariable(m_processId);
	addVariable(m_frameTime);
	addVariable(m_profilerData);
	
	unpack(source);
}

// ----------------------------------------------------------------------

FrameEndMessage::~FrameEndMessage()
{
}

// ======================================================================

