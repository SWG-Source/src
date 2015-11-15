#include "FirstConnectionServer.h"
#include "ConfigConnectionServer.h"
#include "ConnectionServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

#include <time.h>

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

//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

//	setupFoundationData.hInstance                 = hInstance;
	setupFoundationData.commandLine               = cmdLine.c_str();
	setupFoundationData.createWindow              = false;
	setupFoundationData.clockUsesSleep            = true;
	
	SetupSharedFoundation::install (setupFoundationData);
	SetupSharedFile::install(false);
	SetupSharedCompression::install();

	SetupSharedNetworkMessages::install();
	SetupSharedRandom::install(int(time(NULL)));

	//-- setup game server
	ConfigConnectionServer::install ();

	ConnectionServer::install();
	//-- run game
	SetupSharedFoundation::callbackWithExceptionHandling(ConnectionServer::run);
	ConnectionServer::remove();
	ConfigConnectionServer::remove();
	SetupSharedFoundation::remove();
	PerThreadData::threadRemove();
	return 0;
}
