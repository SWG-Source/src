//========================================================================
//
// MessageQueueResourceTypeDepleted.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueResourceTypeDepleted_H
#define INCLUDED_MessageQueueResourceTypeDepleted_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

/**
 * Sent by:  Resource Type
 * Sent to:  Resource Class
 * Action:  Resource Type has been depleted.  Spawn a new one or take other
 * appropriate action.
 */

class MessageQueueResourceTypeDepleted : public MessageQueue::Data
{
  public:
	MessageQueueResourceTypeDepleted(NetworkId typeId);
	virtual ~MessageQueueResourceTypeDepleted();
	
	MessageQueueResourceTypeDepleted&	operator=	(const MessageQueueResourceTypeDepleted & source);
	MessageQueueResourceTypeDepleted(const MessageQueueResourceTypeDepleted & source);

  public:
	NetworkId getTypeId() const;
	
  private:
	NetworkId m_typeId;
};

// ======================================================================

inline NetworkId MessageQueueResourceTypeDepleted::getTypeId() const
{
	return m_typeId;
}

// ======================================================================

#endif
