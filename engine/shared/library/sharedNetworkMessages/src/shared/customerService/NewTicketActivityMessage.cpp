// ======================================================================
//
// NewTicketActivityMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/NewTicketActivityMessage.h"

// ======================================================================

NewTicketActivityMessage::NewTicketActivityMessage()
 : GameNetworkMessage("NewTicketActivityMessage")
 , m_stationId(0)
{
	addVariable(m_stationId);
}

// ======================================================================

NewTicketActivityMessage::NewTicketActivityMessage(Archive::ReadIterator &source)
 : GameNetworkMessage("NewTicketActivityMessage")
 , m_stationId(0)
{
	addVariable(m_stationId);
	unpack(source);
}

// ======================================================================

NewTicketActivityMessage::~NewTicketActivityMessage()
{
}
