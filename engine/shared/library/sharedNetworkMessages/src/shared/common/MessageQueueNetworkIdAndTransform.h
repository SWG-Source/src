// MessageQueueNetworkIdAndTransform.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_MessageQueueNetworkIdAndTransform_H
#define	_INCLUDED_MessageQueueNetworkIdAndTransform_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Transform.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

class MessageQueueNetworkIdAndTransform : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueNetworkIdAndTransform(const NetworkId & networkId, const Transform & transform);
	~MessageQueueNetworkIdAndTransform();

	const NetworkId &  getNetworkId  () const;
	const Transform &  getTransform  () const;

private:
	MessageQueueNetworkIdAndTransform & operator = (const MessageQueueNetworkIdAndTransform & rhs);
	MessageQueueNetworkIdAndTransform(const MessageQueueNetworkIdAndTransform & source);

	NetworkId  m_networkId;
	Transform  m_transform;
};

//-----------------------------------------------------------------------

inline const NetworkId & MessageQueueNetworkIdAndTransform::getNetworkId() const
{
	return m_networkId;
}

//-----------------------------------------------------------------------

inline const Transform & MessageQueueNetworkIdAndTransform::getTransform() const
{
	return m_transform;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MessageQueueNetworkIdAndTransform_H
