//========================================================================
//
// MessageQueueChangeRoleIconChoice.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueChangeRoleIconChoice_H
#define INCLUDED_MessageQueueChangeRoleIconChoice_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

class MessageQueueChangeRoleIconChoice : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueChangeRoleIconChoice (int newRoleIconChoice, uint8 sequenceId);
	virtual ~MessageQueueChangeRoleIconChoice ();

	int               getRoleIconChoice  () const;
	uint8             getSequenceId      () const;

private:
	int               m_roleIconChoice;
	uint8             m_sequenceId;

private:
	MessageQueueChangeRoleIconChoice (const MessageQueueChangeRoleIconChoice &);
	MessageQueueChangeRoleIconChoice & operator= (const MessageQueueChangeRoleIconChoice &);
};

//----------------------------------------------------------------------

inline int MessageQueueChangeRoleIconChoice::getRoleIconChoice () const
{
	return m_roleIconChoice;
}

//----------------------------------------------------------------------

inline uint8 MessageQueueChangeRoleIconChoice::getSequenceId () const
{
	return m_sequenceId;
}

//----------------------------------------------------------------------

#endif	// INCLUDED_MessageQueueChangeRoleIconChoice_H
