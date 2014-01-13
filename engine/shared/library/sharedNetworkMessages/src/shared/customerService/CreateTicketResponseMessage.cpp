// ======================================================================
//
// CreateTicketResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CreateTicketResponseMessage.h"

// ======================================================================

CreateTicketResponseMessage::CreateTicketResponseMessage(int32 result, unsigned int ticketId) :
	GameNetworkMessage("CreateTicketResponseMessage"),
	m_result(result),
	m_ticketId(ticketId)
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_ticketId);
}

// ======================================================================

CreateTicketResponseMessage::CreateTicketResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("CreateTicketResponseMessage"),
	m_result(0),
	m_ticketId(0)
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_ticketId);
	unpack(source);
}

// ======================================================================

CreateTicketResponseMessage::~CreateTicketResponseMessage()
{
}
