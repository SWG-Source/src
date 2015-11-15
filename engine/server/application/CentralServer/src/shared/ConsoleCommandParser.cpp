// ConsoleCommandParser.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "ConsoleCommandParser.h"
#include "UnicodeUtils.h"

#pragma warning(disable:4355) //'this' : used in base member initializer list

//-----------------------------------------------------------------------

namespace CommandNames
{
#define MAKE_COMMAND(a) const char * const a = #a
#undef MAKE_COMMAND
}

const CommandParser::CmdInfo cmds[] =
{
	{"transfer", 0, "", "send a request to the transfer server"},
	{"runState", 0, "", "Return the running state of the CentralServer. It should always return 'running'"},
	{"dbconnected", 0, "", "returns 1 if the database has connected"},
	{"getPlanetServersCount", 0, "", "returns the number of planet servers connected to the central server"},
	{"planet", 0, "", "send a request to the planet server"},
	{"hasPlanetServer", 1, "", "returns true if the CentralServer has a connection with the specified planet server"},
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

bool ConsoleCommandParser::performParsing(const NetworkId & track, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser *)
{
	bool successResult = false;
	if(argv.size() > 0)
	{
		if(isCommand(argv[0], "transfer"))
		{
			ConGenericMessage cm(Unicode::wideToNarrow(originalCommand), static_cast<const unsigned int>(track.getValue()));
			CentralServer::getInstance().sendToTransferServer(cm);
		}
		if(isCommand(argv[0], "planet"))
		{
			if(argv.size() > 1)
			{
				ConGenericMessage cm(Unicode::wideToNarrow(originalCommand), static_cast<const unsigned int>(track.getValue()));
				// which planet server?
				std::string sceneName = Unicode::wideToNarrow(argv[1]);

				if(sceneName == "all")
				{
					CentralServer::getInstance().sendToAllPlanetServers(cm, true);
					successResult = true;
				}
				else
				{
					if(CentralServer::getInstance().hasPlanetServer(sceneName))
					{
						CentralServer::getInstance().sendToPlanetServer(sceneName, cm, true);
						successResult = true;
					}
				}					
			}
		}
		
		if(isCommand(argv[0], "runState"))
		{
			result += Unicode::narrowToWide("running");
			successResult = true;
		}
		if(isCommand(argv[0], "dbconnected"))
		{
			GameServerConnection * g = CentralServer::getInstance().getGameServer(CentralServer::getInstance().getDbProcessServerProcessId());
			if(g)
			{
				result += Unicode::narrowToWide("1");
			}
			else
			{
				result += Unicode::narrowToWide("0");
			}
			successResult = true;			
		}
		if(isCommand(argv[0], "getPlanetServersCount"))
		{
			char countBuf[4];
			snprintf(countBuf, sizeof(countBuf), "%d", CentralServer::getInstance().getPlanetServersCount());
			countBuf[3] = 0;
			result += Unicode::narrowToWide(countBuf);
			successResult = true;
		}
		if(isCommand(argv[0], "hasPlanetServer"))
		{
			if(argv.size() > 1)
			{
				std::string sceneName = Unicode::wideToNarrow(argv[1]);
				successResult = true;
				if(CentralServer::getInstance().hasPlanetServer(sceneName))
				{
					result += Unicode::narrowToWide("1");
				}
				else
				{
					result += Unicode::narrowToWide("0");
				}
			}
															  
		}		
	}
	return successResult;
}

//-----------------------------------------------------------------------

