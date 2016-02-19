// ConsoleCommandParserLoginServer.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstLoginServer.h"
#include "ConsoleCommandParserLoginServer.h"
#include "LoginServer.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "UnicodeUtils.h"
#include "sharedLog/Log.h"


//-----------------------------------------------------------------------

namespace CommandNames
{
#define MAKE_COMMAND(a) const char * const a = #a
	MAKE_COMMAND(login);
#undef MAKE_COMMAND
}

//-----------------------------------------------------------------------

ConsoleCommandParserLoginServer::ConsoleCommandParserLoginServer() :
CommandParser ("login", 0, "...", "console commands", 0)
{
}

//-----------------------------------------------------------------------

ConsoleCommandParserLoginServer::~ConsoleCommandParserLoginServer()
{
}

//-----------------------------------------------------------------------

bool ConsoleCommandParserLoginServer::performParsing(const NetworkId & track, const StringVector_t & argv,const String_t & originalCommand,String_t & result,const CommandParser *)
{
	bool successResult = false;

	if(isCommand(argv[0], CommandNames::login))
	{
		if(argv.size() > 1)
		{
			std::string cmd = Unicode::wideToNarrow(argv[1]);
			if(cmd == "runState")
			{
				result += Unicode::narrowToWide("running");
				successResult = true;
			}
			else if(cmd == "exit")
			{
				result += Unicode::narrowToWide("exiting");
				LoginServer::getInstance().setDone(true);
				successResult = true;
			}
		}
	}

	return successResult;
}

//-----------------------------------------------------------------------

