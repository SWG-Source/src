// ConsoleImplementation.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "ConsoleImplementation.h"

//-----------------------------------------------------------------------

ConsoleImplementation::ConsoleImplementation()
{
}

//-----------------------------------------------------------------------

ConsoleImplementation::~ConsoleImplementation()
{
}

//-----------------------------------------------------------------------

const bool ConsoleImplementation::hasPendingCommand() const
{
	return (!m_pendingCommands.empty());
}

//-----------------------------------------------------------------------

const std::string ConsoleImplementation::popNextCommand()
{
	std::string result;
	if(hasPendingCommand())
	{
		result = m_pendingCommands.front();
		m_pendingCommands.pop_front();
	}
	return result;
}

//-----------------------------------------------------------------------

void ConsoleImplementation::pushCommand(const std::string & newCommand)
{
	m_pendingCommands.push_back(newCommand);
}

//-----------------------------------------------------------------------

