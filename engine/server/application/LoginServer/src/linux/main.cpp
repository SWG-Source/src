#include "sharedFoundation/FirstSharedFoundation.h"

#ifdef ENABLE_PROFILING
#include <signal.h>
#endif

#include "ConfigLoginServer.h"
#include "LoginServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

// ======================================================================

#ifdef ENABLE_PROFILING
inline void signalHandler(int s){
    printf("LoginServer terminating, signal %d\n",s);
    exit(0);
}
#endif

int main(int argc, char **argv) {
#ifdef ENABLE_PROFILING
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
#endif

    SetupSharedThread::install();
    SetupSharedDebug::install(1024);

    //-- setup foundation
    SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);
    setupFoundationData.lpCmdLine = ConvertCommandLine(argc, argv);
    setupFoundationData.configFile = "loginServer.cfg";
    SetupSharedFoundation::install(setupFoundationData);

    if (ConfigFile::isEmpty()) {
        FATAL(true, ("No config file specified"));
    }

    SetupSharedCompression::install();

    SetupSharedFile::install(false);
    SetupSharedNetworkMessages::install();

    SetupSharedRandom::install(static_cast<uint32>(time(nullptr))); //lint !e1924 !e64 // nullptr is a C-Style cast?

    Os::setProgramName("LoginServer");
    //setup the server
    ConfigLoginServer::install();

    //-- run game
    SetupSharedFoundation::callbackWithExceptionHandling(LoginServer::run);
    SetupSharedFoundation::remove();

    return 0;
}
