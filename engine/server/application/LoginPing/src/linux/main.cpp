#include "sharedFoundation/FirstSharedFoundation.h"

#include "ConfigLoginPing.h"
#include "LoginPing.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetwork/NetworkHandler.h"
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
	setupFoundationData.configFile = "loginPing.cfg";
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedNetworkMessages::install();

	//setup the ping
	ConfigLoginPing::install();
        
	//-- run game
	NetworkHandler::install();
	bool result = LoginPing::ping();
	if (result)
	{
		printf("Ok\n");
	}
	else
	{
		printf("Ping failed\n");
	}
	NetworkHandler::remove();
	ConfigLoginPing::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}
