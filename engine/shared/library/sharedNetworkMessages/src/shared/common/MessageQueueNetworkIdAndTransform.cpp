// MessageQueueNetworkIdAndTransform.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "MessageQueueNetworkIdAndTransform.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedMathArchive/TransformArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(MessageQueueNetworkIdAndTransform);

//===================================================================

void MessageQueueNetworkIdAndTransform::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_transferObjectToCell, pack, unpack); 
	ExitChain::add(&remove, "MessageQueueNetworkIdAndTransform::remove"); 
} 

//-----------------------------------------------------------------------

MessageQueueNetworkIdAndTransform::MessageQueueNetworkIdAndTransform(const NetworkId & n, const Transform & t) :
MessageQueue::Data(),
m_networkId(n),
m_transform(t)
{
}

//-----------------------------------------------------------------------

MessageQueueNetworkIdAndTransform::~MessageQueueNetworkIdAndTransform()
{
}

//-----------------------------------------------------------------------

void MessageQueueNetworkIdAndTransform::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueNetworkIdAndTransform* const msg = safe_cast<const MessageQueueNetworkIdAndTransform*> (data);
	if(msg)
	{
		Archive::put(target, msg->getNetworkId());
		Archive::put(target, msg->getTransform());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueNetworkIdAndTransform::unpack(Archive::ReadIterator & source)
{
	NetworkId id;
	Transform t;
	
	Archive::get(source, id);
	Archive::get(source, t);
	
	return new MessageQueueNetworkIdAndTransform(id, t);
}

//-----------------------------------------------------------------------

