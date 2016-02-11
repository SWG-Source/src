#include "FirstSwgDatabaseServer.h"

#include "SwgDatabaseServer/SwgDatabaseServer.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedLog/LogManager.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "swgSharedNetworkMessages/SetupSwgSharedNetworkMessages.h"
#include "swgServerNetworkMessages/SetupSwgServerNetworkMessages.h"

// ======================================================================

void dumpPid(const char * argv)
{
	pid_t p = getpid();
	char fileName[1024];
	sprintf(fileName, "%s.%d", argv, p);
	FILE * f = fopen(fileName, "w+");
	fclose(f);
}

int main(int argc, char ** argv)
{
//	dumpPid(argv[0]);

	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);
	setupFoundationData.lpCmdLine                 = ConvertCommandLine(argc,argv);
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedFile::install(false);
	SetupSharedRandom::install(time(nullptr));
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	SetupSharedNetworkMessages::install();
	SetupSwgSharedNetworkMessages::install();
	SetupSwgServerNetworkMessages::install();

	SetupSharedLog::install("SwgDatabaseServer");

	TreeFile::addSearchAbsolute(0);
	TreeFile::addSearchPath(".",0);

	//-- setup server
	ConfigServerDatabase::install ();

	NetworkHandler::install();
	Os::setProgramName("SwgDatabaseServer");
	SwgDatabaseServer::install();

	//-- run server
	SwgDatabaseServer::getInstance().run();
	NetworkHandler::remove();
	SetupSharedLog::remove();

	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}
