#include "FirstConnectionServer.h"
#include "ConfigConnectionServer.h"
#include "ConnectionServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

// ======================================================================

void dumpPid(const char * argv)
{
	pid_t p = getpid();
	char fileName[1024];
	sprintf(fileName, "%s.%d", argv, p);
	FILE * f = fopen(fileName, "w+");
	fclose(f);
}

int main(int argc, char ** argv)
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);
	setupFoundationData.lpCmdLine                   = ConvertCommandLine(argc,argv);
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();
	SetupSharedRandom::install(time(nullptr));

	//setup the server
	NetworkHandler::install();
	Os::setProgramName("ConnectionServer");
	ConfigConnectionServer::install();
        
	//-- run game
	ConnectionServer::install();
	SetupSharedFoundation::callbackWithExceptionHandling(ConnectionServer::run);
	ConnectionServer::remove();
  
	ConfigConnectionServer::remove();
	NetworkHandler::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

#ifdef ENABLE_PROFILING
	exit(0);
#endif
	
	return 0;
}
