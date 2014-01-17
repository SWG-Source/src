// ConsoleCommandParserDatabase.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "CentralServer.h"
#include "ConsoleCommandParserDatabase.h"
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
	MAKE_COMMAND(database);
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

ConsoleCommandParserDatabase::ConsoleCommandParserDatabase() :
CommandParser ("database", 0, "...", "console commands", 0)
{
}

//-----------------------------------------------------------------------

ConsoleCommandParserDatabase::~ConsoleCommandParserDatabase()
{
}

//-----------------------------------------------------------------------

bool ConsoleCommandParserDatabase::performParsing(const NetworkId & track, const StringVector_t & argv,const String_t & originalCommand,String_t & result,const CommandParser *)
{
	bool successResult = false;

	if(isCommand(argv[0], CommandNames::database))
	{
		ConGenericMessage cm(Unicode::wideToNarrow(originalCommand), static_cast<unsigned int>(track.getValue()));
		CentralServer::getInstance().sendToDBProcess(cm, true);
		successResult = true;
	}

	return successResult;
}

//-----------------------------------------------------------------------

