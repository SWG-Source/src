#include "sharedFoundation/FirstSharedFoundation.h"

#include "ConfigCustomerServiceServer.h"
#include "CustomerServiceServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"
#include <unordered_map>

// ======================================================================

int main(int argc, char *argv[])
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);
	setupFoundationData.lpCmdLine = ConvertCommandLine(argc, argv);
	SetupSharedFoundation::install(setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();

	SetupSharedRandom::install(static_cast<uint32>(time(nullptr)));
	Os::setProgramName("CustomerServiceServer");
	ConfigCustomerServiceServer::install();
	NetworkHandler::install();
	CustomerServiceServer::getInstance().run();
	NetworkHandler::remove();
	SetupSharedFoundation::remove();

	return 0;
}
