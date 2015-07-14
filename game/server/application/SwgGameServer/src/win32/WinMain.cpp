#include "FirstSwgGameServer.h"
#include "serverGame/GameServer.h"

#include "LocalizationManager.h"
#include "serverGame/SetupServerGame.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ServerObjectTemplate.h"
#include "ServerObjectLint.h"
#include "serverPathfinding/SetupServerPathfinding.h"
#include "serverScript/SetupScript.h"
#include "serverUtility/SetupServerUtility.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedGame/SetupSharedGame.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedImage/SetupSharedImage.h"
#include "sharedLog/LogManager.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedRegex/SetupSharedRegex.h"
#include "sharedRemoteDebugServer/SharedRemoteDebugServer.h"
#include "sharedTerrain/SetupSharedTerrain.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "SwgGameServer/SwgGameServer.h"
#include "SwgGameServer/WorldSnapshotParser.h"
#include "swgSharedNetworkMessages/SetupSwgSharedNetworkMessages.h"
#include "swgServerNetworkMessages/SetupSwgServerNetworkMessages.h"

#include <cstdio>
#include <ctime>
#include <direct.h>

#include <vector>

#ifdef _DEBUG
#include "sharedDebug/DataLint.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/Os.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"

void runDataLint(char const *responsePath);
void verifyUpdateRanges (const char* filename);
#endif // _DEBUG

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
{
	int i = 0;

//	UNREF(hPrevInstance);
//	UNREF(nCmdShow);
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
	
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	SetupSharedFoundation::install (setupFoundationData);

	SetupServerUtility::install();

	SetupSharedRegex::install();

	SetupSharedFile::install(false);

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	SetupSharedMath::install();

	SetupSharedUtility::Data setupUtilityData;
	SetupSharedUtility::setupGameData (setupUtilityData);
	SetupSharedUtility::install (setupUtilityData);

	SetupSharedRandom::install(int(time(NULL)));
	{
		SetupSharedObject::Data data;
		SetupSharedObject::setupDefaultGameData(data);
		// we want the SlotIdManager initialized, but we don't need the associated hardpoint names on the server.
		SetupSharedObject::addSlotIdManagerData(data, false);
		// we want movement table information on this server
		SetupSharedObject::addMovementTableData(data);
		// we want CustomizationData support on this server.
		SetupSharedObject::addCustomizationSupportData(data);
		// we want POB ejection point override support.
		SetupSharedObject::addPobEjectionTransformData(data);
		// objects should not automatically alter their children and contents
		data.objectsAlterChildrenAndContents = false;
		SetupSharedObject::install(data);
	}
	
	SetupSharedLog::install(FormattedString<92>().sprintf("SwgGameServer:%d", Os::getProcessId()));

	TreeFile::addSearchAbsolute(0);
	TreeFile::addSearchPath(".",0);
	
	SetupSharedNetworkMessages::install();
	SetupSwgServerNetworkMessages::install();
	SetupSwgSharedNetworkMessages::install();

	SetupSharedImage::Data setupImageData;
	SetupSharedImage::setupDefaultData (setupImageData);
	SetupSharedImage::install (setupImageData);
	
	SetupSharedGame::Data setupSharedGameData;
	setupSharedGameData.setUseMountValidScaleRangeTable(true);
	setupSharedGameData.setUseClientCombatManagerSupport(true);
	SetupSharedGame::install (setupSharedGameData);
	
	SetupSharedTerrain::Data setupSharedTerrainData;
	SetupSharedTerrain::setupGameData (setupSharedTerrainData);
	SetupSharedTerrain::install (setupSharedTerrainData);
	
	SetupScript::Data setupScriptData;
	SetupScript::setupDefaultGameData(setupScriptData);
	SetupScript::install();
	
	SetupServerPathfinding::install();
	
	//-- setup game server
	SetupServerGame::install();

	SharedRemoteDebugServer::install();

	//-- setup game server
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


	Archive::ByteStream bs;
	UNREF(bs);

	NetworkHandler::install();
	SwgGameServer::install();
	GameServer::getInstance().setCommandLine(cmdLine);

#ifdef _DEBUG
	//-- see if the game server is being run in a mode to parse the database dump to create planetary snapshot files
	const char* const createWorldSnapshots = ConfigFile::getKeyString("WorldSnapshot", "createWorldSnapshots", 0);
	if (createWorldSnapshots)
	{
		WorldSnapshotParser::createWorldSnapshots (createWorldSnapshots);
	}
	else
	//-- see if the gameserver is to be run in a mode to scan update ranges
	if (ConfigFile::getKeyBool ("SwgGameServer", "verifyUpdateRanges", false))
	{
		ServerWorld::install ();
		verifyUpdateRanges ("../../exe/win32/serverobjecttemplates.txt");
		ServerWorld::remove ();
	}
	else
	if (!ConfigFile::getKeyBool("SwgGameServer/DataLint", "disable", true))
	{
		runDataLint("../../exe/win32/DataLintServer.rsp");
	}
	else
#endif // _DEBUG
	{
		//-- run game
		SetupSharedFoundation::callbackWithExceptionHandling(GameServer::run);
	}

	NetworkHandler::remove();
	SetupServerGame::remove();

	SharedRemoteDebugServer::remove();
	SetupSharedLog::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove ();

	return 0;
}

