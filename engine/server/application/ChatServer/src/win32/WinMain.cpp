#include "FirstChatServer.h"
#include "ConfigChatServer.h"
#include "ChatServer.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedThread/SetupSharedThread.h"

#include <string>
#include <time.h>

int main( int argc, char ** argv )
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	//setupFoundationData.hInstance               = hInstance;
	setupFoundationData.argc                      = argc;
	setupFoundationData.argv                      = argv;
	setupFoundationData.createWindow              = false;
	setupFoundationData.clockUsesSleep            = true;
	
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedNetworkMessages::install();

	//-- setup game server
	ConfigChatServer::install ();

	//-- run game
	SetupSharedFoundation::callbackWithExceptionHandling(ChatServer::run);

	SetupSharedFoundation::remove();

	return 0;
}
