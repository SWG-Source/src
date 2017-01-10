#include "FirstChatServer.h"

#include "ConfigChatServer.h"
#include "ChatServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
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
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();

	SetupSharedRandom::install(static_cast<uint32>(time(nullptr))); //lint !e1924 !e64 // nullptr is a C-Style cast?

	Os::setProgramName("ChatServer");
	//setup the server
	ConfigChatServer::install();

	//-- run game
	ChatServer::run();
	SetupSharedFoundation::remove();

#ifdef ENABLE_PROFILING
	exit(0);
#endif

	return 0;
}
