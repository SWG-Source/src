// ConsoleCommandParser.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedLog/Log.h"
#include "ConsoleCommandParser.h"
#include "TransferServer.h"
#include "UnicodeUtils.h"
#include <vector>
#include "Unicode.h"

//-----------------------------------------------------------------------

namespace CommandNames
{
#define MAKE_COMMAND(a) const char * const a = #a
#undef MAKE_COMMAND
}

const CommandParser::CmdInfo cmds[] =
{
	{"transfer", 4, "<requestMove> <station id> <character name> <source server> <destination server>", "Request a character transfer for <character name> from <source server> to <destination server>"},
	{"transfer", 2, "<requestAccountMove> <source station id> <destination station id>", "Request an account transfer from <source station id> to <destination station id>"},
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

bool ConsoleCommandParser::performParsing(const NetworkId & track, const StringVector_t & argv, const String_t &, String_t & result, const CommandParser *)
{
	bool successResult = false;

	if(isCommand(argv[0], "transfer"))
	{
		if(isCommand(argv[1], "requestMove"))
		{
			if(argv.size() > 5)
			{
				std::string stationIdString = Unicode::wideToNarrow(argv[2]);
				std::string characterName = Unicode::wideToNarrow(argv[3]);
				std::string sourceServer = Unicode::wideToNarrow(argv[4]);
				std::string destinationServer = Unicode::wideToNarrow(argv[5]);
				
				LOG("CustomerService", ("CharacterTransfer: received console request to transfer %s, belonging to %s from %s to %s", characterName.c_str(), stationIdString.c_str(), sourceServer.c_str(), destinationServer.c_str()));

				result += Unicode::narrowToWide("Received request to move ");
				result += Unicode::narrowToWide(characterName);
				result += Unicode::narrowToWide(" owned by station ID ");
				result += Unicode::narrowToWide(stationIdString);
				result += Unicode::narrowToWide(" on cluster ");
				result += Unicode::narrowToWide(sourceServer);
				result += Unicode::narrowToWide(" to cluster ");
				result += Unicode::narrowToWide(destinationServer);

				const unsigned int stationId = static_cast<const unsigned int>(atoi (stationIdString.c_str()));
				const unsigned int trackId = static_cast<const unsigned int>(track.getValue());

				//todo : drive these through the command handler, just testing
				// basic functionality for now.
				const bool withItems = true;
				const bool allowOverride = true;
				
				TransferServer::requestMove(trackId, "en", sourceServer, destinationServer, characterName, characterName, stationId, stationId, trackId, withItems, allowOverride);
				successResult = true;
			}
			else
			{
				std::string messageString = "not enough arguments to 'requestMove <station id> <character name> <source server> <destination server>'";
				StringVector_t::const_iterator i;
				
				for(i = argv.begin(); i != argv.end(); ++i)
				{
					messageString = messageString + Unicode::wideToNarrow(*i) + " | ";
				}
				LOG("CustomerService", ("CharacterTransfer: %s", messageString.c_str()));
				result += Unicode::narrowToWide(messageString);
			}
		}
		else if (isCommand(argv[1], "requestAccountMove"))
		{
			if (argv.size() > 3)
			{
				std::string sourceStationIdString      = Unicode::wideToNarrow(argv[2]);
				std::string destinationStationIdString = Unicode::wideToNarrow(argv[3]);

				LOG("CustomerService", ("CharacterTransfer: received console request to transfer account from station ID %s to station ID %s", sourceStationIdString.c_str(), destinationStationIdString.c_str()));

				result += Unicode::narrowToWide("Received request to move from station ID ");
				result += Unicode::narrowToWide(sourceStationIdString);
				result += Unicode::narrowToWide(" to station ID ");
				result += Unicode::narrowToWide(destinationStationIdString);
				result += Unicode::narrowToWide("\n");

				const unsigned int sourceStationId      = static_cast<const unsigned int>(atoi (sourceStationIdString.c_str()));
				const unsigned int destinationStationId = static_cast<const unsigned int>(atoi (destinationStationIdString.c_str()));
				const unsigned int trackId              = static_cast<const unsigned int>(track.getValue());

				TransferServer::requestTransferAccount(trackId, sourceStationId, destinationStationId, trackId);
				successResult = true;
			}
			else
			{
				std::string messageString = "not enough arguments to 'requestAccountMove <source station id> <destination station id>'";
				StringVector_t::const_iterator i;
				
				for(i = argv.begin(); i != argv.end(); ++i)
				{
					messageString = messageString + Unicode::wideToNarrow(*i) + " | ";
				}
				LOG("CustomerService", ("CharacterTransfer: %s\n", messageString.c_str()));
				result += Unicode::narrowToWide(messageString);
				result += Unicode::narrowToWide("\n");
			}
		}
	}
		
	return successResult;
}

//-----------------------------------------------------------------------
