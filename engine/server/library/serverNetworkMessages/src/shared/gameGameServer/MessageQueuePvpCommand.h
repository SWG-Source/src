// ======================================================================
//
// MessageQueuePvpCommand.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageQueuePvpCommand_H
#define INCLUDED_MessageQueuePvpCommand_H

// ======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

// ======================================================================

class MessageQueuePvpCommand: public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueuePvpCommand();
	MessageQueuePvpCommand(uint32 command, NetworkId const &idParam, int intParam1, int intParam2);
	MessageQueuePvpCommand(const MessageQueuePvpCommand &source);

	struct PvpCommand
	{
		uint32    m_command;
		NetworkId m_idParam;
		int       m_intParam1;
		int       m_intParam2;
	};

	typedef std::vector<PvpCommand> PvpCommands;

	void addPvpCommand(uint32 command, NetworkId const &idParam, int intParam1, int intParam2);
	PvpCommands const & getPvpCommands() const;

private:
	MessageQueuePvpCommand &operator=(const MessageQueuePvpCommand &source);

	PvpCommands m_pvpCommands;
};

// ======================================================================

#endif // INCLUDED_MessageQueuePvpCommand_H

