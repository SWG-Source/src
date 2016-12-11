// ======================================================================
//
// MessageQueueObjectMenuRequest.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueObjectMenuRequest.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(MessageQueueObjectMenuRequest);

//===================================================================

void MessageQueueObjectMenuRequest::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_objectMenuRequest, pack, unpack, true); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_objectMenuResponse, pack, unpack); 
	ExitChain::add(&remove, "MessageQueueObjectMenuRequest::remove"); 
} 

//-----------------------------------------------------------------------

MessageQueueObjectMenuRequest::MessageQueueObjectMenuRequest (const NetworkId & targetId, const NetworkId & requestorId, const std::vector<ObjectMenuRequestData> & data, uint8 sequence) :
m_sequence (sequence),
m_data (data),
m_requestorId (requestorId),
m_targetId (targetId)
{
}

//----------------------------------------------------------------------

MessageQueueObjectMenuRequest::~MessageQueueObjectMenuRequest ()
{
}

//-----------------------------------------------------------------------

void MessageQueueObjectMenuRequest::pack(const MessageQueue::Data* data, Archive::ByteStream & target)
{
	const MessageQueueObjectMenuRequest * const msg = safe_cast<const MessageQueueObjectMenuRequest *>(data);
	if(msg)
	{
		Archive::put(target, msg->getTargetId());
		Archive::put(target, msg->getRequestorId());
		Archive::put(target, msg->getData());
		Archive::put(target, msg->m_sequence);
	}
}

//-----------------------------------------------------------------------
MessageQueue::Data * MessageQueueObjectMenuRequest::unpack(Archive::ReadIterator & source)
{
	NetworkId targetId;
	NetworkId requestorId;
	std::vector<ObjectMenuRequestData> requestData;
	uint8 sequence;

	Archive::get(source, targetId);
	Archive::get(source, requestorId);
	Archive::get(source, requestData);
	Archive::get(source, sequence);

	MessageQueue::Data * result = new MessageQueueObjectMenuRequest(targetId, requestorId, requestData, sequence);
	return result;
}

//-----------------------------------------------------------------------
