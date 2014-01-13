// DroidCommandProgrammingMessage.cpp
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/DroidCommandProgrammingMessage.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

#include <string>

//-----------------------------------------------------------------------

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(DroidCommandProgrammingMessage, CM_droidCommandProgramming);

//-----------------------------------------------------------------------

DroidCommandProgrammingMessage::DroidCommandProgrammingMessage() :
MessageQueue::Data(),
m_commands(),
m_chipsToAdd(),
m_chipsToRemove(),
m_droidControlDevice()
{
}

//-----------------------------------------------------------------------

DroidCommandProgrammingMessage & DroidCommandProgrammingMessage::operator=(const DroidCommandProgrammingMessage & rhs)
{
	if (this == &rhs)
		return *this;

	m_commands = rhs.m_commands;
	m_chipsToAdd = rhs.m_chipsToAdd;
	m_chipsToRemove = rhs.m_chipsToRemove;
	m_droidControlDevice = rhs.m_droidControlDevice;

	return *this;
}

//-----------------------------------------------------------------------

void DroidCommandProgrammingMessage::pack(MessageQueue::Data const * const data, Archive::ByteStream & target)
{
	DroidCommandProgrammingMessage const * const msg = safe_cast<DroidCommandProgrammingMessage const *> (data);

	if (msg)
	{
		std::vector<std::string> const & commands = msg->getCommands();
		int size = commands.size(); //lint !e713 signed / unsigned
		Archive::put(target, size);
		for(std::vector<std::string>::const_iterator i = commands.begin(); i != commands.end(); ++i)
		{
			Archive::put(target, *i);
		}

		std::vector<NetworkId> const & chipsToAdd = msg->getChipsToAdd();
		size = chipsToAdd.size(); //lint !e713 signed / unsigned
		Archive::put(target, size);
		for(std::vector<NetworkId>::const_iterator j = chipsToAdd.begin(); j != chipsToAdd.end(); ++j)
		{
			Archive::put(target, *j);
		}

		std::vector<NetworkId> const & chipsToRemove = msg->getChipsToRemove();
		size = chipsToRemove.size(); //lint !e713 signed / unsigned
		Archive::put(target, size);
		for(std::vector<NetworkId>::const_iterator k = chipsToRemove.begin(); k != chipsToRemove.end(); ++k)
		{
			Archive::put(target, *k);
		}

		Archive::put(target, msg->getDroidControlDevice());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* DroidCommandProgrammingMessage::unpack(Archive::ReadIterator & source)
{
	DroidCommandProgrammingMessage * msg = new DroidCommandProgrammingMessage;

	std::string command;
	std::vector<std::string> commands;
	int size = 0;
	Archive::get(source, size);
	for(int i = 0; i < size; ++i)
	{
		Archive::get(source, command);
		commands.push_back(command);
	}
	msg->setCommands(commands);

	std::vector<NetworkId> chips;
	NetworkId chip;
	Archive::get(source, size);
	for(int j = 0; j < size; ++j)
	{
		Archive::get(source, chip);
		chips.push_back(chip);
	}
	msg->setChipsToAdd(chips);

	chips.clear();
	Archive::get(source, size);
	for(int k = 0; k < size; ++k)
	{
		Archive::get(source, chip);
		chips.push_back(chip);
	}
	msg->setChipsToRemove(chips);

	NetworkId networkId;
	Archive::get(source, networkId);
	msg->setDroidControlDevice(networkId);
	return msg;
}

//======================================================================
