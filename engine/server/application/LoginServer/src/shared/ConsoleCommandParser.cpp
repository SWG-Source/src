// ConsoleCommandParser.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstLoginServer.h"
#include "ConsoleCommandParser.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

namespace CommandNames
{
#define MAKE_COMMAND(a) const char * const a = #a
#undef MAKE_COMMAND
}

const CommandParser::CmdInfo cmds[] =
{
	{"runState", 0, "", "Return the running state of the CentralServer. It should always return 'running'"},
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

bool ConsoleCommandParser::performParsing(const NetworkId &, const StringVector_t & argv, const String_t &, String_t & result, const CommandParser *)
{
	bool successResult = false;

	if(isCommand(argv[0], "runState"))
	{
		result += Unicode::narrowToWide("running");
		successResult = true;
	}

	return successResult;
}

//-----------------------------------------------------------------------

