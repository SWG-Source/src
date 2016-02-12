// ConsoleCommandParser.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstPlanetServer.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedLog/Log.h"
#include "ConsoleCommandParser.h"
#include "UnicodeUtils.h"
#include <vector>
#include "Unicode.h"

#pragma warning(disable:4355) // Passing "this" as a parameter to the base class constructor
//-----------------------------------------------------------------------

namespace CommandNames
{
#define MAKE_COMMAND(a) const char * const a = #a
#undef MAKE_COMMAND
}

const CommandParser::CmdInfo cmds[] =
{
	{"planet", 0, "", ""},
	{"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------------

ConsoleCommandParser::ConsoleCommandParser() :
CommandParser ("", 0, "...", "console commands", nullptr)
{
	createDelegateCommands(cmds);
}

//-----------------------------------------------------------------------

ConsoleCommandParser::~ConsoleCommandParser()
{
}

//-----------------------------------------------------------------------

bool ConsoleCommandParser::performParsing(const NetworkId & /* track */, const StringVector_t & argv, const String_t &, String_t & result, const CommandParser *)
{
	bool successResult = false;

	if(isCommand(argv[0], "planet"))
	{
		if(argv.size() > 2)
		{
			if(isCommand(argv[2], "runState"))
			{
				successResult = true;
				result += Unicode::narrowToWide("running");
			}
		}
	}
		
	return successResult;
}

//-----------------------------------------------------------------------
