// main.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTransferServer.h"

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedThread/SetupSharedThread.h"
#include "TransferServer.h"
#include "ConfigTransferServer.h"

//-----------------------------------------------------------------------

int main(int argc, char ** argv)
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	setupFoundationData.argc                      = argc;
	setupFoundationData.argv                      = argv;
	setupFoundationData.createWindow           = true;
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();
	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);
	NetworkHandler::install();

	//Os::setProgramName("TransferServer");
	ConfigTransferServer::install();

	//-- run server
	SetupSharedFoundation::callbackWithExceptionHandling(TransferServer::run);

	NetworkHandler::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

//-----------------------------------------------------------------------

