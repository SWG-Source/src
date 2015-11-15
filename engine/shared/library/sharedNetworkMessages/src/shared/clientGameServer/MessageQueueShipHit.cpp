//======================================================================
//
// MessageQueueShipHit.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueShipHit.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//======================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(MessageQueueShipHit);

//===================================================================

void MessageQueueShipHit::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_spaceShipHit, pack, unpack); 	
	ExitChain::add(&remove, "MessageQueueShipHit::remove"); 
}

//----------------------------------------------------------------------

/**
* @_type _must_ be one of ShipHitEffectsManager::HitType
*/ 

MessageQueueShipHit::MessageQueueShipHit(Vector const & _up_w, Vector const & _hitLocation_o, int _type, float _integrity, float _previousIntegrity) :
up_w(_up_w),
hitLocation_o(_hitLocation_o),
type(_type),
integrity(_integrity),
previousIntegrity(_previousIntegrity)
{
}

//----------------------------------------------------------------------

MessageQueueShipHit::MessageQueueShipHit() :
up_w(),
hitLocation_o(),
type(0),
integrity(1.0f),
previousIntegrity(1.0f)
{
}

//----------------------------------------------------------------------

void MessageQueueShipHit::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueShipHit* const msg = safe_cast<const MessageQueueShipHit*> (data);
	if (msg)
	{
		Archive::put(target, msg->up_w);
		Archive::put(target, msg->hitLocation_o);
		Archive::put(target, msg->type);
		Archive::put(target, msg->integrity);
		Archive::put(target, msg->previousIntegrity);
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data * MessageQueueShipHit::unpack(Archive::ReadIterator & source)
{
	MessageQueueShipHit * const msg = new MessageQueueShipHit;

	Archive::get(source, msg->up_w);
	Archive::get(source, msg->hitLocation_o);
	Archive::get(source, msg->type);
	Archive::get(source, msg->integrity);
	Archive::get(source, msg->previousIntegrity);

	return msg;
}

//======================================================================
