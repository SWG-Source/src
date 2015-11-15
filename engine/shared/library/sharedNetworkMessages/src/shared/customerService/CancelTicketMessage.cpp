// ======================================================================
//
// CancelTicketMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CancelTicketMessage.h"

// ======================================================================

CancelTicketMessage::CancelTicketMessage(
	unsigned int ticketId, 
	const Unicode::String &comment 
) :
GameNetworkMessage("CancelTicketMessage"),
m_ticketId(ticketId),
m_comment(comment),
m_stationId(0)
{
	addVariable(m_ticketId);
	addVariable(m_comment);
	addVariable(m_stationId);
}

// ======================================================================

CancelTicketMessage::CancelTicketMessage(Archive::ReadIterator &source) :
GameNetworkMessage("CancelTicketMessage"),
m_ticketId(0),
m_comment(),
m_stationId(0)
{
	addVariable(m_ticketId);
	addVariable(m_comment);
	addVariable(m_stationId);
	unpack(source);
}

// ======================================================================

CancelTicketMessage::~CancelTicketMessage()
{
}
