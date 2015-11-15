//========================================================================
//
// MessageQueueResourceAdd.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueResourceAdd_H
#define INCLUDED_MessageQueueResourceAdd_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class MessageQueueResourceAdd : public MessageQueue::Data
{
public:
	MessageQueueResourceAdd(const NetworkId &resourceId, int32 amount);
	virtual ~MessageQueueResourceAdd();
	
	MessageQueueResourceAdd&	operator=	(const MessageQueueResourceAdd & source);
	MessageQueueResourceAdd(const MessageQueueResourceAdd & source);
	
	const NetworkId &getResourceId() const;
	int getAmount() const;

private:
	NetworkId m_resourceId;
	int m_amount;
};

inline const NetworkId &MessageQueueResourceAdd::getResourceId() const
{
	return m_resourceId;
}

inline int MessageQueueResourceAdd::getAmount() const
{
	return m_amount;
}

#endif	// INCLUDED_MessageQueueResourceAdd_H
