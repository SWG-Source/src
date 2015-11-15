// ======================================================================
//
// MessageQueueSlowDownEffect.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueSlowDownEffect.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueSlowDownEffect, CM_slowDownEffect);

//===================================================================



MessageQueueSlowDownEffect::MessageQueueSlowDownEffect(const NetworkId & target, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime) :
	MessageQueue::Data(),
	m_target(target),
	m_coneLength(coneLength),
	m_coneAngle(coneAngle),
	m_slopeAngle(slopeAngle),
	m_expireTime(expireTime)
{
}

// ----------------------------------------------------------------------

MessageQueueSlowDownEffect::MessageQueueSlowDownEffect(
	MessageQueueSlowDownEffect const &source) :
	MessageQueue::Data(),
	m_target(source.m_target),
	m_coneLength(source.m_coneLength),
	m_coneAngle(source.m_coneAngle),
	m_slopeAngle(source.m_slopeAngle),
	m_expireTime(source.m_expireTime)
{
}	

// ----------------------------------------------------------------------

void MessageQueueSlowDownEffect::pack(const MessageQueue::Data* data, Archive::ByteStream & target)
{
	const MessageQueueSlowDownEffect * const msg = safe_cast<const MessageQueueSlowDownEffect *>(data);
	if (msg)
	{
		Archive::put(target, msg->getTarget());
		Archive::put(target, msg->getConeLength());
		Archive::put(target, msg->getConeAngle());
		Archive::put(target, msg->getSlopeAngle());
		Archive::put(target, msg->getExpireTime());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data * MessageQueueSlowDownEffect::unpack(Archive::ReadIterator & source)
{
	NetworkId target;
	float coneLength;
	float coneAngle;
	float slopeAngle;
	unsigned long expireTime;

	Archive::get(source, target);
	Archive::get(source, coneLength);
	Archive::get(source, coneAngle);
	Archive::get(source, slopeAngle);
	Archive::get(source, expireTime);

	return new MessageQueueSlowDownEffect(target, coneLength, coneAngle, slopeAngle, expireTime);
}

