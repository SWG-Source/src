#include "FirstLoginServer.h"
#include "ConfigLoginServer.h"
#include "LoginServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

#include <time.h>

int main(int argc, char **argv)
{

	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	setupFoundationData.argc                      = argc;
	setupFoundationData.argv                      = argv;
	setupFoundationData.createWindow              = false;
	
	SetupSharedFoundation::install (setupFoundationData);
	SetupSharedNetworkMessages::install();

	SetupSharedCompression::install();

	SetupSharedFile::install(false);
	
	SetupSharedRandom::install(time(NULL));

	//-- setup game server
	ConfigLoginServer::install ();

	//-- run game
	SetupSharedFoundation::callbackWithExceptionHandling(LoginServer::run);

	SetupSharedFoundation::remove();

	return 0;
}