#ifdef _DEBUG

void verifyUpdateRanges (const char* const filename)
{
	FILE* const infile = fopen (filename, "rt");
	if (!infile)
	{
		DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: response file %s could not be opened\n", filename));
		return;
	}

	char serverObjectTemplateName [1024];
	while (fscanf (infile, "%s", serverObjectTemplateName) != EOF)
	{
		//-- does the name contain test?
		if (strstr (serverObjectTemplateName, "test") != 0)
		{
			DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: skipping test object template %s\n", serverObjectTemplateName));
			continue;
		}

		if (strstr (serverObjectTemplateName, "e3_") != 0)
		{
			DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: skipping test object template %s\n", serverObjectTemplateName));
			continue;
		}

		//-- get the object template
		const ObjectTemplate* const objectTemplate = ObjectTemplateList::fetch (serverObjectTemplateName);
		if (!objectTemplate)
		{
			DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: %s is not a valid object template\n", serverObjectTemplateName));
			continue;
		}

		//-- make sure its a server object template
		const ServerObjectTemplate* const serverObjectTemplate = dynamic_cast<const ServerObjectTemplate*> (objectTemplate);
		if (!serverObjectTemplate)
		{
			objectTemplate->releaseReference ();
			DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: %s is not a valid server object template\n", serverObjectTemplateName));
			continue;
		}

		//-- make sure it has a shared object template
		const std::string& sharedObjectTemplateName = serverObjectTemplate->getSharedTemplate ();
		if (sharedObjectTemplateName.empty ())
		{
			objectTemplate->releaseReference ();
			continue;
		}

		//-- make sure the shared template exists
		const SharedObjectTemplate* const sharedObjectTemplate = safe_cast<const SharedObjectTemplate*> (ObjectTemplateList::fetch (sharedObjectTemplateName.c_str ()));
		if (!sharedObjectTemplate)
		{
			DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: %s is not a valid shared object template for server object template %s\n", sharedObjectTemplateName.c_str (), serverObjectTemplateName));
			objectTemplate->releaseReference ();
			continue;
		}

		const float farUpdateRange = serverObjectTemplate->getUpdateRanges (ServerObjectTemplate::UR_far);
		DEBUG_REPORT_LOG (true, ("OK:\t%s\t%s\t%1.1f\n", serverObjectTemplateName, sharedObjectTemplateName.c_str (), farUpdateRange));
		objectTemplate->releaseReference ();
	}
}

static std::string currentLintedAsset;
static int m_startIndex = 0;
static int m_numberOfFilesToLint = -1;

//-----------------------------------------------------------------------------
static int ExceptionHandler(LPEXCEPTION_POINTERS exceptionPointers)
{
	UNREF(exceptionPointers);

	static bool entered = false;

	// make the routine safe from re-entrance

	if (entered)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	entered = true;

	// tell the Os not to abort so we can rethrow the exception

	Os::returnFromAbort();

	// Let the ExitChain do its job

	DataLint::pushAsset(currentLintedAsset.c_str());
	FATAL(true, ("ExceptionHandler invoked - A crash just occured. The asset is bad or it is constructed improperly with DataLint."));
	DataLint::popAsset();

	// rethrow the exception so that the debugger can catch it

	return EXCEPTION_CONTINUE_SEARCH;  //lint !e527 // Unreachable
}

//-----------------------------------------------------------------------------
static void lintType(char const *filePath, DataLint::AssetType const assetType, bool const skip)
{
	DataLint::setCurrentAssetType(assetType);

	static int assetNumber = 0;

	if (!skip)
	{
		DEBUG_REPORT_LOG (true, ("%5i Linting file %s... ", assetNumber, filePath));

		bool failed = false;

		try
		{
			switch (assetType)
			{
				case DataLint::AT_objectTemplate:
					{
						const ObjectTemplate *objectTemplate = ObjectTemplateList::fetch(filePath);
						if (objectTemplate)
						{
							objectTemplate->testValues();

							// if file has the directory "base" in it's path, don't 
							// create an object
							if (strstr(filePath, "/base/") == NULL)
							{
								Object* const object = objectTemplate->createObject ();
								if (object != NULL)
									delete object;
							}

							objectTemplate->releaseReference();
						}
						else
						{
							failed = true;
						}
					}
					break;
				case DataLint::AT_unSupported:
					{
					}
					break;

			}

			if (failed)
			{
				DEBUG_REPORT_LOG (true, ("failed\n"));
			}
			else
			{
				DEBUG_REPORT_LOG (true, ("ok\n"));
			}
		}
		catch (FatalException const &e)
		{
			DEBUG_REPORT_LOG (true, ("failed\n"));

			DataLint::logWarning(e.getMessage());
		}
	}

	++assetNumber;
}

