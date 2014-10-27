//======================================================================
//
// MessageQueueUpdateShipOnCollision.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueUpdateShipOnCollision.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"


#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//======================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueUpdateShipOnCollision, CM_updateShipOnCollision);

//===================================================================

MessageQueueUpdateShipOnCollision::MessageQueueUpdateShipOnCollision(Vector const & deltaToMove, Vector const & newVelocity, float const clampedStrengthOfImpact)
: m_deltaToMove(deltaToMove)
, m_newVelocity(newVelocity)
, m_clampedStrengthOfImpact(clampedStrengthOfImpact)
{ }

// ----------------------------------------------------------------------

MessageQueueUpdateShipOnCollision::~MessageQueueUpdateShipOnCollision()
{ }

//----------------------------------------------------------------------

void MessageQueueUpdateShipOnCollision::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueUpdateShipOnCollision* const msg = safe_cast<const MessageQueueUpdateShipOnCollision*> (data);
	if (msg)
	{
		Vector const deltaToMove = msg->getDeltaToMove();
		Vector const newVelocity = msg->getNewVelocity();
		float const clampedStrengthOfImpact = msg->getClampedStrengthOfImpact();

		Archive::put(target, deltaToMove.x);
		Archive::put(target, deltaToMove.y);
		Archive::put(target, deltaToMove.z);
		Archive::put(target, newVelocity.x);
		Archive::put(target, newVelocity.y);
		Archive::put(target, newVelocity.z);
		Archive::put(target, clampedStrengthOfImpact);

	}
}

//----------------------------------------------------------------------

MessageQueue::Data* MessageQueueUpdateShipOnCollision::unpack(Archive::ReadIterator & source)
{
	Vector deltaToMove;
	Vector newVelocity;
	float clampedStrengthOfImpact;

	Archive::get(source, deltaToMove.x);
	Archive::get(source, deltaToMove.y);
	Archive::get(source, deltaToMove.z);
	Archive::get(source, newVelocity.x);
	Archive::get(source, newVelocity.y);
	Archive::get(source, newVelocity.z);
	Archive::get(source, clampedStrengthOfImpact);

	MessageQueueUpdateShipOnCollision * const msg = new MessageQueueUpdateShipOnCollision(deltaToMove, newVelocity, clampedStrengthOfImpact);

	return msg;
}

//======================================================================
