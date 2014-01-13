// ======================================================================
//
// MessageQueueDataTransform.h
// copyright 2000 Verant Interactive
//
// ======================================================================

#ifndef	INCLUDED_MessageQueueDataTransform_H
#define	INCLUDED_MessageQueueDataTransform_H

// ======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedMath/Transform.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
class Transform;

// ======================================================================

class MessageQueueDataTransform : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueDataTransform (uint32 syncStamp, int sequenceNumber, const Transform& source, float speed, const float lookAtYaw, const bool useLookAtYaw);
	virtual ~MessageQueueDataTransform ();

	uint32           getSyncStamp () const;
	int              getSequenceNumber () const;
	const Transform& getTransform () const;
	float            getSpeed () const;
	float            getLookAtYaw () const;
	bool             getUseLookAtYaw () const;

private:

	MessageQueueDataTransform ();
	MessageQueueDataTransform& operator= (const MessageQueueDataTransform& source);
	MessageQueueDataTransform (const MessageQueueDataTransform& source);

private:

	const uint32    m_syncStamp;
	const int32     m_sequenceNumber;
	const Transform m_transform;
	const float     m_speed;
	float           m_lookAtYaw;
	bool            m_useLookAtYaw;
};

// ----------------------------------------------------------------------

inline uint32 MessageQueueDataTransform::getSyncStamp () const
{
	return m_syncStamp;
}

// ----------------------------------------------------------------------

inline int MessageQueueDataTransform::getSequenceNumber () const
{
	return m_sequenceNumber;
}

// ----------------------------------------------------------------------

inline const Transform& MessageQueueDataTransform::getTransform () const
{
	return m_transform;
}

// ----------------------------------------------------------------------

inline float MessageQueueDataTransform::getSpeed () const
{
	return m_speed;
}

// ----------------------------------------------------------------------

inline float MessageQueueDataTransform::getLookAtYaw() const
{
	return m_lookAtYaw;
}

// ----------------------------------------------------------------------

inline bool MessageQueueDataTransform::getUseLookAtYaw() const
{
	return m_useLookAtYaw;
}


// ======================================================================

#endif
