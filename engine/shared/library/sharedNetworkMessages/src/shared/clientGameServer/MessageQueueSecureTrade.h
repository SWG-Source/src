//========================================================================
//
// MessageQueueSecureTrade.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueSecureTrade_H
#define INCLUDED_MessageQueueSecureTrade_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
class MessageQueueSecureTrade : public MessageQueue::Data
{

	CONTROLLER_MESSAGE_INTERFACE;

public:
	enum TradeMessageId
	{
		TMI_RequestTrade,
		TMI_TradeRequested,
		TMI_AcceptTrade,
		TMI_DeniedTrade,
		TMI_DeniedPlayerBusy,
		TMI_DeniedPlayerUnreachable,
		TMI_RequestTradeReversed,
		TMI_LastTradeMessageId
	};


	MessageQueueSecureTrade(TradeMessageId messageType, const NetworkId & initiator, const NetworkId & recipient);
	virtual ~MessageQueueSecureTrade();

	MessageQueueSecureTrade&operator=(const MessageQueueSecureTrade & source);
	MessageQueueSecureTrade(const MessageQueueSecureTrade & source);

	TradeMessageId    getTradeMessageId() const;
	const NetworkId & getInitiator() const;
	const NetworkId & getRecipient() const;


private:
	MessageQueueSecureTrade();
	
	TradeMessageId    m_id;
	NetworkId         m_initiator; 
	NetworkId         m_recipient;

};

#endif	
