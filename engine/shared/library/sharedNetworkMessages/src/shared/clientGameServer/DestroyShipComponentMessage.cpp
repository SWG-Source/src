// ======================================================================
//
// DestroyShipComponentMessage.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/DestroyShipComponentMessage.h"

// ======================================================================

char const * const DestroyShipComponentMessage::MessageType = "DestroyShipComponentMessage";

//----------------------------------------------------------------------

DestroyShipComponentMessage::DestroyShipComponentMessage(NetworkId const &shipId, int chassisSlot, float severity) :
GameNetworkMessage(MessageType),
m_shipId(shipId),
m_chassisSlot(chassisSlot),
m_severity(severity)
{
	addVariable(m_shipId);
	addVariable(m_chassisSlot);
	addVariable(m_severity);
}

// ----------------------------------------------------------------------

DestroyShipComponentMessage::DestroyShipComponentMessage(Archive::ReadIterator &source) :
GameNetworkMessage(MessageType),
m_shipId(),
m_chassisSlot(0),
m_severity(0.0f)
{
	addVariable(m_shipId);
	addVariable(m_chassisSlot);
	addVariable(m_severity);

	unpack(source);
}


//----------------------------------------------------------------------

NetworkId const &DestroyShipComponentMessage::getShipId() const
{
	return m_shipId.get();
}

//----------------------------------------------------------------------

int DestroyShipComponentMessage::getChassisSlot() const
{
	return m_chassisSlot.get();
}

//----------------------------------------------------------------------

float DestroyShipComponentMessage::getSeverity() const
{
	return m_severity.get();
}

//----------------------------------------------------------------------
