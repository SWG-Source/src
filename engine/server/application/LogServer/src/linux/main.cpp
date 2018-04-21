// ======================================================================
//
// main.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "LogServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/Os.h"
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

	setupFoundationData.lpCmdLine = ConvertCommandLine(argc, argv);

	SetupSharedFoundation::install(setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();

	Os::setProgramName("LogServer");
	//-- setup server
	LogServer::install();

	//-- run server
	LogServer::run();

	LogServer::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

#ifdef ENABLE_PROFILING
	exit(0);
#endif

	return 0;
}

// ======================================================================

