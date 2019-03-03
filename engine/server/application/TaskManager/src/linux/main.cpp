#include <signal.h>

#include "sharedFoundation/FirstSharedFoundation.h"

#include "ConfigTaskManager.h"
#include "TaskManager.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

#ifdef ENABLE_PROFILING
extern "C" int __llvm_profile_write_file(void);
#endif

inline void signalHandler(int s){
    printf("TaskManager terminating, signal %d\n",s);

#ifdef ENABLE_PROFILING
    __llvm_profile_write_file();
#endif

    exit(0);
}

// ======================================================================

int main(int argc, char ** argv)
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
	setupFoundationData.lpCmdLine = ConvertCommandLine(argc,argv);
	setupFoundationData.configFile = "taskmanager.cfg";
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();
	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();
	SetupSharedRandom::install(static_cast<uint32>(time(nullptr))); //lint !e1924 !e64 // nullptr is a C-Style cast?

	Os::setProgramName("TaskManager");
	//setup the server
	ConfigTaskManager::install();
        
	//-- run game
	TaskManager::install();
	TaskManager::run();
	TaskManager::remove();
	SetupSharedFoundation::remove();

#ifdef ENABLE_PROFILING
	__llvm_profile_write_file();
	exit(0);
#endif

	return 0;
}
