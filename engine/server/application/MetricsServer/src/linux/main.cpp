#include "sharedFoundation/FirstSharedFoundation.h"

#include "ConfigMetricsServer.h"
#include "MetricsServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

#include <string>

// ======================================================================

int main(int argc, char ** argv)
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);
	setupFoundationData.lpCmdLine = ConvertCommandLine(argc,argv);
	SetupSharedFoundation::install (setupFoundationData);

	{
		//SetupSharedObject::Data data;
		//SetupSharedObject::setupDefaultGameData(data);
		//SetupSharedObject::install(data);
	}
	SetupSharedCompression::install();
	SetupSharedFile::install(32);

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	SetupSharedRandom::install(static_cast<uint32>(time(nullptr))); //lint !e1924 !e64 // nullptr is a C-Style cast?

	Os::setProgramName("MetricsServer");
	//setup the server
	ConfigMetricsServer::install();

	//set command line
	std::string cmdLine = setupFoundationData.lpCmdLine;
	size_t firstArg = cmdLine.find(" ", 0);
	size_t lastSlash = 0;
	size_t nextSlash = 0;
	while(nextSlash < firstArg)
	{
		nextSlash = cmdLine.find("/", lastSlash);
		if(nextSlash == cmdLine.npos || nextSlash >= firstArg) //lint !e1705 static class members may be accessed by the scoping operator (huh?)
			break;
		lastSlash = nextSlash + 1;
	}
	cmdLine = cmdLine.substr(lastSlash);
	MetricsServer::setCommandLine(cmdLine);

	
	//-- run game
	NetworkHandler::install();
	MetricsServer::install();
	MetricsServer::run();
	MetricsServer::remove();
	NetworkHandler::remove();
	SetupSharedFoundation::remove();

	return 0;
}


