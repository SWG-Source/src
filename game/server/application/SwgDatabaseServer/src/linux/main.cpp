#include <signal.h>

#include "FirstSwgDatabaseServer.h"

#include "SwgDatabaseServer/SwgDatabaseServer.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedLog/LogManager.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "swgSharedNetworkMessages/SetupSwgSharedNetworkMessages.h"
#include "swgServerNetworkMessages/SetupSwgServerNetworkMessages.h"

#ifdef ENABLE_PROFILING
extern "C" int __llvm_profile_write_file(void);
#endif

inline void signalHandler(int s){
    printf("SwgDatabaseServer terminating, signal %d\n",s);

#ifdef ENABLE_PROFILING
    __llvm_profile_write_file();
#endif

    exit(0);
}

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
	struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

//	dumpPid(argv[0]);

	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);
	setupFoundationData.lpCmdLine                 = ConvertCommandLine(argc,argv);
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedFile::install(false);
	SetupSharedRandom::install(time(nullptr));
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	SetupSharedNetworkMessages::install();
	SetupSwgSharedNetworkMessages::install();
	SetupSwgServerNetworkMessages::install();

	SetupSharedLog::install("SwgDatabaseServer");

	TreeFile::addSearchAbsolute(0);
	TreeFile::addSearchPath(".",0);

	//-- setup server
	ConfigServerDatabase::install ();

	NetworkHandler::install();
	Os::setProgramName("SwgDatabaseServer");
	SwgDatabaseServer::install();

	//-- run server
	SwgDatabaseServer::getInstance().run();
	NetworkHandler::remove();
	SetupSharedLog::remove();

	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

#ifdef ENABLE_PROFILING
    __llvm_profile_write_file();
#endif

	return 0;
}
