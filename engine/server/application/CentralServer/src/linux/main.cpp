#include "sharedFoundation/FirstSharedFoundation.h"

#include "ConfigCentralServer.h"
#include "CentralServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/Os.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

// ======================================================================

int main(int argc, char ** argv)
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);
	setupFoundationData.lpCmdLine  = ConvertCommandLine(argc,argv);
	SetupSharedFoundation::install(setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedFile::install(false);
	SetupSharedRandom::install(time(0)); //lint !e732

	NetworkHandler::install();
	Os::setProgramName("CentralServer");
	ConfigCentralServer::install();

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
	CentralServer::getInstance().setCommandLine(cmdLine);
	//-- run game
	SetupSharedFoundation::callbackWithExceptionHandling(CentralServer::run);
	NetworkHandler::remove();
	SetupSharedFoundation::remove();

	return 0;
}
