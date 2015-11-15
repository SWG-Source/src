// ======================================================================
//
// MessageQueueDataTransformWithParent.cpp
// copyright 2001, sony online entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

// ======================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION (MessageQueueDataTransformWithParent, CM_netUpdateTransformWithParent);

// ======================================================================

MessageQueueDataTransformWithParent::MessageQueueDataTransformWithParent (const uint32 syncStamp, const int sequenceNumber, const NetworkId& parent, const Transform& transform, const float speed,
																		  const float lookAtYaw, const bool useLookAtYaw) :
	Data (),
	m_syncStamp (syncStamp),
	m_sequenceNumber (sequenceNumber),
	m_parent (parent),
	m_transform (transform),
	m_speed (speed),
	m_lookAtYaw (lookAtYaw),
	m_useLookAtYaw (useLookAtYaw)
{
}

// ----------------------------------------------------------------------

MessageQueueDataTransformWithParent::~MessageQueueDataTransformWithParent ()
{
}

// ----------------------------------------------------------------------

void MessageQueueDataTransformWithParent::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueDataTransformWithParent* const message = safe_cast<const MessageQueueDataTransformWithParent*> (data);
	if (message)
	{
		Archive::put (target, message->getSyncStamp ());
		Archive::put (target, message->getSequenceNumber ());
		Archive::put (target, message->getParent ());
		Archive::put (target, message->getTransform ());
		Archive::put (target, message->getSpeed ());
		Archive::put (target, message->getLookAtYaw ());
		Archive::put (target, message->getUseLookAtYaw ());
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueDataTransformWithParent::unpack (Archive::ReadIterator & source)
{
	uint32 syncStamp = 0;
	Archive::get (source, syncStamp);

	int sequenceNumber = 0;
	Archive::get (source, sequenceNumber);

	NetworkId parent;
	Archive::get (source, parent);

	Transform transform;
	Archive::get (source, transform);

	float speed = 0.f;
	Archive::get (source, speed);

	float lookAtYaw = 0.f;
	Archive::get (source, lookAtYaw);

	bool useLookAtYaw = false;
	Archive::get (source, useLookAtYaw);

	return new MessageQueueDataTransformWithParent (syncStamp, sequenceNumber, parent, transform, speed, lookAtYaw, useLookAtYaw);
}

// ======================================================================

