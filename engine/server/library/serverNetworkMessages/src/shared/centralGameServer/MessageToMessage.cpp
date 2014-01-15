// ======================================================================
//
// MessageToMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/MessageToMessage.h"
#include "sharedNetwork/NetworkHandler.h"

// ======================================================================

MessageToMessage::MessageToMessage(MessageToPayload const &data, uint32 sourceServerPid) :
	GameNetworkMessage("MessageToMessage"),
	m_data(data),
	m_sourceServerPid(sourceServerPid)
{
	addVariable(m_data);
	addVariable(m_sourceServerPid);
	std::string messageName = "MessageToPayload." + m_data.get().getMethod();
	NetworkHandler::reportMessage(messageName, m_data.get().getPackedDataVector().size());
}

//-----------------------------------------------------------------------

MessageToMessage::MessageToMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("MessageToMessage"),
	m_data(),
	m_sourceServerPid()
{
	addVariable(m_data);
	addVariable(m_sourceServerPid);
	unpack(source);
	std::string messageName = "MessageToPayload." + m_data.get().getMethod();
	NetworkHandler::reportMessage(messageName, m_data.get().getPackedDataVector().size());
}

//-----------------------------------------------------------------------

MessageToMessage::~MessageToMessage()
{
}

// ======================================================================

