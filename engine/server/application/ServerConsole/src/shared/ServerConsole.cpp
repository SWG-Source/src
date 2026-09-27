// ServerConsole.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstServerConsole.h"
#include "ConfigServerConsole.h"
#include "sharedFoundation/Os.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "ServerConsole.h"
#include "ServerConsoleConnection.h"
#include <cstdio>
#include <string>

//-----------------------------------------------------------------------

namespace ServerConsoleNamespace
{
	ServerConsoleConnection * s_serverConnection = 0;
	bool                   s_done = false;
}

using namespace ServerConsoleNamespace;

//-----------------------------------------------------------------------

ServerConsole::ServerConsole()
{
}

//-----------------------------------------------------------------------

ServerConsole::~ServerConsole()
{
}

//-----------------------------------------------------------------------

void ServerConsole::done()
{
	s_done = true;
}

//-----------------------------------------------------------------------

void ServerConsole::run()
{
	if(!ConfigServerConsole::getServerAddress())
		return;

	if(!ConfigServerConsole::getServerPort())
		return;

	if(stdin)
	{
		// Read stdin to end of file as bytes. Each fread returns the number
		// of bytes it read, so input shorter than the buffer is kept.
		std::string input;
		char inBuf[1024];
		size_t bytesRead;
		while ((bytesRead = fread(inBuf, 1, sizeof(inBuf), stdin)) > 0)
			input.append(inBuf, bytesRead);

		// fread also returns 0 on a read error; never send a partial command.
		if(ferror(stdin))
		{
			fprintf(stderr, "Error reading the command from stdin. Aborting");
			return;
		}

		if(input.length() > 0)
		{
			// connect to the server
			s_serverConnection = new ServerConsoleConnection(ConfigServerConsole::getServerAddress(), ConfigServerConsole::getServerPort());
			ConGenericMessage msg(input);
			s_serverConnection->send(msg);

			while(! s_done)
			{
				NetworkHandler::update();
				NetworkHandler::dispatch();
				Os::sleep(1);
			}
		}
		else
		{
			fprintf(stderr, "Nothing to send to the server. Aborting");
		}
	}
}

//-----------------------------------------------------------------------
