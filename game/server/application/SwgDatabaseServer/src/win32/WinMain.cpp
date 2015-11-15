#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"

#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"

#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedLog/LogManager.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"
#include "SwgDatabaseServer/SwgDatabaseServer.h"

#include <time.h>


//_____________________________________________________________________
int main(int argc, char ** argv)
{
	// command line hack
	std::string cmdLine;
	for(int i = 1; i < argc; ++i)
	{
		cmdLine += argv[i];
		if(i + 1 < argc)
		{
			cmdLine += " ";
		}
	}

	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	// -- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	setupFoundationData.commandLine               = cmdLine.c_str();
	setupFoundationData.createWindow              = false;
	setupFoundationData.clockUsesSleep            = true;
	
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedFile::install(false);
	SetupSharedRandom::install(int(time(NULL)));
	{
		SetupSharedObject::Data data;
		SetupSharedObject::setupDefaultGameData(data);
		SetupSharedObject::install(data);
	}

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	SetupSharedNetworkMessages::install();

	SetupSharedLog::install("SwgDatabaseServer");
        
	TreeFile::addSearchAbsolute(0);
	TreeFile::addSearchPath(".",0);
        
	//-- setup server
	ConfigServerDatabase::install ();

	NetworkHandler::install();

	SwgDatabaseServer::install();
        
	//-- run server
	SetupSharedFoundation::callbackWithExceptionHandling(SwgDatabaseServer::runStatic);

	NetworkHandler::remove();
	SetupSharedLog::remove();

	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

//_____________________________________________________________________
