//======================================================================
//
// MessageQueueCombatCommand.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueCombatCommand_H
#define INCLUDED_MessageQueueCombatCommand_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

/**
* A generic message queue data that contains a simple payload of one String.
* The meaning of the String is context and message dependant.
*
*/

class MessageQueueCombatCommand : public MessageQueue::Data
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	explicit                MessageQueueCombatCommand (const std::string & str, uint32 sequenceId);
	virtual                ~MessageQueueCombatCommand();

	const std::string &     getString     (void) const;
	uint32                  getSequenceId (void) const;

private:

	MessageQueueCombatCommand&  operator= (const MessageQueueCombatCommand & source);
	                     MessageQueueCombatCommand(const MessageQueueCombatCommand & source);

	std::string m_string;
	uint32      m_sequenceId;
};

//-----------------------------------------------------------------------

inline const std::string & MessageQueueCombatCommand::getString (void) const
{
	return m_string;
}

//-----------------------------------------------------------------------

inline uint32 MessageQueueCombatCommand::getSequenceId (void) const
{
	return m_sequenceId;
}

//-----------------------------------------------------------------------

#endif	// _MessageQueueCombatCommand_H

