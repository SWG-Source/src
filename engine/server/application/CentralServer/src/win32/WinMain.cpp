#include "FirstCentralServer.h"
#include "ConfigCentralServer.h"
#include "CentralServer.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"

#include <string>
#include <time.h>

//_____________________________________________________________________
/*
int WINAPI WinMain(
	HINSTANCE hInstance,      // handle to current instance 
	HINSTANCE hPrevInstance,  // handle to previous instance 
	LPSTR     lpCmdLine,      // pointer to command line 
	int       nCmdShow        // show state of window 
	)
	*/
int main(int argc, char ** argv)
{							//lint !e1065 //WinMain conflicts with clib		
	int i = 0;

//	UNREF(hPrevInstance);
//	UNREF(nCmdShow);

	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	// command line hack
	std::string cmdLine;
	for(i = 1; i < argc; ++i)
	{
		cmdLine += argv[i];
		if(i + 1 < argc)
		{
			cmdLine += " ";
		}
	}
//	setupFoundationData.hInstance                 = hInstance;
	setupFoundationData.commandLine               = cmdLine.c_str();
	setupFoundationData.createWindow              = false;
	setupFoundationData.clockUsesSleep            = true;
	
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();

	SetupSharedFile::install(false);

	SetupSharedNetworkMessages::install();

	ConfigCentralServer::install();

	cmdLine = "";
	// now, the real command line
	for(i = 0; i < argc; ++i)
	{
		cmdLine += argv[i];
		if(i + 1 < argc)
		{
			cmdLine += " ";
		}
	}

	CentralServer::getInstance().setCommandLine(cmdLine);

	//-- run game
	SetupSharedFoundation::callbackWithExceptionHandling(CentralServer::run);

	SetupSharedFoundation::remove();

	return 0;
}

//_____________________________________________________________________
