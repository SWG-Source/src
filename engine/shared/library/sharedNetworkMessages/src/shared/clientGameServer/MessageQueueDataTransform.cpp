// ======================================================================
//
// MessageQueueDataTransform.cpp
// copyright 2000 Verant Interactive
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"

#include "sharedMathArchive/TransformArchive.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

// ======================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION (MessageQueueDataTransform, CM_netUpdateTransform);

// ======================================================================

MessageQueueDataTransform::MessageQueueDataTransform (const uint32 syncStamp, const int sequenceNumber, const Transform& source, const float speed,
													  const float lookAtYaw, const bool useLookAtYaw) :
	Data (),
	m_syncStamp (syncStamp),
	m_sequenceNumber (sequenceNumber),
	m_transform (source),
	m_speed (speed),
	m_lookAtYaw (lookAtYaw),
	m_useLookAtYaw (useLookAtYaw)
{	
}

// ----------------------------------------------------------------------

MessageQueueDataTransform::~MessageQueueDataTransform ()
{
}

// ----------------------------------------------------------------------

void MessageQueueDataTransform::pack (const MessageQueue::Data* const data, Archive::ByteStream& target)
{
	const MessageQueueDataTransform* const message = safe_cast<const MessageQueueDataTransform*> (data);
	if (message)
	{
		Archive::put (target, message->getSyncStamp ());
		Archive::put (target, message->getSequenceNumber ());
		Archive::put (target, message->getTransform ());
		Archive::put (target, message->getSpeed ());
		Archive::put (target, message->getLookAtYaw());
		Archive::put (target, message->getUseLookAtYaw());
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueDataTransform::unpack (Archive::ReadIterator& source)
{
	uint32 syncStamp = 0;
	Archive::get (source, syncStamp);

	int sequenceNumber = 0;
	Archive::get (source, sequenceNumber);

	Transform transform;
	Archive::get (source, transform);

	float speed = 0.f;
	Archive::get (source, speed);

	float lookAtYaw = 0.f;
	Archive::get (source, lookAtYaw);

	bool useLookAtYaw = false;
	Archive::get (source, useLookAtYaw);

	return new MessageQueueDataTransform (syncStamp, sequenceNumber, transform, speed, lookAtYaw, useLookAtYaw);
}

// ======================================================================

