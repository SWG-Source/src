// ======================================================================
//
// NewTicketActivityResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/NewTicketActivityResponseMessage.h"

// ======================================================================

NewTicketActivityResponseMessage::NewTicketActivityResponseMessage(bool const newActivity, unsigned int const ticketCount)
 : GameNetworkMessage("NewTicketActivityResponseMessage")
 , m_newActivity(newActivity)
 , m_ticketCount(ticketCount)
{
	AutoByteStream::addVariable(m_newActivity);
	AutoByteStream::addVariable(m_ticketCount);
}

// ======================================================================

NewTicketActivityResponseMessage::NewTicketActivityResponseMessage(Archive::ReadIterator &source)
 : GameNetworkMessage("NewTicketActivityResponseMessage")
 , m_newActivity(false)
 , m_ticketCount(0)
{
	AutoByteStream::addVariable(m_newActivity);
	AutoByteStream::addVariable(m_ticketCount);
	unpack(source);
}

// ======================================================================

NewTicketActivityResponseMessage::~NewTicketActivityResponseMessage()
{
}
