#include "sharedFoundation/FirstSharedFoundation.h"

#include "ConfigStationPlayersCollector.h"
#include "StationPlayersCollector.h"

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

int main(int argc, char ** argv)
{

	SetupSharedThread::install();
	SetupSharedDebug::install(1024);
 
	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);
	setupFoundationData.lpCmdLine = ConvertCommandLine(argc,argv);
	setupFoundationData.configFile = "stationPlayersCollector.cfg";
	SetupSharedFoundation::install (setupFoundationData);

	if (ConfigFile::isEmpty())
		FATAL(true, ("No config file specified"));
	
	SetupSharedCompression::install();

	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();

	SetupSharedRandom::install(static_cast<uint32>(time(nullptr))); //lint !e1924 !e64 // nullptr is a C-Style cast?

	Os::setProgramName("StationPlayersCollector");
	        
	//-- run game
	StationPlayersCollector::install();
	SetupSharedFoundation::callbackWithExceptionHandling(StationPlayersCollector::run);
	SetupSharedFoundation::remove();

	return 0;
}
