// ======================================================================
//
// main.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include <signal.h>

#include "sharedFoundation/FirstSharedFoundation.h"
#include "LogServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedThread/SetupSharedThread.h"

#ifdef ENABLE_PROFILING
inline void signalHandler(int s){
    printf("LogServer terminating, signal %d\n",s);
    exit(0);
}
#endif

// ======================================================================

int main(int argc, char **argv)
{
	struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

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

	return 0;
}

// ======================================================================

