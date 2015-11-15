//========================================================================
//
// MessageQueueCraftEmptySlot.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueCraftEmptySlot_H
#define INCLUDED_MessageQueueCraftEmptySlot_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

class MessageQueueCraftEmptySlot : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueCraftEmptySlot (int slot, const NetworkId & targetContainer, uint8 sequenceId);
	virtual ~MessageQueueCraftEmptySlot ();

	int               getSlot            () const;
	uint8             getSequenceId      () const;
	const NetworkId & getTargetContainer () const;

private:
	int               m_slot;
	NetworkId         m_targetContainer;
	uint8             m_sequenceId;

private:
	MessageQueueCraftEmptySlot (const MessageQueueCraftEmptySlot &);
	MessageQueueCraftEmptySlot & operator= (const MessageQueueCraftEmptySlot &);
};

//----------------------------------------------------------------------

inline int MessageQueueCraftEmptySlot::getSlot () const
{
	return m_slot;
}

//----------------------------------------------------------------------

inline uint8 MessageQueueCraftEmptySlot::getSequenceId () const
{
	return m_sequenceId;
}

//----------------------------------------------------------------------

inline const NetworkId & MessageQueueCraftEmptySlot::getTargetContainer () const
{
	return m_targetContainer;
}

//----------------------------------------------------------------------

#endif	// INCLUDED_MessageQueueCraftEmptySlot_H
