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

#include <cstdio>

//_____________________________________________________________________
int main(int argc, char ** argv)
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	setupFoundationData.argc                      = argc;
	setupFoundationData.argv                      = argv;
	setupFoundationData.createWindow              = false;
	setupFoundationData.clockUsesSleep            = true;
	
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();
	SetupSharedFile::install(false);

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	SetupSharedNetworkMessages::install();

	SetupSharedRandom::install(int(time(NULL)));

	SetupSharedUtility::Data sharedUtilityData;
	SetupSharedUtility::setupGameData (sharedUtilityData);
	SetupSharedUtility::install (sharedUtilityData);

	//-- setup server
	ConfigPlanetServer::install ();
        
	char tmp[92];
	sprintf(tmp, "PlanetServer:%d", Os::getProcessId());
	SetupSharedLog::install(tmp);

	//-- run server
	SetupSharedFoundation::callbackWithExceptionHandling(PlanetServer::run);

	SetupSharedLog::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

//_____________________________________________________________________
