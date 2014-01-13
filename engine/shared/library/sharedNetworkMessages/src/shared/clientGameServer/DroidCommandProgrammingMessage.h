// DroidCommandProgrammingMessage.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_DroidCommandProgrammingMessage_H
#define	_INCLUDED_DroidCommandProgrammingMessage_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------

class DroidCommandProgrammingMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

public:
	DroidCommandProgrammingMessage();
	DroidCommandProgrammingMessage & operator=(const DroidCommandProgrammingMessage & rhs);

	//accessors
	std::vector<std::string> const & getCommands() const;
	std::vector<NetworkId> const & getChipsToAdd() const;
	std::vector<NetworkId> const & getChipsToRemove() const;
	NetworkId const & getDroidControlDevice() const;

	//mutators
	void setCommands(std::vector<std::string> const & commands);
	void setChipsToAdd(std::vector<NetworkId> const & chipsToAdd);
	void setChipsToRemove(std::vector<NetworkId> const & chipsToRemove);
	void setDroidControlDevice(NetworkId const & droidControlDevice);

private:
	//disabled
	DroidCommandProgrammingMessage(DroidCommandProgrammingMessage const & source);

private:
	std::vector<std::string> m_commands;
	std::vector<NetworkId> m_chipsToAdd;
	std::vector<NetworkId> m_chipsToRemove;
	NetworkId m_droidControlDevice;
};

//-----------------------------------------------------------------------

inline std::vector<std::string> const & DroidCommandProgrammingMessage::getCommands() const
{
	return m_commands;
}

//-----------------------------------------------------------------------

inline std::vector<NetworkId> const & DroidCommandProgrammingMessage::getChipsToAdd() const
{
	return m_chipsToAdd;
}

//-----------------------------------------------------------------------

inline std::vector<NetworkId> const & DroidCommandProgrammingMessage::getChipsToRemove() const
{
	return m_chipsToRemove;
}

//-----------------------------------------------------------------------

inline NetworkId const & DroidCommandProgrammingMessage::getDroidControlDevice() const
{
	return m_droidControlDevice;
}

//-----------------------------------------------------------------------

inline void DroidCommandProgrammingMessage::setCommands(std::vector<std::string> const & commands)
{
	m_commands = commands;
}

//-----------------------------------------------------------------------

inline void DroidCommandProgrammingMessage::setChipsToAdd(std::vector<NetworkId> const & chipsToAdd)
{
	m_chipsToAdd = chipsToAdd;
}

//-----------------------------------------------------------------------

inline void DroidCommandProgrammingMessage::setChipsToRemove(std::vector<NetworkId> const & chipsToRemove)
{
	m_chipsToRemove = chipsToRemove;
}

//-----------------------------------------------------------------------

inline void DroidCommandProgrammingMessage::setDroidControlDevice(NetworkId const & droidControlDevice)
{
	m_droidControlDevice = droidControlDevice;
}

//-----------------------------------------------------------------------

#endif
// _INCLUDED_DroidCommandProgrammingMessage_H
