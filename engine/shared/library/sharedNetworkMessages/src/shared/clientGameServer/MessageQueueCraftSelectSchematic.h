//========================================================================
//
// MessageQueueCraftSelectSchematic.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueCraftSelectSchematic_H
#define INCLUDED_MessageQueueCraftSelectSchematic_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//----------------------------------------------------------------------

class MessageQueueCraftSelectSchematic : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueCraftSelectSchematic(int schematic);
	virtual ~MessageQueueCraftSelectSchematic();

	int               getSchematic(void) const;

private:
	int               m_schematic;

private:
	MessageQueueCraftSelectSchematic (const MessageQueueCraftSelectSchematic &);
	MessageQueueCraftSelectSchematic & operator= (const MessageQueueCraftSelectSchematic &);
};

//----------------------------------------------------------------------

inline MessageQueueCraftSelectSchematic::MessageQueueCraftSelectSchematic(int schematic) :
m_schematic(schematic)
{
}	// MessageQueueCraftSelectSchematic::MessageQueueCraftSelectSchematic

//----------------------------------------------------------------------

inline int MessageQueueCraftSelectSchematic::getSchematic(void) const
{
	return m_schematic;
}	// MessageQueueCraftSelectSchematic::getSchematic

//----------------------------------------------------------------------

#endif	// INCLUDED_MessageQueueCraftSelectSchematic_H
