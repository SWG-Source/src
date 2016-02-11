// ConsoleCommandParserGame.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "CentralServer.h"
#include "ConsoleCommandParserGame.h"
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
	MAKE_COMMAND(game);
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

ConsoleCommandParserGame::ConsoleCommandParserGame() :
CommandParser ("game", 0, "...", "console commands", 0)
{
}

//-----------------------------------------------------------------------

ConsoleCommandParserGame::~ConsoleCommandParserGame()
{
}

//-----------------------------------------------------------------------

bool ConsoleCommandParserGame::performParsing(const NetworkId & track, const StringVector_t & argv,const String_t & originalCommand,String_t & result,const CommandParser *)
{
	bool successResult = false;

	if(isCommand(argv[0], CommandNames::game))
	{
		if(argv.size() > 1)
		{
			std::string cmd = Unicode::wideToNarrow(argv[1]);
			if(cmd == "any")
			{
				// grab a random game server to execute this command
				GameServerConnection * gameConn = CentralServer::getInstance().getRandomGameServer();
				if(gameConn)
				{
					ConGenericMessage cm(Unicode::wideToNarrow(originalCommand), static_cast<unsigned int>(track.getValue()));
					gameConn->send(cm, true);
					successResult = true;
				}
				else
				{
					result += Unicode::narrowToWide("No game servers available to service request");
					ConsoleConnection::onCommandComplete(Unicode::wideToNarrow(result), static_cast<int>(track.getValue()));
				}

			}
			else if(cmd == "kill")
			{
				// kill the game server process specified (by disconnecting it from central)
				if(argv.size() > 2)
				{
					unsigned int pid;
					sscanf(Unicode::wideToNarrow(argv[2]).c_str(), "%d", &pid);
					GameServerConnection * gameConn = CentralServer::getInstance().getGameServer(pid);
					if(gameConn)
					{
						gameConn->setDisconnectReason("Central ConsoleCommandParserGame kill command");
						gameConn->disconnect();
						successResult = true;
						ConsoleConnection::onCommandComplete(Unicode::wideToNarrow(result), static_cast<int>(track.getValue()));

					}
				}
			}
			else if(cmd == "enumerate")
			{
				std::vector<const GameServerConnection *> gameServers = CentralServer::getInstance().getGameServers();
				std::vector<const GameServerConnection *>::const_iterator i;
				for(i = gameServers.begin(); i != gameServers.end(); ++i)
				{
					char entry[256] = {"\0"};
					IGNORE_RETURN(snprintf(entry, sizeof(entry), "%lu, %lu, %s, %s\n", (*i)->getProcessId(), (*i)->getOsProcessId(), (*i)->getRemoteAddress().c_str(), (*i)->getSceneId().c_str()));
					result += Unicode::narrowToWide(entry);
				}
				ConsoleConnection::onCommandComplete(Unicode::wideToNarrow(result), static_cast<int>(track.getValue()));
			}
			else if( cmd == "abortShutdown" )
			{
				LOG("ServerConsole", ("Received command to abort the current shutdown sequence."));
				CentralServer::getInstance().abortShutdownProcess();
				result += Unicode::narrowToWide("Instructing Central Server to abort shutdown sequence.\n");
				successResult = true;
				ConsoleConnection::onCommandComplete(Unicode::wideToNarrow(result), static_cast<int>(track.getValue()));
			}
			// starts the shutdown process
			else if(cmd == "shutdown")
			{
				if( argv.size() > 4 )
				{
					LOG("ServerConsole", ("Received command to shutdown the cluster."));
					uint32 timeToShutdown = strtoul(Unicode::wideToNarrow(argv[2]).c_str(), nullptr, 10);
					uint32 maxTime = strtoul(Unicode::wideToNarrow(argv[3]).c_str(), nullptr, 10);
					Unicode::String systemMessage = Unicode::narrowToWide("");
					for(unsigned int i = 4; i < argv.size(); ++i)
					{
						systemMessage += argv[i] + Unicode::narrowToWide(" ");
					}
					CentralServer::getInstance().startShutdownProcess(timeToShutdown, maxTime, systemMessage);
					result += Unicode::narrowToWide("Instructing Central Server to begin shutdown sequence.\n");
					successResult = true;
					ConsoleConnection::onCommandComplete(Unicode::wideToNarrow(result), static_cast<int>(track.getValue()));
				}
				// failed to provide enough parameters
				else
				{
					LOG("ServerConsole", ("Received an improperly formatted shutdown command."));
					result += Unicode::narrowToWide("Not enough parameters specified. Usage is \"game shutdown <time to shutdown in seconds> <max time to wait in seconds> <system broadcast message>\"\n");
					ConsoleConnection::onCommandComplete(Unicode::wideToNarrow(result), static_cast<int>(track.getValue()));
				}
			}
			else if(cmd == "authorizeTransfer")
			{
				if(argv.size() > 1)
				{
					unsigned int stationId = strtoul(Unicode::wideToNarrow(argv[2]).c_str(), nullptr, 10);
					if(stationId > 0)
					{
						GenericValueTypeMessage<unsigned int> auth("AuthorizeDownload", stationId);
						CentralServer::getInstance().sendToTransferServer(auth);
						result += Unicode::narrowToWide("sent authorization request");
					}
				}
				ConsoleConnection::onCommandComplete(Unicode::wideToNarrow(result), static_cast<int>(track.getValue()));
			}
			else if(cmd == "requestMove")
			{
				if(argv.size() > 5)
				{
					ConGenericMessage cm(Unicode::wideToNarrow(originalCommand), static_cast<unsigned int>(track.getValue()));
					CentralServer::getInstance().sendToTransferServer(cm);
				}
			}
			else
			{
				if(! isalpha(cmd.c_str()[0]))
				{
					// check to see if the next parameter is a pid to send the command to
					uint32 pid;
					sscanf(Unicode::wideToNarrow(argv[1]).c_str(), "%lu", &pid);
					GameServerConnection * gameConn = CentralServer::getInstance().getGameServer(pid);
					if(gameConn)
					{
						ConGenericMessage cm(Unicode::wideToNarrow(originalCommand), static_cast<unsigned int>(track.getValue()));
						gameConn->send(cm, true);
						successResult = true;
					}
					else
					{
						result += Unicode::narrowToWide("No game servers available to service request");
						ConsoleConnection::onCommandComplete(Unicode::wideToNarrow(result), static_cast<int>(track.getValue()));
					}
				}
				else
				{
					// a planet 
					GameServerConnection * gameConn = CentralServer::getInstance().getGameServer(Unicode::wideToNarrow(argv[1]));
					if(gameConn)
					{
						ConGenericMessage cm(Unicode::wideToNarrow(originalCommand), static_cast<unsigned int>(track.getValue()));
						gameConn->send(cm, true);
						successResult = true;
					}
					else
					{
						result += Unicode::narrowToWide("No game servers available to service request");
						ConsoleConnection::onCommandComplete(Unicode::wideToNarrow(result), static_cast<int>(track.getValue()));
					}
				}
			}
		}
		else
		{
			// select a game server to execute this command
			result += Unicode::narrowToWide("insufficient arguments");
			successResult = true;
		}
	}

	return successResult;
}

//-----------------------------------------------------------------------

