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
#include <iostream>

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

    std::string input;
    getline(std::cin, input); 
    
    s_serverConnection = new ServerConsoleConnection(ConfigServerConsole::getServerAddress(), ConfigServerConsole::getServerPort());
    ConGenericMessage msg(input);
    s_serverConnection->send(msg);
    
    while(! s_done)
    {
        NetworkHandler::update();
        NetworkHandler::dispatch();
        Os::sleep(1);
    }

    fprintf(stdout, "\n");
}

//-----------------------------------------------------------------------

