// ======================================================================
//
// CancelTicketResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CancelTicketResponseMessage.h"

// ======================================================================

CancelTicketResponseMessage::CancelTicketResponseMessage(int32 result, unsigned int ticketId) :
	GameNetworkMessage("CancelTicketResponseMessage"),
	m_result(result),
	m_ticketId(ticketId)
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_ticketId);
}

// ======================================================================

CancelTicketResponseMessage::CancelTicketResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("CancelTicketResponseMessage"),
	m_result(0),
	m_ticketId(0)
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_ticketId);
	unpack(source);
}

// ======================================================================

CancelTicketResponseMessage::~CancelTicketResponseMessage()
{
}
