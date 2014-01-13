// ======================================================================
//
// DestroyShipMessage.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/DestroyShipMessage.h"

// ======================================================================

char const * const DestroyShipMessage::MessageType = "DestroyShipMessage";

//----------------------------------------------------------------------

DestroyShipMessage::DestroyShipMessage(NetworkId const &shipId, float severity) :
GameNetworkMessage(MessageType),
m_shipId(shipId),
m_severity(severity)
{
	addVariable(m_shipId);
	addVariable(m_severity);
}

// ----------------------------------------------------------------------

DestroyShipMessage::DestroyShipMessage(Archive::ReadIterator &source) :
GameNetworkMessage(MessageType),
m_shipId(),
m_severity(0.0f)
{
	addVariable(m_shipId);
	addVariable(m_severity);

	unpack(source);
}


//----------------------------------------------------------------------

NetworkId const &DestroyShipMessage::getShipId() const
{
	return m_shipId.get();
}

//----------------------------------------------------------------------

float DestroyShipMessage::getSeverity() const
{
	return m_severity.get();
}

//----------------------------------------------------------------------
