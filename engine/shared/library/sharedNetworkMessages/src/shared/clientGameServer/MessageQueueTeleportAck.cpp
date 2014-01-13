//======================================================================
//
// MessageQueueTeleportAck.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueTeleportAck.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueTeleportAck, CM_teleportAck);

//===================================================================


//----------------------------------------------------------------------

MessageQueueTeleportAck::MessageQueueTeleportAck(int sequenceId) :
	MessageQueue::Data(),
	m_sequenceId(sequenceId)
{
}

//----------------------------------------------------------------------

MessageQueueTeleportAck::MessageQueueTeleportAck(MessageQueueTeleportAck const &rhs) :
	MessageQueue::Data(),
	m_sequenceId(rhs.m_sequenceId)
{
}

//----------------------------------------------------------------------

void MessageQueueTeleportAck::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueTeleportAck* const msg = safe_cast<const MessageQueueTeleportAck*> (data);
	if(msg)
	{
		Archive::put(target, msg->getSequenceId());
	}
}

//-----------------------------------------------------------------------


MessageQueue::Data* MessageQueueTeleportAck::unpack(Archive::ReadIterator & source)
{
	int sequenceId;
	
	Archive::get(source, sequenceId);
	
	return new MessageQueueTeleportAck(sequenceId);
}

//-----------------------------------------------------------------------

