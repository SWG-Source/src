// ======================================================================
//
// MessageToAckMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/MessageToAckMessage.h"

// ======================================================================

MessageToAckMessage::MessageToAckMessage(const NetworkId &messageId) :
		GameNetworkMessage("MessageToAckMessage"),
		m_messageId(messageId)
{
	addVariable(m_messageId);
}

//-----------------------------------------------------------------------

MessageToAckMessage::MessageToAckMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("MessageToAckMessage"),
		m_messageId()
{
	addVariable(m_messageId);
	unpack(source);
}

//-----------------------------------------------------------------------

MessageToAckMessage::~MessageToAckMessage()
{
}

// ======================================================================
