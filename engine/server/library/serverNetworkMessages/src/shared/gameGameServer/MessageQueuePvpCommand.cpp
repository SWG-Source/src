// ======================================================================
//
// MessageQueuePvpCommand.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/MessageQueuePvpCommand.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueuePvpCommand, CM_pvpMessage);

//===================================================================

MessageQueuePvpCommand::MessageQueuePvpCommand() :
	MessageQueue::Data(),
	m_pvpCommands()
{
}

// ----------------------------------------------------------------------

MessageQueuePvpCommand::MessageQueuePvpCommand(uint32 command, NetworkId const &idParam, int intParam1, int intParam2) :
	MessageQueue::Data(),
	m_pvpCommands()
{
	PvpCommand pvpCommand;
	pvpCommand.m_command = command;
	pvpCommand.m_idParam = idParam;
	pvpCommand.m_intParam1 = intParam1;
	pvpCommand.m_intParam2 = intParam2;

	m_pvpCommands.push_back(pvpCommand);
}

// ----------------------------------------------------------------------

MessageQueuePvpCommand::MessageQueuePvpCommand(MessageQueuePvpCommand const &source) :
	MessageQueue::Data(),
	m_pvpCommands(source.m_pvpCommands)
{
}	

// ----------------------------------------------------------------------

void MessageQueuePvpCommand::addPvpCommand(uint32 command, NetworkId const &idParam, int intParam1, int intParam2)
{
	PvpCommand pvpCommand;
	pvpCommand.m_command = command;
	pvpCommand.m_idParam = idParam;
	pvpCommand.m_intParam1 = intParam1;
	pvpCommand.m_intParam2 = intParam2;

	m_pvpCommands.push_back(pvpCommand);
}

// ----------------------------------------------------------------------

MessageQueuePvpCommand::PvpCommands const & MessageQueuePvpCommand::getPvpCommands() const
{
	return m_pvpCommands;
}

//-----------------------------------------------------------------------

void MessageQueuePvpCommand::pack(const MessageQueue::Data* const data, Archive::ByteStream &target)
{
	MessageQueuePvpCommand const* const msg = safe_cast<MessageQueuePvpCommand const*> (data);
	
	if (msg)
	{
		Archive::put(target, static_cast<int>(msg->m_pvpCommands.size()));

		for (PvpCommands::const_iterator i = msg->m_pvpCommands.begin(); i != msg->m_pvpCommands.end(); ++i)
		{
			Archive::put(target, i->m_command);
			Archive::put(target, i->m_idParam);
			Archive::put(target, i->m_intParam1);
			Archive::put(target, i->m_intParam2);
		}
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueuePvpCommand::unpack(Archive::ReadIterator &source)
{
	MessageQueuePvpCommand * const msg = new MessageQueuePvpCommand();
	PvpCommand pvpCommand;
	int size;

	Archive::get(source, size);

	for (int i = 0; i < size; ++i)
	{
		Archive::get(source, pvpCommand.m_command);
		Archive::get(source, pvpCommand.m_idParam);
		Archive::get(source, pvpCommand.m_intParam1);
		Archive::get(source, pvpCommand.m_intParam2);

		msg->m_pvpCommands.push_back(pvpCommand);
	}

	return msg;
}

// ======================================================================
