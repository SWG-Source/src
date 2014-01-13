//========================================================================
//
// MessageQueueCraftFillSlot.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueCraftFillSlot_H
#define INCLUDED_MessageQueueCraftFillSlot_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

class MessageQueueCraftFillSlot : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueCraftFillSlot(const NetworkId & ingredient, int slot, int option, uint8 sequenceId);
	virtual ~MessageQueueCraftFillSlot();

	const NetworkId & getIngredient(void) const;
	int               getSlot(void) const;
	int               getOption(void) const;
	uint8             getSequenceId(void) const;

private:
	NetworkId         m_ingredient;
	int               m_slot;
	int               m_option;
	uint8             m_sequenceId;

private:
	MessageQueueCraftFillSlot (const MessageQueueCraftFillSlot &);
	MessageQueueCraftFillSlot & operator= (const MessageQueueCraftFillSlot &);
};


inline MessageQueueCraftFillSlot::MessageQueueCraftFillSlot(const NetworkId & ingredient, int slot, int option, uint8 sequenceId) :
	m_ingredient(ingredient),
	m_slot(slot),
	m_option(option),
	m_sequenceId(sequenceId)
{
}	// MessageQueueCraftFillSlot::MessageQueueCraftFillSlot

inline const NetworkId & MessageQueueCraftFillSlot::getIngredient(void) const
{
	return m_ingredient;
}	// MessageQueueCraftFillSlot::getIngredient

inline int MessageQueueCraftFillSlot::getSlot(void) const
{
	return m_slot;
}	// MessageQueueCraftFillSlot::getSlot

inline int MessageQueueCraftFillSlot::getOption(void) const
{
	return m_option;
}	// MessageQueueCraftFillSlot::getOption

inline uint8 MessageQueueCraftFillSlot::getSequenceId(void) const
{
	return m_sequenceId;
}	// MessageQueueCraftFillSlot::getSequenceId


#endif	// INCLUDED_MessageQueueCraftFillSlot_H
