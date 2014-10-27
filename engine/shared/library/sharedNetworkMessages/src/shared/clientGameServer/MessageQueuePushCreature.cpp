//========================================================================
//
// MessageQueuePushCreature.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueuePushCreature.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueuePushCreature, CM_pushCreature);

//===================================================================

/**
 * Class constructor.
 */
MessageQueuePushCreature::MessageQueuePushCreature(const NetworkId & attacker, const NetworkId & defender, const Vector & attackerPos, const Vector & defenderPos, float distance):
	MessageQueue::Data(),
	m_attacker(attacker),
	m_defender(defender),
	m_attackerPos(attackerPos),
	m_defenderPos(defenderPos),
	m_distance(distance)
{
}

//----------------------------------------------------------------------

/**
 * Class destructor.
 */
MessageQueuePushCreature::~MessageQueuePushCreature()
{
}

//----------------------------------------------------------------------

void MessageQueuePushCreature::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueuePushCreature* const msg = safe_cast<const MessageQueuePushCreature*> (data);
	if (msg)
	{
		Archive::put (target, msg->getAttacker());
		Archive::put (target, msg->getDefender());
		Archive::put (target, msg->getAttackerPos());
		Archive::put (target, msg->getDefenderPos());
		Archive::put (target, msg->getDistance());
	}
}

//----------------------------------------------------------------------

MessageQueue::Data* MessageQueuePushCreature::unpack(Archive::ReadIterator & source)
{
	NetworkId attacker;
	NetworkId defender;
	Vector    attackerPos;
	Vector    defenderPos;
	float     distance;
	
	Archive::get(source, attacker);
	Archive::get(source, defender);
	Archive::get(source, attackerPos);
	Archive::get(source, defenderPos);
	Archive::get(source, distance);
	
	MessageQueuePushCreature * const msg = new MessageQueuePushCreature(attacker, defender, attackerPos, defenderPos, distance);

	return msg;
}
//-----------------------------------------------------------------------

