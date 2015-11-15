//======================================================================
//
// MessageQueueNetworkIdPair.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueNetworkIdPair.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedMathArchive/TransformArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(MessageQueueNetworkIdPair);

//===================================================================

void MessageQueueNetworkIdPair::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientResourceHarvesterResourceSelect, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientResourceHarvesterSurvey, pack, unpack); 
	ExitChain::add(&remove, "MessageQueueNetworkIdPair::remove"); 
} 

//-----------------------------------------------------------------------

MessageQueueNetworkIdPair::MessageQueueNetworkIdPair (const NetworkId & first, const NetworkId & second) :
Data (),
m_firstId (first),
m_secondId (second)
{
}

//-----------------------------------------------------------------------

MessageQueueNetworkIdPair::~MessageQueueNetworkIdPair()
{
}

//-----------------------------------------------------------------------

void MessageQueueNetworkIdPair::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueNetworkIdPair* const msg = safe_cast<const MessageQueueNetworkIdPair*> (data);
	if (msg)
	{
		Archive::put (target, msg->getFirstNetworkId ());
		Archive::put (target, msg->getSecondNetworkId ());
	}
}

//----------------------------------------------------------------------
MessageQueue::Data* MessageQueueNetworkIdPair::unpack(Archive::ReadIterator & source)
{
	NetworkId id [2];
	Archive::get(source, id [0]);
	Archive::get(source, id [1]);
	MessageQueueNetworkIdPair* const message = new MessageQueueNetworkIdPair (id [0], id [1]);
	return message;
}

//----------------------------------------------------------------------
