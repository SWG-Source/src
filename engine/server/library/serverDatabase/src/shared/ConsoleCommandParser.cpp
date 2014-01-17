// ConsoleCommandParser.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverDatabase/FirstServerDatabase.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedLog/Log.h"
#include "ConsoleCommandParser.h"
#include "UnicodeUtils.h"
#include <vector>
#include "Unicode.h"

#pragma warning(disable: 4355) // Passing "this" to a base-class constructor

//-----------------------------------------------------------------------

namespace CommandNames
{
#define MAKE_COMMAND(a) const char * const a = #a
#undef MAKE_COMMAND
}

const CommandParser::CmdInfo cmds[] =
{
	{"database", 0, "", ""},
	{"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------------

ConsoleCommandParser::ConsoleCommandParser() :
CommandParser ("", 0, "...", "console commands", this)
{
	createDelegateCommands(cmds);
}

//-----------------------------------------------------------------------

ConsoleCommandParser::~ConsoleCommandParser()
{
}

//-----------------------------------------------------------------------

bool ConsoleCommandParser::performParsing(const NetworkId & /* track*/, const StringVector_t & argv, const String_t &, String_t & result, const CommandParser *)
{
	bool successResult = false;

	if(isCommand(argv[0], "database"))
	{
		if(argv.size() > 1)
		{
			if(isCommand(argv[1], "runState"))
			{
				successResult = true;
				result += Unicode::narrowToWide("running");
			}
		}
	}
	
	return successResult;
}

//-----------------------------------------------------------------------
