// ======================================================================
//
// MessageQueueSlowDownEffect.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageQueueSlowDownEffect_H
#define INCLUDED_MessageQueueSlowDownEffect_H

// ======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

// ======================================================================

class MessageQueueSlowDownEffect: public MessageQueue::Data
{

	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueSlowDownEffect(const NetworkId & target, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime);
	MessageQueueSlowDownEffect(const MessageQueueSlowDownEffect &source);

	const NetworkId & getTarget() const;
	float getConeLength() const;
	float getConeAngle() const;
	float getSlopeAngle() const;
	uint32_t getExpireTime() const;

private:
	MessageQueueSlowDownEffect();
	MessageQueueSlowDownEffect &operator=(const MessageQueueSlowDownEffect &source);

	NetworkId m_target;
	float m_coneLength;
	float m_coneAngle;
	float m_slopeAngle;
	uint32_t m_expireTime;
};

// ----------------------------------------------------------------------

inline const NetworkId & MessageQueueSlowDownEffect::getTarget() const
{
	return m_target;
}

inline float MessageQueueSlowDownEffect::getConeLength() const
{
	return m_coneLength;
}

inline float MessageQueueSlowDownEffect::getConeAngle() const
{
	return m_coneAngle;
}

inline float MessageQueueSlowDownEffect::getSlopeAngle() const
{
	return m_slopeAngle;
}

inline uint32_t MessageQueueSlowDownEffect::getExpireTime() const
{
	return m_expireTime;
}


// ======================================================================

#endif // INCLUDED_MessageQueueSlowDownEffect_H

