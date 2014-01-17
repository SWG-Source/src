// Console.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "ConfigTaskManager.h"
#include "Console.h"
#include "ConsoleImplementation.h"
#include <cstdio>

//-----------------------------------------------------------------------

Console::Console() :
consoleImplementation(new ConsoleImplementation)
{
}

//-----------------------------------------------------------------------

Console::~Console()
{
	delete consoleImplementation;
}

//-----------------------------------------------------------------------

Console & Console::instance()
{
	static Console console;
	return console;
}

//-----------------------------------------------------------------------

const std::string Console::getNextCommand()
{
	return instance().consoleImplementation->popNextCommand();
}

//-----------------------------------------------------------------------

const bool Console::hasPendingCommand()
{
	return instance().consoleImplementation->hasPendingCommand();
}

//-----------------------------------------------------------------------

void Console::update()
{
	if(ConfigTaskManager::getAutoStart())
		return;
	
	static Console & con = instance();
	
	char ch = 0;
	while((ch = con.getNextChar()) != 0)
	{
		if(ch == 10 || ch == 13)
		{
			con.consoleImplementation->pushCommand(con.currentCommand);
			con.currentCommand.clear();
		}
		else
		{
			con.currentCommand += ch;
		}
	}
}

//-----------------------------------------------------------------------
