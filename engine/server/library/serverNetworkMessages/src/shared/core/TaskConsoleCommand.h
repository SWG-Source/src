// TaskConsoleCommand.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskConsoleCommand_H
#define	_INCLUDED_TaskConsoleCommand_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include <string>

//-----------------------------------------------------------------------

class TaskConsoleCommand : public GameNetworkMessage
{
public:
	explicit TaskConsoleCommand(const std::string & command);
	explicit TaskConsoleCommand(Archive::ReadIterator & source);
	~TaskConsoleCommand();

	const std::string & getCommand() const;

private:
	TaskConsoleCommand & operator = (const TaskConsoleCommand & rhs);
	TaskConsoleCommand(const TaskConsoleCommand & source);

	Archive::AutoVariable<std::string>  m_command;

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskConsoleCommand_H
