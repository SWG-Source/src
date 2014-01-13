// ======================================================================
//
// GetTicketsResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GetTicketsResponseMessage.h"

// ======================================================================

GetTicketsResponseMessage::GetTicketsResponseMessage(
	int32 result, 
	unsigned int totalNumTickets,
	const std::vector<CustomerServiceTicket> &tickets
) :
	GameNetworkMessage("GetTicketsResponseMessage"),
	m_result(result),
	m_totalNumTickets(totalNumTickets),
	m_tickets()
{
	m_tickets.set(tickets);
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_totalNumTickets);
	AutoByteStream::addVariable(m_tickets);
}

// ======================================================================

GetTicketsResponseMessage::GetTicketsResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("GetTicketsResponseMessage"),
	m_result(0),
	m_totalNumTickets(0),
	m_tickets()
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_totalNumTickets);
	AutoByteStream::addVariable(m_tickets);
	unpack(source);
}

// ======================================================================

GetTicketsResponseMessage::~GetTicketsResponseMessage()
{
}
