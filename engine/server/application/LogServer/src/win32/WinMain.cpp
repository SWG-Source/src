// ======================================================================
//
// WinMain.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstLogServer.h"
#include "sharedFoundation/FirstSharedFoundation.h"
#include "LogServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedThread/SetupSharedThread.h"

// ======================================================================

int main(int argc, char **argv)
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	setupFoundationData.argc            = argc;
	setupFoundationData.argv            = argv;
	setupFoundationData.createWindow    = false;
	setupFoundationData.clockUsesSleep  = true;

	SetupSharedFoundation::install(setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();

	//-- setup server
	LogServer::install();

	//-- run server
	SetupSharedFoundation::callbackWithExceptionHandling(LogServer::run);

	LogServer::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

// ======================================================================
