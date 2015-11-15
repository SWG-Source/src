// ======================================================================
//
// BuffBuilderStartMessage.cpp
//
// Copyright 2006 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/BuffBuilderStartMessage.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

// ======================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(BuffBuilderStartMessage, CM_buffBuilderStart);

//----------------------------------------------------------------------

BuffBuilderStartMessage::BuffBuilderStartMessage(NetworkId const & bufferId, NetworkId const & recipientId ) :
MessageQueue::Data(),
m_bufferId(bufferId),
m_recipientId(recipientId)
{
}

//----------------------------------------------------------------------

void BuffBuilderStartMessage::pack(const MessageQueue::Data * const data, Archive::ByteStream & target)
{
	BuffBuilderStartMessage const * const message = safe_cast<BuffBuilderStartMessage const *> (data);
	if(message)
	{
		Archive::put(target, message->getBufferId());
		Archive::put(target, message->getRecipientId());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data * BuffBuilderStartMessage::unpack(Archive::ReadIterator & source)
{
	NetworkId bufferId;
	NetworkId recipientId;
	
	Archive::get(source, bufferId);
	Archive::get(source, recipientId);
	
	return new BuffBuilderStartMessage(bufferId, recipientId /*, terminalId, currentHoloEmote*/);
}

//----------------------------------------------------------------------
