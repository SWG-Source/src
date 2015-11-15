// ======================================================================
//
// GetCommentsMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GetCommentsMessage.h"

// ======================================================================

GetCommentsMessage::GetCommentsMessage(
	unsigned int ticketId 
) :
GameNetworkMessage("GetCommentsMessage"),
m_ticketId(ticketId)
{
	addVariable(m_ticketId);
}

// ======================================================================

GetCommentsMessage::GetCommentsMessage(Archive::ReadIterator &source) :
GameNetworkMessage("GetCommentsMessage"),
m_ticketId(0)
{
	addVariable(m_ticketId);
	unpack(source);
}

// ======================================================================

GetCommentsMessage::~GetCommentsMessage()
{
}
