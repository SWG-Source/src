//======================================================================
//
// MessageQueuePosture.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueuePosture.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueuePosture, CM_setPosture);

//===================================================================


MessageQueuePosture::~MessageQueuePosture ()
{
}
//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueuePosture::unpack(Archive::ReadIterator & source)
{
	uint8 posture;
	uint8 isClientImmediate;
	
	Archive::get(source, posture);
	Archive::get(source, isClientImmediate);
	
	return new MessageQueuePosture(posture, isClientImmediate != 0);
}

//-----------------------------------------------------------------------
void MessageQueuePosture::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueuePosture* const msg = safe_cast<const MessageQueuePosture*> (data);
	if(msg)
	{
		uint8 const isClientImmediate = static_cast<uint8>(msg->isClientImmediate() ? 1 : 0);
		
		Archive::put(target, msg->getPosture());
		Archive::put(target, isClientImmediate);
	}
}

//======================================================================
