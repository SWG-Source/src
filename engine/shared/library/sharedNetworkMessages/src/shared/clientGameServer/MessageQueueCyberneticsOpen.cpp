//========================================================================
//
// MessageQueueCyberneticsOpen.cpp
//
// copyright 2005 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCyberneticsOpen.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueCyberneticsOpen, CM_cyberneticsOpen);

//===================================================================

/**
 * Class destructor.
 */
MessageQueueCyberneticsOpen::~MessageQueueCyberneticsOpen()
{
}	// MessageQueueCyberneticsOpen::~MessageQueueCyberneticsOpen

//----------------------------------------------------------------------

void MessageQueueCyberneticsOpen::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCyberneticsOpen* const msg = safe_cast<const MessageQueueCyberneticsOpen*> (data);
	if (msg)
	{
		Archive::put(target, msg->getOpenType());
		Archive::put(target, msg->getNPC());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueCyberneticsOpen::unpack (Archive::ReadIterator & source)
{
	int openTypeInt = -1;
	NetworkId npc;

	Archive::get(source, openTypeInt);
	Archive::get(source, npc);

	return new MessageQueueCyberneticsOpen(static_cast<OpenType>(openTypeInt), npc);
}

//----------------------------------------------------------------------
