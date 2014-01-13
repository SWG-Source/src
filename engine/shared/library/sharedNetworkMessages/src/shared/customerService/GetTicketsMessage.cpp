// ======================================================================
//
// GetTicketsMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GetTicketsMessage.h"

// ======================================================================

GetTicketsMessage::GetTicketsMessage(
	unsigned int start, 
	unsigned int count 
) :
GameNetworkMessage("GetTicketsMessage"),
m_start(start),
m_count(count),
m_stationId(0)
{
	addVariable(m_start);
	addVariable(m_count);
	addVariable(m_stationId);
}

// ======================================================================

GetTicketsMessage::GetTicketsMessage(Archive::ReadIterator &source) :
GameNetworkMessage("GetTicketsMessage"),
m_start(0),
m_count(0),
m_stationId(0)
{
	addVariable(m_start);
	addVariable(m_count);
	addVariable(m_stationId);
	unpack(source);
}

// ======================================================================

GetTicketsMessage::~GetTicketsMessage()
{
}
