#include "FirstSwgGameServer.h"
#include "serverGame/GameServer.h"

#include "LocalizationManager.h"
#include "serverGame/SetupServerGame.h"
#include "serverPathfinding/SetupServerPathfinding.h"
#include "serverScript/SetupScript.h"
#include "serverUtility/SetupServerUtility.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedGame/SetupSharedGame.h"
#include "sharedImage/SetupSharedImage.h"
#include "sharedLog/LogManager.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedRegex/SetupSharedRegex.h"
#include "sharedTerrain/SetupSharedTerrain.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "SwgGameServer/SwgGameServer.h"
#include "SwgGameServer/WorldSnapshotParser.h"
#include "swgSharedNetworkMessages/SetupSwgSharedNetworkMessages.h"
#include "swgServerNetworkMessages/SetupSwgServerNetworkMessages.h"


// ======================================================================

int main(int argc, char ** argv)
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);
	setupFoundationData.lpCmdLine                 = ConvertCommandLine(argc,argv);
	setupFoundationData.runInBackground           = true;
	SetupSharedFoundation::install (setupFoundationData);

	SetupServerUtility::install();

	SetupSharedRegex::install();
	
	SetupSharedFile::install(false);
	SetupSharedMath::install();

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	SetupSharedUtility::Data setupUtilityData;
	SetupSharedUtility::setupGameData (setupUtilityData);
	SetupSharedUtility::install (setupUtilityData);

	SetupSharedNetworkMessages::install();
	SetupSwgSharedNetworkMessages::install();
	SetupSwgServerNetworkMessages::install();

	SetupSharedRandom::install(time(0));//lint !e732
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

	char tmp[92];
	sprintf(tmp, "SwgGameServer:%d", Os::getProcessId());
	SetupSharedLog::install(tmp);

	TreeFile::addSearchAbsolute(0);
	TreeFile::addSearchPath(".",0);

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

	NetworkHandler::install();
	Os::setProgramName("SwgGameServer");
	SwgGameServer::install();

#ifdef _DEBUG
	//-- see if the game server is being run in a mode to parse the database dump to create planetary snapshot files
	const char* const createWorldSnapshots = ConfigFile::getKeyString("WorldSnapshot", "createWorldSnapshots", 0);
	if (createWorldSnapshots)
	{
		WorldSnapshotParser::createWorldSnapshots (createWorldSnapshots);
	}
	else
#endif
	{
		//-- run game
		SetupSharedFoundation::callbackWithExceptionHandling(GameServer::run);
	}

	NetworkHandler::remove();
	SetupServerGame::remove();

	SetupSharedLog::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove ();

	return 0;
}

