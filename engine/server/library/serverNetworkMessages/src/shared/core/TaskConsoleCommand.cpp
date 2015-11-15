// TaskConsoleCommand.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "TaskConsoleCommand.h"

//-----------------------------------------------------------------------

TaskConsoleCommand::TaskConsoleCommand(const std::string & consoleCommand) :
GameNetworkMessage("TaskConsoleCommand"),
m_command(consoleCommand)
{
	addVariable(m_command);
}

//-----------------------------------------------------------------------

TaskConsoleCommand::TaskConsoleCommand(Archive::ReadIterator & source) :
GameNetworkMessage("TaskConsoleCommand"),
m_command()
{
	addVariable(m_command);
	unpack(source);
}

//-----------------------------------------------------------------------

TaskConsoleCommand::~TaskConsoleCommand()
{
}

//-----------------------------------------------------------------------

const std::string & TaskConsoleCommand::getCommand() const
{
	return m_command.get();
}

//-----------------------------------------------------------------------

