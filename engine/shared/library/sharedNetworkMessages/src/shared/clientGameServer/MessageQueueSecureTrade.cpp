// ======================================================================
//
// MessageQueueSecureTrade.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueSecureTrade.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueSecureTrade, CM_secureTrade);

//===================================================================




MessageQueueSecureTrade::MessageQueueSecureTrade(TradeMessageId messageType, const NetworkId & initiator, const NetworkId & recipient) :
MessageQueue::Data(),
	m_id(messageType),
	m_initiator(initiator),
	m_recipient(recipient)
{
}	

MessageQueueSecureTrade::MessageQueueSecureTrade(const MessageQueueSecureTrade & source) :
	MessageQueue::Data(),
	m_id(source.m_id),
	m_initiator(source.m_initiator),
	m_recipient(source.m_recipient)
{
}	

MessageQueueSecureTrade & MessageQueueSecureTrade::operator= (const MessageQueueSecureTrade & source)
{
	if (this == &source)
		return *this;
	m_id = source.m_id;
	m_initiator = source.m_initiator;
	m_recipient = source.m_recipient;
	return *this;
}	

MessageQueueSecureTrade::~MessageQueueSecureTrade()
{
};

// -----------------------------------------------------------

MessageQueueSecureTrade::TradeMessageId MessageQueueSecureTrade::getTradeMessageId() const
{
	return m_id;
}

// -----------------------------------------------------------

const NetworkId & MessageQueueSecureTrade::getInitiator() const
{
	return m_initiator;
}

// -----------------------------------------------------------

const NetworkId & MessageQueueSecureTrade::getRecipient() const
{
	return m_recipient;
}

// -----------------------------------------------------------


void MessageQueueSecureTrade::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueSecureTrade* const msg = safe_cast<const MessageQueueSecureTrade*> (data);
	
	if (msg)
	{
		Archive::put(target, static_cast<int>(msg->getTradeMessageId()));
		Archive::put(target, msg->getInitiator());
		Archive::put(target, msg->getRecipient());
	}
}


//-----------------------------------------------------------------------
MessageQueue::Data* MessageQueueSecureTrade::unpack(Archive::ReadIterator & source)
{
	int id;
	NetworkId initiator;
	NetworkId recipient;
	
	Archive::get(source, id);
	Archive::get(source, initiator);
	Archive::get(source, recipient);
	
	WARNING_STRICT_FATAL(id < 0 || id >= MessageQueueSecureTrade::TMI_LastTradeMessageId, ("Invalid trade message type in control message %d", id));
	
	return new MessageQueueSecureTrade(static_cast<MessageQueueSecureTrade::TradeMessageId>(id), initiator, recipient);
}


