// main.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstServerConsole.h"

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedThread/SetupSharedThread.h"
#include "ServerConsole.h"
#include "ConfigServerConsole.h"

//-----------------------------------------------------------------------

int main(int argc, char ** argv)
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	setupFoundationData.argc                      = argc;
	setupFoundationData.argv                      = argv;
	setupFoundationData.clockUsesSleep            = true;
	setupFoundationData.createWindow              = false;
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();
	SetupSharedNetworkMessages::install();

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultClientSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);
	NetworkHandler::install();

	ConfigServerConsole::install();

	//-- run server
	SetupSharedFoundation::callbackWithExceptionHandling(ServerConsole::run);

	NetworkHandler::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

//-----------------------------------------------------------------------

