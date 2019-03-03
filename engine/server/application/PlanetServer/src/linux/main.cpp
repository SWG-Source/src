#include <signal.h>

#include "FirstPlanetServer.h"
#include "ConfigPlanetServer.h"
#include "PlanetServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"

#ifdef ENABLE_PROFILING
extern "C" int __llvm_profile_write_file(void);
#endif

inline void signalHandler(int s){
    printf("PlanetServer terminating, signal %d\n",s);

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
	fclose(f); //lint !e668
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
	setupFoundationData.lpCmdLine = ConvertCommandLine(argc,argv);
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();
	SetupSharedFile::install(false);

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	SetupSharedNetworkMessages::install();

	SetupSharedRandom::install(time(nullptr)); //lint !e732 loss of sign (of 0!)

	SetupSharedUtility::Data sharedUtilityData;
	SetupSharedUtility::setupGameData (sharedUtilityData);
	SetupSharedUtility::install (sharedUtilityData);

	Os::setProgramName("PlanetServer");
	ConfigPlanetServer::install();

	char tmp[92];
	sprintf(tmp, "PlanetServer:%d", Os::getProcessId());
	SetupSharedLog::install(tmp);

//-- run game
	PlanetServer::run();

	SetupSharedLog::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

#ifdef ENABLE_PROFILING
	__llvm_profile_write_file();
	exit(0);
#endif
	return 0;
}