//-----------------------------------------------------------------------------
static void makeMicrosoftHappyAboutObjectUnWinding(char const *filePath, DataLint::AssetType const assetType, bool const skip)
{
	__try
	{
		lintType(filePath, assetType, skip);
	}
	__except (ExceptionHandler(GetExceptionInformation()))
	{
	}
}

//-----------------------------------------------------------------------------
static void lint(char const *dataTypeString, DataLint::AssetType const assetType)
{
	DataLint::StringPairList stringPairList(DataLint::getList(assetType));
	DataLint::StringPairList::const_iterator dataLintStringListIter = stringPairList.begin();

	DEBUG_REPORT_LOG_PRINT(1, ("Linting %d %s assets\n", stringPairList.size(), dataTypeString));

	int currentIndex = 0;
	for (; dataLintStringListIter != stringPairList.end(); ++dataLintStringListIter)
	{
		{
			if ((currentIndex % 10) == 0)
			{
				//-- see if a file exists to abort

				FILE *file = fopen("stoplint.dat", "rt");

				if (file)
				{
					fclose(file);
					DEBUG_REPORT_LOG_PRINT(1, ("Stopping all DataLint processing for %s.\n", dataTypeString));
					DataLint::logWarning("Forced DataLint stop. Delete \"stoplint.dat\" to DataLint all the assets.");
					break;
				}
			}
		}

		char const *filePath = dataLintStringListIter->first.c_str();
		currentLintedAsset = dataLintStringListIter->second.c_str();

		//DataLint::pushAsset(filePath);
		makeMicrosoftHappyAboutObjectUnWinding(filePath, assetType, (currentIndex < m_startIndex));
		//DataLint::popAsset();

		DataLint::clearAssetStack();
		++currentIndex;

		if ((m_numberOfFilesToLint != -1) && (currentIndex >= (m_startIndex + m_numberOfFilesToLint)))
		{
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void runDataLint(char const *responsePath)
{
	ServerObjectLint::install();
	ServerWorld::install();
	DataLint::setServerMode();

	PerformanceTimer performanceTimer;
	performanceTimer.start();

	FILE *fp = fopen(responsePath, "r");

	if (fp)
	{
		// Default to start with the first asset

		m_startIndex = ConfigFile::getKeyInt("SwgGameServer/DataLint", "startIndex", 0);
		
		// Default to lint all assets

		m_numberOfFilesToLint = ConfigFile::getKeyInt("SwgGameServer/DataLint", "numberOfFilesToLint", -1);

		// Verify this is a valid resonse file

		char text[4096];
		fgets(text, sizeof(text), fp);

		if (strstr(text, "Valid DataLint Rsp") == NULL)
		{
			DEBUG_REPORT_LOG_PRINT(1, ("DataLint: Invalid Rsp file: %s\n", responsePath));

#ifdef WIN32
			char text[4096];
			sprintf(text, "Invalid Rsp file specified: %s. Make sure to run DataLintRspBuilder before running DataLint.", responsePath);
			MessageBox(NULL, text, "DataLint Error!", MB_OK | MB_ICONERROR);
#endif // WIN32
		}
		else
		{
			DataLint::install();

			typedef std::vector<std::string> StringList;
			StringList stringList;
			stringList.reserve(32768);

			DEBUG_REPORT_LOG_PRINT(1, ("Loading assets to lint from: %s\n", responsePath));

			while (fgets(text, sizeof(text), fp))
			{
				// Remove the newline character

				char *newLineTest = strchr(text, '\n');

				if (newLineTest)
				{
					*newLineTest = '\0';
				}

				// Add the files to the master file list

				stringList.push_back(text);
			}

			fclose(fp);

			// Add files to data lint

			DEBUG_REPORT_LOG_PRINT(1, ("Adding %d assets to DataLint to be categorized\n", stringList.size()));

			StringList::iterator stringListIter = stringList.begin();

			for (; stringListIter != stringList.end(); ++stringListIter)
			{
				DataLint::addFilePath((*stringListIter).c_str());
			}

			if (ConfigFile::getKeyBool("SwgGameServer/DataLint", "objectTemplate", true))
			{
				lint("Objects Template",          DataLint::AT_objectTemplate);
			}

			DataLint::report();
		}
	}
	else
	{
		DEBUG_REPORT_LOG_PRINT(1, ("Bad response path specified: %s", responsePath));

#ifdef WIN32
		char text[4096];
		sprintf(text, "Bad response file specified: %s. Make sure to run DataLintRspBuilder before running DataLint.", responsePath);
		MessageBox(NULL, text, "DataLint Error!", MB_OK | MB_ICONERROR);
#endif // WIN32
	}

	// Dump the time required to data lint

	performanceTimer.stop();
	float const dataLintTime = performanceTimer.getElapsedTime();
	int const seconds = static_cast<int>(dataLintTime) % 60;
	int const minutes = (static_cast<int>(dataLintTime) - seconds) / 60;
	DEBUG_REPORT_LOG_PRINT(1, ("DateLint took: %dm %ds\n", minutes, seconds));
}
#endif // _DEBUG

//_____________________________________________________________________
