//======================================================================
//
// MessageQueueNetworkId.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(MessageQueueNetworkId);

//===================================================================

void MessageQueueNetworkId::install() 
{ 
	installMemoryBlockManager(); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_getTokenForObject, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_combatAimToClient, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_getWaypointForObject, pack, unpack, true); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientResourceHarvesterActivate, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientResourceHarvesterDeactivate, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientResourceHarvesterListen, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientResourceHarvesterStopListening, pack,unpack ); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientResourceHarvesterGetResourceData, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_missionAbort, pack, unpack, true); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_setOwnerId, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientLookAtTarget, pack, unpack, true);	
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientIntendedTarget, pack, unpack, true);  
	ExitChain::add(&remove, "MessageQueueNetworkId::remove"); 
} 

//-----------------------------------------------------------------------

MessageQueueNetworkId::MessageQueueNetworkId (const NetworkId & id) :
Data (),
m_id (id)
{
}

//-----------------------------------------------------------------------

MessageQueueNetworkId::~MessageQueueNetworkId()
{
}

//-----------------------------------------------------------------------
void MessageQueueNetworkId::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueNetworkId* const msg = safe_cast<const MessageQueueNetworkId*> (data);
	if (msg)
		Archive::put (target, msg->getNetworkId ());
}


//----------------------------------------------------------------------

MessageQueue::Data* MessageQueueNetworkId::unpack(Archive::ReadIterator & source)
{
	NetworkId id;
	Archive::get(source, id);
	
	return new MessageQueueNetworkId (id);
}

//----------------------------------------------------------------------
