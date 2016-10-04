#include "FirstTaskManager.h"

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"
#include "TaskManager.h"
#include "ConfigTaskManager.h"

//=====================================================================

int main(int argc, char **argv)
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	setupFoundationData.argc                      = argc;
	setupFoundationData.argv                      = argv;
	setupFoundationData.createWindow              = false;
	setupFoundationData.clockUsesSleep            = true;
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();
	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();
	SetupSharedRandom::install(static_cast<uint32>(time(NULL))); //lint !e1924 !e64 // NULL is a C-Style cast?

	ConfigTaskManager::install();

	TaskManager::install();

	//-- run server
	SetupSharedFoundation::callbackWithExceptionHandling(TaskManager::run);

	TaskManager::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

//=====================================================================
