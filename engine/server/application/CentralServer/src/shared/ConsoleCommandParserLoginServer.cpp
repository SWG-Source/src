// ConsoleCommandParserLoginServer.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "CentralServer.h"
#include "ConsoleCommandParserLoginServer.h"
#include "ConsoleConnection.h"
#include "GameServerConnection.h"
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

/*
const CommandParser::CmdInfo cmds[] =
{
	{CommandNames::game, 1, "", ""},
	{"", 0, "", ""} // this must be last
};
*/

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
		ConGenericMessage cm(Unicode::wideToNarrow(originalCommand), static_cast<unsigned int>(track.getValue()));
		CentralServer::getInstance().sendToArbitraryLoginServer(cm);
	}

	return successResult;
}

//-----------------------------------------------------------------------

