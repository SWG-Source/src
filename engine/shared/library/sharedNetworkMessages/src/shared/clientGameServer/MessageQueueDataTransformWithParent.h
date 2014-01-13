// ======================================================================
//
// MessageQueueDataTransformWithParent.h
// copyright 2001, sony online entertainment
//
// ======================================================================

#ifndef	INCLUDED_MessageQueueDataTransformWithParent_H
#define	INCLUDED_MessageQueueDataTransformWithParent_H

// ======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Transform.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

// ======================================================================

class MessageQueueDataTransformWithParent : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueDataTransformWithParent (uint32 syncStamp, int sequenceNumber, const NetworkId& parent, const Transform& transform, float speed, const float lookAtYaw, const bool lookAtYawIsValid);
	virtual ~MessageQueueDataTransformWithParent ();

	uint32           getSyncStamp () const;
	int              getSequenceNumber () const;
	const NetworkId& getParent () const;
	const Transform& getTransform () const;
	float            getSpeed () const;
	float            getLookAtYaw () const;
	bool             getUseLookAtYaw () const;

private:

	MessageQueueDataTransformWithParent ();
	MessageQueueDataTransformWithParent& operator= (const MessageQueueDataTransformWithParent& source);
	MessageQueueDataTransformWithParent (const MessageQueueDataTransformWithParent& source);

private:

	const uint32    m_syncStamp;
	const int32     m_sequenceNumber;
	const NetworkId m_parent;
	const Transform m_transform;
	const float     m_speed;
	const float     m_lookAtYaw;
	const bool      m_useLookAtYaw;
};

// ======================================================================

inline const NetworkId& MessageQueueDataTransformWithParent::getParent () const
{
	return m_parent;
}

// ----------------------------------------------------------------------

inline uint32 MessageQueueDataTransformWithParent::getSyncStamp () const
{
	return m_syncStamp;
}

// ----------------------------------------------------------------------

inline int MessageQueueDataTransformWithParent::getSequenceNumber () const
{
	return m_sequenceNumber;
}

// ----------------------------------------------------------------------

inline const Transform& MessageQueueDataTransformWithParent::getTransform () const
{
	return m_transform;
}

// ----------------------------------------------------------------------

inline float MessageQueueDataTransformWithParent::getSpeed () const
{
	return m_speed;
}

// ----------------------------------------------------------------------

inline float MessageQueueDataTransformWithParent::getLookAtYaw() const
{
	return m_lookAtYaw;
}

// ----------------------------------------------------------------------

inline bool MessageQueueDataTransformWithParent::getUseLookAtYaw() const
{
	return m_useLookAtYaw;
}

// ======================================================================

#endif


