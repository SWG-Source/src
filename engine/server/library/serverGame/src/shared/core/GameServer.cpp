// GameServer.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/GameServer.h"

#include "serverGame/AiMovementBase.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/AiShipController.h"
#include "serverGame/AttribModNameManager.h"
#include "serverGame/AuthTransferTracker.h"
#include "serverGame/BiographyManager.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/CentralConnection.h"
#include "serverGame/CharacterMatchManager.h"
#include "serverGame/Chat.h"
#include "serverGame/ChatServerConnection.h"
#include "serverGame/CitizenInfo.h"
#include "serverGame/CityInfo.h"
#include "serverGame/CityInterface.h"
#include "serverGame/CityObject.h"
#include "serverGame/Client.h"
#include "serverGame/ClusterWideDataClient.h"
#include "serverGame/CombatTracker.h"
#include "serverGame/CommandCppFuncs.h"
#include "serverGame/CommoditiesMarket.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConnectionServerConnection.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/CustomerServiceServerConnection.h"
#include "serverGame/DatabaseProcessConnection.h"
#include "serverGame/GameServerMetricsData.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/GuildObject.h"
#include "serverGame/HarvesterInstallationObject.h"
#include "serverGame/LogoutTracker.h"
#include "serverGame/ManufactureInstallationObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/MissileManager.h"
#include "serverGame/NameManager.h"
#include "serverGame/NewbieTutorial.h"
#include "serverGame/NonCriticalTaskQueue.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlanetServerConnection.h"
#include "serverGame/PlayerCreationManagerServer.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PlayerSanityChecker.h"
#include "serverGame/PlayerShipController.h"
#include "serverGame/PositionUpdateTracker.h"
#include "serverGame/PurgeManager.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerAsteroidManager.h"
#include "serverGame/SceneGlobalData.h"
#include "serverGame/ServerBuildoutManager.h"
#include "serverGame/ServerCommandPermissionManager.h"
#include "serverGame/ServerCommandTable.h"
#include "serverGame/ServerController.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerImageDesignerManager.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObjectTerrainModificationNotification.h"
#include "serverGame/ServerUIManager.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipAiReactionManager.h"
#include "serverGame/ShipClientUpdateTracker.h"
#include "serverGame/ShipObject.h"
#include "serverGame/StationPlayersCollectorAPI.h"
#include "serverGame/SurveySystem.h"
#include "serverGame/TangibleObject.h"
#include "serverGame/TaskManagerConnection.h"
#include "serverGame/VeteranRewardManager.h"
#include "serverMetrics/MetricsManager.h"
#include "serverNetworkMessages/AccountFeatureIdRequest.h"
#include "serverNetworkMessages/AccountFeatureIdResponse.h"
#include "serverNetworkMessages/AddCharacterMessage.h"
#include "serverNetworkMessages/AddResourceTypeMessage.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdResponse.h"
#include "serverNetworkMessages/AiMovementMessage.h"
#include "serverNetworkMessages/AiCreatureStateMessage.h"
#include "serverNetworkMessages/AuthTransferClientMessage.h"
#include "serverNetworkMessages/BiographyMessage.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/CentralPingMessage.h"
#include "serverNetworkMessages/CharacterNamesMessage.h"
#include "serverNetworkMessages/ChatServerOnline.h"
#include "serverNetworkMessages/ChunkCompleteMessage.h"
#include "serverNetworkMessages/ClaimRewardsReplyMessage.h"
#include "serverNetworkMessages/CreateDynamicRegionCircleMessage.h"
#include "serverNetworkMessages/CreateDynamicRegionRectangleMessage.h"
#include "serverNetworkMessages/CreateDynamicSpawnRegionCircleMessage.h"
#include "serverNetworkMessages/CreateGroupMessage.h"
#include "serverNetworkMessages/CreateObjectMessage.h"
#include "serverNetworkMessages/CreateSyncUiMessage.h"
#include "serverNetworkMessages/EnableNewJediTrackingMessage.h"
#include "serverNetworkMessages/EndBaselinesMessage.h"
#include "serverNetworkMessages/ExcommunicateGameServerMessage.h"
#include "serverNetworkMessages/FactionalSystemMessage.h"
#include "serverNetworkMessages/FailedToLoadObjectMessage.h"
#include "serverNetworkMessages/FeatureIdTransactionResponse.h"
#include "serverNetworkMessages/FirstPlanetGameServerIdMessage.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverNetworkMessages/GameGameServerMessages.h"
#include "serverNetworkMessages/GameServerConnectAck.h"
#include "serverNetworkMessages/GameServerReadyMessage.h"
#include "serverNetworkMessages/GameServerUniverseLoadedMessage.h"
#include "serverNetworkMessages/GameTaskManagerMessages.h"
#include "serverNetworkMessages/GetMoneyFromOfflineObjectMessage.h"
#include "serverNetworkMessages/LoadContainedObjectMessage.h"
#include "serverNetworkMessages/LoadContentsMessage.h"
#include "serverNetworkMessages/LocateObjectResponseMessage.h"
#include "serverNetworkMessages/LocatePlayerResponseMessage.h"
#include "serverNetworkMessages/LoadObjectMessage.h"
#include "serverNetworkMessages/LoadStructureMessage.h"
#include "serverNetworkMessages/LoadUniverseMessage.h"
#include "serverNetworkMessages/LocateStructureMessage.h"
#include "serverNetworkMessages/LocationResponse.h"
#include "serverNetworkMessages/MessageToAckMessage.h"
#include "serverNetworkMessages/NewClient.h"
#include "serverNetworkMessages/PersistedPlayerMessage.h"
#include "serverNetworkMessages/PlayedTimeAccumMessage.h"
#include "serverNetworkMessages/PopulationListMessage.h"
#include "serverNetworkMessages/PreloadRequestCompleteMessage.h"
#include "serverNetworkMessages/ProfilerOperationMessage.h"
#include "serverNetworkMessages/RandomName.h"
#include "serverNetworkMessages/ReloadAdminTableMessage.h"
#include "serverNetworkMessages/ReloadCommandTableMessage.h"
#include "serverNetworkMessages/ReloadDatatableMessage.h"
#include "serverNetworkMessages/ReloadScriptMessage.h"
#include "serverNetworkMessages/ReloadTemplateMessage.h"
#include "serverNetworkMessages/RenameCharacterMessage.h"
#include "serverNetworkMessages/RetrievedItemLoadMessage.h"
#include "serverNetworkMessages/SceneTransferMessages.h"
#include "serverNetworkMessages/SetAuthoritativeMessage.h"
#include "serverNetworkMessages/SetPlanetServerMessage.h"
#include "serverNetworkMessages/SetUniverseAuthoritativeMessage.h"
#include "serverNetworkMessages/StructuresForPurgeMessage.h"
#include "serverNetworkMessages/SynchronizeScriptVarDeltasMessage.h"
#include "serverNetworkMessages/SynchronizeScriptVarsMessage.h"
#include "serverNetworkMessages/TeleportMessage.h"
#include "serverNetworkMessages/TeleportToMessage.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/TransferReplyMoveValidation.h"
#include "serverNetworkMessages/UnloadObjectMessage.h"
#include "serverNetworkMessages/UnloadProxyMessage.h"
#include "serverNetworkMessages/UpdateObjectPositionMessage.h"
#include "serverNetworkMessages/UploadCharacterMessage.h"
#include "serverNetworkMessages/VerifyAndLockName.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/AdminAccountManager.h"
#include "serverUtility/ServerClock.h"
#include "Session/CommonAPI/CommonAPI.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/DebugMonitor.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/AppearanceManager.h"
#include "sharedGame/CombatDataTable.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/QuestManager.h"
#include "sharedLog/FileLogObserver.h"
#include "sharedLog/Log.h"
#include "sharedMessageDispatch/MessageManager.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetworkMessages/BatchBaselinesMessage.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/DeltasMessage.h"
#include "sharedNetworkMessages/FrameEndMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/HyperspaceMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/NameErrors.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedNetworkMessages/SlowDownEffectMessage.h"
#include "sharedNetworkMessages/UpdateContainmentMessage.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/NetworkController.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectList.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedPathfinding/Pathfinding.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgSharedUtility/SpeciesRestrictions.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "sharedFoundation/CrcConstexpr.hpp"

#include <limits>

//-----------------------------------------------------------------------

//#undef PROFILE_INDIVIDUAL_MESSAGES
#define PROFILE_INDIVIDUAL_MESSAGES 1

#ifdef PROFILE_INDIVIDUAL_MESSAGES
	#define MESSAGE_PROFILER_BLOCK(a) PROFILER_AUTO_BLOCK_DEFINE(a)
#else
	#define MESSAGE_PROFILER_BLOCK(a) NOP
#endif

//-----------------------------------------------------------------------

namespace GameServerNamespace
{
	bool gs_gameServerReady = false;
	const float IDLE_CLIENTS_CHECK_TIME_SEC = 30.0f;
	ServerCommandPermissionManager * s_permissionManager = 0;

	unsigned long  s_frameTime;
	uint64  s_totalObjectCreateMessagesReceived;
	uint64  s_totalObjectCreateMessagesSent;
	bool    s_metricsManagerInstalled = false;
	uint32  s_lastTaskKeepaliveTime = 0;

#ifdef _DEBUG
	int s_extraDelayPerFrameMs = 0; // to emulate long loop time
#endif

	std::set<uint32> s_clusterStartupResidenceStructureListResponse;
	std::map<NetworkId, std::pair<int, NetworkId> > s_clusterStartupResidenceStructureListByStructure;

	unsigned long getFrameRateLimit();
	void broadCastHyperspaceOnWarp(ServerObject const * owner);
	ShipObject *getAttachedShip(CreatureObject *creature);

	std::map<NetworkId, unsigned long> s_pendingLoadRequests;

	struct CtsSourceCharacterInfo
	{
		time_t transferTime;
		std::string sourceCluster;
		NetworkId sourceCharacterId;
		std::string sourceCharacterName;
		int sourceCharacterBornDate;
		std::vector<std::pair<std::string, DynamicVariable> > sourceCharacterRetroactiveObjvars;
	};

	// any particular target character may have gone through multiple transfers
	// so we want to keep a list (ordered by transfer time) of all the transfers
	// the character has gone through
	//
	// for release build, we only care about transfers into our cluster;
	// for debug build, read them all so we can test out the logic to
	// make sure it works for all clusters
	//
	//                                            transfer time, source character info
	//                       target character id
	//     target cluster
	//           \/                   \/                       \/
#ifdef _DEBUG
	std::map<std::string, std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > > s_retroactiveCtsHistoryList;
#else
	                      std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > s_retroactiveCtsHistoryListThisCluster;
#endif

	void loadRetroactiveCtsHistory();

	// retroactive player city creation time
	//
	//                              city creation time
	//                           city id
	//        cluster
	//           \/                 \/    \/
#ifdef _DEBUG
	std::map<std::string, std::map<int, time_t> > s_retroactivePlayerCityCreationTime;
#else
	std::map<int, time_t> s_retroactivePlayerCityCreationTimeThisCluster;
#endif

	void loadRetroactivePlayerCityCreationTime();

	bool getConfigSetting(const char *section, const char *key, int & value)
	{
		const ConfigFile::Section * sec = ConfigFile::getSection(section);
		if (sec == nullptr)
			return false;

		const ConfigFile::Key * ky = sec->findKey(key);
		if (ky == nullptr)
			return false;

		value = ky->getAsInt(ky->getCount()-1, value);
		return true;
	}

	const std::string OBJVAR_HAS_OUTSTANDING_REQUEST_SCENE_TRANSFER("hasOutstandingRequestSceneTransfer");
	bool checkAndSetOutstandingRequestSceneTransfer(ServerObject & object);

	void handleFactionalSystemMessage(FactionalSystemMessage const & msg);
	void handleMessageToPlayersOnPlanet(std::string const & methodName, std::vector<int8> const & params, float delay, Vector const & loc, float radius, bool includeDisconnectedPlayers);
	void handleMessageLocateStructureByOwnerIdReq(GameServer & gsInstance, uint32 responsePid, bool isGod, NetworkId const & ownerId, NetworkId const & ownerResidenceId, NetworkId const & responseId);
	void handleMessageLocateStructureByOwnerIdRsp(NetworkId const & responseId, std::vector<std::string> const & response);
	void handleAdjustAccountFeatureIdResponse(AdjustAccountFeatureIdResponse const & msg);
	void handleAccountFeatureIdResponse(AccountFeatureIdResponse const & msg);

	namespace SlotNames
	{
		static const ConstCharCrcLowerString appearance ("appearance_inventory");
	}

	const char* s_appearanceTemplate = "object/tangible/inventory/appearance_inventory.iff";

}

using namespace GameServerNamespace;

//----------------------------------------------------------------------

GameServer * GameServer::ms_instance = 0;

//-----------------------------------------------------------------------

GameServer::GameServer() :
MessageDispatch::Receiver(),
m_done                                (false),
m_processId                           (0),
m_preloadAreaId                       (0),
m_databaseProcessId                   (0),
m_gameServerPids                      (),
m_centralService                      (),
m_centralServerConnection             (0),
m_customerServiceServerConnection     (0),
m_connectionServerVector              (new ConnectionServerVector),
m_planetServerConnection              (0),
m_databaseProcessConnection           (0),
m_pendingDatabaseProcessConnection    (0),
m_taskManagerConnection               (),
m_clients                             (new ClientMap),
m_clientService                       (),
m_clusterName                         (),
m_clusterId                           (0),
m_firstGameServerForPlanet            (0),
m_pendingContainedObjects             (new PendingContainedObjectsMap),
m_charactersPendingCreation           (new CharactersPendingCreationMap),
m_metricsData                         (0),
m_gameServerReadyObjectIds            (false),
m_gameServerReadyDatabaseConnected    (false),
m_gameServerReadyPlanetConnected      (false),
m_connectionTimeout                   (false),
m_chatServerConnection                (0)
{
	IGNORE_RETURN(SkillManager::getInstance().getRoot ());

	ObjectIdManager::getInstance();
	// set up central server connection
	m_centralService = new CentralServerConnection(ConfigServerGame::getCentralServerAddress(), ConfigServerGame::getCentralServerPort());
	connectToMessage("CentralConnectionOpened");
	connectToMessage("CentralConnectionClosed");

	//Add connection server messages
	connectToMessage("ConnectionServerAddress");
	connectToMessage("ConnectionServerClosed");
	connectToMessage("NewClient");

	connectToMessage("TaskConnectionClosed");
	
	// set up Commodity server connection
	connectToMessage("CommoditiesServerConnectionClosed");
	
	// application messages
	connectToMessage("BaselinesMessage");
	connectToMessage("BatchBaselinesMessage");
	connectToMessage("DeltasMessage");
	connectToMessage("ChunkCompleteMessage");
	connectToMessage("CentralGameServerSetProcessId");
	connectToMessage("GameServerSetupMessage");
	connectToMessage("LoadUniverseMessage");
	connectToMessage("PlanetConnectionClosed");
	connectToMessage("PlanetConnectionOpened");
	connectToMessage("SetPlanetServerMessage");
	connectToMessage("SetUniverseAuthoritativeMessage");
	connectToMessage("CentralCreateCharacter");
	connectToMessage("CreateDynamicSpawnRegionCircle");
	connectToMessage("CreateDynamicRegionCircleMessage");
	connectToMessage("CreateDynamicRegionRectangleMessage");
	connectToMessage("CreateGroupMessage");
	connectToMessage("CreateObjectByCrcMessage");
	connectToMessage("CreateSyncUiMessage");
	connectToMessage("DropClient");
	connectToMessage("EnableNewJediTrackingMessage");
	connectToMessage("EndBaselinesMessage");
	connectToMessage("GameSetProcessId");
	connectToMessage("GameServerConnectAck");
	connectToMessage("LoadObjectMessage");
	connectToMessage("ObjControllerMessage");
	connectToMessage("ProfilerOperationMessage");
	connectToMessage("SetAuthoritativeMessage");
	connectToMessage("SynchronizationMessage");
	connectToMessage("TaskShutdownGameServer");
	connectToMessage("UniverseCompleteMessage");
	connectToMessage("GameServerUniverseLoadedMessage");
	connectToMessage("AddResourceTypeMessage");
	connectToMessage("AddImportedResourceType");
	connectToMessage("PreloadRequestCompleteMessage");
	connectToMessage("UnloadObjectMessage");
	connectToMessage("VerifyNameResponse");
	connectToMessage("SceneTransferMessage");
	connectToMessage("RandomNameRequest"); // from connection server
	connectToMessage("VerifyAndLockNameRequest"); // from connection server

	connectToMessage("ChatServerOnline");
	connectToMessage("ChatServerConnectionClosed");
	connectToMessage("TeleportMessage");
	connectToMessage("TeleportToMessage");
	connectToMessage("RetrievedItemLoadMessage");
	connectToMessage("UnloadProxyMessage");
	connectToMessage("BiographyMessage");
	connectToMessage("LoadContainedObjectMessage");
	connectToMessage("LoadContentsMessage");
	connectToMessage("FirstPlanetGameServerIdMessage");
	connectToMessage("ReloadAdminTableMessage");
	connectToMessage("ReloadCommandTableMessage");
	connectToMessage(ReloadDatatableMessage::ms_messageName);
	connectToMessage("ReloadScriptMessage");
	connectToMessage("ReloadTemplateMessage");
	connectToMessage("AuthTransferConfirmMessage");
	connectToMessage("ConnectionServerConnectionDestroyed");
	connectToMessage("PersistedPlayerMessage");
	connectToMessage("RenameCharacterMessageEx");
	connectToMessage("PopulationListMessage");
	connectToMessage("UpdateObjectPositionMessage");
	connectToMessage("UpdateContainmentMessage");
	connectToMessage("SynchronizeScriptVarsMessage");
	connectToMessage("SynchronizeScriptVarDeltasMessage");
	connectToMessage("CentralPingMessage");
	connectToMessage("AddGameServer");
	connectToMessage("ExcommunicateGameServerMessage");
	connectToMessage("LocateObject");
	connectToMessage("LocateObjectByTemplateName");
	connectToMessage("LocatePlayerByPartialName");
	connectToMessage("LocateWarden");
	connectToMessage("LocateCreatureByCreatureName");
	connectToMessage("LSBOIReq");
	connectToMessage("LSBOIRsp");
	connectToMessage("LocateStructureMessage");
	connectToMessage("LoadStructureMessage");
	connectToMessage("LocateObjectResponseMessage");
	connectToMessage("LocatePlayerResponseMessage");
	connectToMessage("PlayerSanityCheck");
	connectToMessage("PlayerSanityCheckSuccess");
	connectToMessage("PlayerSanityCheckProxy");
	connectToMessage("PlayerSanityCheckProxyFail");
	connectToMessage("EnablePlayerSanityCheckerMessage");
	connectToMessage("StartSaveReplyMessage");
	connectToMessage("DatabaseConsoleReplyMessage");
	connectToMessage("FindAuthObject");
	connectToMessage("CharacterNamesMessage");
	connectToMessage("DownloadCharacterMessage");
	connectToMessage("UploadCharacterMessage");
	connectToMessage("LocationResponse");
	connectToMessage("ReleaseCharacterNameByIdMessage");
	connectToMessage("AddAttribModName");
	connectToMessage("AddAttribModNamesList");
	connectToMessage("CustomerServiceServerGameServerServiceAddress");
	connectToMessage("AiCreatureStateMessage");
	connectToMessage("UnloadPersistedCharacter");
	connectToMessage("AboutToLoadCharacterFromDB");
	connectToMessage("AuthTransferClientMessage");
	connectToMessage("ClearTheaterMessage");
	connectToMessage("SetTheaterMessage");
	connectToMessage("PageChangeAuthority");
	connectToMessage("ManualDepleteResourceMessage");
	connectToMessage("GetMoneyFromOfflineObjectMessage");
	connectToMessage("ClaimRewardsReplyMessage");
	connectToMessage("SetOverrideAccountAgeMessage");
	connectToMessage("StructuresForPurgeMessage");
	connectToMessage(SlowDownEffectMessage::MessageType);
	connectToMessage("PlayedTimeAccumMessage");
	connectToMessage("RequestLoadAckMessage");
	connectToMessage("FactionalSystemMessage");
	connectToMessage("MessageToPlayersOnPlanet");
	connectToMessage("ReportSystemClockTime");
	connectToMessage("ReportSystemClockTimeResponse");
	connectToMessage("ReportPlanetaryTime");
	connectToMessage("ReportPlanetaryTimeResponse");
	connectToMessage("ClusterStartComplete");
	connectToMessage("CSRSLReq");
	connectToMessage("CSRSLRsp");
	connectToMessage("PopStatReq");
	connectToMessage("GcwScoreStatReq");
	connectToMessage("GcwScoreStatRaw");
	connectToMessage("GcwScoreStatPct");
	connectToMessage("LLTStatReq");
	connectToMessage("LfgStatReq");
	connectToMessage("ClusterId");
	connectToMessage("OccupyUnlockedSlotRsp");
	connectToMessage("VacateUnlockedSlotRsp");
	connectToMessage("SwapUnlockedSlotRsp");
	connectToMessage("AdjustAccountFeatureIdResponse");
	connectToMessage("AccountFeatureIdResponse");
	connectToMessage("FeatureIdTransactionResponse");
	connectToMessage("TransferReplyMoveValidation");
	connectToMessage("TransferReplyNameValidation");
	
	// Cluster wide data client
	ClusterWideDataClient::install();
	ClusterWideDataClient::registerMessage(*this);

	m_taskManagerConnection = new TaskManagerConnection("127.0.0.1", ConfigServerGame::getTaskManagerPort());
}

//-----------------------------------------------------------------------

GameServer::~GameServer()
{
	m_chatServerConnection = 0;
	delete m_clients;
	m_clients = 0;
	delete m_connectionServerVector;
	m_connectionServerVector = 0;

	delete m_pendingContainedObjects;
	m_pendingContainedObjects = 0;
	delete m_charactersPendingCreation;
	m_charactersPendingCreation = 0;
}

//----------------------------------------------------------------------

void GameServer::install     ()
{
	DEBUG_FATAL (ms_instance, ("already installed"));
	ms_instance = new GameServer;
}

//----------------------------------------------------------------------

void GameServer::remove      ()
{
	// ok to remove an uninstalled GameServer singleton.
	delete ms_instance;
	ms_instance = 0;
}

//-----------------------------------------------------------------------

uint64 GameServer::getTotalObjectCreatesReceived() // static
{
	return s_totalObjectCreateMessagesReceived;
}

//-----------------------------------------------------------------------

uint64 GameServer::getTotalObjectCreatesSent() // static
	{
	return s_totalObjectCreateMessagesSent;
}

//-----------------------------------------------------------------------

void GameServer::incrementTotalObjectCreatesSent() // static
	{
	++s_totalObjectCreateMessagesSent;
}

//-----------------------------------------------------------------------
void GameServer::reportClusterStartupResidenceStructure(int cityId, const NetworkId & structureId, const NetworkId & ownerId) // static
{
	s_clusterStartupResidenceStructureListByStructure[structureId] = std::make_pair(cityId, ownerId);
}

//-----------------------------------------------------------------------

void GameServer::reportBuildingObjectDestroyed(const NetworkId & structureId)
{
	if (!s_clusterStartupResidenceStructureListByStructure.empty())
		IGNORE_RETURN(s_clusterStartupResidenceStructureListByStructure.erase(structureId));
}

//-----------------------------------------------------------------------

/**
 * Establish a connection to a game server (if not already connected).
 */
void GameServer::connectToDatabaseProcess(std::string const &address, uint16 port, uint32 pid)
{
	FATAL(m_pendingDatabaseProcessConnection, ("Tried to connect to database process with connection still pending?"));
	FATAL(m_databaseProcessConnection, ("Tried to connect to database process when alrady connected?"));

	m_pendingDatabaseProcessConnection = new DatabaseProcessConnection(address, port, pid);

	LOG("GameGameConnect", ("Game Server %lu is connecting to database process, pid %lu, addr %s port %d", m_processId, pid, address.c_str(), static_cast<int>(port)));
}

//-------------------------------------------------------------------

/**
 * Create a proxy for an object we own on another server.
 *
 * If address is specified, it will establish a connection and queue
 * the task to be done later, if necessary.  Otherwise, it will FATAL
 * if not connected.
 * @see GameServerConnectionHandler::addGameServer
 */

void GameServer::createRemoteProxy(uint32 remoteProcessId, ServerObject *object)
{

	if (!object) {
		WARNING(!object, ("Told to create an object on %d we know nothing about!\n", remoteProcessId));
		return;
	}
	
	NetworkId objectId = object->getNetworkId();

	DEBUG_REPORT_LOG(ConfigServerGame::getLogObjectLoading(),("createRemoteProxy(%lu,%s)\n",remoteProcessId, objectId.getValueString().c_str()));

	if (remoteProcessId == 0)
	{
		DEBUG_FATAL(true, ("Invalid ProcessId in GameServer::CreateRemoteProxy()\n"));
		return;
	}


	//If we have the object in the world and we are auth for it, create it.
	if (object)
	{
		if (object->isAuthoritative())
		{
			if(GameServer::getProcessId() != remoteProcessId)
			{
				object->addServerToProxyList(remoteProcessId);
			}
		}
		else
		{
			//Can't create an object if we're not authoritative.
			// Forward the message to the auth server.

			DEBUG_REPORT_LOG(ConfigServerGame::getLogObjectLoading(),("Forwarding proxy request for %s to authoritative server\n",objectId.getValueString().c_str()));

			ServerMessageForwarding::begin(object->getAuthServerProcessId());

			LoadObjectMessage const loadObjectMessage(objectId, remoteProcessId);
			ServerMessageForwarding::send(loadObjectMessage);

			ServerMessageForwarding::end();
		}
	}
}

// ----------------------------------------------------------------------

void GameServer::createProxyOnAllServers(ServerObject *object)
{
	for (std::vector<uint32>::const_iterator i = m_gameServerPids.begin(); i != m_gameServerPids.end(); ++i)
		createRemoteProxy(*i, object);
}

// ----------------------------------------------------------------------

void GameServer::debugIO (void)
{
#ifdef _DEBUG
//	DebugMonitor::flushOutput();
//	DebugMonitor::clearScreen();
	DebugFlags::callReportRoutines();
#endif
}

//-----------------------------------------------------------------------

void GameServer::deliverMessageToClientObject(const NetworkId &oid, const Archive::ByteStream & source)
{
	const ClientMap::iterator i = m_clients->find(oid);
	if(i != m_clients->end())
	{
		Archive::ReadIterator ri = source.begin();
		GameNetworkMessage const msg(ri);
		(*i).second->receiveClientMessage(msg);
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("Received message for unknown client %s\n", oid.getValueString().c_str()));
	}
}

//-----------------------------------------------------------------------

void GameServer::dropClient(const NetworkId& oid)
{
	dropClient(oid, false);
}
//-----------------------------------------------------------------------

void GameServer::dropClient(const NetworkId& oid, const bool immediate)
{
	DEBUG_REPORT_LOG(true,("Dropping client %s\n",oid.getValueString().c_str()));

	const ClientMap::iterator i = m_clients->find(oid);
	if(i != m_clients->end())
	{
		Client* client = const_cast<Client *>((*i).second);
		m_clients->erase(i);
		client->selfDestruct(immediate);
	}
}

//-----------------------------------------------------------------------

unsigned long GameServer::getFrameTime() const
{
	return s_frameTime;
}

//-----------------------------------------------------------------------

Client * GameServer::getClient(const NetworkId& networkId)
{
	ClientMap::iterator i = m_clients->find(networkId);
	if (i != m_clients->end())
	{
		return i->second;
	}
	return nullptr;
}

//-----------------------------------------------------------------------
const Service * GameServer::getClientService() const
{
	return m_clientService;
}

// ----------------------------------------------------------------------

bool GameServer::isGameServerConnected(uint32 processId) const
{
	return std::binary_search(m_gameServerPids.begin(), m_gameServerPids.end(), processId);
}

//-----------------------------------------------------------------------

ConnectionServerConnection * GameServer::getConnectionServerConnection(const std::string & connectionServerIp, const uint16 connectionServerPort)
{
	ConnectionServerConnection * result = 0;

	if (m_connectionServerVector)
	{
		for (ConnectionServerVector::iterator i = m_connectionServerVector->begin(); i != m_connectionServerVector->end(); ++i)
		{
			if (((*i)->getRemoteAddress() == connectionServerIp) && ((*i)->getRemotePort() == connectionServerPort))
			{
				result = *i;
				break;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------

void GameServer::initialize()
{
	//@todo, this function seems wrong to me.  This should happen in install or in the constructor.
	//Right now this gets called at the beginning of ::run

	ServerCommandTable::load();
	loadTerrain ();

	ShipObject::install();
	AiShipController::install();
	AICreatureController::install();
	ShipAiReactionManager::install();
	PlayerShipController::install();
	MissileManager::install();
	ShipClientUpdateTracker::install();
	ServerAsteroidManager::install();
	QuestManager::install();
	VeteranRewardManager::install();
	SceneGlobalData::install();
	StationPlayersCollectorAPI::install();
	ServerObject::install();
	TangibleObject::install();
	Client::install();

	m_metricsData = new GameServerMetricsData;
	s_permissionManager = new ServerCommandPermissionManager();
}

// ----------------------------------------------------------------------

void GameServer::loadTerrain ()
{
	if (TerrainObject::getInstance ())
	{
		TerrainObject::getInstance ()->removeFromWorld ();
		delete TerrainObject::getInstance ();
	}

	char const * const terrainFileName = ConfigServerGame::getGroundScene();
	FATAL(!terrainFileName || !TreeFile::exists(terrainFileName), ("%s is not a valid terrain file", terrainFileName));

	TerrainObject * const terrainObject = new TerrainObject(ServerWorld::getTerrainObjectNotification());
	terrainObject->setDebugName("terrain");

	Appearance * const appearance = AppearanceTemplateList::createAppearance(terrainFileName);
	if (appearance != nullptr) {
		terrainObject->setAppearance(appearance);
	} else {
		DEBUG_WARNING(true, ("FIX ME: Appearance template for GameServer::loadTerrain missing for %s.", terrainFileName));
	}
	  

	ProceduralTerrainAppearance * const proceduralTerrainAppearance = dynamic_cast<ProceduralTerrainAppearance *>(appearance);
	if (proceduralTerrainAppearance) {
		ServerObjectTerrainModificationNotification::setTerrainAppearance(proceduralTerrainAppearance);
	}

	terrainObject->addToWorld();
}

// ----------------------------------------------------------------------

void GameServer::shutdown()
{
	delete s_permissionManager;
	s_permissionManager = 0;
	if(s_metricsManagerInstalled)
		MetricsManager::remove();
	delete m_metricsData;
	m_metricsData = 0;

	m_centralService = 0;
	m_planetServerConnection = 0;

	if (m_customerServiceServerConnection != nullptr)
	{
		m_customerServiceServerConnection->disconnect();
	}
}

// ----------------------------------------------------------------------

void GameServer::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	static Archive::ReadIterator ri;
	static Archive::ByteStream bs;

	PROFILER_AUTO_BLOCK_DEFINE("GameServer::receiveMessage");

	// advise the TaskManager that the gameserver isn't dead in cases where
	// the server is handling a LOT of LoadObjectMessages and creating
	// proxies.
	if(getInstance().m_taskManagerConnection)
	{
		if(Clock::timeMs() - s_lastTaskKeepaliveTime > 60000)
		{
			DEBUG_WARNING(true, ("Sending keepalive message to taskmanager for process %i", Os::getProcessId()));
			static const GenericValueTypeMessage<unsigned long> gameServerTaskManagerKeepAlive("GameServerTaskManagerKeepAlive", Os::getProcessId());
			getInstance().m_taskManagerConnection->send(gameServerTaskManagerKeepAlive, true);
			s_lastTaskKeepaliveTime = Clock::timeMs();
		}
	}

	ServerConnection * const serverConnection = const_cast<ServerConnection *>(dynamic_cast<ServerConnection const *>(&source));

	const uint32 messageType = message.getType();

	switch (messageType) {
		case constcrc("CentralConnectionOpened") : {
			MESSAGE_PROFILER_BLOCK("CentralConnectionOpened");
			if (!m_centralServerConnection)
				m_centralServerConnection = const_cast<CentralServerConnection *>(static_cast<CentralServerConnection const *>(&source));
			break;
		}
		case constcrc("CustomerServiceServerGameServerServiceAddress"): {
			Archive::ReadIterator ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<std::string, unsigned short>> const address(ri);

			DEBUG_REPORT_LOG(true, ("GameServer: Creating customer service server connection @ (%s:%d)\n", address
					.getValue().first.c_str(), address.getValue().second));

			if (m_customerServiceServerConnection != nullptr) {
				m_customerServiceServerConnection->disconnect();
			}

			m_customerServiceServerConnection = new CustomerServiceServerConnection(address.getValue().first, address
					.getValue().second);
			break;
		}
		case constcrc("BaselinesMessage") : {
			MESSAGE_PROFILER_BLOCK("BaselinesMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			BaselinesMessage const m(ri);
			ServerObject *so = ServerWorld::findUninitializedObjectByNetworkId(m.getTarget());
			if (so) {
				so->applyBaselines(m);
			}
			else {
				// find any object
				so = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(m.getTarget()));
				if (so) {
					DEBUG_WARNING(true, ("Applied baselines on object %s which is already initialized.", m.getTarget()
					                                                                                      .getValueString()
					                                                                                      .c_str()));
					so->applyBaselines(m);
				}
				else {
					DEBUG_WARNING(true, ("Got baselines on object %s which could not be found.", m.getTarget()
					                                                                              .getValueString()
					                                                                              .c_str()));
				}
			}
			break;
		}
		case constcrc("BatchBaselinesMessage") : {
			MESSAGE_PROFILER_BLOCK("BaselinesMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			BatchBaselinesMessage const m(ri);

			std::vector <BatchBaselinesMessageData> const &baselines = m.getData();

			ServerObject *lastObject = nullptr;

			for (std::vector<BatchBaselinesMessageData>::const_iterator i = baselines.begin();
					i != baselines.end(); ++i) {
				// There's a good chance the baselines for an object will come together, so remember the previous object
				// and don't look it up again if the network id hasn't changed.
				if (!lastObject || lastObject->getNetworkId() != i->m_networkId) {
					lastObject = ServerWorld::findUninitializedObjectByNetworkId(i->m_networkId);

					if (!lastObject) {
						lastObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(i->m_networkId));
						if (lastObject) {
							WARNING(true, ("Applied baselines on object %s which is already initialized.", i
									->m_networkId.getValueString().c_str()));

						}
						else {
							WARNING(true, ("Got baselines on object %s which could not be found.", i->m_networkId
							                                                                        .getValueString()
							                                                                        .c_str()));
						}
					}
				}
				if (lastObject) {
					lastObject->applyBaselines(i->m_packageId, i->m_package);
				}
			}
			break;
		}
		case constcrc("DeltasMessage") : {
			MESSAGE_PROFILER_BLOCK("DeltasMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			DeltasMessage const m(ri);
			ServerObject *const so = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(m.getTarget()));
			if (so)
				so->applyDeltas(m);
			else
				DEBUG_WARNING(true, ("Got DeltasMessage on object %s, which could not be found.", m.getTarget()
				                                                                                   .getValueString()
				                                                                                   .c_str()));
			break;
		}

		case constcrc("ChunkCompleteMessage") : {
			MESSAGE_PROFILER_BLOCK("ChunkCompleteMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			ChunkCompleteMessage const m(ri);
			GameServer::getInstance().sendToPlanetServer(m);

			std::vector <std::pair<int, int>> const &chunks = m.getChunks();
			for (std::vector < std::pair < int, int > > ::const_iterator i = chunks.begin(); i != chunks.end();
			++i)
			ServerBuildoutManager::onChunkComplete(i->first, i->second);
			break;
		}

		case constcrc("CentralConnectionClosed") : {
			MESSAGE_PROFILER_BLOCK("CentralConnectionClosed");
			// @ todo : shutdown gracefully
			DEBUG_REPORT_LOG(true, ("The connection to the central server has been closed\n"));
			setDone("CentralConnectionClosed : %s", serverConnection->getDisconnectReason().c_str());
			m_centralServerConnection = 0;
			break;
		}
		case constcrc("TaskConnectionOpened") : {
			MESSAGE_PROFILER_BLOCK("TaskConnectionOpened");
			DEBUG_REPORT_LOG(true, ("The connection with the task manager has been established\n"));
			break;
		}
		case constcrc("TaskConnectionClosed") : {
			MESSAGE_PROFILER_BLOCK("TaskConnectionClosed");
			DEBUG_REPORT_LOG(true, ("The connection to the task manager has closed\n"));
			m_taskManagerConnection = 0;
			break;
		}
		case constcrc("AddGameServer") : {
			MESSAGE_PROFILER_BLOCK("AddGameServer");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <uint32> const addGameServerMessage(ri);

			uint32 const pid = addGameServerMessage.getValue();

			DEBUG_REPORT_LOG(true, ("Received AddGameServer for server %lu.\n", pid));

			FATAL(std::find(m_gameServerPids.begin(), m_gameServerPids.end(), pid) != m_gameServerPids
					.end(), ("Got AddGameServer for server %lu which we thought was already running?", pid));

			m_gameServerPids.push_back(pid);
			std::sort(m_gameServerPids.begin(), m_gameServerPids.end());
			break;
		}
		case constcrc("ExcommunicateGameServerMessage") : {
			MESSAGE_PROFILER_BLOCK("ExcommunicateGameServerMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			ExcommunicateGameServerMessage msg(ri);

			uint32 const pid = msg.getServerId();

			DEBUG_REPORT_LOG(true, ("Received ExcommunicateGameServer for server %lu.\n", pid));
			LOG("GameGameConnect", ("Server %lu was told that server %lu has gone away by Central", getProcessId(), pid));

			FATAL(pid ==
			      getProcessId(), ("Crashing because Central told us to (probably indicates we weren't responding to pings)"));

			std::vector<uint32>::iterator i = std::find(m_gameServerPids.begin(), m_gameServerPids.end(), pid);
			if (i != m_gameServerPids.end()) {
				m_gameServerPids.erase(i);

				MessageToQueue::getInstance().onGameServerDisconnect(pid);
				ServerUniverse::getInstance().onServerConnectionClosed(pid);
				AuthTransferTracker::handleGameServerDisconnect(pid);
			}
			break;
		}
		case constcrc("CommoditiesServerConnectionClosed") : {
			DEBUG_REPORT_LOG(true, ("CommoditiesServer connection closed\n"));
			CommoditiesMarket::closeCommoditiesServerConnection();
			break;
		}
		case constcrc("ConnectionServerAddress") : {
			MESSAGE_PROFILER_BLOCK("ConnectionServerAddress");
			// Central is telling us about a new connection server.
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			ConnectionServerAddress const msg(ri);
			ConnectionServerConnection *const newConn = new ConnectionServerConnection(msg.getGameServiceAddress(), msg
					.getGameServicePort());
			m_connectionServerVector->push_back(newConn);
			break;
		}
		case constcrc("ConnectionServerConnectionClosed"):
		case constcrc("ConnectionServerConnectionDestroyed") : {
			MESSAGE_PROFILER_BLOCK("ConnectionServerConnectionClosed");
			ConnectionServerConnection const *const c = dynamic_cast<const ConnectionServerConnection *>(&source);
			ConnectionServerVector::iterator i = std::find(m_connectionServerVector->begin(), m_connectionServerVector
					->end(), c);
			if (i != m_connectionServerVector->end()) {
				m_connectionServerVector->erase(i);
			}
			break;
		}
		case constcrc("NewClient") : {
			MESSAGE_PROFILER_BLOCK("NewClient");
			static std::string const loginTrace("TRACE_LOGIN");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			NewClient const msg(ri);
			LOG(loginTrace, ("NewClient(%s)", msg.getNetworkId().getValueString().c_str()));

			//@todo make sure this holds up with non players (like vehicles)
			//because right not, we construct a new client object for EACH object
			//even ones that could be controlled by the same client.
			//Also is it possible to receive this message for an object we already control?

			if (m_clients->find(msg.getNetworkId()) != m_clients->end()) {
				// inconsistant state, drop the client
				LOG(loginTrace, ("NewClient(%s) : Error!  Getting a NewClient message for a client that already exists", msg
						.getNetworkId().getValueString().c_str()));

				KickPlayer const kickMessage(msg.getNetworkId(), "Duplicate Client");
				sendToConnectionServers(kickMessage);
				dropClient(msg.getNetworkId());
				return;
			}

			// don't allow login if the character is in the middle of CTS or has been successfully CTS(ed)
			ServerObject *const obj = ServerWorld::findObjectByNetworkId(msg.getNetworkId());
			if (obj) {
				bool kickRequested = false;

				DynamicVariableList const &objVars = obj->getObjVars();
				int timeOut;
				if (objVars.getItem("disableLoginCtsInProgress", timeOut)) {
					if (timeOut > static_cast<int>(ServerClock::getInstance().getGameTimeSeconds())) {
						MessageToQueue::getInstance().sendMessageToC(obj
								->getNetworkId(), "C++kickPlayerCtsCompletedOrInProgress", "", 0, false);
						kickRequested = true;
					}
					else {
						obj->removeObjVarItem("disableLoginCtsInProgress");
					}
				}

				int transferredTime;
				if (!kickRequested && objVars.getItem("disableLoginCtsCompleted", transferredTime) &&
				    !msg.getUsingAdminLogin()) {
					MessageToQueue::getInstance()
							.sendMessageToC(obj->getNetworkId(), "C++kickPlayerCtsCompletedOrInProgress", "", 0, false);
					kickRequested = true;
				}
			}

			LOG(loginTrace, ("NewClient(%s) : A new client is being created from connection server", msg.getNetworkId()
			                                                                                            .getValueString()
			                                                                                            .c_str()));
			std::set <NetworkId> const observedObjectSet(msg.getObservedObjects().begin(), msg.getObservedObjects()
			                                                                                  .end());
			Client *const c = new Client(
					const_cast<ConnectionServerConnection &>(static_cast<const ConnectionServerConnection &>(source)),
					msg.getNetworkId(),
					msg.getAccountName(),
					msg.getIpAddress(),
					msg.getIsSecure(),
					msg.getIsSkipLoadScreen(),
					msg.getStationId(),
					observedObjectSet,
					msg.getGameFeatures(),
					msg.getSubscriptionFeatures(),
					Client::AccountFeatureIdList(),
					msg.getEntitlementTotalTime(),
					msg.getEntitlementEntitledTime(),
					msg.getEntitlementTotalTimeSinceLastLogin(),
					msg.getEntitlementEntitledTimeSinceLastLogin(),
					msg.getBuddyPoints(),
					msg.getConsumedRewardEvents(),
					msg.getClaimedRewardItems(),
					msg.getUsingAdminLogin(),
					CombatDataTable::CSFT_All,
					128,
					90,
					false,
					false,
					msg.getSendToStarport());

			ClientMap::iterator f = m_clients->find(msg.getNetworkId());

			if (f != m_clients->end())
				m_clients->erase(msg.getNetworkId());

			m_clients->insert(std::make_pair(msg.getNetworkId(), c));

			if (!msg.getUsingAdminLogin()) {
				AccountFeatureIdRequest const req(NetworkId::cms_invalid, GameServer::getInstance().getProcessId(), msg
						.getNetworkId(), static_cast<StationId>(msg
						.getStationId()), PlatformGameCode::SWG, AccountFeatureIdRequest::RR_Reload);
				c->sendToConnectionServer(req);
			}
			break;
		}
		case constcrc("AuthTransferClientMessage") : {
			MESSAGE_PROFILER_BLOCK("AuthTransferClientMessage");
			static const std::string loginTrace("TRACE_LOGIN");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			AuthTransferClientMessage const msg(ri);
			LOG(loginTrace, ("AuthTransferClientMessage(%s)", msg.getNetworkId().getValueString().c_str()));

			//@todo make sure this holds up with non players (like vehicles)
			//because right not, we construct a new client object for EACH object
			//even ones that could be controlled by the same client.
			//Also is it possible to receive this message for an object we already control?

			if (m_clients->find(msg.getNetworkId()) != m_clients->end()) {
				// inconsistant state, drop the client
				LOG(loginTrace, ("AuthTransferClientMessage(%s) : Error!  Getting a AuthTransferClientMessage message for a client that already exists", msg
						.getNetworkId().getValueString().c_str()));
				KickPlayer const kickMessage(msg.getNetworkId(), "Duplicate Client");
				sendToConnectionServers(kickMessage);
				dropClient(msg.getNetworkId());
				return;
			}

			// locate the connection server for the client
			ConnectionServerConnection *const connectionServer = getConnectionServerConnection(msg
					.getConnectionServerIp(), msg.getConnectionServerPort());

			if (!connectionServer) {
				// inconsistant state, drop the client
				LOG(loginTrace, ("AuthTransferClientMessage(%s) : Error!  Getting a AuthTransferClientMessage message for a client but could not locate connection server (%s:%hu)", msg
						.getNetworkId().getValueString().c_str(), msg.getConnectionServerIp().c_str(), msg
						.getConnectionServerPort()));
				KickPlayer const kickMessage(msg.getNetworkId(), "Invalid Connection Server");
				sendToConnectionServers(kickMessage);
				dropClient(msg.getNetworkId());
				return;
			}

			LOG(loginTrace, ("AuthTransferClientMessage(%s) : A new client is being created from game server %lu", msg
					.getNetworkId().getValueString().c_str(), msg.getSourceServerPid()));

			std::set <NetworkId> observedObjectSet(msg.getObservedObjects().begin(), msg.getObservedObjects().end());
			Client *c = new Client(*connectionServer, msg.getNetworkId(), msg.getAccount(), msg.getIpAddress(), msg
					.getSecure(), msg.getSkipLoadScreen(), msg.getStationId(), observedObjectSet, msg
					.getGameFeatures(), msg.getSubscriptionFeatures(), msg.getAccountFeatureIds(), msg
					.getEntitlementTotalTime(), msg.getEntitlementEntitledTime(), msg
					.getEntitlementTotalTimeSinceLastLogin(), msg.getEntitlementEntitledTimeSinceLastLogin(), msg
					.getBuddyPoints(), msg.getConsumedRewardEvents(), msg.getClaimedRewardItems(), msg
					.getUsingAdminLogin(), static_cast<CombatDataTable::CombatSpamFilterType>(msg
					.getCombatSpamFilter()), msg.getCombatSpamRangeSquaredFilter(), msg
					.getFurnitureRotationDegree(), msg.getHasUnoccupiedJediSlot(), msg.getIsJediSlotCharacter());
			ClientMap::iterator f = m_clients->find(msg.getNetworkId());

			if (f != m_clients->end())
				m_clients->erase(msg.getNetworkId());

			m_clients->insert(std::make_pair(msg.getNetworkId(), c));
			break;
		}

		//---
		// application messages
		case constcrc("CentralGameServerSetProcessId") : {
			MESSAGE_PROFILER_BLOCK("CentralGameServerSetProcessId");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			CentralGameServerSetProcessId const m(ri);

			m_processId = m.getProcessId();
			LOG("GameServer", ("I am process %d", m_processId));
			LOG("GameGameConnect", ("Received CentralGameServerSetProcessId message for pid %d", m_processId));
			DEBUG_FATAL(m.getClockSubtractInterval() ==
			            0, ("Got 0 for the clock subtract interval (probably indicates we connected to Central too early).\n"));
			ServerClock::getInstance().setSubtractInterval(m.getClockSubtractInterval());
			m_clusterName = m.getClusterName();

			ScriptParams s;
			s.addParam(m_clusterName.c_str());
			GameScriptObject::runOneScript("base_class", "setGalaxyName", "s", s);
			Chat::createSystemRooms(m_clusterName, ServerWorld::getSceneId());

			if (!m_centralServerConnection) {
				m_centralServerConnection = const_cast<CentralServerConnection *>(static_cast<CentralServerConnection const *>(&source));
				DEBUG_FATAL(true, ("m_centralServerConnection should have already been set when receiving CentralConnectionOpened."));
			}

			CentralGameServerConnect const connectMessage(
					ConfigServerGame::getSceneID(),
					"",
					0,
					"",
					0);

			m_centralServerConnection->send(connectMessage, true);

			loadRetroactiveCtsHistory();
			loadRetroactivePlayerCityCreationTime();
			break;
		}
		case constcrc("SetAuthoritativeMessage") : {
			MESSAGE_PROFILER_BLOCK("SetAuthoritativeMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			SetAuthoritativeMessage const t(ri);

			ServerObject *const object = ServerWorld::findObjectByNetworkId(t.getId());
			if (object) {
				bool allowed = true;

				// if the PlanetServer is asking us to transfer authority for an object that
				// is contained, reject it, because contained object must have the same authority
				// as its container; this can happen in a race condition where authority transfer
				// is requested, and while the request is happening, the object moves into a container
				if ((&source == m_planetServerConnection) && !t.getSceneChange() && object->isAuthoritative()) {
					Object const *const topmostContainer = ContainerInterface::getTopmostContainer(*object);

					if (topmostContainer != object) {
						WARNING(true, ("Denying authority transfer for (%s) from game server (%lu) to game server (%lu) because it is contained by (%s)", object
								->getDebugInformation().c_str(), GameServer::getInstance().getProcessId(), t
								.getProcess(), (topmostContainer ? topmostContainer->getDebugInformation().c_str()
						                                         : "nullptr")));

						allowed = false;

						GenericValueTypeMessage <std::pair<NetworkId, uint32>> const denyMessage(
								"DenySetAuthoritativeMessage",
								std::make_pair(t.getId(), GameServer::getInstance().getProcessId()));

						sendToPlanetServer(denyMessage);

						object->updatePositionOnPlanetServer(true);
					}
				}

				if (allowed) {
					if (t.getGoalIsValid())
						object->transferAuthority(t.getProcess(), !t.getSceneChange(), t.getHandlingCrash(), t
								.getGoalCell(), t.getGoalTransform(), false);
					else
						object->transferAuthority(t.getProcess(), !t.getSceneChange(), t.getHandlingCrash(), false);
				}
			}
			break;
		}
		case constcrc("LoadObjectMessage") : {
			MESSAGE_PROFILER_BLOCK("LoadObjectMessage");

			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			LoadObjectMessage const t(ri);

			ServerObject *const target = ServerWorld::findObjectByNetworkId(t.getId());
			if (target) {
				createRemoteProxy(t.getProcess(), target);
			} else {
				WARNING(true, ("Told to create object %s on %d we know nothing about", t.getId().getValueString().c_str(), t.getProcess()));
			}
			break;
		}
		case constcrc("UnloadObjectMessage") : {
			MESSAGE_PROFILER_BLOCK("UnloadObjectMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			UnloadObjectMessage const t(ri);

			DEBUG_REPORT_LOG(ConfigServerGame::getLogObjectLoading(), ("Got unload object message for %s\n", t.getId()
			                                                                                                  .getValueString()
			                                                                                                  .c_str()));

			ServerObject *const object = ServerWorld::findObjectByNetworkId(t.getId());
			if (object)
				object->unload();
			DEBUG_WARNING(!object, ("Handling unload object message but the object does not exist\n"));
			break;
		}
		case constcrc("UnloadProxyMessage") : {
			MESSAGE_PROFILER_BLOCK("UnloadProxyMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			UnloadProxyMessage const unloadProxyMessage(ri);

			DEBUG_REPORT_LOG(ConfigServerGame::getLogObjectLoading(), ("Received UnloadProxyMessage for object %s server %lu\n", unloadProxyMessage
					.getObjectId().getValueString().c_str(), unloadProxyMessage.getProxyGameServerId()));

			ServerObject *const object = ServerWorld::findObjectByNetworkId(unloadProxyMessage.getObjectId());
			if (object) {
				if (object->isAuthoritative()) {
					if (unloadProxyMessage.getProxyGameServerId() != getProcessId())
						object->removeServerFromProxyList(unloadProxyMessage.getProxyGameServerId());
					else
						DEBUG_WARNING(true, ("Can't unproxy object %s from this server because this server is authoritative.", unloadProxyMessage
								.getObjectId().getValueString().c_str()));
				}
				else {
					DEBUG_REPORT_LOG(ConfigServerGame::getLogObjectLoading(), ("Forwarding UnloadProxyMessage for %s to authoritative server\n", unloadProxyMessage
							.getObjectId().getValueString().c_str()));

					ServerMessageForwarding::begin(object->getAuthServerProcessId());

					ServerMessageForwarding::send(unloadProxyMessage);

					ServerMessageForwarding::end();
				}
			}
			else
				DEBUG_WARNING(true, ("Received UnloadProxyMessage for object %s server %lu, but the object could not be found", unloadProxyMessage
						.getObjectId().getValueString().c_str(), unloadProxyMessage.getProxyGameServerId()));
			break;
		}
		case constcrc("ProfilerOperationMessage") : {
			MESSAGE_PROFILER_BLOCK("ProfilerOperationMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			ProfilerOperationMessage const m(ri);
			uint32 const processId = m.getProcessId();
			if (processId == 0 || processId == getProcessId())
				Profiler::handleOperation(m.getOperation().c_str());
			break;
		}
		case constcrc("CentralCreateCharacter") : {
			MESSAGE_PROFILER_BLOCK("CentralCreateCharacter");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			handleCreateCharacter(new CentralCreateCharacter(ri));
			break;
		}
		case constcrc("VerifyNameResponse") : {
			MESSAGE_PROFILER_BLOCK("VerifyNameResponse");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			VerifyNameResponse vrn(ri);

			if (isCreatePending(vrn.getStationId())) {
				handleCharacterCreateNameVerification(vrn);
			}
			else {
				handleVerifyAndLockNameVerification(vrn);
			}
			break;
		}
		case constcrc("RandomNameRequest") : {
			MESSAGE_PROFILER_BLOCK("RandomNameRequest");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			RandomNameRequest const crnr(ri);
			handleNameRequest(crnr);
			break;
		}
		case constcrc("VerifyAndLockNameRequest") : {
			MESSAGE_PROFILER_BLOCK("VerifyAndLockNameRequest");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			VerifyAndLockNameRequest const valnr(ri);
			handleVerifyAndLockNameRequest(valnr, true, true);
			break;
		}
		case constcrc("ObjControllerMessage") : {
			MESSAGE_PROFILER_BLOCK("ObjControllerMessage");

			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			ObjControllerMessage c(ri);

			// Could be in baselines so it may be initialized or it may not.  Since we don't know we have to use NetworkIdManager to get it.
			// The object could also have been destroyed due to an auth transfer, so if it has been and not all servers have confirmed it,
			// we need to forward the message.

			bool appended = false;

			ServerObject *const target = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(c.getNetworkId()));
			if (target != nullptr) {
				// valid target, get its controller
				ServerController *const controller = safe_cast<ServerController *>(target->getController());
				if (controller != nullptr) {
					uint32 flags = c.getFlags();
					if (flags & GameControllerMessageFlags::DEST_AUTH_SERVER) {
						if (ConfigServerGame::getEnableDebugControllerMessageSpam()) {
							WARNING(!target
									->isAuthoritative(), ("Received a controller message (%d) destined for the authoritative object (%s:%s) on this server, but the object is NOT authoritative on this server", c
									.getMessage(), target->getObjectTemplateName(), target->getNetworkId()
							                                                              .getValueString().c_str()));
						}
					}
					flags |= GameControllerMessageFlags::SOURCE_REMOTE_SERVER;
					controller->appendMessage(c.getMessage(), c.getValue(), c.getData(), flags);
					appended = true;
				}
			}
			else {
				uint32 const destServer = AuthTransferTracker::getAuthTransferDest(c.getNetworkId());
				if (destServer) {
					if (ConfigServerGame::getEnableDebugControllerMessageSpam()) {
						WARNING(true, ("ObjControllerMessage %d for object %s arrived, but the object has transferred authority to server %d", c
								.getMessage(), c.getNetworkId().getValueString().c_str(), destServer));
					}

					ServerMessageForwarding::begin(destServer);

					ServerMessageForwarding::send(c);

					ServerMessageForwarding::end();
				}
				else
					DEBUG_WARNING(true, ("Got ObjControllerMessage for object %s, which could not be found.", c
							.getNetworkId().getValueString().c_str()));
			}

			if (!appended)
				delete c.getData();

			break;
		}
		case constcrc("EndBaselinesMessage") : {
			MESSAGE_PROFILER_BLOCK("EndBaselinesMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			EndBaselinesMessage const t(ri);
			ServerObject *const object = ServerWorld::findUninitializedObjectByNetworkId(t.getId());
			if (object != nullptr) {
				DEBUG_REPORT_LOG(ConfigServerGame::getLogObjectLoading(), ("Received EndBaselinesMessage for %s\n", t
						.getId().getValueString().c_str()));
				ServerController *const controller = dynamic_cast<ServerController *>(object->getController());
				NOT_NULL(controller);

				bool const fromDbServer = serverConnection && serverConnection == m_databaseProcessConnection;
				object->serverObjectEndBaselines(fromDbServer);
			}
			else {
				ServerObject *const badObject = ServerWorld::findObjectByNetworkId(t.getId());
				WARNING(!badObject, ("Got EndBaeslinesMessage on object (%s) that doesn't exist!", t.getId()
				                                                                                    .getValueString()
				                                                                                    .c_str()));
				WARNING(badObject, ("Got EndBaselinesMessage on object (%s : %s) which has already been created and placed in the world!", badObject
						->getTemplateName(), badObject->getNetworkId().getValueString().c_str()));
			}

			break;
		}
		case constcrc("TaskShutdownGameServer") : {
			MESSAGE_PROFILER_BLOCK("TaskShutdownGameServer");
			setDone("TaskShutdownGameServer");
			break;
		}
		case constcrc("CreateObjectByCrcMessage") : {
			s_totalObjectCreateMessagesReceived++;

			MESSAGE_PROFILER_BLOCK("CreateObjectByCrcMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();

			NetworkId networkId;
			uint32 templateCrc;
			bool createAuthoritative;

			CreateObjectByCrcMessage const t(ri);
			networkId = t.getId();
			templateCrc = t.getCrc();
			createAuthoritative = t.getCreateAuthoritative();

			NOT_NULL(serverConnection);
			DEBUG_REPORT_LOG(ConfigServerGame::getLogObjectLoading(), ("CreateObjectMessageByName %s:%s\n", ObjectTemplateList::lookUp(templateCrc)
					.getString(), networkId.getValueString().c_str()));

			Object *const object = NetworkIdManager::getObjectById(networkId);
			bool okToCreate = true;
			if (object) {
				DEBUG_REPORT_LOG(true, ("WARNING: GOT create message for object %s that already exists!\n", networkId
						.getValueString().c_str()));
				if (object->getKill()) {
					delete object;
					okToCreate = true;
				}
				else {
					WARNING_STRICT_FATAL(true, ("Hey, Got create message for object %s That isn't being deleted!", networkId
							.getValueString().c_str()));
					okToCreate = false;
				}
			}

			if (okToCreate) {
				//-- Since space is single-server, we will never have proxied space objects and we can ignore the hyperspace variable in the CreateObjectBy*Message message.
				ServerWorld::createProxyObject(templateCrc, networkId, createAuthoritative);
			}

			break;
		}
		case constcrc("CreateSyncUiMessage") : {
			MESSAGE_PROFILER_BLOCK("CreateSyncUiMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			CreateSyncUiMessage const msg(ri);
			ServerObject *const so = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg.getId()));
			if (so)
				so->addSynchronizedUi(msg.getClients());

			break;
		}
		case constcrc("SynchronizeScriptVarsMessage") : {
			MESSAGE_PROFILER_BLOCK("SynchronizeScriptVarsMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			SynchronizeScriptVarsMessage const msg(ri);
			ServerObject *so = ServerWorld::findUninitializedObjectByNetworkId(msg.getNetworkId());
			if (so) {
				GameScriptObject const *const script = so->getScriptObject();
				if (script)
					script->unpackScriptVars(msg.getData());
			}
			else {
				so = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg.getNetworkId()));
				DEBUG_WARNING(so, ("Got SynchronizeScriptVarsMessage for initialized object"));
			}

			break;
		}
		case constcrc("SynchronizeScriptVarDeltasMessage") : {
			MESSAGE_PROFILER_BLOCK("SynchronizeScriptVarDeltasMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			SynchronizeScriptVarDeltasMessage const msg(ri);
			ServerObject *const so = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg.getNetworkId()));
			if (so) {
				GameScriptObject const *const script = so->getScriptObject();
				if (script)
					script->unpackDeltaScriptVars(msg.getData());
			}
			else
				DEBUG_WARNING(true, ("Got SynchronizeScriptVarDeltasMessage on object %s, which could not be found.", msg
						.getNetworkId().getValueString().c_str()));

			break;
		}
		case constcrc("UpdateObjectPositionMessage") : {
			MESSAGE_PROFILER_BLOCK("UpdateObjectPositionMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			UpdateObjectPositionMessage const msg(ri);
			ServerObject *const so = ServerWorld::findUninitializedObjectByNetworkId(msg.getNetworkId());
			if (so)
				so->applyObjectPositionUpdate(msg);

			break;
		}
		case constcrc("UpdateContainmentMessage") : {
			MESSAGE_PROFILER_BLOCK("UpdateContainmentMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			UpdateContainmentMessage const msg(ri);
			ServerObject *const so = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg.getNetworkId()));
			if (so) {
				// Handle the containment update.
				so->updateContainment(msg.getContainerId(), msg.getSlotArrangement());
			}

			break;
		}
		case constcrc("DropClient") : {
			MESSAGE_PROFILER_BLOCK("DropClient");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			DropClient const msg(ri);
			dropClient(msg.getNetworkId(), msg.getImmediate());

			break;
		}
		case constcrc("SetPlanetServerMessage") : {
			MESSAGE_PROFILER_BLOCK("SetPlanetServerMessage");
			DEBUG_REPORT_LOG(true, ("Received SetPlanetServerMessage\n"));
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			SetPlanetServerMessage const m(ri);
			m_planetServerConnection = new PlanetServerConnection(m.getAddress(), m.getPort());
			break;
		}
		case constcrc("PlanetConnectionOpened") : {
			MESSAGE_PROFILER_BLOCK("PlanetConnectionOpened");
			m_planetServerConnection = const_cast<PlanetServerConnection *>(static_cast<const PlanetServerConnection *>(&source));
			DEBUG_REPORT_LOG(true, ("Connection with the Planet server has been established\n"));
			onPlanetServerConnectionEstablished();
			break;
		}
		case constcrc("PlanetConnectionClosed") : {
			MESSAGE_PROFILER_BLOCK("PlanetConnectionClosed");
			if (m_planetServerConnection) {
				m_planetServerConnection->setDisconnectReason("received PlanetConnectionClosed");
				m_planetServerConnection->disconnect();
			}
			m_planetServerConnection = 0;
			setDone("PlanetConnectionClosed : %s", serverConnection->getDisconnectReason().c_str());
			break;
		}
		case constcrc("GameServerSetupMessage") : {
			MESSAGE_PROFILER_BLOCK("GameServerSetupMessage");
			LOG("GameGameConnect", ("Game Server %d Received GameServerSetupMessage.", m_processId));

			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < std::vector < uint32 > , std::pair < uint32, std::pair < std::string,
					uint16 > > > >
			const m(ri);

			m_gameServerPids = m.getValue().first;
			std::sort(m_gameServerPids.begin(), m_gameServerPids.end());

			ServerMessageForwarding::beginBroadcast();

			GenericValueTypeMessage <uint32> const addGameServerMessage("AddGameServer", getProcessId());
			ServerMessageForwarding::send(addGameServerMessage);

			ServerMessageForwarding::end();

			uint32 const databaseProcessId = m.getValue().second.first;
			std::string const &databaseProcessAddress = m.getValue().second.second.first;
			uint16 const databaseProcessPort = m.getValue().second.second.second;

			connectToDatabaseProcess(databaseProcessAddress, databaseProcessPort, databaseProcessId);

			m_connectionTimeout = Clock::timeMs() + ConfigServerGame::getConnectToAllGameServersTimeout() * 1000;

			break;
		}
		case constcrc("LoadUniverseMessage") : {
			MESSAGE_PROFILER_BLOCK("LoadUniverseMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			LoadUniverseMessage const msg(ri);

			DEBUG_REPORT_LOG(true, ("Got LoadUniverseMessage\n"));
			ServerUniverse::getInstance().requestCreateProxiesOnServer(msg.getProcess());
			break;
		}
		case constcrc("SetUniverseAuthoritativeMessage") : {
			MESSAGE_PROFILER_BLOCK("SetUniverseAuthoritativeMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			SetUniverseAuthoritativeMessage const m(ri);
			ServerUniverse::getInstance().setUniverseProcess(m.getProcess());
			break;
		}
		case constcrc("AddResourceTypeMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			AddResourceTypeMessage const msg(ri);
			ServerUniverse::getInstance().handleAddResourceTypeMessage(msg);
			break;
		}
		case constcrc("AddImportedResourceType") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<NetworkId, std::string>> const msg(ri);

			if (!ServerUniverse::getInstance().getImportedResourceTypeById(msg.getValue().first))
				IGNORE_RETURN(ResourceTypeObject::addImportedResourceType(msg.getValue().second));

			break;
		}
		case constcrc("UniverseCompleteMessage") : {
			MESSAGE_PROFILER_BLOCK("UniverseCompleteMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <uint32> const msg(ri);

			ServerUniverse::getInstance().universeComplete(msg.getValue());
			GameServerUniverseLoadedMessage const reply(getProcessId(), msg.getValue());
			GameServer::getInstance().sendToCentralServer(reply);
			GameServer::getInstance().sendToPlanetServer(reply);
			gs_gameServerReady = true;

			break;
		}
		case constcrc("GameServerUniverseLoadedMessage") : {
			MESSAGE_PROFILER_BLOCK("GameServerUniverseLoadedMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GameServerUniverseLoadedMessage const msg(ri);

			ServerUniverse::getInstance().universeLoadedAck(msg.getProcessId());

			break;
		}
		case constcrc("PreloadRequestCompleteMessage") : {
			MESSAGE_PROFILER_BLOCK("PreloadRequestCompleteMessage");
			LOG("Preload", ("Game Server %d received preloadRequestCompleteMessage from database", getInstance()
					.m_processId));
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			PreloadRequestCompleteMessage const msg(ri);

			DEBUG_REPORT_LOG(true, ("Got PreloadRequestCompleteMessage.  We are server %lu in the preload list\n", msg
					.getPreloadAreaId()));

			m_preloadAreaId = msg.getPreloadAreaId();
			MetricsManager::install(m_metricsData, true, "GameServer", ServerWorld::getSceneId(), msg
					.getPreloadAreaId());
			s_metricsManagerInstalled = true;
			GameServer::getInstance().sendToPlanetServer(msg);

			// Our assigned preload number should be the same as the preload number we were started with;
			// The cluster will still work but the restartgameserver.pl script may end up restarting the wrong
			// game server because of the mismatch; it's worth putting out a log to look into what went wrong
#ifndef _DEBUG
			if (static_cast<int>(msg.getPreloadAreaId()) != ConfigServerGame::getPreloadNumber())
				LOG("Preload", ("Game Server %d got assigned preload number %lu but was launched with preload number %d", getInstance()
						.m_processId, msg.getPreloadAreaId(), ConfigServerGame::getPreloadNumber()));
#endif

			// Notify ServerWorld that preloading is complete, so it can trigger scripts as appropriate
			ServerWorld::onPreloadComplete();

			break;
		}
		case constcrc("SceneTransferMessage") : {
			MESSAGE_PROFILER_BLOCK("SceneTransferMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			SceneTransferMessage const msg(ri);

			ServerObject *obj = ServerWorld::findObjectByNetworkId(msg.getNetworkId());
			if (obj)
				obj->removeObjVarItem(OBJVAR_HAS_OUTSTANDING_REQUEST_SCENE_TRANSFER);

			if (obj && obj->isAuthoritative() && isGameServerConnected(msg.getDestinationGameServer()) &&
			    (!obj->isPlayerControlled() || obj->getClient())) {
				if (obj->getSceneId() == msg.getSceneName()) {
					WARNING_STRICT_FATAL(true, ("Same scene planetwarps are no longer supported!"));
					if (obj->getClient())
						dropClient(obj->getNetworkId());
					obj->unload();
				}
				else {
					// Mounts: if it is a rider that is trying to planet warp, change the planet warp object
					// to be the mount.
					CreatureObject *const objAsCreature = obj->asCreatureObject();
					if (objAsCreature) {
						if (objAsCreature->getState(States::RidingMount)) {
							// The obj instance is a mounted rider.  Replace obj with the mount.
							CreatureObject *const mountObject = objAsCreature->getMountedCreature();
							if (mountObject) {
								// Set the proper position for the rider to that for the
								// scene transfer.  Failure to take this step causes the
								// ServerObject::setClient() call on the destination server
								// to have the wrong position for the player, thus setting
								// up location-based containment improperly, screwing up
								// the client's view of the world.
								obj->setPosition_p(msg.getPosition_w());

								// Replace obj with mount.
								obj = mountObject;
							}
							else {
								LOG("mounts-bug", ("GS::SceneTransferMessage: server id=[%d],rider id=[%s] has RidingMount state but getMountedCreature() returns nullptr.", static_cast<int>(getProcessId()), obj
										->getNetworkId().getValueString().c_str()));
								objAsCreature->emergencyDismountForRider();
							}
						}
						else {
							ShipObject *const ship = getAttachedShip(objAsCreature);
							if (ship) {
								// Set the proper position for the rider to that for the
								// scene transfer.  Failure to take this step causes the
								// ServerObject::setClient() call on the destination server
								// to have the wrong position for the player, thus setting
								// up location-based containment improperly, screwing up
								// the client's view of the world.
								obj->setPosition_w(msg.getPosition_w());

								obj = ship;
							}
						}
					}

					// Proceed with scene transfer on the transfer focus object.
					bool doSceneChange = false;

					if (ContainerInterface::getTopmostContainer(*obj) == obj)
						doSceneChange = true;
					else {
						Transform tr(obj->getTransform_o2w());
						if (tr.isNaN()) {
							WARNING(true, ("Scene change for %s with nan in transform (i=%g,%g,%g j=%g,%g,%g k=%g,%g,%g, p=%g,%g,%g)",
									obj->getDebugInformation().c_str(),
									tr.getLocalFrameI_p().x, tr.getLocalFrameI_p().y, tr.getLocalFrameI_p().z,
									tr.getLocalFrameJ_p().x, tr.getLocalFrameJ_p().y, tr.getLocalFrameJ_p().z,
									tr.getLocalFrameK_p().x, tr.getLocalFrameK_p().y, tr.getLocalFrameK_p().z,
									tr.getPosition_p().x, tr.getPosition_p().y, tr.getPosition_p().z));
							tr.resetRotate_l2p();
						}
						Container::ContainerErrorCode err = Container::CEC_Success;
						doSceneChange = ContainerInterface::transferItemToWorld(*obj, tr, 0, err);
					}

					if (doSceneChange) {
						obj->setSceneIdOnThisAndContents(msg.getSceneName());

						//-- As soon as we change the scene id on the object, tell central about the
						//   scene change.  This fixes a bug that can occur if the authority transfer
						//   never completes where Central is left thinking we're still on this gameserver's
						//   scene (the source scene) but the object's data says we're on the destination
						//   due to the scene set above.
						GenericValueTypeMessage < std::pair < NetworkId, std::pair < std::string, bool > > >
						const setSceneMsg(
						"SetSceneForPlayer", std::make_pair(msg.getNetworkId(), std::make_pair(msg
								.getSceneName(), false)));
						GameServer::getInstance().sendToCentralServer(setSceneMsg);

						if (!msg.getSceneName().empty()) {
							std::map <NetworkId, LfgCharacterData> const &connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
							std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = connectedCharacterLfgData
									.find(msg.getNetworkId());
							if ((iterFind != connectedCharacterLfgData.end()) &&
							    (iterFind->second.locationPlanet != msg.getSceneName()))
								ServerUniverse::setConnectedCharacterPlanetData(msg.getNetworkId(), msg.getSceneName());
						}

						obj->getContainedByProperty()->setContainedBy(NetworkId::cms_invalid);
						obj->setPosition_p(msg.getPosition_w());
						if (msg.getContainerId() != NetworkId::cms_invalid)
							obj->setInteriorTeleportDestination(msg.getContainerId(), msg.getContainerName(), msg
									.getPosition_p());
						if (!msg.getScriptCallback().empty())
							obj->setTeleportScriptCallback(msg.getScriptCallback());
						obj->transferAuthority(msg.getDestinationGameServer(), false, false, false);
						obj->clearProxyList();
					}
				}
			}

			break;
		}
		case constcrc("ChatServerOnline") : {
			MESSAGE_PROFILER_BLOCK("ChatServerOnline");
			// a chat server advised the central server that it is online and
			// is ready for game servers to connect to it.
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			ChatServerOnline const cso(ri);

			if (m_chatServerConnection) {
				//we shouldn't have one already, but if we do then close it and
				//accept the new one.
				WARNING(true, ("We got notification of a new chat server coming online while we still had connection to an existing.  We're going to disconnect from the old one and connect to the new one.  This shouldn't cause any real problems, but we shouldn't have lingering chat connections laying around."));
				m_chatServerConnection->setDisconnectReason("got ChatServerOnline message");
				m_chatServerConnection->disconnect();
				m_chatServerConnection = 0;
				Chat::setChatServer(m_chatServerConnection);
			}

			if (!m_chatServerConnection) {
				REPORT_LOG(true, ("New chat server connection active\n"));
				m_chatServerConnection = new ChatServerConnection(cso.getAddress(), cso.getPort());
				Chat::setChatServer(m_chatServerConnection);
				if (!m_clusterName.empty()) {
					Chat::createSystemRooms(m_clusterName, ServerWorld::getSceneId());
				}
			}

			break;
		}
		case constcrc("ChatServerConnectionClosed") : {
			MESSAGE_PROFILER_BLOCK("ChatServerConnectionClosed");
			REPORT_LOG(true, ("GameServer: Chat Server connection closed\n"));
			if (m_chatServerConnection) {
				m_chatServerConnection->setDisconnectReason("got ChatServerConnectionClosed message");
				m_chatServerConnection->disconnect();

				// if there is an interruption of service on the network
				// between the GameServer and ChatServer, but the ChatServer
				// is still running, the game server might never reconnect. Advise
				// Central that the GameServer has lost a connection with the chat
				// server
				GameNetworkMessage const chatClosed("ChatClosedConnectionWithGameServer");
				sendToCentralServer(chatClosed);
			}
			m_chatServerConnection = 0;

			break;
		}
		case constcrc("TeleportMessage") : {
			MESSAGE_PROFILER_BLOCK("TeleportMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			TeleportMessage const msg(ri);
			handleTeleportMessage(msg);
			break;
		}
		case constcrc("TeleportToMessage") : {
			MESSAGE_PROFILER_BLOCK("TeleportToMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			TeleportToMessage const msg(ri);
			ServerObject *const serverObj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg
					.getTargetId()));
			if (serverObj && serverObj->isAuthoritative()) {

				// send teleport message
				Object const *const firstParent = ContainerInterface::getFirstParentInWorld(*serverObj);
				if (firstParent) {
					ServerMessageForwarding::begin(msg.getProcessId());

					TeleportMessage const teleportMessage(
							msg.getActorId(),
							serverObj->getSceneId(),
							firstParent->getPosition_w(),
							firstParent->getAttachedTo() ? firstParent->getAttachedTo()->getNetworkId()
							                             : NetworkId::cms_invalid,
							firstParent->getPosition_p());
					ServerMessageForwarding::send(teleportMessage);

					ServerMessageForwarding::end();
				}
			}
			break;
		}
		case constcrc("RetrievedItemLoadMessage") : {
			MESSAGE_PROFILER_BLOCK("RetrievedItemLoadMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			RetrievedItemLoadMessage const msg(ri);
			handleRetrievedItemLoadMessage(msg);
			break;
		}
		case constcrc("BiographyMessage") : {
			MESSAGE_PROFILER_BLOCK("BiographyMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			BiographyMessage const msg(ri);
			BiographyManager::onBiographyRetrieved(msg.getOwner(), msg.getBio());
			break;
		}
		case constcrc("LoadContainedObjectMessage") : {
			MESSAGE_PROFILER_BLOCK("LoadContainedObjectMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			LoadContainedObjectMessage const m(ri);
			ServerObject *const obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(m.getContainerId()));
			if (obj) {
				WARNING(!obj
						->isAuthoritative(), ("Got LoadContainedObjectMessage, but the container is not authoritative.  Container is %s, Object id %s",
						obj->getDebugInformation().c_str(), m.getObjectId().getValueString().c_str()));

				obj->onContainedObjectLoaded(m.getObjectId());
			}
			else {
				WARNING(true, ("Got LoadContainedObjectMessage, but could not find the container.  Container id %s, Object id %s",
						m.getContainerId().getValueString().c_str(), m.getObjectId().getValueString().c_str()));
			}
			break;
		}
		case constcrc("LoadContentsMessage") : {
			MESSAGE_PROFILER_BLOCK("LoadContentsMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			LoadContentsMessage const m(ri);
			ServerObject *const obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(m.getContainerId()));
			if (obj) {
				WARNING(!obj
						->isAuthoritative(), ("Got LoadContentsMessage, but the container is not authoritative.  Container is %s",
						obj->getDebugInformation().c_str()));

				obj->onAllContentsLoaded();
			}
			else {
				WARNING(true, ("Got LoadContentsMessage, but could not find the container.  Container id %s",
						m.getContainerId().getValueString().c_str()));
			}
			break;
		}
		case constcrc("FirstPlanetGameServerIdMessage") : {
			MESSAGE_PROFILER_BLOCK("FirstPlanetGameServerIdMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			FirstPlanetGameServerIdMessage const msg(ri);
			m_firstGameServerForPlanet = msg.getGameServerId();
			DEBUG_REPORT_LOG(true, ("First game server for %s is %lu\n", ServerWorld::getSceneId()
					.c_str(), m_firstGameServerForPlanet));
			break;
		}
		case constcrc("CreateDynamicRegionCircleMessage") : {
			MESSAGE_PROFILER_BLOCK("CreateDynamicRegionCircleMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			CreateDynamicRegionCircleMessage const msg(ri);
			RegionMaster::createNewDynamicRegion(msg.getCenterX(), msg.getCenterZ(), msg.getRadius(), msg.getName(), msg
					.getPlanet(), msg.getPvp(), msg.getBuildable(), msg.getMunicipal(), msg.getGeography(), msg
					.getMinDifficulty(), msg.getMaxDifficulty(), msg.getSpawnable(), msg.getMission(), msg
					.getVisible(), msg.getNotify());
			break;
		}
		case constcrc("CreateDynamicRegionRectangleMessage") : {
			MESSAGE_PROFILER_BLOCK("CreateDynamicRegionRectangleMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			CreateDynamicRegionRectangleMessage const msg(ri);
			RegionMaster::createNewDynamicRegion(msg.getMinX(), msg.getMinZ(), msg.getMaxX(), msg.getMaxZ(), msg
					.getName(), msg.getPlanet(), msg.getPvp(), msg.getBuildable(), msg.getMunicipal(), msg
					.getGeography(), msg.getMinDifficulty(), msg.getMaxDifficulty(), msg.getSpawnable(), msg
					.getMission(), msg.getVisible(), msg.getNotify());
			break;
		}
		case constcrc("CreateGroupMessage") : {
			MESSAGE_PROFILER_BLOCK("CreateGroupMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			CreateGroupMessage const msg(ri);
			ServerUniverse::getInstance().createGroup(msg.getLeader(), msg.getMembers());
			break;
		}
		case constcrc("ReloadAdminTableMessage") : {
			MESSAGE_PROFILER_BLOCK("ReloadAdminTableMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			ReloadAdminTableMessage const msg(ri);
			handleReloadAdminTableMessage(msg);
			break;
		}
		case constcrc("ReloadCommandTableMessage") : {
			MESSAGE_PROFILER_BLOCK("ReloadCommandTableMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			ReloadCommandTableMessage const msg(ri);
			handleReloadCommandTableMessage(msg);
			break;
		}
		case constcrc("ReloadScriptMessage") : {
			MESSAGE_PROFILER_BLOCK("ReloadScriptMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			ReloadScriptMessage const msg(ri);
			handleReloadScriptMessage(msg);
			break;
		}
		case constcrc("ReloadTemplateMessage") : {
			MESSAGE_PROFILER_BLOCK("ReloadTemplateMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			ReloadTemplateMessage const msg(ri);
			handleReloadTemplateMessage(msg);
			break;
		}
		case constcrc("EnableNewJediTrackingMessage") : {
			MESSAGE_PROFILER_BLOCK("EnableNewJediTrackingMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			EnableNewJediTrackingMessage const msg(ri);
			handleEnableNewJediTrackingMessage(msg);
			break;
		}
		default: {
			if (!message.isType(ReloadDatatableMessage::ms_messageName)) {
				// GameServer::receiveMessage() is getting too big for the compiler to handle
				// so some of the message handling is done in the private helper receiveMessage2()
				receiveMessage2(source, message);
			} else {
				MESSAGE_PROFILER_BLOCK(ReloadDatatableMessage::ms_messageName);
				ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
				ReloadDatatableMessage const msg(ri);
				handleReloadDatatableMessage(msg);
			}

			break;
		}
	}
}

// ----------------------------------------------------------------------
// GameServer::receiveMessage() is getting too big for the compiler to handle
// so some of the message handling is done in this private helper receiveMessage2()

void GameServer::receiveMessage2(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	static Archive::ReadIterator ri;
	static Archive::ByteStream bs;

	PROFILER_AUTO_BLOCK_DEFINE("GameServer::receiveMessage2");

	const uint32 messageType = message.getType();

	switch (messageType) {
		case constcrc("AuthTransferConfirmMessage") : {
			MESSAGE_PROFILER_BLOCK("AuthTransferConfirmMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<NetworkId, uint32>> const msg(ri);
			AuthTransferTracker::handleConfirmAuthTransfer(msg.getValue().first, msg.getValue().second);
			break;
		}
		case constcrc("PersistedPlayerMessage") : {
			MESSAGE_PROFILER_BLOCK("PersistedPlayerMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			PersistedPlayerMessage const msg(ri);
			LogoutTracker::onPersisted(msg.getPlayerId());
			sendToPlanetServer(msg);
			break;
		}
		case constcrc("RenameCharacterMessageEx") : {
			MESSAGE_PROFILER_BLOCK("RenameCharacterMessageEx");
			ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			RenameCharacterMessageEx msg(ri);
			NameManager::getInstance().renamePlayer(msg.getCharacterId(), msg.getNewName(), msg.getNewName());

			// update the character's name in the citizen and guild listing
			if (ServerUniverse::getInstance().isAuthoritative()) {
				std::string const newName(Unicode::wideToNarrow(msg.getNewName()));
				GuildInterface::verifyGuildMemberName(msg.getCharacterId(), newName);
				CityInterface::verifyCitizenName(msg.getCharacterId(), newName);
				CityInterface::verifyPgcChroniclerName(msg.getCharacterId(), newName);
			}
			break;
		}
		case constcrc("PopulationListMessage") : {
			MESSAGE_PROFILER_BLOCK("PopulationListMessage");
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			PopulationListMessage const msg(ri);
			ServerUniverse::getInstance().updatePopulationList(msg.getList());
			break;
		}
		case constcrc("CentralPingMessage") : {
			MESSAGE_PROFILER_BLOCK("CentralPingMessage");
			CentralPingMessage const reply;
			sendToCentralServer(reply);
			break;
		}
		case constcrc("LocateObject") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < uint32, std::pair < NetworkId, NetworkId > > >
			const msg(ri);

			uint32 const responsePid = msg.getValue().first;
			NetworkId const &targetId = msg.getValue().second.first;
			NetworkId const &responseId = msg.getValue().second.second;

			ServerObject const *
			const targetObj = safe_cast < ServerObject const * > (NetworkIdManager::getObjectById(targetId));
			if (targetObj) {
				std::vector <NetworkId> containers;
				for (Object const *o = ContainerInterface::getContainedByObject(*targetObj); o; o = ContainerInterface::getContainedByObject(*o))
					containers.push_back(o->getNetworkId());

				NetworkId residenceOf;
				if (!targetObj->getObjVars().getItem("player_structure.residence.building", residenceOf))
					residenceOf = NetworkId::cms_invalid;

				LocateObjectResponseMessage const msg(
						targetId,
						responseId,
						responsePid,
						targetObj->findPosition_w(),
						ConfigServerGame::getSceneID(),
						targetObj->getObjectTemplateName(),
						getProcessId(),
						containers,
						targetObj->isAuthoritative(),
						residenceOf);

				sendToCentralServer(msg);
			}
			break;
		}
		case constcrc("LocateObjectByTemplateName") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < uint32, std::pair < uint32, NetworkId > > >
			const msg(ri);

			uint32 const responsePid = msg.getValue().first;
			uint32 const templateCrc = msg.getValue().second.first;
			NetworkId const &responseId = msg.getValue().second.second;

			NetworkIdManager::NetworkIdObjectHashMap const &allObjects = NetworkIdManager::getAllObjects();
			for (NetworkIdManager::NetworkIdObjectHashMap::const_iterator iter = allObjects.begin();
					iter != allObjects.end(); ++iter) {
				if (!iter->second->isAuthoritative()) {
					continue;
				}

				ObjectTemplate const *const objectTemplate = iter->second->getObjectTemplate();
				if (objectTemplate && (objectTemplate->getCrcName().getCrc() == templateCrc)) {
					std::vector <NetworkId> containers;
					for (Object const *o = ContainerInterface::getContainedByObject(*(iter
							->second)); o; o = ContainerInterface::getContainedByObject(*o))
						containers.push_back(o->getNetworkId());

					LocateObjectResponseMessage const msg(
							iter->second->getNetworkId(),
							responseId,
							responsePid,
							iter->second->findPosition_w(),
							ConfigServerGame::getSceneID(),
							objectTemplate->getName(),
							getProcessId(),
							containers,
							true,
							NetworkId::cms_invalid);

					sendToCentralServer(msg);
				}
			}
			break;
		}
		case constcrc("LocatePlayerByPartialName") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < uint32, std::pair < std::string, NetworkId > > >
			const msg(ri);

			uint32 const responsePid = msg.getValue().first;
			std::string const &partialName = msg.getValue().second.first;
			NetworkId const &responseId = msg.getValue().second.second;

			// Try to find player objects with matching names
			std::set < PlayerObject const * > const &players = PlayerObject::getAllPlayerObjects();
			if (!players.empty()) {
				// Convert the player name to wide character string
				Unicode::String const widePartialName = Unicode::toLower(Unicode::utf8ToWide(partialName));

				// See if a player object has a "matching" first name
				std::set < PlayerObject const * > ::const_iterator
				i;
				for (i = players.begin(); i != players.end(); ++i) {
					PlayerObject const *const playerObject = *i;
					CreatureObject const *const creatureObject = playerObject->getCreatureObject();

					// don't include logged out players waiting to be saved or not initialized
					if (!creatureObject || !creatureObject->isInWorld() || !creatureObject->isAuthoritative() ||
					    !creatureObject->getClient())
						continue;

					// Lower case the first name of the player for the comparison
					Unicode::String lowerCasePlayerName = Unicode::toLower(creatureObject
							->getAssignedObjectFirstName());

					// Try to find the partial name somewhere within the player name
					if (partialName == "*" || lowerCasePlayerName.find(widePartialName) != Unicode::String::npos) {
						LocatePlayerResponseMessage const msg(
								creatureObject->getNetworkId(),
								responseId,
								responsePid,
								ConfigServerGame::getSceneID(),
								creatureObject->findPosition_w(),
								getProcessId());

						sendToCentralServer(msg);
					}
				}
			}
			break;
		}
		case constcrc("LocateWarden") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<uint32, NetworkId>> const msg(ri);

			uint32 const responsePid = msg.getValue().first;
			NetworkId const &responseId = msg.getValue().second;

			// Try to find warden player objects
			std::set < PlayerObject const * > const &players = PlayerObject::getAllPlayerObjects();
			if (!players.empty()) {
				std::set < PlayerObject const * > ::const_iterator
				i;
				for (i = players.begin(); i != players.end(); ++i) {
					PlayerObject const *const playerObject = *i;
					CreatureObject const *const creatureObject = playerObject->getCreatureObject();

					// don't include logged out players waiting to be saved or not initialized
					if (!creatureObject || !creatureObject->isInWorld() || !creatureObject->isAuthoritative() ||
					    !creatureObject->getClient())
						continue;

					if (playerObject->isWarden()) {
						LocatePlayerResponseMessage const msg(
								creatureObject->getNetworkId(),
								responseId,
								responsePid,
								ConfigServerGame::getSceneID(),
								creatureObject->findPosition_w(),
								getProcessId());

						sendToCentralServer(msg);
					}
				}
			}
			break;
		}
		case constcrc("LocateCreatureByCreatureName") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < uint32, std::pair < std::string, NetworkId > > >
			const msg(ri);

			uint32 const responsePid = msg.getValue().first;
			std::string const &creatureName = msg.getValue().second.first;
			NetworkId const &responseId = msg.getValue().second.second;

			CreatureObject::AllCreaturesSet const &creatureList = CreatureObject::getAllCreatures();
			for (CreatureObject::AllCreaturesSet::const_iterator i = creatureList.begin();
					i != creatureList.end(); ++i) {
				CreatureObject const *const creatureObj = *i;
				if (creatureObj->isAuthoritative()) {
					AICreatureController const *const aiCreatureController = AICreatureController::asAiCreatureController(creatureObj
							->getController());
					if (aiCreatureController == nullptr) {
						continue;
					}

					PersistentCrcString const &pcsCreatureName = aiCreatureController->getCreatureName();
					if (pcsCreatureName.isEmpty()) {
						continue;
					}

					if (creatureName != pcsCreatureName.getString()) {
						continue;
					}

					std::vector <NetworkId> containers;
					for (Object const *o = ContainerInterface::getContainedByObject(*creatureObj); o; o = ContainerInterface::getContainedByObject(*o))
						containers.push_back(o->getNetworkId());

					LocateObjectResponseMessage const msg(
							creatureObj->getNetworkId(),
							responseId,
							responsePid,
							creatureObj->findPosition_w(),
							ConfigServerGame::getSceneID(),
							creatureObj->getObjectTemplateName(),
							getProcessId(),
							containers,
							true,
							NetworkId::cms_invalid);

					sendToCentralServer(msg);
				}
			}
			break;
		}
		case constcrc("LSBOIReq") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < std::pair < uint32, bool >, std::pair < std::pair < NetworkId,
					NetworkId >, NetworkId > > >
			const locateStructureByOwnerIdReq(ri);

			handleMessageLocateStructureByOwnerIdReq(*this, locateStructureByOwnerIdReq.getValue().first
			                                                                           .first, locateStructureByOwnerIdReq
					.getValue().first.second, locateStructureByOwnerIdReq.getValue().second.first
			                                                             .first, locateStructureByOwnerIdReq.getValue()
			                                                                                                .second
			                                                                                                .first
			                                                                                                .second, locateStructureByOwnerIdReq
					.getValue().second.second);
			break;
		}
		case constcrc("LSBOIRsp") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < std::pair < uint32, NetworkId >, std::vector < std::string > > >
			const locateStructureByOwnerIdRsp(ri);

			handleMessageLocateStructureByOwnerIdRsp(locateStructureByOwnerIdRsp.getValue().first
			                                                                    .second, locateStructureByOwnerIdRsp
					.getValue().second);
			break;
		}
		case constcrc("ReportSystemClockTime") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<uint32, NetworkId>> const msg(ri);

#ifdef WIN32
			std::string hostName = NetworkHandler::getHostName();
#else
			std::string hostName = NetworkHandler::getHumanReadableHostName();
#endif

			std::string time = FormattedString<1024>()
					.sprintf("(%3d,%5dms) %30s.%-2lu (%3lu) on %35s:%-7d (%lu) ", ObjectTracker::getNumPlayers(), static_cast<int>(
							Clock::frameTime() * 1000), ServerWorld::getSceneId()
							.c_str(), m_preloadAreaId, m_processId, hostName
							.c_str(), Os::getProcessId(), ServerClock::getInstance().getGameTimeSeconds());
			time += CalendarTime::convertEpochToTimeStringGMT(::time(nullptr));

			GenericValueTypeMessage < std::pair < uint32, std::pair < std::string, NetworkId > > > rsctr(
					"ReportSystemClockTimeResponse", std::make_pair(msg.getValue().first, std::make_pair(time, msg
							.getValue().second)));

			sendToCentralServer(rsctr);
			break;
		}
		case constcrc("ReportSystemClockTimeResponse") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < uint32, std::pair < std::string, NetworkId > > >
			const msg(ri);

			ConsoleMgr::broadcastString(msg.getValue().second.first, msg.getValue().second.second);
			break;
		}
		case constcrc("ReportPlanetaryTime") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<uint32, NetworkId>> const msg(ri);

			time_t const timeNow = ::time(nullptr);
			std::string time = FormattedString<1024>()
					.sprintf("%30s.%-2lu (%3lu) (%lu) (%ld, ", ServerWorld::getSceneId()
							.c_str(), m_preloadAreaId, m_processId, ServerClock::getInstance()
							.getGameTimeSeconds(), timeNow);
			time += CalendarTime::convertEpochToTimeStringGMT(timeNow);
			time += ")";

			const TerrainObject *const terrainObject = TerrainObject::getInstance();
			if (terrainObject) {
				const float environmentCycleTime = terrainObject->getEnvironmentCycleTime();
				if (environmentCycleTime > 0.f) {
					const float day = fmodf(static_cast<float>(ServerClock::getInstance()
							.getGameTimeSeconds()), environmentCycleTime) / environmentCycleTime;
					int hour = 6 + static_cast<int>(day * 24.f * 60.f) / 60;
					if (hour >= 24)
						hour -= 24;
					const int minute = static_cast<int>(fmodf(day * 24.f * 60.f, 60.f));

					time += FormattedString<1024>()
							.sprintf(" (%g, %02d:%02d, %f)", environmentCycleTime, hour, minute, day);
				}
				else {
					time += FormattedString<1024>()
							.sprintf(" (environmentCycleTime %g is <= 0.f)", environmentCycleTime);
				}
			}
			else {
				time += " (terrainObject is nullptr)";
			}

			GenericValueTypeMessage < std::pair < uint32, std::pair < std::string, NetworkId > > > rptr(
					"ReportPlanetaryTimeResponse", std::make_pair(msg.getValue().first, std::make_pair(time, msg
							.getValue().second)));

			sendToCentralServer(rptr);
			break;
		}
		case constcrc("ReportPlanetaryTimeResponse") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < uint32, std::pair < std::string, NetworkId > > >
			const msg(ri);

			ConsoleMgr::broadcastString(msg.getValue().second.first, msg.getValue().second.second);
			break;
		}
		case constcrc("ClusterStartComplete") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage<bool> const msg(ri);

			if (msg.getValue()) {
				// cluster initial start has completed
				if (ServerUniverse::getInstance().isAuthoritative()) {
					// add player city travel points
					std::map<int, CityInfo> const &allCities = CityInterface::getAllCityInfo();
					for (std::map<int, CityInfo>::const_iterator iter = allCities.begin();
							iter != allCities.end(); ++iter) {
						CityInfo const &ci = iter->second;
						if (ci.getCityName().empty())
							continue;

						if ((ci.getRadius() >= 400) && (ci.getTravelCost() > 0)) {
							PlanetObject *planet = ServerUniverse::getInstance().getPlanetByName(ci.getPlanet());
							if (planet) {
								planet->addTravelPoint(
										ci.getCityName(),
										ci.getTravelLoc(),
										ci.getTravelCost(),
										ci.getTravelInterplanetary(),
										TravelPoint::TPT_PC_Shuttleport);
							}
						}
					}

					// do some sanity check of GCW score
					GuildObject *const go = ServerUniverse::getInstance().getMasterGuildObject();
					CityObject const *const co = ServerUniverse::getInstance().getMasterCityObject();
					if (go && go->isAuthoritative() && co && co->isAuthoritative()) {
						// depersist GCW score, if it hasn't already been done
						std::map<std::string, int> const &gcwImperialScorePercentile = go
								->getGcwImperialScorePercentile();
						if (gcwImperialScorePercentile.empty()) {
							// this will also calculate the GCW Region Defender Bonus
							go->depersistGcwImperialScorePercentile();
						}
						else {
							// for sanity's sake, (re)calculate the GCW Region Defender Bonus
							for (std::map<std::string, int>::const_iterator iter = gcwImperialScorePercentile.begin();
									iter != gcwImperialScorePercentile.end(); ++iter)
								go->updateGcwRegionDefenderBonus(iter->first);
						}
					}
				}

				if (ConfigServerGame::getEnableCityCitizenshipFixup() &&
				    ServerUniverse::getInstance().isAuthoritative()) {
					std::vector <uint32> const &allGameServerPids = getAllGameServerPids();
					if (!allGameServerPids.empty()) {
						ServerMessageForwarding::beginBroadcast();
						GenericValueTypeMessage <uint32> clusterStartupResidenceStructureListRequest("CSRSLReq", getProcessId());
						ServerMessageForwarding::send(clusterStartupResidenceStructureListRequest);
						ServerMessageForwarding::end();

						s_clusterStartupResidenceStructureListResponse.clear();
						for (std::vector<uint32>::const_iterator iter = allGameServerPids.begin();
								iter != allGameServerPids.end(); ++iter)
							IGNORE_RETURN(s_clusterStartupResidenceStructureListResponse.insert(*iter));
					}
					else {
						s_clusterStartupResidenceStructureListByStructure.clear();
						s_clusterStartupResidenceStructureListResponse.clear();
					}
				}
				else if (!ConfigServerGame::getEnableCityCitizenshipFixup()) {
					s_clusterStartupResidenceStructureListByStructure.clear();
					s_clusterStartupResidenceStructureListResponse.clear();
				}
			}
			else {
				s_clusterStartupResidenceStructureListByStructure.clear();
				s_clusterStartupResidenceStructureListResponse.clear();
			}
			break;
		}
		case constcrc("CSRSLReq") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <uint32> const clusterStartupResidenceStructureListRequest(ri);

			ServerMessageForwarding::begin(clusterStartupResidenceStructureListRequest.getValue());

			std::map<int, std::set<NetworkId> > clusterStartupResidenceStructureListByCity;
			if (!s_clusterStartupResidenceStructureListByStructure.empty()) {
				for (std::map < NetworkId, std::pair < int, NetworkId > >
				                                            ::const_iterator iter = s_clusterStartupResidenceStructureListByStructure
						.begin(); iter != s_clusterStartupResidenceStructureListByStructure.end();
				++iter)
				IGNORE_RETURN(clusterStartupResidenceStructureListByCity[iter->second.first]
						.insert(iter->second.second));
			}

			GenericValueTypeMessage < std::pair < uint32, std::map < int, std::set < NetworkId > > > >
			                                                              clusterStartupResidenceStructureListResponse("CSRSLRsp", std::make_pair(getProcessId(), clusterStartupResidenceStructureListByCity));
			ServerMessageForwarding::send(clusterStartupResidenceStructureListResponse);
			ServerMessageForwarding::end();

			s_clusterStartupResidenceStructureListByStructure.clear();
			s_clusterStartupResidenceStructureListResponse.clear();

			break;
		}
		case constcrc("CSRSLRsp") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < uint32, std::map < int, std::set < NetworkId > > > >
			const clusterStartupResidenceStructureListResponse(ri);

			if (s_clusterStartupResidenceStructureListResponse
					    .erase(clusterStartupResidenceStructureListResponse.getValue().first) != 0) {
				static std::map<int, std::set<NetworkId> > s_clusterStartupResidenceStructureListByCity;

				std::map<int, std::set<NetworkId> > const &residenceList = clusterStartupResidenceStructureListResponse
						.getValue().second;
				if (!residenceList.empty()) {
					for (std::map < int, std::set < NetworkId > > ::const_iterator iter = residenceList.begin(); iter !=
					                                                                                             residenceList
							                                                                                             .end();
					++iter)
					{
						if (!iter->second.empty())
							s_clusterStartupResidenceStructureListByCity[iter->first]
									.insert(iter->second.begin(), iter->second.end());
					}
				}

				if (s_clusterStartupResidenceStructureListResponse.empty()) {
					// add our own information to the list
					if (!s_clusterStartupResidenceStructureListByStructure.empty()) {
						for (std::map < NetworkId, std::pair < int, NetworkId > >
						                                            ::const_iterator iter = s_clusterStartupResidenceStructureListByStructure
								.begin(); iter != s_clusterStartupResidenceStructureListByStructure.end();
						++iter)
						IGNORE_RETURN(s_clusterStartupResidenceStructureListByCity[iter->second.first]
								.insert(iter->second.second));
					}

					// we now have the complete list of every declared structure within
					// city limits; besides the mayor, only owners of these structures
					// can be in the citizenship list, so fix up the citizenship list
					// of all cities right now

					// skip the fixup if it looks like the cluster was started
					// without loading the entire planet because it means structures
					// were not loaded, so we don't have the information to determine citizenship
					if (!s_clusterStartupResidenceStructureListByCity.empty()) {
						// cities that have no declared structures don't have anything in
						// the report, so we have to manually add in those cities
						std::map <NetworkId, std::pair<int, std::string>> allCityMayors;
						std::map<int, CityInfo> const &allCities = CityInterface::getAllCityInfo();

						for (std::map<int, CityInfo>::const_iterator iterAllCities = allCities.begin();
								iterAllCities != allCities.end(); ++iterAllCities) {
							if (s_clusterStartupResidenceStructureListByCity.count(iterAllCities->first) <= 0)
								IGNORE_RETURN(s_clusterStartupResidenceStructureListByCity
										.insert(std::make_pair(iterAllCities->first, std::set<NetworkId>())));

							if (iterAllCities->second.getLeaderId().isValid())
								allCityMayors[iterAllCities->second.getLeaderId()] = std::make_pair(iterAllCities
										->first, iterAllCities->second.getCityName());
						}

						// remove citizens (except the mayor) that do not have a declared structure inside the city limits
						// if necessary, add as citizen those that have a declared structure inside the city limits
						std::string cityName;
						std::string citizenName;
						int citizenLastLoginTime = 0;
						NetworkId currentCityMayor;
						CitizenInfo const *currentCityMayorCitizenInfo;
						std::string currentCityMayorName;
						std::vector <NetworkId> citizensToRemove;
						std::string citizensToRemoveNames;
						std::string citizensToRemoveDeletedNames;
						std::string citizensToRemoveInactiveNames;
						std::string citizensToAddNames;
						bool needToAddMayorAsCitizen;
						bool removeCurrentCitizen;
						bool removeCurrentCitizenDeleted;
						bool removeCurrentCitizenInactive;
						bool hasDeclaredResidence;
						int const timeNow = static_cast<int>(::time(nullptr));
						bool const citizenInactivePackupActive = (
								ConfigServerGame::getCityCitizenshipInactivePackupStartTimeEpoch() <= timeNow);
						std::map <std::pair<int, NetworkId>, CitizenInfo> const &allCitizens = CityInterface::getAllCitizensInfo();
						for (std::map < int, std::set < NetworkId > >
						                     ::iterator iterCityId = s_clusterStartupResidenceStructureListByCity
								.begin(); iterCityId != s_clusterStartupResidenceStructureListByCity.end();
						++iterCityId)
						{
							CityInfo const &cityInfo = CityInterface::getCityInfo(iterCityId->first);

							cityName = cityInfo.getCityName();
							if (cityName.empty())
								continue;

							if (!cityInfo.getCityHallId().isValid())
								continue;

							currentCityMayor = cityInfo.getLeaderId();
							if (currentCityMayor.isValid())
								currentCityMayorCitizenInfo = CityInterface::getCitizenInfo(iterCityId
										->first, currentCityMayor);
							else
								currentCityMayorCitizenInfo = nullptr;

							if (currentCityMayorCitizenInfo)
								currentCityMayorName = currentCityMayorCitizenInfo->m_citizenName;
							else
								currentCityMayorName.clear();

							if (currentCityMayor.isValid() && !currentCityMayorCitizenInfo)
								needToAddMayorAsCitizen = true;
							else
								needToAddMayorAsCitizen = false;

							LOG("CityFixup", ("City %d (%s:%s, %s:%s) has %d declared structures within its city limits.", iterCityId
									->first, cityName.c_str(), cityInfo.getCityHallId().getValueString()
							                                           .c_str(), currentCityMayorName
									.c_str(), currentCityMayor.getValueString().c_str(), iterCityId->second.size()));

							// remove any current citizen who has not logged in for 90 days and is not marked
							// as a "protected citizen" OR who no longer exists in the DB OR who does not have
							// a declared structure in the city EXCEPT the mayor of the city; also remove any
							// current citizen who is a mayor of another city, since mayor can only be the
							// citizen of the city that they are mayor of
							static const Unicode::String mailSubjectRemove = Unicode::narrowToWide("@" +
							                                                                       StringId("city/city", "city_fixup_remove_citizens_subject")
									                                                                       .getCanonicalRepresentation());
							citizensToRemove.clear();
							citizensToRemoveNames.clear();
							citizensToRemoveDeletedNames.clear();
							citizensToRemoveInactiveNames.clear();
							for (std::map < std::pair < int, NetworkId >, CitizenInfo >
							                                              ::const_iterator iterCurrentCitizen = allCitizens
									.lower_bound(std::make_pair(iterCityId->first, NetworkId::cms_invalid));
									iterCurrentCitizen != allCitizens.end();
							++iterCurrentCitizen)
							{
								if (iterCurrentCitizen->first.first != iterCityId->first)
									break;

								// the erase() will tell us whether this current citizen is confirmed to have
								// a declared structure in the city; what it also does for us is whoever is left
								// in the list after all the erase() is done are people who have a declared
								// structure in the city but are not current citizens, and will need to be added
								// as citizens
								citizenName = iterCurrentCitizen->second.m_citizenName;
								removeCurrentCitizen = false;
								removeCurrentCitizenDeleted = false;
								removeCurrentCitizenInactive = false;
								hasDeclaredResidence = (iterCityId->second.erase(iterCurrentCitizen->first.second) >=
								                        1);

								// mayor never gets removed
								if (iterCurrentCitizen->first.second != currentCityMayor) {
									if (!hasDeclaredResidence) {
										// remove citizen because he doesn't have a declared residence inside the city limits
										removeCurrentCitizen = true;
										LOG("CustomerService", ("CityFixup: removed %s (%s) (no declared residence) as citizen of city %d (%s).", iterCurrentCitizen
												->first.second.getValueString().c_str(), citizenName.c_str(), iterCityId
												->first, cityName.c_str()));
									}
									else if (allCityMayors.count(iterCurrentCitizen->first.second) >= 1) {
										// remove citizen because he is already the mayor of another city
										removeCurrentCitizen = true;
										LOG("CustomerService", ("CityFixup: removed %s (%s) (already mayor of %d:%s) as citizen of city %d (%s).", iterCurrentCitizen
												->first.second.getValueString().c_str(), citizenName
												.c_str(), allCityMayors[iterCurrentCitizen->first.second]
												.first, allCityMayors[iterCurrentCitizen->first.second].second
										                                                               .c_str(), iterCityId
												->first, cityName.c_str()));
									}
									else {
										citizenLastLoginTime = NameManager::getInstance()
												.getPlayerLastLoginTime(iterCurrentCitizen->first.second);
										if (citizenLastLoginTime <= 0) {
											// remove citizen because he no longer exists in the DB
											removeCurrentCitizen = true;
											removeCurrentCitizenDeleted = true;
											LOG("CustomerService", ("CityFixup: removed %s (%s) (deleted) as citizen of city %d (%s).", iterCurrentCitizen
													->first.second.getValueString().c_str(), citizenName
													.c_str(), iterCityId->first, cityName.c_str()));
										}
										else if (citizenInactivePackupActive && ((citizenLastLoginTime +
										                                          ConfigServerGame::getCityCitizenshipInactivePackupInactiveTimeSeconds()) <=
										                                         timeNow) &&
										         !(iterCurrentCitizen->second.m_citizenPermissions &
										           CitizenPermissions::InactiveProtected)) {
											// remove citizen because he has not logged in for 90 days and is not marked as a "protected citizen"
											removeCurrentCitizen = true;
											removeCurrentCitizenInactive = true;
											LOG("CustomerService", ("CityFixup: removed %s (%s) (inactive %s) as citizen of city %d (%s).", iterCurrentCitizen
													->first.second.getValueString().c_str(), citizenName
													.c_str(), CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(
													timeNow - citizenLastLoginTime)).c_str(), iterCityId
													->first, cityName.c_str()));
										}
									}
								}

								if (removeCurrentCitizen) {
									// build up list of citizens to remove
									citizensToRemove.push_back(iterCurrentCitizen->first.second);

									// build list of citizens that gets removed to send mail to mayor
									if (!currentCityMayorName.empty()) {
										if (removeCurrentCitizenDeleted) {
											if (!citizensToRemoveDeletedNames.empty())
												citizensToRemoveDeletedNames += "\r\n";

											citizensToRemoveDeletedNames += citizenName;

											// send multiple mail if there are too many names
											if (citizensToRemoveDeletedNames.size() > 2000) {
												ProsePackage pp;
												pp.stringId = StringId("city/city", "city_fixup_remove_deleted_citizens_body");
												pp.target.str = Unicode::narrowToWide(citizensToRemoveDeletedNames);

												Unicode::String oob;
												OutOfBandPackager::pack(pp, -1, oob);

												Chat::sendPersistentMessage("City Hall", currentCityMayorName, mailSubjectRemove, Unicode::emptyString, oob);

												citizensToRemoveDeletedNames.clear();
											}
										}
										else if (removeCurrentCitizenInactive) {
											if (!citizensToRemoveInactiveNames.empty())
												citizensToRemoveInactiveNames += "\r\n";

											citizensToRemoveInactiveNames += citizenName;
											citizensToRemoveInactiveNames += " (offline ";
											citizensToRemoveInactiveNames += CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(
													timeNow - citizenLastLoginTime));
											citizensToRemoveInactiveNames += ")";

											// send multiple mail if there are too many names
											if (citizensToRemoveInactiveNames.size() > 2000) {
												ProsePackage pp;
												pp.stringId = StringId("city/city", "city_fixup_remove_inactive_citizens_body");
												pp.target.str = Unicode::narrowToWide(citizensToRemoveInactiveNames);

												Unicode::String oob;
												OutOfBandPackager::pack(pp, -1, oob);

												Chat::sendPersistentMessage("City Hall", currentCityMayorName, mailSubjectRemove, Unicode::emptyString, oob);

												citizensToRemoveInactiveNames.clear();
											}
										}
										else {
											if (!citizensToRemoveNames.empty())
												citizensToRemoveNames += "\r\n";

											citizensToRemoveNames += citizenName;

											// send multiple mail if there are too many names
											if (citizensToRemoveNames.size() > 2000) {
												ProsePackage pp;
												pp.stringId = StringId("city/city", "city_fixup_remove_citizens_body");
												pp.target.str = Unicode::narrowToWide(citizensToRemoveNames);

												Unicode::String oob;
												OutOfBandPackager::pack(pp, -1, oob);

												Chat::sendPersistentMessage("City Hall", currentCityMayorName, mailSubjectRemove, Unicode::emptyString, oob);

												citizensToRemoveNames.clear();
											}
										}
									}
								}
							}

							if (!currentCityMayorName.empty()) {
								if (!citizensToRemoveDeletedNames.empty()) {
									ProsePackage pp;
									pp.stringId = StringId("city/city", "city_fixup_remove_deleted_citizens_body");
									pp.target.str = Unicode::narrowToWide(citizensToRemoveDeletedNames);

									Unicode::String oob;
									OutOfBandPackager::pack(pp, -1, oob);

									Chat::sendPersistentMessage("City Hall", currentCityMayorName, mailSubjectRemove, Unicode::emptyString, oob);
								}

								if (!citizensToRemoveInactiveNames.empty()) {
									ProsePackage pp;
									pp.stringId = StringId("city/city", "city_fixup_remove_inactive_citizens_body");
									pp.target.str = Unicode::narrowToWide(citizensToRemoveInactiveNames);

									Unicode::String oob;
									OutOfBandPackager::pack(pp, -1, oob);

									Chat::sendPersistentMessage("City Hall", currentCityMayorName, mailSubjectRemove, Unicode::emptyString, oob);
								}

								if (!citizensToRemoveNames.empty()) {
									ProsePackage pp;
									pp.stringId = StringId("city/city", "city_fixup_remove_citizens_body");
									pp.target.str = Unicode::narrowToWide(citizensToRemoveNames);

									Unicode::String oob;
									OutOfBandPackager::pack(pp, -1, oob);

									Chat::sendPersistentMessage("City Hall", currentCityMayorName, mailSubjectRemove, Unicode::emptyString, oob);
								}
							}

							// remove citizens
							for (std::vector<NetworkId>::const_iterator iterCitizenToRemove = citizensToRemove.begin();
									iterCitizenToRemove != citizensToRemove.end(); ++iterCitizenToRemove) {
								CityInterface::removeCitizen(iterCityId->first, *iterCitizenToRemove, false);
							}

							// add mayor as a citizen if necessary
							if (needToAddMayorAsCitizen)
								IGNORE_RETURN(iterCityId->second.insert(currentCityMayor));

							// add citizens
							static const Unicode::String mailSubjectAdd = Unicode::narrowToWide("@" +
							                                                                    StringId("city/city", "city_fixup_add_citizens_subject")
									                                                                    .getCanonicalRepresentation());
							citizensToAddNames.clear();
							for (std::set<NetworkId>::const_iterator iterCitizenToAdd = iterCityId->second.begin();
									iterCitizenToAdd != iterCityId->second.end(); ++iterCitizenToAdd) {
								// make sure it's a valid player
								if (!NameManager::getInstance().isPlayer(*iterCitizenToAdd))
									continue;

								// citizens who have not logged in within the past 90 days should not be
								// added as a citizen EXCEPT for the mayor who should always be a citizen
								if ((*iterCitizenToAdd != currentCityMayor) && citizenInactivePackupActive) {
									citizenLastLoginTime = NameManager::getInstance()
											.getPlayerLastLoginTime(*iterCitizenToAdd);
									if ((citizenLastLoginTime <= 0) || ((citizenLastLoginTime +
									                                     ConfigServerGame::getCityCitizenshipInactivePackupInactiveTimeSeconds()) <=
									                                    timeNow))
										continue;
								}

								// don't add someone who is already a mayor of another city, unless
								// we are adding this city's mayor as a citizen of this city
								if ((*iterCitizenToAdd != currentCityMayor) &&
								    (allCityMayors.count(*iterCitizenToAdd) >= 1))
									continue;

								// add the new citizen
								citizenName = NameManager::getInstance().getPlayerFullName(*iterCitizenToAdd);
								if (citizenName.empty())
									continue;

								// build list of citizens that gets added to send mail to mayor
								if (!currentCityMayorName.empty()) {
									if (!citizensToAddNames.empty())
										citizensToAddNames += "\r\n";

									citizensToAddNames += citizenName;

									// send multiple mail if there are too many names
									if (citizensToAddNames.size() > 2000) {
										ProsePackage pp;
										pp.stringId = StringId("city/city", "city_fixup_add_citizens_body");
										pp.target.str = Unicode::narrowToWide(citizensToAddNames);

										Unicode::String oob;
										OutOfBandPackager::pack(pp, -1, oob);

										Chat::sendPersistentMessage("City Hall", currentCityMayorName, mailSubjectAdd, Unicode::emptyString, oob);

										citizensToAddNames.clear();
									}
								}

								if (*iterCitizenToAdd == currentCityMayor)
									LOG("CustomerService", ("CityFixup: added %s (%s) (is mayor) as citizen of city %d (%s).", iterCitizenToAdd
											->getValueString().c_str(), citizenName.c_str(), iterCityId->first, cityName
											.c_str()));
								else
									LOG("CustomerService", ("CityFixup: added %s (%s) (has declared residence) as citizen of city %d (%s).", iterCitizenToAdd
											->getValueString().c_str(), citizenName.c_str(), iterCityId->first, cityName
											.c_str()));

								CityInterface::setCitizenInfo(iterCityId
										->first, *iterCitizenToAdd, citizenName, NetworkId::cms_invalid, CitizenPermissions::Citizen);
							}

							if (!citizensToAddNames.empty() && !currentCityMayorName.empty()) {
								ProsePackage pp;
								pp.stringId = StringId("city/city", "city_fixup_add_citizens_body");
								pp.target.str = Unicode::narrowToWide(citizensToAddNames);

								Unicode::String oob;
								OutOfBandPackager::pack(pp, -1, oob);

								Chat::sendPersistentMessage("City Hall", currentCityMayorName, mailSubjectAdd, Unicode::emptyString, oob);
							}
						}
					}

					s_clusterStartupResidenceStructureListByCity.clear();
					s_clusterStartupResidenceStructureListByStructure.clear();
					s_clusterStartupResidenceStructureListResponse.clear();

					CityInterface::checkForDualCitizenship();
				}
			}
			break;
		}
		case constcrc("LocateStructureMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			LocateStructureMessage const msg(ri);
			GameServer::getInstance().sendToCentralServer(msg);
			break;
		}
		case constcrc("LoadStructureMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			LoadStructureMessage const msg(ri);

			ServerObject *const house = ServerWorld::findObjectByNetworkId(msg.getStructureId());
			if (house) {
				//result += Unicode::narrowToWide("Error. Object already loaded.");
				std::string replyMessage = "Structure " + msg.getStructureId().getValueString() + " already loaded";
				Chat::sendSystemMessage(msg.getWhoRequested(), Unicode::narrowToWide(replyMessage), Unicode::String());
			}
			else {
				LoadContentsMessage const lcm(msg.getStructureId());
				GameServer::getInstance().sendToDatabaseServer(lcm);
				std::string const replyMessage =
						"Structure " + msg.getStructureId().getValueString() + " is being loaded";
				Chat::sendSystemMessage(msg.getWhoRequested(), Unicode::narrowToWide(replyMessage), Unicode::String());
			}
			break;
		}
		case constcrc("LocateObjectResponseMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			LocateObjectResponseMessage const msg(ri);

			ServerObject *const responseObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg
					.getResponseId()));
			if (responseObject && responseObject->isAuthoritative()) {
				int const cityId = CityInterface::getCityAtLocation(msg.getScene(), static_cast<int>(msg.getPosition_w()
				                                                                                        .x), static_cast<int>(msg
						.getPosition_w().z), 0);
				std::string const cityName = CityInterface::getCityInfo(cityId).getCityName();

				NetworkId const residenceOf = msg.getResidenceOf();
				std::string residenceOfName;
				if (residenceOf.isValid())
					residenceOfName = NameManager::getInstance().getPlayerFullName(residenceOf);

				ScriptParams params;
				params.addParam(msg.getTargetId(), "target");
				params.addParam(msg.getPosition_w(), "location");
				params.addParam(msg.getScene().c_str(), "scene");
				params.addParam(msg.getSharedTemplateName().c_str(), "sharedTemplateName");
				params.addParam(static_cast<int>(msg.getTargetPid()), "pid");
				params.addParam(msg.getContainers(), "containers");
				params.addParam(msg.getIsAuthoritative(), "isAuthoritative");
				params.addParam(cityId, "cityId");
				params.addParam(cityName.c_str(), "cityName");
				params.addParam(residenceOf, "residenceOfId");
				params.addParam(residenceOfName.c_str(), "residenceOfName");

				ScriptDictionaryPtr dictionary;
				responseObject->getScriptObject()->makeScriptDictionary(params, dictionary);
				if (dictionary.get() != nullptr) {
					dictionary->serialize();
					MessageToQueue::getInstance()
							.sendMessageToJava(responseObject->getNetworkId(), "foundObject", dictionary
									->getSerializedData(), 0, false);
				}
			}
			break;
		}
		case constcrc("LocatePlayerResponseMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			LocatePlayerResponseMessage const msg(ri);

			ServerObject *const responseObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg
					.getResponseId()));
			if (responseObject && responseObject->isAuthoritative() && responseObject->getClient()) {
				std::string playerInfo = FormattedString<512>()
						.sprintf("%s (%s) is on server %lu. %s (%.2f, %.2f, %.2f)",
								msg.getTargetId().getValueString().c_str(),
								NameManager::getInstance().getPlayerFullName(msg.getTargetId()).c_str(),
								msg.getTargetPid(), msg.getScene().c_str(),
								msg.getPosition_w().x, msg.getPosition_w().y, msg.getPosition_w().z);

				ConsoleMgr::broadcastString(playerInfo, responseObject->getClient());
			}
			break;
		}
		case constcrc("PlayerSanityCheck") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<NetworkId, uint32>> const msg(ri);
			PlayerSanityChecker::handlePlayerSanityCheck(msg.getValue().first, msg.getValue().second);
			break;
		}
		case constcrc("PlayerSanityCheckSuccess") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <NetworkId> const msg(ri);
			PlayerSanityChecker::handlePlayerSanityCheckSuccess(msg.getValue());
			break;
		}
		case constcrc("PlayerSanityCheckProxy") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<NetworkId, uint32>> const msg(ri);
			PlayerSanityChecker::handlePlayerSanityCheckProxy(msg.getValue().first, msg.getValue().second);
			break;
		}
		case constcrc("PlayerSanityCheckProxyFail") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<NetworkId, uint32>> const msg(ri);
			PlayerSanityChecker::handlePlayerSanityCheckProxyFail(msg.getValue().first, msg.getValue().second);
			break;
		}
		case constcrc("EnablePlayerSanityCheckerMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage<bool> const msg(ri);
			PlayerSanityChecker::enable(msg.getValue());
			break;
		}
		case constcrc("StartSaveReplyMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<NetworkId, bool>> const msg(ri);

			ServerObject const *
			const requestedBy = safe_cast < ServerObject const * >
			                                                   (NetworkIdManager::getObjectById(msg.getValue().first));

			if (requestedBy) {
				if (msg.getValue().second)
					Chat::sendSystemMessage(*requestedBy, Unicode::narrowToWide("Save has been started"), Unicode::String());
				else
					Chat::sendSystemMessage(*requestedBy, Unicode::narrowToWide("Save could not be started because one was already in progress"), Unicode::String());
			}
			break;
		}
		case constcrc("DatabaseConsoleReplyMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<std::string, std::string>> const msg(ri);

			Chat::sendSystemMessage(msg.getValue().first, Unicode::narrowToWide(msg.getValue()
			                                                                       .second), Unicode::String());
			break;
		}
		case constcrc("FindAuthObject") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<NetworkId, unsigned int>> const msg(ri);
			ServerObject const *
			const so = safe_cast < ServerObject const * > (NetworkIdManager::getObjectById(msg.getValue().first));
			bool const found = so && so->isAuthoritative();

			GenericValueTypeMessage < std::pair < std::pair < NetworkId, unsigned
			int > , bool > >
			const authResponse(
			"FindAuthObjectResponse",
					std::make_pair(msg.getValue(), found));
			sendToPlanetServer(authResponse);
			break;
		}
		case constcrc("CharacterNamesMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			CharacterNamesMessage const msg(ri);
			NameManager::getInstance()
					.addPlayers(msg.getIds(), msg.getStationIds(), msg.getNames(), msg.getFullNames(), msg
							.getCreateTimes(), msg.getLoginTimes());
			break;
		}
		case constcrc("LocationResponse") : {
			Archive::ReadIterator readIterator = static_cast<GameNetworkMessage const &> (message).getByteStream()
			                                                                                      .begin();
			LocationResponse const locationResponse(readIterator);

			//-- get the script object to invole the trigger on
			Object *const object = NetworkIdManager::getObjectById(locationResponse.getNetworkId());
			if (!object) {
				DEBUG_WARNING(true, ("LocationResponse: could not resolve object %s which asked for a location", locationResponse
						.getNetworkId().getValueString().c_str()));
				return;
			}

			ServerObject *const scriptObject = object->asServerObject();
			if (!scriptObject) {
				DEBUG_WARNING(true, ("LocationResponse: object id=%s template=%s is not a server object", object
						->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));
				return;
			}

			//-- if the location is valid, create a reservation object
			Vector position_w(locationResponse.getX(), 0.f, locationResponse.getZ());
			ServerObject *serverObject = 0;
			if (locationResponse.getValid()) {
				//-- construct the name
				char objectTemplateName[512];
				snprintf(objectTemplateName, 512, "object/tangible/location/location_%i.iff", static_cast<int> (locationResponse
						.getRadius()));

				//-- get the height of the terrain
				TerrainObject const *const terrainObject = TerrainObject::getConstInstance();
				if (terrainObject)
					terrainObject->getHeightForceChunkCreation(position_w, position_w.y);

				//-- create the object
				Transform tr;
				tr.setPosition_p(position_w);
				serverObject = ServerWorld::createNewObject(objectTemplateName, tr, 0, false);
				DEBUG_REPORT_LOG(!serverObject, ("LocationResponse: failed to create object %s\n", objectTemplateName));
				if (serverObject)
					serverObject->addToWorld();
			}

			//-- inform script of the result
			ScriptParams scriptParameters;
			scriptParameters.addParam(locationResponse.getLocationId().c_str());
			scriptParameters.addParam(serverObject ? serverObject->getNetworkId() : NetworkId::cms_invalid);
			scriptParameters.addParam(position_w);
			scriptParameters.addParam(locationResponse.getRadius());

			if (scriptObject->getScriptObject()->trigAllScripts(Scripting::TRIG_LOCATION_RECEIVED, scriptParameters) !=
			    SCRIPT_CONTINUE)
				DEBUG_REPORT_LOG(true, ("LocationResponse: TRIG_LOCATION_RECEIVED: did not return SCRIPT_CONTINUE\n"));
			break;
		}
		case constcrc("ReleaseCharacterNameByIdMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <NetworkId> const msg(ri);

			NameManager::getInstance().releasePlayerName(msg.getValue());
			break;
		}
		case constcrc("AddAttribModName") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::string> const msg(ri);
			AttribModNameManager::getInstance().addAttribModNameFromRemote(msg.getValue().c_str());
			break;
		}
		case constcrc("AddAttribModNamesList") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::string> const msg(ri);
			DEBUG_REPORT_LOG(true, ("Received attrib mod names list <%s> from remote server\n", msg.getValue()
			                                                                                       .c_str()));
			AttribModNameManager::getInstance().addAttribModNamesListFromRemote(msg.getValue().c_str());
			break;
		}
		case constcrc("AiCreatureStateMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <AiCreatureStateMessage> const msg(ri);

			NetworkId const &networkId = msg.getValue().m_networkId;
			AiMovementMessage const &movement = msg.getValue().m_movement;

			CreatureObject *const creatureObject = CreatureObject::getCreatureObject(networkId);

			if (creatureObject != nullptr) {
				Controller *const controller = creatureObject->getController();

				if (controller != nullptr) {
					DEBUG_LOG("debug_ai", ("GameServer::receiveMessage() Received AiCreatureStateMessage for networkId(%s) movementType(%s)", networkId
							.getValueString().c_str(), AiMovementBase::getMovementString(movement.getMovementType())));

					int const flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE |
					                  GameControllerMessageFlags::DEST_SERVER;

					// Movement

					controller->appendMessage(CM_aiSetMovement, 0, new AiMovementMessage(movement), flags);
				}
				else {
					WARNING(true, ("GameServer::receiveMessage() Received AiCreatureStateMessage for networkId(%s) movementType(%s) but the object does not have a Controller", networkId
							.getValueString().c_str(), AiMovementBase::getMovementString(movement.getMovementType())));
				}
			}
			else {
				WARNING(true, ("GameServer::receiveMessage() Received AiCreatureStateMessage for networkId(%s) movementType(%s) but could not resolve to a CreatureObject", networkId
						.getValueString().c_str(), AiMovementBase::getMovementString(movement.getMovementType())));
			}
			break;
		}
		case constcrc("UnloadPersistedCharacter") : {
			Archive::ReadIterator ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <NetworkId> const upc(ri);

			ServerObject *const object = ServerWorld::findObjectByNetworkId(upc.getValue());
			if (object) {
				WARNING(true, ("Unloaded character object (%s) which has been persisted but still exists on this game server", object
						->getDebugInformation().c_str()));
				object->unload();
			}
			break;
		}
		case constcrc("AboutToLoadCharacterFromDB") : {
			Archive::ReadIterator ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<NetworkId, uint32>> const atlcfdb(ri);

			ServerObject *const object = ServerWorld::findObjectByNetworkId(atlcfdb.getValue().first);
			if (object) {
				WARNING(true, ("Unloaded character object (%s) because the character object is about to be loaded from the DB onto game server (%lu), I am game server (%lu)", object
						->getDebugInformation().c_str(), atlcfdb.getValue().second, getProcessId()));
				object->unload();
			}
			break;
		}
		case constcrc("ClearTheaterMessage") : {
			Archive::ReadIterator ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::string> const ctm(ri);

			ServerUniverse::getInstance().remoteClearTheater(ctm.getValue());
			break;
		}
		case constcrc("SetTheaterMessage") : {
			Archive::ReadIterator ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<std::string, NetworkId>> const stm(ri);

			ServerUniverse::getInstance().remoteSetTheater(stm.getValue().first, stm.getValue().second);
			break;
		}
		case constcrc("PageChangeAuthority") : {
			ServerUIManager::receiveMessage(message);
			break;
		}
		case constcrc("PlayedTimeAccumMessage") : {
			Archive::ReadIterator ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			PlayedTimeAccumMessage const ptam(ri);
			Object *obj = NetworkIdManager::getObjectById(ptam.getNetworkId());
			if (obj != nullptr && obj->asServerObject() != nullptr &&
			    obj->asServerObject()->asCreatureObject() != nullptr) {
				CreatureObject *target = obj->asServerObject()->asCreatureObject();
				PlayerObject *targetPlayer = target->asPlayerObject();
				if (targetPlayer) {
					targetPlayer->setPlayedTimeAccumOnly(ptam.getPlayedTimeAccum());
				}
				else {
					target->setPseudoPlayedTime(ptam.getPlayedTimeAccum());
				}
			}

			break;

		}
		case constcrc("ManualDepleteResourceMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <NetworkId> const msg(ri);
			ServerUniverse::getInstance().manualDepleteResource(msg.getValue());
			break;
		}
		case constcrc("ClaimRewardsReplyMessage") : {
			ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			ClaimRewardsReplyMessage msg(ri);
			VeteranRewardManager::handleClaimRewardsReply(msg.getStationId(), msg.getPlayer(), msg.getRewardEvent(), msg
					.getRewardItem(), msg.getAccountFeatureId(), msg.getConsumeAccountFeatureId(), msg
					.getPreviousAccountFeatureIdCount(), msg.getCurrentAccountFeatureIdCount(), msg.getResult(), true);
			break;
		}
		case constcrc("SetOverrideAccountAgeMessage") : {
			ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<int> msg(ri);

			VeteranRewardManager::setOverrideAccountAge(msg.getValue());
			break;
		}
		case constcrc("GetMoneyFromOfflineObjectMessage") : {
			ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GetMoneyFromOfflineObjectMessage msg(ri);

			std::string const &callback = msg.getSuccess() ? msg.getSuccessCallback() : msg.getFailCallback();
			MessageToQueue::getInstance()
					.sendMessageToJava(msg.getReplyTo(), callback, msg.getPackedDictionary(), 0, false);
			break;
		}
		case constcrc("StructuresForPurgeMessage") : {
			ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			StructuresForPurgeMessage msg(ri);
			PurgeManager::handleStructuresAndVendorsForPurge(msg.getStationId(), msg.getStructures(), msg
					.getVendors(), msg.getWarnOnly());
			break;
		}
		case constcrc("RequestLoadAckMessage") : {
			ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<int> msg(ri);

			GenericValueTypeMessage<int> reply("LoadAckMessage", msg.getValue());
			sendToDatabaseServer(reply);
			break;
		}
		case constcrc("FactionalSystemMessage") : {
			ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			FactionalSystemMessage msg(ri);
			handleFactionalSystemMessage(msg);
			break;
		}
		case constcrc("MessageToPlayersOnPlanet") : {
			ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < std::pair < std::pair < std::string, std::vector < int8 > >,
					std::pair < float, bool > >, std::pair < Vector, float > > >
			const msg(ri);
			handleMessageToPlayersOnPlanet(msg.getValue().first.first.first, msg.getValue().first.first.second, msg
					.getValue().first.second.first, msg.getValue().second.first, msg.getValue().second.second, msg
					.getValue().first.second.second);
			break;
		}
		case constcrc("PopStatReq") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <uint8> const populationStatisticsRequest(ri);
			UNREF(populationStatisticsRequest);

			const GenericValueTypeMessage <std::map<std::string, int>> populationStatisticsResponse("PopStatRsp", LfgCharacterData::calculateStatistics(ServerUniverse::getConnectedCharacterLfgData()));
			sendToCentralServer(populationStatisticsResponse);
			break;
		}
		case constcrc("GcwScoreStatReq") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <uint8> const gcwScoreStatisticsRequest(ri);
			UNREF(gcwScoreStatisticsRequest);

			static const std::map <std::string, std::pair<int64, int64>> emptyGcwScore;
			PlanetObject const *const tatooine = ServerUniverse::getInstance().getTatooinePlanet();

			static const std::map<std::string, int> emptyGcwScorePercentile;
			GuildObject const *const guildObject = ServerUniverse::getInstance().getMasterGuildObject();

			const GenericValueTypeMessage <std::pair<std::map < std::string, int>, std::pair<
					std::map < std::string, std::pair < int64, int64>>, std::map < std::string, std::pair < int64,
					int64 > > > > >
					gcwScoreStatisticsResponse("GcwScoreStatRsp", std::make_pair((guildObject ? guildObject
							->getGcwImperialScorePercentile() : emptyGcwScorePercentile), std::make_pair((tatooine
					                                                                                      ? tatooine
									                                                                              ->getGcwImperialScore()
					                                                                                      : emptyGcwScore), (tatooine
					                                                                                                         ? tatooine
							                                                                                                         ->getGcwRebelScore()
					                                                                                                         : emptyGcwScore))));
			sendToCentralServer(gcwScoreStatisticsResponse);

			break;
		}
		case constcrc("GcwScoreStatRaw") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < std::string, std::pair < std::map < std::string, std::pair < int64,
					int64 > >, std::map < std::string, std::pair < int64, int64 > > > > >
			const msg(ri);
			if (ConfigServerGame::getReceiveGcwScoreFromOtherGalaxies() &&
			    ServerUniverse::getInstance().isAuthoritative() &&
			    _stricmp(getClusterName().c_str(), msg.getValue().first.c_str())) {
				GuildObject *const go = ServerUniverse::getInstance().getMasterGuildObject();
				if (go && go->isAuthoritative()) {
					go->setGcwRawScoreFromOtherGalaxy(msg.getValue().first, msg.getValue().second.first, msg.getValue()
					                                                                                        .second
					                                                                                        .second);
				}
			}

			break;
		}
		case constcrc("GcwScoreStatPct") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < std::string, std::pair < std::map < std::string, int >, std::map <
			                                                                                              std::string,
					int > > > >
			const msg(ri);
			if (ConfigServerGame::getReceiveGcwScoreFromOtherGalaxies() &&
			    ServerUniverse::getInstance().isAuthoritative() &&
			    _stricmp(getClusterName().c_str(), msg.getValue().first.c_str())) {
				GuildObject *const go = ServerUniverse::getInstance().getMasterGuildObject();
				if (go && go->isAuthoritative()) {
					go->setGcwImperialScorePercentileFromOtherGalaxy(msg.getValue().first, msg.getValue().second
					                                                                          .first, msg.getValue()
					                                                                                     .second
					                                                                                     .second);
				}
			}

			break;
		}
		case constcrc("LLTStatReq") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <uint8> const lastLoginTimeStatisticsRequest(ri);
			UNREF(lastLoginTimeStatisticsRequest);

			static std::map<int, std::pair<std::string, int> > lastLoginTimeStatistics;
			if (lastLoginTimeStatistics.empty()) {
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         1)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day001"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         2)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day002"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         3)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day003"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         4)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day004"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         5)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day005"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         6)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day006"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         7)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day007"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         14)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day014"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         21)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day021"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         30)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day030"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         60)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day060"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         90)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day090"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         180)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day180"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         270)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Day270"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         365)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Year1"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         730)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Year2"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 *
				                         1095)] = std::make_pair(std::string("totalCharacters.loginWithinPast_Year3"), 0);
				lastLoginTimeStatistics[(60 * 60 * 24 * 9999)] = std::make_pair(std::string("totalCharacters"), 0);
			}

			NameManager::getInstance().getPlayerWithLastLoginTimeAfterDistribution(lastLoginTimeStatistics);

			std::map < int, std::pair < std::string, int > > ::iterator
			iter = lastLoginTimeStatistics.begin();
			std::map < int, std::pair < std::string, int > > ::iterator
			previousIter = lastLoginTimeStatistics.begin();
			for (; iter != lastLoginTimeStatistics.end(); ++iter) {
				if (iter != previousIter)
					iter->second.second += previousIter->second.second;

				previousIter = iter;
			}

			static std::map<int, std::pair<std::string, int> > createTimeStatistics;
			if (createTimeStatistics.empty()) {
				createTimeStatistics[(60 * 60 * 24 *
				                      1)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day001"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      2)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day002"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      3)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day003"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      4)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day004"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      5)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day005"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      6)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day006"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      7)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day007"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      14)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day014"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      21)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day021"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      30)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day030"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      60)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day060"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      90)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day090"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      180)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day180"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      270)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Day270"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      365)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Year1"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      730)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Year2"), 0);
				createTimeStatistics[(60 * 60 * 24 *
				                      1095)] = std::make_pair(std::string("totalCharacters.createdWithinPast_Year3"), 0);
				createTimeStatistics[(60 * 60 * 24 * 9999)] = std::make_pair(std::string("totalCharacters"), 0);
			}

			NameManager::getInstance().getPlayerWithCreateTimeAfterDistribution(createTimeStatistics);

			iter = createTimeStatistics.begin();
			previousIter = createTimeStatistics.begin();
			for (; iter != createTimeStatistics.end(); ++iter) {
				if (iter != previousIter)
					iter->second.second += previousIter->second.second;

				previousIter = iter;
			}

			const GenericValueTypeMessage <std::pair<std::map < int, std::pair < std::string, int>>, std::map < int,
					std::pair < std::string, int > > > >
			                                 lastLoginTimeStatisticsResponse("LLTStatRsp", std::make_pair(lastLoginTimeStatistics, createTimeStatistics));
			sendToCentralServer(lastLoginTimeStatisticsResponse);

			break;
		}
		case constcrc("LfgStatReq") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <uint8> const characterMatchStatisticsRequest(ri);
			UNREF(characterMatchStatisticsRequest);

			unsigned long numberOfCharacterMatchRequests, numberOfCharacterMatchResults, timeSpentOnCharacterMatchRequestsMs;
			CharacterMatchManager::getMatchStatistics(numberOfCharacterMatchRequests, numberOfCharacterMatchResults, timeSpentOnCharacterMatchRequestsMs);

			if (numberOfCharacterMatchRequests || numberOfCharacterMatchResults ||
			    timeSpentOnCharacterMatchRequestsMs) {
				CharacterMatchManager::clearMatchStatistics();

				const GenericValueTypeMessage <std::pair<unsigned long, std::pair<unsigned long, unsigned long> >> characterMatchStatisticsResponse("LfgStatRsp", std::make_pair(numberOfCharacterMatchRequests, std::make_pair(numberOfCharacterMatchResults, timeSpentOnCharacterMatchRequestsMs)));
				sendToCentralServer(characterMatchStatisticsResponse);
			}

			break;
		}
		case constcrc("ClusterId") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <uint32> const msg(ri);

			if (m_clusterId == 0) {
				FATAL(((msg.getValue() < 1) ||
				       (msg.getValue() > 255)), ("Cluster Id (%lu) must be between 1 and 255 inclusive", msg
						.getValue()));

				m_clusterId = static_cast<uint8>(msg.getValue());
			}
			break;
		}
		case constcrc("OccupyUnlockedSlotRsp") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < std::pair < int, NetworkId >, uint32 > >
			const occupyUnlockedSlotRsp(ri);

			char buffer[32];
			snprintf(buffer, sizeof(buffer) - 1, "%d", occupyUnlockedSlotRsp.getValue().first.first);
			buffer[sizeof(buffer) - 1] = '\0';

			MessageToQueue::getInstance().sendMessageToC(occupyUnlockedSlotRsp.getValue().first.second,
					"C++OccupyUnlockedSlotRsp",
					buffer,
					0,
					false);

			break;
		}
		case constcrc("VacateUnlockedSlotRsp") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < std::pair < int, NetworkId >, std::pair < uint32, uint32 > > >
			const vacateUnlockedSlotRsp(ri);

			char buffer[32];
			snprintf(buffer, sizeof(buffer) - 1, "%d", vacateUnlockedSlotRsp.getValue().first.first);
			buffer[sizeof(buffer) - 1] = '\0';

			MessageToQueue::getInstance().sendMessageToC(vacateUnlockedSlotRsp.getValue().first.second,
					"C++VacateUnlockedSlotRsp",
					buffer,
					0,
					false);
			break;
		}
		case constcrc("SwapUnlockedSlotRsp") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage < std::pair < std::pair < int, NetworkId >, std::pair < uint32, std::pair <
			                                                                                        NetworkId,
					std::string > > > >
			const swapUnlockedSlotRsp(ri);

			Archive::ByteStream bs;
			swapUnlockedSlotRsp.pack(bs);

			MessageToQueue::getInstance().sendMessageToC(swapUnlockedSlotRsp.getValue().first.second,
					"C++SwapUnlockedSlotRsp",
					std::string(reinterpret_cast<const char *>(bs.getBuffer()), static_cast<size_t>(bs.getSize())),
					0,
					false);
			break;
		}
		case constcrc("AdjustAccountFeatureIdResponse") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			AdjustAccountFeatureIdResponse const msg(ri);

			handleAdjustAccountFeatureIdResponse(msg);
			break;
		}
		case constcrc("AccountFeatureIdResponse") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			AccountFeatureIdResponse const msg(ri);

			handleAccountFeatureIdResponse(msg);
			break;
		}
		case constcrc("FeatureIdTransactionResponse") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			FeatureIdTransactionResponse const msg(ri);

			Archive::ByteStream bs;
			msg.pack(bs);

			MessageToQueue::getInstance().sendMessageToC(msg.getPlayer(),
					"C++FeatureIdTransactionResponse",
					std::string(reinterpret_cast<const char *>(bs.getBuffer()), static_cast<size_t>(bs.getSize())),
					0,
					false);
			break;
		}
		case constcrc("TransferReplyMoveValidation") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			TransferReplyMoveValidation const msg(ri);

			Archive::ByteStream bs;
			msg.pack(bs);

			MessageToQueue::getInstance().sendMessageToC(msg.getSourceCharacterId(),
					"C++TransferReplyMoveValidation",
					std::string(reinterpret_cast<const char *>(bs.getBuffer()), static_cast<size_t>(bs.getSize())),
					0,
					false);
			break;
		}
		case constcrc("TransferReplyNameValidation") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			GenericValueTypeMessage <std::pair<std::string, TransferCharacterData>> const msg(ri);

			Archive::ByteStream bs;
			msg.pack(bs);

			MessageToQueue::getInstance().sendMessageToC(msg.getValue().second.getCharacterId(),
					"C++TransferReplyNameValidation",
					std::string(reinterpret_cast<const char *>(bs.getBuffer()), static_cast<size_t>(bs.getSize())),
					0,
					false);
			break;
		}
		case constcrc("CreateDynamicSpawnRegionCircleMessage") : {
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			CreateDynamicSpawnRegionCircleMessage const msg(ri);

			RegionMaster::createNewDynamicRegionWithSpawn(msg.getCenterX(), msg.getCenterY(), msg.getCenterZ(), msg
							.getRadius(), msg.getName(), msg.getPlanet(),
					msg.getPvp(), msg.getBuildable(), msg.getMunicipal(), msg.getGeography(), msg
							.getMinDifficulty(), msg.getMaxDifficulty(),
					msg.getSpawnable(), msg.getMission(), msg.getVisible(), msg.getNotify(), msg.getSpawntable(), msg
							.getDuration());
			break;
		}
		default :
		{
			if(ClusterWideDataClient::handleMessage(source, message)) {
				// nothing else to do with the message since it was
				// handled by the Cluster wide data client
			} else if (message.isType(SlowDownEffectMessage::MessageType)) {
				ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
				SlowDownEffectMessage msg(ri);

				// get the attacker
				Object * obj = NetworkIdManager::getObjectById(msg.getSource());
				if (obj != nullptr && obj->asServerObject() != nullptr && obj->asServerObject()->asCreatureObject() != nullptr)
				{
					CreatureObject * attacker = obj->asServerObject()->asCreatureObject();

					// get the target
					obj = NetworkIdManager::getObjectById(msg.getTarget());
					if (obj != nullptr && obj->asServerObject() != nullptr && obj->asServerObject()->asTangibleObject() != nullptr)
					{
						TangibleObject * defender = obj->asServerObject()->asTangibleObject();
						if (attacker->isAuthoritative())
							attacker->addSlowDownEffect(*defender, msg.getConeLength(), msg.getConeAngle(), msg.getSlopeAngle(), msg.getExpireTime());
						else
							attacker->addSlowDownEffectProxy(*defender, msg.getConeLength(), msg.getConeAngle(), msg.getSlopeAngle(), msg.getExpireTime());
					}
				}
			}

			break;
		}
	}
}

// ----------------------------------------------------------------------

bool GameServer::isPlanetEnabledForCluster(std::string const &sceneName) const
{
	// look up [CentralServer] startPlanet=sceneName[:whatever]

	int index = 0;
	char const * result = 0;
	do
	{
		result = ConfigFile::getKeyString("CentralServer", "startPlanet", index++, 0);
		// result must either match sceneName completely, or match all of sceneName and be followed by a ':'
		if (   result
		    && (   !strcmp(result, sceneName.c_str())
		        || (   !strncmp(result, sceneName.c_str(), sceneName.size())
		            && result[sceneName.size()] == ':')))
			return true;
	} while (result);

	return false;
}

// ----------------------------------------------------------------------

void GameServerNamespace::broadCastHyperspaceOnWarp(ServerObject const * const owner)
{
	// warpingClient can be nullptr if the owner is AI
	Client const * const warpingClient = owner->getClient();

	typedef std::map<ConnectionServerConnection *, std::vector<NetworkId> > DistributionList;
	DistributionList distributionList;

	// Build distribution list of clients observing ship
	{
		typedef std::set<Client *> ObserverList;
		ObserverList const & observers = owner->getObservers();

		ObserverList::const_iterator ii = observers.begin();
		ObserverList::const_iterator iiEnd = observers.end();
		for (; ii != iiEnd; ++ii)
		{
			Client * const client = *ii;
			if (client != warpingClient)
			{
				distributionList[client->getConnection()].push_back(client->getCharacterObjectId());
			}
		}
	}

	// Send Message to distribution list
	{
		DistributionList::const_iterator ii = distributionList.begin();
		DistributionList::const_iterator iiEnd = distributionList.end();
		for (; ii != iiEnd; ++ii)
		{
			HyperspaceMessage const hyperspaceMessage(owner->getNetworkId());
			GameClientMessage const gameClientMessage(ii->second, true, hyperspaceMessage);
			ii->first->send(gameClientMessage, true);
		}
	}
}

/**
 * Warping an object to a different scene.
 */
bool GameServer::requestSceneWarp(const CachedNetworkId &objectId, const std::string &sceneName, const Vector &newPosition_w, const NetworkId &newContainer, const Vector &newPosition_p, const char * scriptCallback, bool forceLoadScreen)
{
	ServerObject * const object = dynamic_cast<ServerObject *>(objectId.getObject());
	if (!object)
		return false;

	if (!isPlanetEnabledForCluster(sceneName))
	{
		DEBUG_WARNING(true, ("GameServer::requestSceneWarp(1) for unknown scene %s", sceneName.c_str()));
		return false;
	}

	Vector destPosition_w(newPosition_w);

	// clamp destination if in space
	if (   !strncmp(sceneName.c_str(), "space_", 6)
	    && (   std::abs(newPosition_w.x) > 8000.f
	        || std::abs(newPosition_w.y) > 8000.f
	        || std::abs(newPosition_w.z) > 8000.f))
	{
		WARNING(true, ("Tried to warp object %s outside of space zone boundaries (%g,%g,%g)", object->getDebugInformation().c_str(), newPosition_w.x, newPosition_w.y, newPosition_w.z));
		destPosition_w.set(
			clamp(-8000.f, newPosition_w.x, 8000.f),
			clamp(-8000.f, newPosition_w.y, 8000.f),
			clamp(-8000.f, newPosition_w.z, 8000.f));
	}
	
	if (ServerWorld::getSceneId() == sceneName)
	{
		// if a pilot is doing a same scene warp, actually move the ship, and only allow in world coords
		CreatureObject * const objAsCreature = object->asCreatureObject();
		if (objAsCreature)
		{
			ShipObject * const ship = getAttachedShip(objAsCreature);
			if (ship)
			{
				broadCastHyperspaceOnWarp(ship);
				ship->teleportObject(destPosition_w, NetworkId::cms_invalid, "", destPosition_w, scriptCallback ? scriptCallback : "", forceLoadScreen);
			}
			else
				object->teleportObject(destPosition_w, newContainer, "", newPosition_p, scriptCallback ? scriptCallback : "", forceLoadScreen);
		}
		else
			object->teleportObject(destPosition_w, newContainer, "", newPosition_p, scriptCallback ? scriptCallback : "", forceLoadScreen);
	}
	else
	{
		// don't send multiple RequestSceneTransfer message
		if (!checkAndSetOutstandingRequestSceneTransfer(*object))
		{
			RequestSceneTransfer const sceneTransferMessage(objectId, sceneName, getProcessId(), newPosition_p, destPosition_w, newContainer, scriptCallback);
			sendToCentralServer(sceneTransferMessage);
		}
	}

	return true;
}

//-----------------------------------------------------------------------

/**
 * Warping an object to a different scene.
 */
bool GameServer::requestSceneWarp(const CachedNetworkId &objectId, const std::string &sceneName, const Vector &newPosition_w, const NetworkId &newBuilding, const std::string &newContainerName, const Vector &newPosition_p, const char * scriptCallback, bool forceLoadScreen)
{
	ServerObject * const object = dynamic_cast<ServerObject *>(objectId.getObject());
	if (!object)
		return false;

	if (!isPlanetEnabledForCluster(sceneName))
	{
		DEBUG_WARNING(true, ("GameServer::requestSceneWarp(1) for unknown scene %s", sceneName.c_str()));
		return false;
	}

	Vector destPosition_w(newPosition_w);

	// clamp destination if in space
	if (   !strncmp(sceneName.c_str(), "space_", 6)
	    && (   std::abs(newPosition_w.x) > 8000.f
	        || std::abs(newPosition_w.y) > 8000.f
	        || std::abs(newPosition_w.z) > 8000.f))
	{
		WARNING(true, ("Tried to warp object %s outside of space zone boundaries (%g,%g,%g)", object->getDebugInformation().c_str(), newPosition_w.x, newPosition_w.y, newPosition_w.z));
		destPosition_w.set(
			clamp(-8000.f, newPosition_w.x, 8000.f),
			clamp(-8000.f, newPosition_w.y, 8000.f),
			clamp(-8000.f, newPosition_w.z, 8000.f));
	}
	
	if (ServerWorld::getSceneId() == sceneName)
	{
		// if a pilot is doing a same scene warp, actually move the ship, and only allow in world coords
		CreatureObject * const objAsCreature = object->asCreatureObject();
		if (objAsCreature)
		{
			ShipObject * const ship = getAttachedShip(objAsCreature);
			if (ship)
			{
				broadCastHyperspaceOnWarp(ship);
				ship->teleportObject(destPosition_w, NetworkId::cms_invalid, "", destPosition_w, scriptCallback ? scriptCallback : "", forceLoadScreen);
			}
			else
				object->teleportObject(destPosition_w, newBuilding, newContainerName, newPosition_p, scriptCallback ? scriptCallback : "", forceLoadScreen);
		}
		else
			object->teleportObject(destPosition_w, newBuilding, newContainerName, newPosition_p, scriptCallback ? scriptCallback : "", forceLoadScreen);
	}
	else
	{
		// don't send multiple RequestSceneTransfer message
		if (!checkAndSetOutstandingRequestSceneTransfer(*object))
		{
			RequestSceneTransfer const sceneTransferMessage(objectId, sceneName, getProcessId(), newPosition_p, destPosition_w, newBuilding, newContainerName, scriptCallback);
			sendToCentralServer(sceneTransferMessage);
		}
	}

	return true;
}

// ----------------------------------------------------------------------

/**
 * Warping an object to a different scene after a delay.
 */
bool GameServer::requestSceneWarpDelayed(const CachedNetworkId &objectId, const std::string &sceneName, const Vector &newPosition_w, const NetworkId &newContainer, const Vector &newPosition_p,  float const delayTime, const char * scriptCallback, bool forceLoadScreen)
{
	Object * const o = objectId.getObject();
	ServerObject * const so = o ? o->asServerObject() : 0;
	ShipObject * const ship = so ? so->asShipObject() : 0;

	if (!ship || !ship->isInWorld())
		return false;

	ship->setHyperspaceOnDestroy(true);

	typedef std::vector<CreatureObject *> Passengers;
	Passengers passengers;

	ship->findAllPassengers(passengers, true);

	Passengers::const_iterator ii = passengers.begin();
	Passengers::const_iterator iiEnd = passengers.end();

	for (; ii != iiEnd; ++ii)
	{
		CreatureObject * const passenger = *ii;
		Controller * const controller = passenger->getController();

		if (controller != 0)
		{
			MessageQueueGenericValueType<std::pair<std::string, Vector> > * const data = new MessageQueueGenericValueType<std::pair<std::string, Vector> >(std::make_pair(sceneName, newPosition_w));

			controller->appendMessage(
 				CM_aboutToHyperspace,
				0.0f,
				data,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}

	char buf[1000];
	std::string callbackStr = scriptCallback;
	snprintf(buf, sizeof(buf) - 1, "%s %.2f %.2f %.2f %s %.2f %.2f %.2f %s %d", sceneName.c_str(), newPosition_w.x, newPosition_w.y, newPosition_w.z, newContainer.getValueString().c_str(), newPosition_p.x, newPosition_p.y, newPosition_p.z, (!callbackStr.empty()) ? callbackStr.c_str() : "noCallback", (forceLoadScreen) ? 1 : 0);
	MessageToQueue::getInstance().sendMessageToC(objectId, "C++SceneWarpDelayed", buf, static_cast<int>(delayTime), false);

	return true;
}

//-----------------------------------------------------------------------

/**
 * Warping an object to a different scene after a delay.
 */
bool GameServer::requestSceneWarpDelayed(const CachedNetworkId &objectId, const std::string &sceneName, const Vector &newPosition_w, const NetworkId &newBuilding, const std::string &newContainerName, const Vector &newPosition_p, float const delayTime, const char * scriptCallback, bool forceLoadScreen)
{
	Object * const o = objectId.getObject();
	ServerObject * const so = o ? o->asServerObject() : 0;
	ShipObject * const ship = so ? so->asShipObject() : 0;

	if (!ship || !ship->isInWorld())
		return false;

	typedef std::vector<CreatureObject *> Passengers;
	Passengers passengers;

	ship->findAllPassengers(passengers, true);

	Passengers::const_iterator ii = passengers.begin();
	Passengers::const_iterator iiEnd = passengers.end();

	for (; ii != iiEnd; ++ii)
	{
		CreatureObject * const passenger = *ii;
		Controller * const controller = passenger->getController();

		if (controller != 0)
		{
			MessageQueueGenericValueType<std::pair<std::string, Vector> > * const data = new MessageQueueGenericValueType<std::pair<std::string, Vector> >(std::make_pair(sceneName, newPosition_w));

			controller->appendMessage(
 				CM_aboutToHyperspace,
				0.0f,
				data,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}

	char buf[1000];
	std::string callbackStr = scriptCallback;
	snprintf(buf, sizeof(buf) - 1, "%s %.2f %.2f %.2f %s %s %.2f %.2f %.2f %s %d", sceneName.c_str(), newPosition_w.x, newPosition_w.y, newPosition_w.z, newBuilding.getValueString().c_str(), newContainerName.c_str(), newPosition_p.x, newPosition_p.y, newPosition_p.z, (!callbackStr.empty()) ? callbackStr.c_str() : "none", (forceLoadScreen) ? 1 : 0);
	MessageToQueue::getInstance().sendMessageToC(objectId, "C++SceneWarpDelayedContainer", buf, static_cast<int>(delayTime), false);

	return true;
}

// ----------------------------------------------------------------------

unsigned long GameServerNamespace::getFrameRateLimit()
{
	return ServerWorld::isSpaceScene() ? ConfigServerGame::getSpaceFrameRateLimit() : ConfigServerGame::getGroundFrameRateLimit();
}

// ----------------------------------------------------------------------

void GameServer::run(void)
{
	getInstance().initialize();

	unsigned long lastFrameTime = 0;
	int oldFilesOpened = TreeFile::getNumberOfFilesOpenedTotal();
	int oldSizeOpened = TreeFile::getSizeOfFilesOpenedTotal();
	int newFilesOpened = 0;
	int newSizeOpened = 0;
	unsigned long startTime = Clock::timeMs();
	unsigned long lastFrameProcessStartTime = startTime;

	const unsigned long targetFrameTime = static_cast<unsigned long>(1000.0f/getFrameRateLimit());

	const GenericValueTypeMessage<unsigned long> gameServerTaskManagerKeepAlive("GameServerTaskManagerKeepAlive", Os::getProcessId());

	PROFILER_BLOCK_DEFINE(profileBlockMainLoop, "main loop");
	PROFILER_BLOCK_ENTER(profileBlockMainLoop);

	LOG("ServerStartup",("GameServer starting on %s", NetworkHandler::getHostName().c_str()));  // no process id yet
	s_frameTime = 0;

	while (!getInstance().getDone())
	{
		if(getInstance().m_taskManagerConnection)
		{
			getInstance().m_taskManagerConnection->send(gameServerTaskManagerKeepAlive, true);
			s_lastTaskKeepaliveTime = Clock::timeMs();
		}

		newFilesOpened = TreeFile::getNumberOfFilesOpenedTotal();
		newSizeOpened = TreeFile::getSizeOfFilesOpenedTotal();

		if (   lastFrameTime > ConfigServerGame::getLoopProfileLogThresholdMs()
		    || (ServerWorld::isSpaceScene() && lastFrameTime > ConfigServerGame::getSpaceLoopProfileLogThresholdMs())
		    || (!ServerWorld::isSpaceScene() && lastFrameTime > ConfigServerGame::getGroundLoopProfileLogThresholdMs()))
		{
			LOG("profile", ("Long loop (%u ms):\n%s", lastFrameTime, PROFILER_GET_LAST_FRAME_DATA()));
			if (newFilesOpened > oldFilesOpened)
				LOG("treefile", ("Long loop file stats: %d opened %d size", newFilesOpened - oldFilesOpened, newSizeOpened - oldSizeOpened));
		}

		oldFilesOpened = newFilesOpened;
		oldSizeOpened = newSizeOpened;

		{
			PROFILER_AUTO_BLOCK_DEFINE("NetFlushAllData");
			NetworkHandler::flushAndConfirmAll();
		}

		bool barrierReached = true;

		do
		{
			PROFILER_AUTO_BLOCK_DEFINE("BarrierWait");

			{
				PROFILER_AUTO_BLOCK_DEFINE("NetworkHandler::update");
				NetworkHandler::update();
			}

			if ((ServerWorld::isSpaceScene() && ConfigServerGame::getSpaceShouldSleep())
			    || (!ServerWorld::isSpaceScene() && ConfigServerGame::getGroundShouldSleep()))
			{
				PROFILER_AUTO_BLOCK_DEFINE("Os::sleep");
				Os::sleep(1);
			}

			if (ConfigServerGame::getCommoditiesMarketEnabled())
			{
				PROFILER_AUTO_BLOCK_DEFINE("CommoditiesMarket::giveTime");
				CommoditiesMarket::giveTime();
			}

			// Note: adding 15 ms to the time passed here to adjust for minimum sleep times
			barrierReached = Clock::timeMs()-lastFrameProcessStartTime+15 >= targetFrameTime;

			getInstance().debugIO();
		} while (!barrierReached && !getInstance().getDone());

		lastFrameProcessStartTime = Clock::timeMs();

		if (!Os::update())
			getInstance().setDone("OS condition (Parent pid change)");

		{
			PROFILER_AUTO_BLOCK_DEFINE("MetricsManager::update");
			if (s_metricsManagerInstalled)
				MetricsManager::update(static_cast<float>(lastFrameTime));
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::update");
			ServerWorld::update(Clock::frameTime());
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerUniverse::update");
			ServerUniverse::getInstance().update(Clock::frameTime());
		}

		NonCriticalTaskQueue::getInstance().update();
		{
			PROFILER_AUTO_BLOCK_DEFINE("MessageToQueue::update");
			MessageToQueue::getInstance().update(Clock::frameTime());
		}

#ifdef _DEBUG
		// do an intentional sleep, to emulate long loop time
		if (s_extraDelayPerFrameMs > 0)
		{
			PROFILER_AUTO_BLOCK_DEFINE("Os::sleep - extra");
			Os::sleep(s_extraDelayPerFrameMs);
		}
#endif

		{
			// Report the population at the same time on all game servers in the cluster
			static bool alreadyReported(false);
			if ((time(0) % ConfigServerGame::getPopulationReportTimeSeconds()) == 0)
			{
				if (!alreadyReported)
				{
					LOG("PopulationReport",("server %s:%lu population %i",ServerWorld::getSceneId().c_str(),getInstance().m_preloadAreaId,getInstance().m_clients->size()));
					alreadyReported=true;
				}
			}
			else
				alreadyReported=false;
		}
		
		unsigned long curTime = Clock::timeMs();
		lastFrameTime = curTime-startTime;
		if (!getInstance().getDone())
			ServerClock::getInstance().incrementServerFrame();
		startTime = curTime;

		getInstance().checkIdleClients(Clock::frameTime());
		if (!getInstance().m_gameServerReadyDatabaseConnected && getInstance().m_connectionTimeout!=0 && (curTime > getInstance().m_connectionTimeout))
			FATAL(true,("Failed to resolve connections to all the other game servers within %i seconds",ConfigServerGame::getConnectToAllGameServersTimeout()));

		PROFILER_BLOCK_LEAVE(profileBlockMainLoop);
		PROFILER_BLOCK_ENTER(profileBlockMainLoop);
		if (getInstance().getDatabaseProcessId())
		{
			FrameEndMessage const emptyFrameEndMessage(getInstance().getProcessId(), lastFrameTime, "");
			getInstance().sendToDatabaseServer(emptyFrameEndMessage);
			if (ConfigServerGame::getSendFrameProfilerInfo())
			{
				FrameEndMessage const frameEndMessage(getInstance().getProcessId(), lastFrameTime, PROFILER_GET_LAST_FRAME_DATA());
				getInstance().sendToPlanetServer(frameEndMessage);
			}
			else
				getInstance().sendToPlanetServer(emptyFrameEndMessage);
			NetworkHandler::clearBytesThisFrame();
		}
		s_frameTime = curTime - lastFrameProcessStartTime;
	}

	LOG("ServerStartup",("GameServer %lu exiting",getInstance().getProcessId()));

	PROFILER_BLOCK_LEAVE(profileBlockMainLoop);

	getInstance().shutdown();

}

//-----------------------------------------------------------------------

void GameServer::sendToDatabaseServer(GameNetworkMessage const &message)
{
	if (m_databaseProcessConnection)
		m_databaseProcessConnection->send(message, true);
	else
		DEBUG_FATAL(!m_done, ("Unable to send network message to database server while not flagged for shutdown"));
	}

// ----------------------------------------------------------------------

std::vector<uint32> const &GameServer::getAllGameServerPids() const
{
	return m_gameServerPids;
}

// ----------------------------------------------------------------------

bool GameServer::hasConnectionsToOtherGameServers() const
{
	return !(m_gameServerPids.empty());
}

// ----------------------------------------------------------------------

void GameServer::sendToCentralServer(GameNetworkMessage const &message)
{
	if(m_centralServerConnection)
	{
		m_centralServerConnection->send(message, true);
	}
	else
	{
		DEBUG_REPORT_LOG(true,("Tried to send a message to Central Server, but it was not connected.\n"));
	}
}

// ----------------------------------------------------------------------

void GameServer::sendToPlanetServer(GameNetworkMessage const &message)
{
	if (m_planetServerConnection)
	{
		m_planetServerConnection->send(message, true);
	}
	else
	{
		DEBUG_REPORT_LOG(true,("Tried to send a message to Planet Server, but it was not connected.\n"));
	}
}

// ----------------------------------------------------------------------

void GameServer::sendToConnectionServers(GameNetworkMessage const &message)
{
	ConnectionServerVector v(*m_connectionServerVector);
	for (ConnectionServerVector::iterator i = v.begin(); i != v.end(); ++i)
		(*i)->send(message, true);
}

// ----------------------------------------------------------------------

void GameServer::sendToCustomerServiceServer(GameNetworkMessage const &message)
{
	if (m_customerServiceServerConnection != nullptr)
	{
		m_customerServiceServerConnection->send(message, true);
	}
	else
	{
		REPORT_LOG(true, ("GameServer::sendToCustomerServiceServer() ERROR: Unable to send to nullptr customer service server connection\n"));
	}
}

// ----------------------------------------------------------------------

void GameServer::clearCustomerServiceServerConnection()
{
	if (m_customerServiceServerConnection != nullptr)
	{
		m_customerServiceServerConnection->disconnect();
	}

	m_customerServiceServerConnection = nullptr;
}

// ----------------------------------------------------------------------

/**
 * Call when we are connected to the database process.
 * Prerequisite to gameServerReady()
 */
void GameServer::onDatabaseProcessConnectionEstablished()
{
	LOG("GameGameConnect", ("Game Server %lu is connected to database process", m_processId));
	m_gameServerReadyDatabaseConnected = true;
	checkWhetherGameServerIsReady();
}

// ----------------------------------------------------------------------

/**
 * Call when we have at least one block of objectId's.
 * Prerequisite to gameServerReady()
 */
void GameServer::onReceivedFirstNetworkIdBlock()
{
	LOG("GameGameConnect", ("Game Server %lu has received first netId block", m_processId));
	m_gameServerReadyObjectIds = true;
	checkWhetherGameServerIsReady();
}

// ----------------------------------------------------------------------

/**
 * Call when we have at least one block of objectId's.
 * Prerequisite to gameServerReady()
 */
void GameServer::onPlanetServerConnectionEstablished()
{
	LOG("GameGameConnect", ("Game Server %lu is connected to planet server", m_processId));
	m_gameServerReadyPlanetConnected = true;
	checkWhetherGameServerIsReady();
}

// ----------------------------------------------------------------------

/**
 * Call after each asynchronous step of initializing the game server.
 * If all steps are completed, calls gameServerReady()
 */
void GameServer::checkWhetherGameServerIsReady()
{
	if (m_gameServerReadyDatabaseConnected && m_gameServerReadyObjectIds && m_gameServerReadyPlanetConnected)
		gameServerReady();
}

// ----------------------------------------------------------------------

/**
 * Call this function when we are ready to handle receiving objects.
 * Notifies Central, who may tell other game servers to give
 * us proxy objects now or may tell us to create new objects.
 */
void GameServer::gameServerReady()
{
	static int callCount(0);
	UNREF(callCount);
	DEBUG_FATAL(getProcessId()==0,("gameServerReady() was called before process ID was set.\n"));
	DEBUG_FATAL((callCount++)!=0,("gameServerReady() was called more than once.\n"));

	LOG("GameGameConnect", ("Game Server %d is ready!", m_processId));
	GameServerReadyMessage const msg(static_cast<int>(NON_NULL(TerrainObject::getInstance())->getMapWidthInMeters()));
	sendToCentralServer(msg);
	sendToPlanetServer(msg);
}

// ----------------------------------------------------------------------

const StringId GameServer::verifyCharacterName(const std::string &templateName, const Unicode::String &name, bool forPlayer) const
{
	const ServerCreatureObjectTemplate * serverTemplate = getServerCreatureObjectTemplate(templateName);
	if (!serverTemplate)
		return NameErrors::nameDeclinedNotCreatureTemplate;

	const StringId & verified = verifyCharacterName(*serverTemplate, name, forPlayer);
	serverTemplate->releaseReference();

	return verified;
}

// ----------------------------------------------------------------------

const StringId GameServer::verifyCharacterName(const ServerCreatureObjectTemplate &serverTemplate, const Unicode::String &name, bool forPlayer) const
{
	if (ConfigServerGame::getNameValidationAcceptAll())
		return NameErrors::nameApproved;

	if (name.empty())
		return NameErrors::nameDeclinedEmpty;

	// Check for appropriateness based on race
	if (!NameManager::getInstance().isNameAppropriate(ConfigServerGame::getCharacterNameGeneratorDirectory(),serverTemplate.getNameGeneratorType(),name))
		return NameErrors::nameDeclinedRaciallyInappropriate;

	// Now match against reserved names and profanity
	std::string reasonDescription;
	if (NameManager::getInstance().isNameReserved(name,reasonDescription))
		return StringId(NameErrors::localizationFile,reasonDescription);
	else
		return NameErrors::nameApproved;
}

//--------------------------------------------------------------------

void GameServer::handleCreateCharacter(const CentralCreateCharacter * createMessage)
{
	LOG("TraceCharacterCreation", ("%d received CentralCreateCharacter(%d, %s, ...)", createMessage->getStationId(), createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
	if (m_charactersPendingCreation->find(createMessage->getStationId()) != m_charactersPendingCreation->end())
	{
		LOG("TraceCharacterCreation", ("%d name verification for that name is still pending, sending GameCreateCharacterFailed(%d, %s, NameErrors::nameDeclinedRetry) to central server", createMessage->getStationId(), createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
		DEBUG_REPORT_LOG(true, ("Received character creation request for account %li, but we are still pending a name verification for that id\n",createMessage->getStationId()));
		GameCreateCharacterFailed const characterCreateFailed(createMessage->getStationId(), createMessage->getCharacterName(), NameErrors::nameDeclinedRetry, FormattedString<2048>().sprintf("%lu name verification for that name is still pending, sending GameCreateCharacterFailed(%lu, %s, NameErrors::nameDeclinedRetry) to central server", createMessage->getStationId(), createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
		sendToCentralServer(characterCreateFailed);
		delete createMessage;
		return;
	}

	const std::string & templateName = createMessage->getTemplateName();
	DEBUG_REPORT_LOG(true, ("Received character creation request for account %li (name %s, template %s)\n",createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str(), templateName.c_str()));
	if(templateName.empty ())
	{
		LOG("TraceCharacterCreation", ("%d sent an empty template name, sending GameCreateCharacterFailed(%d, %s, NameErrors::nameDeclinedNoTemplate) to central server", createMessage->getStationId(), createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
		DEBUG_REPORT_LOG(true, ("The template was empty\n"));
		// Tell central we couldn't create the character
		GameCreateCharacterFailed const characterCreateFailed(createMessage->getStationId(), createMessage->getCharacterName(), NameErrors::nameDeclinedNoTemplate, FormattedString<2048>().sprintf("%lu sent an empty template name, sending GameCreateCharacterFailed(%lu, %s, NameErrors::nameDeclinedNoTemplate) to central server", createMessage->getStationId(), createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
		sendToCentralServer(characterCreateFailed);
		delete createMessage;
		return;
	}

	if (!SpeciesRestrictions::canCreateCharacter(createMessage->getGameFeatures(), Crc::calculate(templateName.c_str())))
	{
		LOG("TraceCharacterCreation", ("%d sent a species template name their account was not authorized to use.  Template was %s", createMessage->getStationId(), templateName.c_str()));
		// Tell central we couldn't create the character
		GameCreateCharacterFailed const characterCreateFailed(createMessage->getStationId(), createMessage->getCharacterName(), NameErrors::nameDeclinedNotAuthorizedForSpecies, FormattedString<2048>().sprintf("%lu sent a species template name their account was not authorized to use.  Template was %s", createMessage->getStationId(), templateName.c_str()));
		sendToCentralServer(characterCreateFailed);
		delete createMessage;
		return;
	}

	StringId avatarReason = canCreateAvatar(templateName);
	if (avatarReason != NameErrors::nameApproved)
	{
		LOG("TraceCharacterCreation", ("%d canCreateAvatar(%s) failed, sending GameCreateCharacterFailed(%d, %s, REASON) to central server", createMessage->getStationId(), createMessage->getTemplateName().c_str(), createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
		DEBUG_REPORT_LOG(true, ("Can't create avatar\n"));
		GameCreateCharacterFailed const characterCreateFailed(createMessage->getStationId(), createMessage->getCharacterName(), avatarReason, FormattedString<2048>().sprintf("%lu canCreateAvatar(%s) failed, sending GameCreateCharacterFailed(%lu, %s, REASON) to central server", createMessage->getStationId(), createMessage->getTemplateName().c_str(), createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
		sendToCentralServer(characterCreateFailed);
		delete createMessage;
		return;
	}

	// verify the name against static tests - length, character set, etc.
	StringId VerifyNameResponse = verifyCharacterName(templateName, createMessage->getCharacterName(), true);
	if (VerifyNameResponse != NameErrors::nameApproved)
	{
		LOG("TraceCharacterCreation", ("%d VerifyNameResponse != NameErrors::nameApproved, sending GameCreateCharacterFailed(%d, %s, REASON) to central server", createMessage->getStationId(), createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
		GameCreateCharacterFailed const characterCreateFailed(createMessage->getStationId(), createMessage->getCharacterName(), VerifyNameResponse, FormattedString<2048>().sprintf("%lu VerifyNameResponse != NameErrors::nameApproved, sending GameCreateCharacterFailed(%lu, %s, REASON) to central server", createMessage->getStationId(), createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
		sendToCentralServer(characterCreateFailed);
		delete createMessage;
		return;
	}

	// verify the name against known existing names
	if (NameManager::getInstance().getPlayerId(NameManager::normalizeName(Unicode::wideToNarrow(createMessage->getCharacterName()))) != NetworkId::cms_invalid)
	{
		LOG("TraceCharacterCreation", ("%d character create name %s is in use, sending GameCreateCharacterFailed", createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
		DEBUG_REPORT_LOG(true, ("Character create name %s is in use, declined for stationId %lu\n", Unicode::wideToNarrow(createMessage->getCharacterName()).c_str(), createMessage->getStationId()));
		GameCreateCharacterFailed const characterCreateFailed(createMessage->getStationId(), createMessage->getCharacterName(), NameErrors::nameDeclinedInUse, FormattedString<2048>().sprintf("%lu character create name %s is in use, sending GameCreateCharacterFailed", createMessage->getStationId(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
		sendToCentralServer(characterCreateFailed);
		delete createMessage;
		return;
	}

	// Now verify against the database.
	LOG("TraceCharacterCreation", ("%d sending VerifyNameRequest(%d, %s, %s) to the database for validation", createMessage->getStationId(), createMessage->getStationId(), createMessage->getTemplateName().c_str(), Unicode::wideToNarrow(createMessage->getCharacterName()).c_str()));
	VerifyNameRequest const vrn(createMessage->getStationId(), NetworkId::cms_invalid, createMessage->getTemplateName(), createMessage->getCharacterName());
	sendToDatabaseServer(vrn);
	(*m_charactersPendingCreation)[createMessage->getStationId()] = createMessage;
}

//--------------------------------------------------------------------

void GameServer::handleNameFailure(const RandomNameRequest &request, const StringId &errorMessage)
{
	LOG("TraceCharacterCreation", ("%d sending RandomNameResponse(%d, %s, REASON) to central server", request.getStationId(), request.getCreatureTemplate().c_str()));
	RandomNameResponse const rnr(request.getStationId(), request.getCreatureTemplate(), Unicode::String(), errorMessage);
	sendToCentralServer(rnr);
}

//--------------------------------------------------------------------

void GameServer::handleNameRequest(const RandomNameRequest &request)
{
	DEBUG_REPORT_LOG(true, ("Random name requested for stationId %lu\n", request.getStationId()));

	std::map<uint32, const CentralCreateCharacter *>::iterator findPendingCreation=m_charactersPendingCreation->find(request.getStationId());
	if (findPendingCreation != m_charactersPendingCreation->end())
	{
		LOG("TraceCharacterCreation", ("%d received random name request after CharacterCreationRequest, ignoring request", request.getStationId()));
		DEBUG_REPORT_LOG(true, ("Received random name request after CharacterCreation request"));
		return;
	}

	bool success = false;

	const ObjectTemplate * const objectTemplate = ObjectTemplateList::fetch(request.getCreatureTemplate());
	if (objectTemplate)
	{
		const ServerCreatureObjectTemplate * const creatureTemplate = dynamic_cast<const ServerCreatureObjectTemplate *>(objectTemplate);
		if (creatureTemplate)
		{
			// generate a name
			Unicode::String randomName = NameManager::getInstance().generateUniqueRandomName(ConfigServerGame::getCharacterNameGeneratorDirectory(),creatureTemplate->getNameGeneratorType());

			LOG("TraceCharacterCreation", ("%d generated name %s", request.getStationId(), Unicode::wideToNarrow(randomName).c_str(), request.getStationId(), request.getCreatureTemplate().c_str(), Unicode::wideToNarrow(randomName).c_str()));

			DEBUG_REPORT_LOG(true, ("VerifyNameResponse: random name %s approved for stationId %lu\n", Unicode::wideToNarrow(randomName).c_str(), request.getStationId()));
			RandomNameResponse const response(request.getStationId(), request.getCreatureTemplate(), randomName, NameErrors::nameApproved);
			sendToCentralServer(response);
			
			success = true;
		}
		else
		{
			LOG("TraceCharacterCreation", ("%d failed to get a creature template to service random name request. The request failed", request.getStationId()));
		}
		objectTemplate->releaseReference();
	}
	else
	{
		LOG("TraceCharacterCreation", ("%d could not get an ObjectTemplate for name request", request.getStationId()));
	}

	if (!success)
	{
		WARNING(true, ("Could not generate a random name for template %s (template may not exist or may not be a CreatureTemplate).\n", request.getCreatureTemplate().c_str()));
		handleNameFailure(request, NameErrors::nameDeclinedNoTemplate);
	}
}

//--------------------------------------------------------------------

const StringId GameServer::handleVerifyAndLockNameRequest(const VerifyAndLockNameRequest &request, bool sendFailedResponseToCentralServer, bool verifyAgainstDb)
{
	REPORT_LOG(true, ("Verify and lock name requested for stationId %lu\n", request.getStationId()));
	
	const std::string & templateName = request.getTemplateName();
	DEBUG_REPORT_LOG(true, ("Received verify and lock name request for account %li (name %s, template %s)\n", request.getStationId(), Unicode::wideToNarrow(request.getCharacterName()).c_str(), templateName.c_str()));
	if(templateName.empty ())
	{
		// Tell central we couldn't create the character
		if (sendFailedResponseToCentralServer)
		{
			LOG("TraceCharacterCreation", ("%lu sent an empty template name for name verification, sending VerifyAndLockNameResponse(%lu, %s, NameErrors::nameDeclinedNoTemplate) to central server", request.getStationId(), request.getStationId(), Unicode::wideToNarrow(request.getCharacterName()).c_str()));
			DEBUG_REPORT_PRINT(true, ("%lu sent an empty template name for name verification, sending VerifyAndLockNameResponse(%lu, %s, NameErrors::nameDeclinedNoTemplate) to central server\n", request.getStationId(), request.getStationId(), Unicode::wideToNarrow(request.getCharacterName()).c_str()));

			VerifyAndLockNameResponse const verifyAndLockNameResponse(request.getStationId(), request.getCharacterName(), NameErrors::nameDeclinedNoTemplate);
			sendToCentralServer(verifyAndLockNameResponse);
		}

		return NameErrors::nameDeclinedNoTemplate;
	}

	if (!SpeciesRestrictions::canCreateCharacter(request.getGameFeatures(), Crc::calculate(templateName.c_str())))
	{
		// Tell central we couldn't create the character
		if (sendFailedResponseToCentralServer)
		{
			LOG("TraceCharacterCreation", ("%lu sent a species template name their account was not authorized to use, sending VerifyAndLockNameResponse to CentralServer.  Template was %s", request.getStationId(), templateName.c_str()));
			DEBUG_REPORT_PRINT(true, ("%lu sent a species template name their account was not authorized to use, sending VerifyAndLockNameResponse to CentralServer.  Template was %s\n", request.getStationId(), templateName.c_str()));

			VerifyAndLockNameResponse const verifyAndLockNameResponse(request.getStationId(), request.getCharacterName(), NameErrors::nameDeclinedNotAuthorizedForSpecies);
			sendToCentralServer(verifyAndLockNameResponse);
		}

		return NameErrors::nameDeclinedNotAuthorizedForSpecies;
	}

	StringId avatarReason = canCreateAvatar(templateName);
	if (avatarReason != NameErrors::nameApproved)
	{
		if (sendFailedResponseToCentralServer)
		{
			LOG("TraceCharacterCreation", ("%lu canCreateAvatar(%s) failed, sending VerifyAndLockNameResponse(%lu, %s, REASON) to central server", request.getStationId(), request.getTemplateName().c_str(), request.getStationId(), Unicode::wideToNarrow(request.getCharacterName()).c_str()));
			DEBUG_REPORT_PRINT(true, ("%lu canCreateAvatar(%s) failed, sending VerifyAndLockNameResponse(%lu, %s, REASON) to central server\n", request.getStationId(), request.getTemplateName().c_str(), request.getStationId(), Unicode::wideToNarrow(request.getCharacterName()).c_str()));

			VerifyAndLockNameResponse const verifyAndLockNameResponse(request.getStationId(), request.getCharacterName(), avatarReason);
			sendToCentralServer(verifyAndLockNameResponse);
		}

		return avatarReason;
	}

	// verify the name against static tests - length, character set, etc.
	StringId VerifyNameResponse = verifyCharacterName(templateName, request.getCharacterName(), true);
	if (VerifyNameResponse != NameErrors::nameApproved)
	{
		if (sendFailedResponseToCentralServer)
		{
			LOG("TraceCharacterCreation", ("%lu VerifyNameResponse != NameErrors::nameApproved, sending VerifyAndLockNameResponse(%lu, %s, REASON) to central server", request.getStationId(), request.getStationId(), Unicode::wideToNarrow(request.getCharacterName()).c_str()));
			DEBUG_REPORT_PRINT(true, ("%lu VerifyNameResponse != NameErrors::nameApproved, sending VerifyAndLockNameResponse(%lu, %s, REASON) to central server\n", request.getStationId(), request.getStationId(), Unicode::wideToNarrow(request.getCharacterName()).c_str()));

			VerifyAndLockNameResponse const verifyAndLockNameResponse(request.getStationId(), request.getCharacterName(), VerifyNameResponse);
			sendToCentralServer(verifyAndLockNameResponse);
		}

		return VerifyNameResponse;
	}

	// verify the name against known existing names
	if (NameManager::getInstance().getPlayerId(NameManager::normalizeName(Unicode::wideToNarrow(request.getCharacterName()))) != NetworkId::cms_invalid)
	{
		if (sendFailedResponseToCentralServer)
		{
			LOG("TraceCharacterCreation", ("%lu character create name %s is in use, sending VerifyAndLockNameResponse", request.getStationId(), Unicode::wideToNarrow(request.getCharacterName()).c_str()));
			DEBUG_REPORT_PRINT("TraceCharacterCreation", ("%lu character create name %s is in use, sending VerifyAndLockNameResponse\n", request.getStationId(), Unicode::wideToNarrow(request.getCharacterName()).c_str()));

			VerifyAndLockNameResponse const verifyAndLockNameResponse(request.getStationId(), request.getCharacterName(), NameErrors::nameDeclinedInUse);
			sendToCentralServer(verifyAndLockNameResponse);
		}

		return NameErrors::nameDeclinedInUse;
	}

	// Now verify against the database.
	if (verifyAgainstDb)
	{
		LOG("TraceCharacterCreation", ("%lu sending VerifyNameRequest(%lu, %s, %s) to the database for validation", request.getStationId(), request.getStationId(), request.getTemplateName().c_str(), Unicode::wideToNarrow(request.getCharacterName()).c_str()));
		VerifyNameRequest const vrn(request.getStationId(), request.getCharacterId(), request.getTemplateName(), request.getCharacterName());
		sendToDatabaseServer(vrn);
	}

	return NameErrors::nameApproved;
}

//--------------------------------------------------------------------

void GameServer::handleCharacterCreateNameVerification(const VerifyNameResponse &vrn)
{
	std::map<uint32, const CentralCreateCharacter *>::iterator findPendingCreation=m_charactersPendingCreation->find(vrn.getStationId());
	const CentralCreateCharacter * createMessage = findPendingCreation->second;

	if (vrn.getErrorMessage() != NameErrors::nameApproved)
	{
		LOG("TraceCharacterCreation", ("%d VeryfyNameResponse character create name %s declined, sending GameCreateCharacterFailed(%d, %s, %s)", vrn.getStationId(), Unicode::wideToNarrow(vrn.getName()).c_str(), vrn.getStationId(), Unicode::wideToNarrow(vrn.getName()).c_str(), vrn.getErrorMessage().getText().c_str()));
		DEBUG_REPORT_LOG(true, ("VerifyNameResponse: character create name %s declined (%s) for stationId %lu\n", Unicode::wideToNarrow(vrn.getName()).c_str(), vrn.getErrorMessage().getText().c_str(), vrn.getStationId()));
		GameCreateCharacterFailed const characterCreateFailed(vrn.getStationId(), vrn.getName(), vrn.getErrorMessage(), FormattedString<2048>().sprintf("%lu VeryfyNameResponse character create name %s declined, sending GameCreateCharacterFailed(%lu, %s, %s)", vrn.getStationId(), Unicode::wideToNarrow(vrn.getName()).c_str(), vrn.getStationId(), Unicode::wideToNarrow(vrn.getName()).c_str(), vrn.getErrorMessage().getText().c_str()));
		sendToCentralServer(characterCreateFailed);
		m_charactersPendingCreation->erase(vrn.getStationId());
		delete createMessage;
		return;
	}
	std::string templateName(createMessage->getTemplateName());

	DEBUG_REPORT_LOG(true, ("VerifyNameResponse: character create name %s approved for stationId %lu\n", Unicode::wideToNarrow(vrn.getName()).c_str(), vrn.getStationId()));

	TangibleObject *newCharacterObject = 0;

	if (createMessage->getUseNewbieTutorial())
	{
		Transform tr;
		tr.setPosition_p(NewbieTutorial::getTutorialLocation());
		newCharacterObject = safe_cast<TangibleObject*>(ServerWorld::createNewObject(templateName, tr, 0, false));
	}
	else
	{
		Transform tr;
		tr.setPosition_p(createMessage->getCoordinates());
		newCharacterObject = safe_cast<TangibleObject *>(ServerWorld::createNewObject(templateName, tr, 0, false));
	}

	if (!newCharacterObject)
	{
		LOG("TraceCharacterCreation", ("%d could not create a new character object on the game server, sending GameCreateCharacterFailed(%d, %s, %s) to central server", vrn.getStationId(), vrn.getStationId(), Unicode::wideToNarrow(vrn.getName()).c_str(), vrn.getErrorMessage().getText().c_str()));
		DEBUG_REPORT_LOG(true, ("handleCharacterCreateNameVerification: failed to create character object (name %s, template %s, stationId %lu)\n", Unicode::wideToNarrow(vrn.getName()).c_str(), vrn.getCreatureTemplate().c_str(), vrn.getStationId()));
		GameCreateCharacterFailed const characterCreateFailed(vrn.getStationId(), vrn.getName(), NameErrors::nameDeclinedInternalError, FormattedString<2048>().sprintf("%lu could not create a new character object on the game server, sending GameCreateCharacterFailed(%lu, %s, %s) to central server", vrn.getStationId(), vrn.getStationId(), Unicode::wideToNarrow(vrn.getName()).c_str(), vrn.getErrorMessage().getText().c_str()));
		sendToCentralServer(characterCreateFailed);
		m_charactersPendingCreation->erase(vrn.getStationId());
		delete createMessage;
		return;
	}

	newCharacterObject->setObjectName(createMessage->getCharacterName());
	newCharacterObject->setOwnerId(newCharacterObject->getNetworkId());
	newCharacterObject->setPlayerControlled(true);
	if (createMessage->getUseNewbieTutorial())
		NewbieTutorial::setupCharacterForTutorial(newCharacterObject);
	else
		NewbieTutorial::setupCharacterToSkipTutorial(newCharacterObject);

	CollisionProperty * collision = newCharacterObject->getCollisionProperty();
	if(collision)
	{
		collision->setPlayerControlled(true);
	}

	//@todo: scale factor should probably be in TangibleObject, not CreatureObject
	CreatureObject * const creature = dynamic_cast<CreatureObject *>(newCharacterObject);
	if (creature)
	{
		float scaleFactor = createMessage->getScaleFactor ();
		const SharedObjectTemplate * const tmpl = safe_cast<const SharedObjectTemplate *>(creature->getSharedTemplate ());

		if (tmpl)
		{
			const float scaleMax   = tmpl->getScaleMax ();
			const float scaleMin   = tmpl->getScaleMin ();

			scaleFactor = std::min (scaleFactor, scaleMax);
			scaleFactor = std::max (scaleFactor, scaleMin);
		}

		creature->setScaleFactor (scaleFactor);
		creature->setScale       (Vector::xyz111 * scaleFactor);
	}

	TangibleObject * tangibleObject = dynamic_cast<TangibleObject*>(newCharacterObject);
	//@todo we might ought enforce this be a CreatureObject
	if (tangibleObject)
		tangibleObject->setAppearanceData(createMessage->getAppearanceData());

	// hair equip hack
	if(!createMessage->getHairTemplateName().empty ())
	{
		ServerObject * const hair = ServerWorld::createNewObject(createMessage->getHairTemplateName(), *newCharacterObject, false);
		if (!hair)
		{
			DEBUG_WARNING(true, ("Could not create hair %s\n", createMessage->getHairTemplateName().c_str()));
		}
		else
		{
			TangibleObject * const tangible_hair = dynamic_cast<TangibleObject *>(hair);

			if (tangible_hair)
				tangible_hair->setAppearanceData (createMessage->getHairAppearanceData ());
			else
				WARNING (true, ("Hair is not tangible, wtf.  Can't customize it.  (among other things, probably)..."));
		}
	}

	if (!createMessage->getProfession ().empty ())
	{
		CreatureObject * const creature = dynamic_cast<CreatureObject *>(newCharacterObject);
		if (creature)
			PlayerCreationManagerServer::setupPlayer (*creature, createMessage->getProfession (), createMessage->getStationId(), createMessage->getJedi());
	}

	if (!createMessage->getBiography().empty())
	{
		BiographyManager::setBiography(newCharacterObject->getNetworkId(),createMessage->getBiography());
	}

	// ----------------------------------------------------------------------
	// Set up the PlayerObject
	ServerObject *playerServerObject = ServerWorld::createNewObject(ConfigServerGame::getPlayerObjectTemplate(), *newCharacterObject, false);
	PlayerObject *play = dynamic_cast<PlayerObject*>(playerServerObject);
	if (play)
	{
		play->setStationId(createMessage->getStationId());
		play->setBornDate();
		play->setSkillTemplate(createMessage->getSkillTemplate(), true);
		play->setWorkingSkill(createMessage->getWorkingSkill(), true);

		// Setup initial A-Tab inventory.
		SlottedContainer * const container = ContainerInterface::getSlottedContainer(*newCharacterObject);
		if(container)
		{
			SlotId slot;
			Container::ContainerErrorCode tmp;
			slot = SlotIdManager::findSlotId(GameServerNamespace::SlotNames::appearance);
			if(slot != SlotId::invalid)
			{
				Container::ContainedItem itemId = container->getObjectInSlot(slot, tmp);
				Object* appearanceInventory = itemId.getObject();
				if(appearanceInventory == nullptr)
				{
					DEBUG_WARNING(true, ("Player %s has lost their appearance inventory", newCharacterObject->getNetworkId().getValueString().c_str()));
					appearanceInventory = ServerWorld::createNewObject(s_appearanceTemplate, *newCharacterObject, slot, false);
					if(!appearanceInventory)
					{
						DEBUG_FATAL(true, ("Could not create an appearance inventory for the player who lost theirs"));
					}
				}
			}
		}

		play->persist();
	}
	else
	{
		LOG("TraceCharacterCreation", ("%d unable to create player object for new character %s", vrn.getStationId(), newCharacterObject->getNetworkId().getValueString().c_str()));
		WARNING_STRICT_FATAL(true,("Unable to create PlayerObject for new character %s.\n",newCharacterObject->getNetworkId().getValueString().c_str()));
	}

	newCharacterObject->setSceneIdOnThisAndContents(createMessage->getPlanetName());

	// ----------------------------------------------------------------------
	// Tell DB Process we're about to send it a character, then send it
	AddCharacterMessage const acm(createMessage->getStationId(), newCharacterObject->getNetworkId(),getProcessId(),createMessage->getCharacterName(),createMessage->getJedi());
	sendToDatabaseServer(acm);
	newCharacterObject->persist();

	// ----------------------------------------------------------------------
	// Register player name with the global list
	std::string const &firstName = Unicode::wideToNarrow(Unicode::toLower(newCharacterObject->getAssignedObjectFirstName()));
	std::string const &fullName = Unicode::wideToNarrow(newCharacterObject->getAssignedObjectName());
	time_t const timeNow = Os::getRealSystemTime();
	NameManager::getInstance().addPlayer(newCharacterObject->getNetworkId(), createMessage->getStationId(), firstName, fullName, timeNow, timeNow, true);

	// ----------------------------------------------------------------------
	// Delete the new character -- will reload it from DB when player logs in
	// (This is so that the player could log in to a different server, or not log in at all, and things would still work.)
	ServerWorld::removeObjectFromGame(*newCharacterObject);
	delete newCharacterObject;

	m_charactersPendingCreation->erase(vrn.getStationId());
	delete createMessage;

	// Destroy any existing chat avatar that may be using the new character's name
	GenericValueTypeMessage<std::string> const chatDestroyAvatar("ChatDestroyAvatar", firstName);
	Chat::sendToChatServer(chatDestroyAvatar);
}

//--------------------------------------------------------------------

void GameServer::handleVerifyAndLockNameVerification(const VerifyNameResponse &vrn)
{
	LOG("TraceCharacterCreation", ("%lu VerifyNameResponse from db for verify and lock name received, sending VerifyAndLockNameResponse(%lu, %s, %s)", vrn.getStationId(), vrn.getStationId(), Unicode::wideToNarrow(vrn.getName()).c_str(), vrn.getErrorMessage().getText().c_str()));
	DEBUG_REPORT_PRINT(true, ("%lu VerifyNameResponse from db for verify and lock name received, sending VerifyAndLockResponse(%lu, %s, %s)\n", vrn.getStationId(), vrn.getStationId(), Unicode::wideToNarrow(vrn.getName()).c_str(), vrn.getErrorMessage().getText().c_str()));

	if (vrn.getCharacterId().isValid())
	{
		if (vrn.getErrorMessage() != NameErrors::nameApproved)
		{
			std::string const errorText(std::string("@") + vrn.getErrorMessage().getCanonicalRepresentation());
			ScriptParams params;
			params.addParam(errorText.c_str(), "reason");
			ScriptDictionaryPtr dictionary;
			GameScriptObject::makeScriptDictionary(params, dictionary);
			if (dictionary.get() != nullptr)
			{
				dictionary->serialize();
				MessageToQueue::getInstance().sendMessageToJava(vrn.getCharacterId(), "renameCharacterNameValidationFail", dictionary->getSerializedData(), 0, false);
			}
		}
		else
		{
			MessageToQueue::getInstance().sendMessageToJava(vrn.getCharacterId(), "renameCharacterNameValidationSuccess", std::vector<int8>(), 0, false);
		}
	}
	else
	{
		VerifyAndLockNameResponse const verifyAndLockNameResponse(vrn.getStationId(), vrn.getName(), vrn.getErrorMessage());
		sendToCentralServer(verifyAndLockNameResponse);
	}
}

//--------------------------------------------------------------------

bool GameServer::isCreatePending(uint32 stationId) const
{
	std::map<uint32, const CentralCreateCharacter *>::const_iterator findPendingCreation=m_charactersPendingCreation->find(stationId);
	// If there is not a creation request pending, just reply with the random name.
	return findPendingCreation != m_charactersPendingCreation->end();
}

//--------------------------------------------------------------------

const StringId & GameServer::canCreateAvatar(const std::string &templateName)
{
	const ServerCreatureObjectTemplate * creatureTemplate = getServerCreatureObjectTemplate(templateName);
	if (!creatureTemplate)
	{
		return NameErrors::nameDeclinedNotCreatureTemplate;
	}

	if (!creatureTemplate->getCanCreateAvatar())
	{
		creatureTemplate->releaseReference();
		return NameErrors::nameDeclinedCantCreateAvatar;
	}

	creatureTemplate->releaseReference();
	return NameErrors::nameApproved;
}

//--------------------------------------------------------------------

const ServerCreatureObjectTemplate * GameServer::getServerCreatureObjectTemplate(const std::string &templateName) const
{
	const ObjectTemplate * objectTemplate = ObjectTemplateList::fetch(templateName);
	if (!objectTemplate)
		return 0;

	const ServerCreatureObjectTemplate * creatureTemplate = dynamic_cast<const ServerCreatureObjectTemplate *>(objectTemplate);

	return creatureTemplate;
}
//-----------------------------------------------------------------------

int GameServer::getNumClients() const
{
	if (m_clients)
		return m_clients->size();
	return 0;
}

//-----------------------------------------------------------------------

void GameServer::handleTeleportMessage(TeleportMessage const &message)
{
	CreatureObject * const creature = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(message.getActorId()));
	if (creature)
	{
		char buf[256];
		IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "%s %g %g %g %s %g %g %g",
			message.getSceneId().c_str(),
			message.getPosition_w().x,
			message.getPosition_w().y,
			message.getPosition_w().z,
			message.getContainerId().getValueString().c_str(),
			message.getPosition_p().x,
			message.getPosition_p().y,
			message.getPosition_p().z));
		buf[sizeof(buf)-1] = '\0';

		Command::ErrorCode status = Command::CEC_Success;
		creature->forceExecuteCommand(CommandTable::getCommand(constcrc("planetwarp")), NetworkId::cms_invalid, Unicode::narrowToWide(buf), status, false);
	}
}

//-----------------------------------------------------------------------

void GameServer::handleRetrievedItemLoadMessage(RetrievedItemLoadMessage const &message)
{
	CachedNetworkId ownerId(message.getOwnerId());
	if (ownerId.getObject())
	{
		CreatureObject *owner = dynamic_cast<CreatureObject*>(ownerId.getObject());
		if (owner)
		{
			ServerObject *inventory = owner->getInventory();
			if (inventory)
			{
				NetworkId objectId = message.getItemId();
				if (objectId != NetworkId::cms_invalid)
				{
					inventory->loadContainedObjectFromDB(message.getItemId());
				}
			}
		}
		UNREF(owner);
	}
}

//-----------------------------------------------------------------------

void GameServer::handleReloadAdminTableMessage(ReloadAdminTableMessage const &message)
{
	AdminAccountManager::reload();
}

//-----------------------------------------------------------------------

void GameServer::handleReloadCommandTableMessage(ReloadCommandTableMessage const &message)
{
	ServerCommandTable::load();
}

//-----------------------------------------------------------------------

void GameServer::handleReloadDatatableMessage(ReloadDatatableMessage const &message)
{
	DataTableManager::reload(message.getTable());
}

//-----------------------------------------------------------------------

void GameServer::handleReloadScriptMessage(ReloadScriptMessage const &message)
{
	GameScriptObject::reloadScript(message.getScript());
}

//-----------------------------------------------------------------------

void GameServer::handleReloadTemplateMessage(ReloadTemplateMessage const &message)
{
	Iff templateFile;
	if (templateFile.open(message.getTemplate().c_str(), true))
	{
		ObjectTemplateList::reload(templateFile);
	}
}

//-----------------------------------------------------------------------

void GameServer::handleEnableNewJediTrackingMessage(EnableNewJediTrackingMessage const &message)
{
	GameScriptObject::enableNewJediTracking(message.getEnableTracking());
}

//-----------------------------------------------------------------------

void GameServer::checkIdleClients(float time)
{
	static float s_idleTimePassed;

	s_idleTimePassed += time;

	if (s_idleTimePassed > IDLE_CLIENTS_CHECK_TIME_SEC)
	{
		s_idleTimePassed = 0.0f;

		std::vector<Client*> idleClients;

		for (ClientMap::iterator i = m_clients->begin(); i != m_clients->end(); ++i)
		{
			if ((*i).second->idledTooLong())
			{
				PlayerObject const * playerObject = PlayerCreatureController::getPlayerObject(safe_cast<CreatureObject const *>((*i).second->getCharacterObject()));
				if (!playerObject || (playerObject->getSessionLastActiveTime() == 0))
				{
				idleClients.push_back((*i).second);
				}
			}
		}

		for (std::vector<Client*>::iterator j = idleClients.begin(); j != idleClients.end(); ++j)
			(*j)->handleIdleDisconnect();
	}
}

// ----------------------------------------------------------------------

void GameServer::getObjectsWithClients(std::vector<ServerObject *> &objects) const
{
	objects.clear();
	for (ClientMap::const_iterator i = m_clients->begin(); i != m_clients->end(); ++i)
		if ((*i).second->getCharacterObject())
			objects.push_back((*i).second->getCharacterObject());
}

// ----------------------------------------------------------------------

void GameServer::setDone(char const *reasonfmt, ...)
{
	if (!m_done)
	{
		char reason[1024];
		va_list ap;
		va_start(ap, reasonfmt);
		_vsnprintf(reason, sizeof(reason), reasonfmt, ap);
		reason[sizeof(reason)-1] = '\0';

		LOG(
			"ServerShutdown",
			(
				"GameServer %d (pid %d) shutdown, reason: %s",
				static_cast<int>(getProcessId()),
				static_cast<int>(Os::getProcessId()),
				reason));

		REPORT_LOG(
			true,
			(
				"GameServer %d (pid %d) shutdown, reason: %s\n",
				static_cast<int>(getProcessId()),
				static_cast<int>(Os::getProcessId()),
				reason));

		va_end(ap);
		m_done = true;
	}
}

// ----------------------------------------------------------------------

void GameServer::setDatabaseProcessConnection(DatabaseProcessConnection *newDatabaseProcessConnection)
{
	if (newDatabaseProcessConnection)
	{
		FATAL(newDatabaseProcessConnection != m_pendingDatabaseProcessConnection, ("Database process connection established but does not match pending?"));
		m_databaseProcessConnection = newDatabaseProcessConnection;
		m_pendingDatabaseProcessConnection = 0;
		m_databaseProcessId = newDatabaseProcessConnection->getPid();

		onDatabaseProcessConnectionEstablished();
	}
	else
	{
		m_databaseProcessConnection = 0;
		m_pendingDatabaseProcessConnection = 0;
	}
}

// ----------------------------------------------------------------------

uint32 GameServer::getDatabaseProcessId() const
{
	return m_databaseProcessId;
}

// ----------------------------------------------------------------------

void GameServer::handleCSRequest( GameServerCSRequestMessage & message )
{
	DEBUG_REPORT_LOG( true, ( "HandleCSRequest:GameServer\n" ) );
}

// ----------------------------------------------------------------------

bool GameServer::addPendingLoadRequest(NetworkId const & id)
{
	if (isAtPendingLoadRequestLimit())
		return false;

	if (s_pendingLoadRequests.find(id) != s_pendingLoadRequests.end())
		return false;

	s_pendingLoadRequests[id] = (unsigned int)::time(nullptr);

	return true;
}

// ----------------------------------------------------------------------

void GameServer::removePendingLoadRequest(NetworkId const & id)
{
	s_pendingLoadRequests.erase(id);
}

// ----------------------------------------------------------------------

bool GameServer::isAtPendingLoadRequestLimit()
{
	return s_pendingLoadRequests.size() >= static_cast<unsigned int>(getPendingLoadRequestLimit());
}

// ----------------------------------------------------------------------

int GameServer::getPendingLoadRequestLimit()
{
	static int pendingLoadRequestLimit = -1;

	if (pendingLoadRequestLimit == -1)
	{
		pendingLoadRequestLimit = ConfigServerGame::getPendingLoadRequestLimit();

		// see if there's a pending load limit set for this particular game server
		char key[128];
		snprintf(key, sizeof(key)-1, "pendingLoadRequestLimit_%s.%lu", ServerWorld::getSceneId().c_str(), GameServer::getInstance().getPreloadAreaId());
		key[sizeof(key)-1] = '\0';

		if (GameServerNamespace::getConfigSetting("GameServer", key, pendingLoadRequestLimit))
			return pendingLoadRequestLimit;

		// see if there's a pending load limit set for this particular planet
		snprintf(key, sizeof(key)-1, "pendingLoadRequestLimit_%s", ServerWorld::getSceneId().c_str());
		key[sizeof(key)-1] = '\0';

		if (GameServerNamespace::getConfigSetting("GameServer", key, pendingLoadRequestLimit))
			return pendingLoadRequestLimit;

		// use default server pending load limit
		pendingLoadRequestLimit = ConfigServerGame::getPendingLoadRequestLimit();
	}

	return pendingLoadRequestLimit;
}

// ----------------------------------------------------------------------

int GameServer::getNumberOfPendingLoadRequests()
{
	return s_pendingLoadRequests.size();
}

// ----------------------------------------------------------------------

unsigned long GameServer::getOldestPendingLoadRequestTime(NetworkId & id)
{
	unsigned long oldestTime = std::numeric_limits<unsigned long>::max();

	std::map<NetworkId, unsigned long>::const_iterator i = s_pendingLoadRequests.begin();
	for (; i != s_pendingLoadRequests.end(); ++i)
		if (i->second < oldestTime)
		{
			id = i->first;
			oldestTime = i->second;
		}

	return oldestTime;
}

// ----------------------------------------------------------------------

ShipObject *GameServerNamespace::getAttachedShip(CreatureObject *creature)
{
	// The "attached" ship is the ship which must move along with the creature.
	// This means if the creature is piloting, the ship is is piloting,
	// or the containing ship if the creature is the ship's owner.
	ShipObject * const ship = ShipObject::getContainingShipObject(creature);
	if (   ship
	    && (   ship->getOwnerId() == creature->getNetworkId()
	        || creature->getPilotedShip() == ship))
		return ship;
	return 0;
}

// ----------------------------------------------------------------------

void GameServerNamespace::loadRetroactiveCtsHistory()
{
	FATAL(GameServer::getInstance().getClusterName().empty(), ("cannot load retroactive CTS history until the cluster has received its cluster name"));

	static bool alreadyDone = false;
	if (alreadyDone)
		return;

	alreadyDone = true;

	// set up data to retroactive CTS history
	std::string const retroactiveCtsHistoryFilename = ConfigServerGame::getRetroactiveCtsHistoryFilename();
	if (!retroactiveCtsHistoryFilename.empty())
	{
		DataTable * table = DataTableManager::getTable(retroactiveCtsHistoryFilename, true);
		if (table)
		{
			int const columnTransferTime = table->findColumnNumber("transfer time epoch");

			int const columnSourceCluster = table->findColumnNumber("source cluster");
			int const columnSourceCharacterId = table->findColumnNumber("source character id");
			int const columnSourceCharacterName = table->findColumnNumber("source character name");
			int const columnSourceCharacterBornDate = table->findColumnNumber("source character born date");

			int const columnTargetCluster = table->findColumnNumber("target cluster");
			int const columnTargetCharacterId = table->findColumnNumber("target character id");

			FATAL((columnTransferTime < 0), ("column \"transfer time epoch\" not found in %s", retroactiveCtsHistoryFilename.c_str()));

			FATAL((columnSourceCluster < 0), ("column \"source cluster\" not found in %s", retroactiveCtsHistoryFilename.c_str()));
			FATAL((columnSourceCharacterId < 0), ("column \"source character id\" not found in %s", retroactiveCtsHistoryFilename.c_str()));
			FATAL((columnSourceCharacterName < 0), ("column \"source character name\" not found in %s", retroactiveCtsHistoryFilename.c_str()));
			FATAL((columnSourceCharacterBornDate < 0), ("column \"source character born date\" not found in %s", retroactiveCtsHistoryFilename.c_str()));

			FATAL((columnTargetCluster < 0), ("column \"target cluster\" not found in %s", retroactiveCtsHistoryFilename.c_str()));
			FATAL((columnTargetCharacterId < 0), ("column \"target character id\" not found in %s", retroactiveCtsHistoryFilename.c_str()));

			// read in retroactive objvars; these are stored in a variable number of
			// colunms that are named in ascending order, so search for all such columns
			std::vector<int> columnsObjvarName;
			std::vector<int> columnsObjvarType;
			std::vector<int> columnsObjvarValue;

			char buffer[128];
			int columnNumber;
			for (int i = 1; i <= 1000000000; ++i)
			{
				snprintf(buffer, sizeof(buffer)-1, "objvar name %d", i);
				buffer[sizeof(buffer)-1] = '\0';

				columnNumber = table->findColumnNumber(buffer);
				if (columnNumber < 0)
					break;

				columnsObjvarName.push_back(columnNumber);

				snprintf(buffer, sizeof(buffer)-1, "objvar type %d", i);
				buffer[sizeof(buffer)-1] = '\0';

				columnNumber = table->findColumnNumber(buffer);
				FATAL((columnNumber < 0), ("column \"%s\" not found in %s", buffer, retroactiveCtsHistoryFilename.c_str()));

				columnsObjvarType.push_back(columnNumber);

				snprintf(buffer, sizeof(buffer)-1, "objvar value %d", i);
				buffer[sizeof(buffer)-1] = '\0';

				columnNumber = table->findColumnNumber(buffer);
				FATAL((columnNumber < 0), ("column \"%s\" not found in %s", buffer, retroactiveCtsHistoryFilename.c_str()));

				columnsObjvarValue.push_back(columnNumber);
			}

			// CTS data can also come from config file, so read that in, if any
			std::list<std::string> ctsDataFromConfig;
			
			{
				int index = 0;
				char const * pszCtsDataFromConfig = nullptr;
				do
				{
					pszCtsDataFromConfig = ConfigFile::getKeyString("GameServer", "retroactiveCtsHistory", index++, nullptr);
					if (pszCtsDataFromConfig != nullptr)
					{
						ctsDataFromConfig.push_back(pszCtsDataFromConfig);
					}
				}
				while (pszCtsDataFromConfig);
			}

			CtsSourceCharacterInfo sourceCharacterInfo;
			std::string retroactiveObjvarName;
			int retroactiveObjvarType;
			std::string retroactiveObjvarValue;
			std::string targetCluster;
			NetworkId targetCharacterId;

			//                                      transfer time, source character info
			//                          target cluster
			//           target character id
			//                    \/          \/                   \/
			std::map<std::pair<NetworkId, std::string>, CtsSourceCharacterInfo> allCtsHistoryList;

#ifdef _DEBUG
			std::set<std::string> allCtsSourceCluster;
#endif

			int const currentBornDate = PlayerObject::getCurrentBornDate();

			std::list<std::string>::const_iterator iterCtsDataFromConfig = ctsDataFromConfig.begin();
			std::list<std::string>::const_iterator const iterCtsDataFromConfigEnd = ctsDataFromConfig.end();
			Unicode::String const ctsDataFromConfigDelimiter(Unicode::narrowToWide("|"));
			Unicode::UnicodeStringVector ctsDataFromConfigTokens;
			std::string ctsDataFromConfigParsedData;

			int const numRows = table->getNumRows();
			int rowIndex = 0;

			while (true)
			{
				if (rowIndex < numRows)
				{
					sourceCharacterInfo.transferTime = static_cast<time_t>(table->getIntValue(columnTransferTime, rowIndex));

					sourceCharacterInfo.sourceCluster = table->getStringValue(columnSourceCluster, rowIndex);
					sourceCharacterInfo.sourceCharacterId = NetworkId(table->getStringValue(columnSourceCharacterId, rowIndex));
					sourceCharacterInfo.sourceCharacterName = table->getStringValue(columnSourceCharacterName, rowIndex);
					sourceCharacterInfo.sourceCharacterBornDate = table->getIntValue(columnSourceCharacterBornDate, rowIndex);

					targetCluster = table->getStringValue(columnTargetCluster, rowIndex);
					targetCharacterId = NetworkId(table->getStringValue(columnTargetCharacterId, rowIndex));

					// read retroactive objvars
					{
						sourceCharacterInfo.sourceCharacterRetroactiveObjvars.clear();

						for (size_t i = 0, size = columnsObjvarName.size(); i < size; ++i)
						{
							retroactiveObjvarName = table->getStringValue(columnsObjvarName[i], rowIndex);
							retroactiveObjvarType = table->getIntValue(columnsObjvarType[i], rowIndex);
							retroactiveObjvarValue = table->getStringValue(columnsObjvarValue[i], rowIndex);

							if (!retroactiveObjvarName.empty() && !retroactiveObjvarValue.empty())
							{
								// can only handle INT (type 0) and INT_ARRAY (type 1) objvar type at this time
								FATAL(((retroactiveObjvarType != static_cast<int>(DynamicVariable::INT)) && (retroactiveObjvarType != static_cast<int>(DynamicVariable::INT_ARRAY))), ("invalid or unsupported objvar type %d in %s, row %d", retroactiveObjvarType, retroactiveCtsHistoryFilename.c_str(), (rowIndex+3)));
								sourceCharacterInfo.sourceCharacterRetroactiveObjvars.push_back(std::make_pair(retroactiveObjvarName, DynamicVariable()));
								sourceCharacterInfo.sourceCharacterRetroactiveObjvars[sourceCharacterInfo.sourceCharacterRetroactiveObjvars.size() - 1].second.load(-1, retroactiveObjvarType, Unicode::narrowToWide(retroactiveObjvarValue));
							}
						}
					}

					++rowIndex;
				}
				else if (iterCtsDataFromConfig == iterCtsDataFromConfigEnd)
				{
					break;
				}
				else
				{
					ctsDataFromConfigTokens.clear();
					if ((Unicode::tokenize(Unicode::narrowToWide(*iterCtsDataFromConfig), ctsDataFromConfigTokens, &ctsDataFromConfigDelimiter, nullptr)) && (ctsDataFromConfigTokens.size() == 7))
					{
						// sanity check
						ctsDataFromConfigParsedData = FormattedString<2048>().sprintf("%s|%s|%s|%s|%s|%s|%s", Unicode::wideToNarrow(ctsDataFromConfigTokens[0]).c_str(), Unicode::wideToNarrow(ctsDataFromConfigTokens[1]).c_str(), Unicode::wideToNarrow(ctsDataFromConfigTokens[2]).c_str(), Unicode::wideToNarrow(ctsDataFromConfigTokens[3]).c_str(), Unicode::wideToNarrow(ctsDataFromConfigTokens[4]).c_str(), Unicode::wideToNarrow(ctsDataFromConfigTokens[5]).c_str(), Unicode::wideToNarrow(ctsDataFromConfigTokens[6]).c_str());
						FATAL((ctsDataFromConfigParsedData != *iterCtsDataFromConfig), ("parsed CTS data (%s) does not equal read CTS data (%s)", ctsDataFromConfigParsedData.c_str(), iterCtsDataFromConfig->c_str()));

						sourceCharacterInfo.transferTime = static_cast<time_t>(::atol(Unicode::wideToNarrow(ctsDataFromConfigTokens[0]).c_str()));

						sourceCharacterInfo.sourceCluster = Unicode::wideToNarrow(ctsDataFromConfigTokens[1]);
						sourceCharacterInfo.sourceCharacterId = NetworkId(Unicode::wideToNarrow(ctsDataFromConfigTokens[2]));
						sourceCharacterInfo.sourceCharacterName = Unicode::wideToNarrow(ctsDataFromConfigTokens[3]);
						sourceCharacterInfo.sourceCharacterBornDate = ::atoi(Unicode::wideToNarrow(ctsDataFromConfigTokens[4]).c_str());

						targetCluster = Unicode::wideToNarrow(ctsDataFromConfigTokens[5]);
						targetCharacterId = NetworkId(Unicode::wideToNarrow(ctsDataFromConfigTokens[6]));

						sourceCharacterInfo.sourceCharacterRetroactiveObjvars.clear();
					}
					else
					{
						FATAL(true, ("cannot parse CTS data (%s) read from config file", iterCtsDataFromConfig->c_str()));
					}

					++iterCtsDataFromConfig;
				}

				FATAL(((sourceCharacterInfo.sourceCharacterBornDate > 0) && (sourceCharacterInfo.sourceCharacterBornDate < 907)), ("source character (%s, %s) has born date (%d) < 907", sourceCharacterInfo.sourceCluster.c_str(), sourceCharacterInfo.sourceCharacterId.getValueString().c_str(), sourceCharacterInfo.sourceCharacterBornDate));
				FATAL((sourceCharacterInfo.sourceCharacterBornDate > currentBornDate), ("source character (%s, %s) has born date (%d) > current born date (%d)", sourceCharacterInfo.sourceCluster.c_str(), sourceCharacterInfo.sourceCharacterId.getValueString().c_str(), sourceCharacterInfo.sourceCharacterBornDate, currentBornDate));

				std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > * clusterCtsHistory = nullptr;
#ifdef _DEBUG
				IGNORE_RETURN(allCtsSourceCluster.insert(sourceCharacterInfo.sourceCluster));
				clusterCtsHistory = &(s_retroactiveCtsHistoryList[targetCluster]);
#else
				// for release build, we only care about transfers into our cluster;
				// for debug build, read them all so we can test out the logic to
				// make sure it works for all clusters
				if (targetCluster == GameServer::getInstance().getClusterName())
					clusterCtsHistory = &s_retroactiveCtsHistoryListThisCluster;
#endif
				if (clusterCtsHistory)
				{
					if (clusterCtsHistory->count(targetCharacterId) <= 0)
					{
						(*clusterCtsHistory)[targetCharacterId].push_back(sourceCharacterInfo);
					}
				}

				std::map<std::pair<NetworkId, std::string>, CtsSourceCharacterInfo>::const_iterator const iterFindCtsHistory = allCtsHistoryList.find(std::make_pair(targetCharacterId, targetCluster));
				if (iterFindCtsHistory == allCtsHistoryList.end())
				{
					allCtsHistoryList[std::make_pair(targetCharacterId, targetCluster)] = sourceCharacterInfo;
				}
				else
				{
					FATAL(((iterFindCtsHistory->second.sourceCharacterId != sourceCharacterInfo.sourceCharacterId) || (iterFindCtsHistory->second.sourceCluster != sourceCharacterInfo.sourceCluster)), ("character (%s, %s) has 2 transfers from different source characters (%s, %s, %s, %ld, %s) and (%s, %s, %s, %ld, %s)", targetCluster.c_str(), targetCharacterId.getValueString().c_str(), iterFindCtsHistory->second.sourceCluster.c_str(), iterFindCtsHistory->second.sourceCharacterId.getValueString().c_str(), iterFindCtsHistory->second.sourceCharacterName.c_str(), iterFindCtsHistory->second.transferTime, CalendarTime::convertEpochToTimeStringLocal(iterFindCtsHistory->second.transferTime).c_str(), sourceCharacterInfo.sourceCluster.c_str(), sourceCharacterInfo.sourceCharacterId.getValueString().c_str(), sourceCharacterInfo.sourceCharacterName.c_str(), sourceCharacterInfo.transferTime, CalendarTime::convertEpochToTimeStringLocal(sourceCharacterInfo.transferTime).c_str()));
				}
			}

			DataTableManager::close(retroactiveCtsHistoryFilename);

#ifdef _DEBUG
			{
				// make sure there are collection slots defined for CTS titles
				for (std::set<std::string>::const_iterator iter = allCtsSourceCluster.begin(); iter != allCtsSourceCluster.end(); ++iter)
				{
					std::string collectionSlotName = std::string("cts_from_") + Unicode::toLower(*iter);

					std::string::size_type pos = collectionSlotName.find('-');
					while (pos != std::string::npos)
					{
						collectionSlotName.replace(pos, 1, 1, '_');
						pos = collectionSlotName.find('-');
					}

					DEBUG_WARNING(!CollectionsDataTable::getSlotByName(collectionSlotName), ("no collection slot defined for CTS title (%s)", collectionSlotName.c_str()));
				}
			}
#endif

			// build CTS history for characters who have transferred multiple times

			//                          source cluster
			//           source character id
			//                    \/          \/
			std::set<std::pair<NetworkId, std::string> > ctsHistorySourceCharacters;

#ifdef _DEBUG
			for (std::map<std::string, std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > >::iterator iterCtsHistoryCluster = s_retroactiveCtsHistoryList.begin(); iterCtsHistoryCluster != s_retroactiveCtsHistoryList.end(); ++iterCtsHistoryCluster)
			{
				std::string const & ctsHistoryClusterName = iterCtsHistoryCluster->first;
				std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > & ctsHistoryCluster = iterCtsHistoryCluster->second;

#else
			{
				std::string const & ctsHistoryClusterName = GameServer::getInstance().getClusterName();
				std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > & ctsHistoryCluster = s_retroactiveCtsHistoryListThisCluster;
#endif
				for (std::map<NetworkId, std::vector<CtsSourceCharacterInfo> >::iterator iterCtsHistoryCharacter = ctsHistoryCluster.begin(); iterCtsHistoryCharacter != ctsHistoryCluster.end(); ++iterCtsHistoryCharacter)
				{
					FATAL((iterCtsHistoryCharacter->second.size() != 1), ("character (%s, %s) should have exactly 1 CTS history at this point in the parsing", ctsHistoryClusterName.c_str(), iterCtsHistoryCharacter->first.getValueString().c_str()));
					CtsSourceCharacterInfo const & ctsHistoryLastSourceCharacter = iterCtsHistoryCharacter->second[0];

					ctsHistorySourceCharacters.clear();
					IGNORE_RETURN(ctsHistorySourceCharacters.insert(std::make_pair(ctsHistoryLastSourceCharacter.sourceCharacterId, ctsHistoryLastSourceCharacter.sourceCluster)));

					std::map<std::pair<NetworkId, std::string>, CtsSourceCharacterInfo>::const_iterator iterFindCharacterCtsHistory = allCtsHistoryList.find(std::make_pair(ctsHistoryLastSourceCharacter.sourceCharacterId, ctsHistoryLastSourceCharacter.sourceCluster));
					while (iterFindCharacterCtsHistory != allCtsHistoryList.end())
					{
						FATAL((ctsHistorySourceCharacters.count(std::make_pair(iterFindCharacterCtsHistory->second.sourceCharacterId, iterFindCharacterCtsHistory->second.sourceCluster)) > 0), ("character (%s, %s) has multiple CTS history for the same source character (%s, %s, %s)", ctsHistoryClusterName.c_str(), iterCtsHistoryCharacter->first.getValueString().c_str(), iterFindCharacterCtsHistory->second.sourceCluster.c_str(), iterFindCharacterCtsHistory->second.sourceCharacterId.getValueString().c_str(), iterFindCharacterCtsHistory->second.sourceCharacterName.c_str()));
						IGNORE_RETURN(ctsHistorySourceCharacters.insert(std::make_pair(iterFindCharacterCtsHistory->second.sourceCharacterId, iterFindCharacterCtsHistory->second.sourceCluster)));

						FATAL((iterCtsHistoryCharacter->second[iterCtsHistoryCharacter->second.size() - 1].transferTime <= iterFindCharacterCtsHistory->second.transferTime), ("character (%s, %s) has out of order CTS history for source characters (%s, %s, %s, %ld, %s) and (%s, %s, %s, %ld, %s)", ctsHistoryClusterName.c_str(), iterCtsHistoryCharacter->first.getValueString().c_str(), iterCtsHistoryCharacter->second[iterCtsHistoryCharacter->second.size() - 1].sourceCluster.c_str(), iterCtsHistoryCharacter->second[iterCtsHistoryCharacter->second.size() - 1].sourceCharacterId.getValueString().c_str(), iterCtsHistoryCharacter->second[iterCtsHistoryCharacter->second.size() - 1].sourceCharacterName.c_str(), iterCtsHistoryCharacter->second[iterCtsHistoryCharacter->second.size() - 1].transferTime, CalendarTime::convertEpochToTimeStringLocal(iterCtsHistoryCharacter->second[iterCtsHistoryCharacter->second.size() - 1].transferTime).c_str(), iterFindCharacterCtsHistory->second.sourceCluster.c_str(), iterFindCharacterCtsHistory->second.sourceCharacterId.getValueString().c_str(), iterFindCharacterCtsHistory->second.sourceCharacterName.c_str(), iterFindCharacterCtsHistory->second.transferTime, CalendarTime::convertEpochToTimeStringLocal(iterFindCharacterCtsHistory->second.transferTime).c_str()));
						iterCtsHistoryCharacter->second.push_back(iterFindCharacterCtsHistory->second);

						iterFindCharacterCtsHistory = allCtsHistoryList.find(std::make_pair(iterFindCharacterCtsHistory->second.sourceCharacterId, iterFindCharacterCtsHistory->second.sourceCluster));
					}
				}
			}

#ifdef _DEBUG
			{
				// make sure that we have the born date of the "root" source character for each transfer
				for (std::map<std::string, std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > >::const_iterator iterCtsHistoryCluster = s_retroactiveCtsHistoryList.begin(); iterCtsHistoryCluster != s_retroactiveCtsHistoryList.end(); ++iterCtsHistoryCluster)
				{
					std::string const & ctsHistoryClusterName = iterCtsHistoryCluster->first;
					std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > const & ctsHistoryCluster = iterCtsHistoryCluster->second;

					for (std::map<NetworkId, std::vector<CtsSourceCharacterInfo> >::const_iterator iterCtsHistoryCharacter = ctsHistoryCluster.begin(); iterCtsHistoryCharacter != ctsHistoryCluster.end(); ++iterCtsHistoryCharacter)
					{
						FATAL(iterCtsHistoryCharacter->second.empty(), ("character (%s, %s) has empty CTS history list", ctsHistoryClusterName.c_str(), iterCtsHistoryCharacter->first.getValueString().c_str()));

						// we only care about missing born date for transfers on or before 
						// Wed Nov 19 23:21:11 2008 Pacific Standard Time (epoch 1227165671)
						// since transfers after that should correctly transfer the born date
						FATAL((((iterCtsHistoryCharacter->second[iterCtsHistoryCharacter->second.size() - 1]).sourceCharacterBornDate <= 0) && ((iterCtsHistoryCharacter->second[iterCtsHistoryCharacter->second.size() - 1]).transferTime <= 1227165671)), ("character (%s, %s) has no born date for the \"root\" source character (%s, %s)", ctsHistoryClusterName.c_str(), iterCtsHistoryCharacter->first.getValueString().c_str(), (iterCtsHistoryCharacter->second[iterCtsHistoryCharacter->second.size() - 1]).sourceCluster.c_str(), (iterCtsHistoryCharacter->second[iterCtsHistoryCharacter->second.size() - 1]).sourceCharacterId.getValueString().c_str()));

						// if the character has transferred multiple times, make sure that only the "root" source character has a born date
						if (iterCtsHistoryCharacter->second.size() > 1)
						{
							for (size_t i = 0, sizeExcludingLastElement = (iterCtsHistoryCharacter->second.size() - 1); i < sizeExcludingLastElement; ++i)
							{
								FATAL(((iterCtsHistoryCharacter->second[i]).sourceCharacterBornDate > 0), ("character (%s, %s) has born date (%d) for a \"non root\" source character (%s, %s)", ctsHistoryClusterName.c_str(), iterCtsHistoryCharacter->first.getValueString().c_str(), (iterCtsHistoryCharacter->second[i]).sourceCharacterBornDate, (iterCtsHistoryCharacter->second[i]).sourceCluster.c_str(), (iterCtsHistoryCharacter->second[i]).sourceCharacterId.getValueString().c_str()));
							}
						}
					}
				}
			}
#endif
		}
	}
}

// ----------------------------------------------------------------------

void GameServerNamespace::loadRetroactivePlayerCityCreationTime()
{
	FATAL(GameServer::getInstance().getClusterName().empty(), ("cannot load retroactive player city creation time until the cluster has received its cluster name"));

	static bool alreadyDone = false;
	if (alreadyDone)
		return;

	alreadyDone = true;

	// set up data to player city creation time
	std::string const retroactivePlayerCityCreationTimeFilename = ConfigServerGame::getRetroactivePlayerCityCreationTimeFilename();
	if (!retroactivePlayerCityCreationTimeFilename.empty())
	{
		DataTable * table = DataTableManager::getTable(retroactivePlayerCityCreationTimeFilename, true);
		if (table)
		{
			int const columnCreationTime = table->findColumnNumber("creation time epoch");
			int const columnCluster = table->findColumnNumber("cluster");
			int const columnCityId = table->findColumnNumber("city id");

			FATAL((columnCreationTime < 0), ("column \"creation time epoch\" not found in %s", retroactivePlayerCityCreationTimeFilename.c_str()));
			FATAL((columnCluster < 0), ("column \"cluster\" not found in %s", retroactivePlayerCityCreationTimeFilename.c_str()));
			FATAL((columnCityId < 0), ("column \"city id\" not found in %s", retroactivePlayerCityCreationTimeFilename.c_str()));

			// player city creation time data can also come from config file, so read that in, if any
			std::list<std::string> playerCityCreationTimeDataFromConfig;

			{
				int index = 0;
				char const * pszPlayerCityCreationTimeDataFromConfig = nullptr;
				do
				{
					pszPlayerCityCreationTimeDataFromConfig = ConfigFile::getKeyString("GameServer", "retroactivePlayerCityCreationTime", index++, nullptr);
					if (pszPlayerCityCreationTimeDataFromConfig != nullptr)
					{
						playerCityCreationTimeDataFromConfig.push_back(pszPlayerCityCreationTimeDataFromConfig);
					}
				}
				while (pszPlayerCityCreationTimeDataFromConfig);
			}

			std::list<std::string>::const_iterator iterPlayerCityCreationTimeDataFromConfig = playerCityCreationTimeDataFromConfig.begin();
			std::list<std::string>::const_iterator const iterPlayerCityCreationTimeDataFromConfigEnd = playerCityCreationTimeDataFromConfig.end();
			Unicode::String const playerCityCreationTimeDataFromConfigDelimiter(Unicode::narrowToWide("|"));
			Unicode::UnicodeStringVector playerCityCreationTimeDataFromConfigTokens;
			std::string playerCityCreationTimeDataFromConfigParsedData;

			time_t creationTime = 0;
			std::string cluster;
			int cityId = 0;

			int const numRows = table->getNumRows();
			int rowIndex = 0;

			while (true)
			{
				if (rowIndex < numRows)
				{
					creationTime = static_cast<time_t>(table->getIntValue(columnCreationTime, rowIndex));
					cluster = table->getStringValue(columnCluster, rowIndex);
					cityId = table->getIntValue(columnCityId, rowIndex);

					FATAL((cluster.empty()), ("cluster name is empty in %s, row %d", retroactivePlayerCityCreationTimeFilename.c_str(), (rowIndex+3)));
					FATAL((cityId <= 0), ("city id for player city (%s, %d) is <= 0 in %s, row %d", cluster.c_str(), cityId, retroactivePlayerCityCreationTimeFilename.c_str(), (rowIndex+3)));
					FATAL((creationTime <= 0), ("creation time (%d) for player city (%s, %d) is <= 0 in %s, row %d", creationTime, cluster.c_str(), cityId, retroactivePlayerCityCreationTimeFilename.c_str(), (rowIndex+3)));

					++rowIndex;
				}
				else if (iterPlayerCityCreationTimeDataFromConfig == iterPlayerCityCreationTimeDataFromConfigEnd)
				{
					break;
				}
				else
				{
					playerCityCreationTimeDataFromConfigTokens.clear();
					if ((Unicode::tokenize(Unicode::narrowToWide(*iterPlayerCityCreationTimeDataFromConfig), playerCityCreationTimeDataFromConfigTokens, &playerCityCreationTimeDataFromConfigDelimiter, nullptr)) && (playerCityCreationTimeDataFromConfigTokens.size() == 3))
					{
						// sanity check
						playerCityCreationTimeDataFromConfigParsedData = FormattedString<2048>().sprintf("%s|%s|%s", Unicode::wideToNarrow(playerCityCreationTimeDataFromConfigTokens[0]).c_str(), Unicode::wideToNarrow(playerCityCreationTimeDataFromConfigTokens[1]).c_str(), Unicode::wideToNarrow(playerCityCreationTimeDataFromConfigTokens[2]).c_str());
						FATAL((playerCityCreationTimeDataFromConfigParsedData != *iterPlayerCityCreationTimeDataFromConfig), ("parsed player city creation time data (%s) does not equal read player city creation time data (%s)", playerCityCreationTimeDataFromConfigParsedData.c_str(), iterPlayerCityCreationTimeDataFromConfig->c_str()));

						creationTime = static_cast<time_t>(::atol(Unicode::wideToNarrow(playerCityCreationTimeDataFromConfigTokens[0]).c_str()));
						cluster = Unicode::wideToNarrow(playerCityCreationTimeDataFromConfigTokens[1]);
						cityId = ::atoi(Unicode::wideToNarrow(playerCityCreationTimeDataFromConfigTokens[2]).c_str());

						FATAL((cluster.empty()), ("cluster name is empty in config option (%s)", iterPlayerCityCreationTimeDataFromConfig->c_str()));
						FATAL((cityId <= 0), ("city id for player city (%s, %d) is <= 0 in config option (%s)", cluster.c_str(), cityId, iterPlayerCityCreationTimeDataFromConfig->c_str()));
						FATAL((creationTime <= 0), ("creation time (%d) for player city (%s, %d) is <= 0 in config option (%s)", creationTime, cluster.c_str(), cityId, iterPlayerCityCreationTimeDataFromConfig->c_str()));
					}
					else
					{
						FATAL(true, ("cannot parse player city creation time data (%s) read from config file", iterPlayerCityCreationTimeDataFromConfig->c_str()));
					}

					++iterPlayerCityCreationTimeDataFromConfig;
				}

				std::map<int, time_t> * clusterPlayerCityCreationTimeHistory = nullptr;
#ifdef _DEBUG
				clusterPlayerCityCreationTimeHistory = &(s_retroactivePlayerCityCreationTime[cluster]);
#else
				// for release build, we only care about player cities created
				// on our cluster cluster; for debug build, read them all so we
				// can test out the logic to make sure it works for all clusters
				if (cluster == GameServer::getInstance().getClusterName())
					clusterPlayerCityCreationTimeHistory = &s_retroactivePlayerCityCreationTimeThisCluster;
#endif
				if (clusterPlayerCityCreationTimeHistory)
				{
					FATAL((clusterPlayerCityCreationTimeHistory->count(cityId) > 0), ("multiple entries found for player city (%s, %d)", cluster.c_str(), cityId));
					(*clusterPlayerCityCreationTimeHistory)[cityId] = creationTime;
				}
			}

			DataTableManager::close(retroactivePlayerCityCreationTimeFilename);
		}
	}
}

// ----------------------------------------------------------------------

bool GameServerNamespace::checkAndSetOutstandingRequestSceneTransfer(ServerObject & object)
{
	int const timeNow = static_cast<int>(::time(nullptr));

	// don't send multiple RequestSceneTransfer message
	if (object.getObjVars().hasItem(OBJVAR_HAS_OUTSTANDING_REQUEST_SCENE_TRANSFER) && (object.getObjVars().getType(OBJVAR_HAS_OUTSTANDING_REQUEST_SCENE_TRANSFER) == DynamicVariable::INT))
	{
		int requestTimeOut = 0;
		if (object.getObjVars().getItem(OBJVAR_HAS_OUTSTANDING_REQUEST_SCENE_TRANSFER, requestTimeOut))
		{
			if (timeNow <= requestTimeOut)
			{
				LOG("RequestSceneWarp", ("%s already has an outstanding RequestSceneTransfer that will expire in %d seconds", PlayerObject::getAccountDescription(&object).c_str(), (requestTimeOut - timeNow)));
				return true;
			}
			else
			{
				LOG("RequestSceneWarp", ("%s has an expired RequestSceneTransfer, so we are allowing a new RequestSceneTransfer", PlayerObject::getAccountDescription(&object).c_str()));
			}
		}
	}

	IGNORE_RETURN(object.setObjVarItem(OBJVAR_HAS_OUTSTANDING_REQUEST_SCENE_TRANSFER, (timeNow + ConfigServerGame::getRequestSceneWarpTimeoutSeconds())));

	return false;
}

// ----------------------------------------------------------------------

void GameServerNamespace::handleFactionalSystemMessage(FactionalSystemMessage const & msg)
{
	// is it a ranged message?
	if (msg.getRadius() >= 0.0f)
	{
		TangibleObject * to;
		std::vector<ServerObject *> playersInRange;
		ServerWorld::findPlayerCreaturesInRange(msg.getLocation(), msg.getRadius(), playersInRange);
		for (std::vector<ServerObject *>::const_iterator i = playersInRange.begin(); i != playersInRange.end(); ++i)
		{
			if ((*i)->isAuthoritative() && (*i)->getClient())
			{
				to = (*i)->asTangibleObject();
				if (to &&
					((msg.getNotifyImperial() && PvpData::isImperialFactionId(to->getPvpFaction())) ||
					(msg.getNotifyRebel() && PvpData::isRebelFactionId(to->getPvpFaction()))
					)
					)
				{
					Chat::sendSystemMessage(**i, Unicode::String(), msg.getProsePackage());
				}
			}
		}
	}
	else
	{
		// not a ranged message, everyone gets it
		std::set<PlayerObject const *> const &players = PlayerObject::getAllPlayerObjects();
		if (!players.empty())
		{
			CreatureObject const * creatureObject;
			for (std::set<PlayerObject const *>::const_iterator i = players.begin(); i != players.end(); ++i)
			{
				creatureObject = (*i)->getCreatureObject();
				if (creatureObject && creatureObject->isAuthoritative() && creatureObject->getClient())
				{
					if ((msg.getNotifyImperial() && PvpData::isImperialFactionId(creatureObject->getPvpFaction())) ||
						(msg.getNotifyRebel() && PvpData::isRebelFactionId(creatureObject->getPvpFaction()))
						)
					{
						Chat::sendSystemMessage(*creatureObject, Unicode::String(), msg.getProsePackage());
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void GameServerNamespace::handleMessageToPlayersOnPlanet(std::string const & methodName, std::vector<int8> const & params, float const delay, Vector const & loc, float const radius, bool const includeDisconnectedPlayers)
{
	// we cannot use ServerWorld::findPlayerCreaturesInRange() because that only looks at creatures
	// that are in the world, and disconnected players creatures are removed from the world
	std::set<PlayerObject const *> const &players = PlayerObject::getAllPlayerObjects();
	if (!players.empty())
	{
		float const radiusSquared = sqr(radius);
		CreatureObject const * creatureObject;
		for (std::set<PlayerObject const *>::const_iterator i = players.begin(); i != players.end(); ++i)
		{
			creatureObject = (*i)->getCreatureObject();
			if (creatureObject && creatureObject->isAuthoritative() && (includeDisconnectedPlayers || creatureObject->getClient()))
			{
				// check range, if necessary
				if ((radius < 0.0f) || (loc.magnitudeBetweenSquared(creatureObject->getPosition_w()) < radiusSquared))
				{
					MessageToQueue::getInstance().sendMessageToJava(creatureObject->getNetworkId(), methodName, params, static_cast<int>(delay), false);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void GameServerNamespace::handleMessageLocateStructureByOwnerIdReq(GameServer & gsInstance, uint32 responsePid, bool isGod, NetworkId const & ownerId, NetworkId const & ownerResidenceId, NetworkId const & responseId)
{
	static std::string const planetName(ServerWorld::getSceneId());
	static std::string const displayPlanetName(Unicode::wideToNarrow(StringId("planet_n", planetName).localize()));
	std::vector<std::string> response;

	// buildings
	{
		BuildingObject::AllBuildingsSet const & allBuildings = BuildingObject::getAllBuildings();
		for (BuildingObject::AllBuildingsSet::const_iterator iterBuilding = allBuildings.begin(); iterBuilding != allBuildings.end(); ++iterBuilding)
		{
			if ((*iterBuilding)->isAuthoritative() && ((*iterBuilding)->getOwnerId() == ownerId) && (*iterBuilding)->isInWorldCell())
			{
				// player assigned name
				std::string name = Unicode::wideToNarrow((*iterBuilding)->getAssignedObjectName());

				// buildings have their name in the sign
				if (name.empty() && (*iterBuilding)->getObjVars().hasItem("player_structure.sign.name") && ((*iterBuilding)->getObjVars().getType("player_structure.sign.name") == DynamicVariable::STRING))
					IGNORE_RETURN((*iterBuilding)->getObjVars().getItem("player_structure.sign.name", name));

				// default/type name
				Unicode::String nameTypeWide;
				std::string nameTypeNarrow = "Building";
				if ((*iterBuilding)->getObjectNameStringId().localize(nameTypeWide))
					nameTypeNarrow = Unicode::wideToNarrow(nameTypeWide);

				if (!name.empty())
				{
					if (name != nameTypeNarrow)
					{
						name += " (";
						name += nameTypeNarrow;
						name += ")";
					}
				}
				else
				{
					name = nameTypeNarrow;
				}

				// check for residence building
				NetworkId residenceOf;
				if ((*iterBuilding)->getObjVars().getItem("player_structure.residence.building", residenceOf) && (residenceOf == ownerId))
				{					
					if (ownerId == responseId)
					{
						if (ownerResidenceId == (*iterBuilding)->getNetworkId())
						{
							name += " (residence)";
						}
						else
						{
							// house thinks it's the residence of the owner, but owner doesn't think so,
							// so fix up house so it no longer thinks it's the residence of the owner
							(const_cast<BuildingObject *>(*iterBuilding))->removeObjVarItem("player_structure.residence.building");
						}
					}
					else
					{
						name += " (residence)";
					}
				}

				Vector const loc = (*iterBuilding)->findPosition_w();
				std::string cityAndPlanetName;
				int const cityId = CityInterface::getCityAtLocation(planetName, static_cast<int>(loc.x), static_cast<int>(loc.z), 0);
				if (cityId > 0)
					cityAndPlanetName = CityInterface::getCityInfo(cityId).getCityName();

				if (!cityAndPlanetName.empty())
					cityAndPlanetName += ", ";

				cityAndPlanetName += displayPlanetName;

				Region const * const region = RegionMaster::getSmallestVisibleRegionAtPoint(planetName, loc.x, loc.z);
				if (region)
				{
					cityAndPlanetName += ":";
					cityAndPlanetName += Unicode::wideToNarrow(StringId::decodeString(region->getName()));
				}

				if (!isGod)
					response.push_back(FormattedString<1024>().sprintf("%s at %s (%d, %d)", name.c_str(), cityAndPlanetName.c_str(), static_cast<int>(loc.x), static_cast<int>(loc.z)));
				else
					response.push_back(FormattedString<1024>().sprintf("%s, %s at %s (%d, %d)", (*iterBuilding)->getNetworkId().getValueString().c_str(), name.c_str(), cityAndPlanetName.c_str(), static_cast<int>(loc.x), static_cast<int>(loc.z)));
			}
		}
	}

	// installations, including harvesters and factories
	{
		InstallationObject::AllInstallationsSet const & allInstallations = InstallationObject::getAllInstallations();
		for (ManufactureInstallationObject::AllInstallationsSet::const_iterator iterInstallation = allInstallations.begin(); iterInstallation != allInstallations.end(); ++iterInstallation)
		{
			if ((*iterInstallation)->isAuthoritative() && ((*iterInstallation)->getOwnerId() == ownerId) && (*iterInstallation)->isInWorldCell())
			{
				// player assigned name
				std::string name = Unicode::wideToNarrow((*iterInstallation)->getAssignedObjectName());

				// default/type name
				Unicode::String nameTypeWide;
				std::string nameTypeNarrow;
				if ((*iterInstallation)->getObjectNameStringId().localize(nameTypeWide))
					nameTypeNarrow = Unicode::wideToNarrow(nameTypeWide);
				else if (dynamic_cast<const HarvesterInstallationObject *>(*iterInstallation))
					nameTypeNarrow = "Harvester";
				else if (dynamic_cast<const ManufactureInstallationObject *>(*iterInstallation))
					nameTypeNarrow = "Factory";
				else
					nameTypeNarrow = "Installation";

				if (!name.empty())
				{
					if (name != nameTypeNarrow)
					{
						name += " (";
						name += nameTypeNarrow;
						name += ")";
					}
				}
				else
				{
					name = nameTypeNarrow;
				}

				Vector const loc = (*iterInstallation)->findPosition_w();
				std::string cityAndPlanetName;
				int const cityId = CityInterface::getCityAtLocation(planetName, static_cast<int>(loc.x), static_cast<int>(loc.z), 0);
				if (cityId > 0)
					cityAndPlanetName = CityInterface::getCityInfo(cityId).getCityName();

				if (!cityAndPlanetName.empty())
					cityAndPlanetName += ", ";

				cityAndPlanetName += displayPlanetName;

				Region const * const region = RegionMaster::getSmallestVisibleRegionAtPoint(planetName, loc.x, loc.z);
				if (region)
				{
					cityAndPlanetName += ":";
					cityAndPlanetName += Unicode::wideToNarrow(StringId::decodeString(region->getName()));
				}

				if (!isGod)
					response.push_back(FormattedString<1024>().sprintf("%s at %s (%d, %d)", name.c_str(), cityAndPlanetName.c_str(), static_cast<int>(loc.x), static_cast<int>(loc.z)));
				else
					response.push_back(FormattedString<1024>().sprintf("%s, %s at %s (%d, %d)", (*iterInstallation)->getNetworkId().getValueString().c_str(), name.c_str(), cityAndPlanetName.c_str(), static_cast<int>(loc.x), static_cast<int>(loc.z)));
			}
		}
	}

	// if there are any matches, send reply
	if (!response.empty())
	{
		GenericValueTypeMessage<std::pair<std::pair<uint32, NetworkId>, std::vector<std::string> > > locateStructureByOwnerIdRsp("LSBOIRsp", std::make_pair(std::make_pair(responsePid, responseId), response));
		gsInstance.sendToCentralServer(locateStructureByOwnerIdRsp);
	}
}

// ----------------------------------------------------------------------

void GameServerNamespace::handleMessageLocateStructureByOwnerIdRsp(NetworkId const & responseId, std::vector<std::string> const & response)
{
	if (response.empty())
		return;

	ServerObject const * const object = ServerWorld::findObjectByNetworkId(responseId);
	if (!object)
		return;

	Client * const clientObj = object->getClient();
	if (!clientObj)
		return;

	for (std::vector<std::string>::const_iterator iter = response.begin(); iter != response.end(); ++iter)
		ConsoleMgr::broadcastString(*iter, clientObj);
}

// ----------------------------------------------------------------------

void GameServerNamespace::handleAdjustAccountFeatureIdResponse(AdjustAccountFeatureIdResponse const & msg)
{
	Archive::ByteStream bs;
	msg.pack(bs);

	if (msg.getRequestingPlayer().isValid())
	{
		MessageToQueue::getInstance().sendMessageToC(msg.getRequestingPlayer(),
			"C++AdjustAccountFeatureIdResponse",
			std::string(reinterpret_cast<const char *>(bs.getBuffer()), static_cast<size_t>(bs.getSize())),
			0,
			false);
	}

	if (msg.getTargetPlayer().isValid() && (msg.getTargetPlayer() != msg.getRequestingPlayer()))
	{
		MessageToQueue::getInstance().sendMessageToC(msg.getTargetPlayer(),
			"C++AdjustAccountFeatureIdResponse",
			std::string(reinterpret_cast<const char *>(bs.getBuffer()), static_cast<size_t>(bs.getSize())),
			0,
			false);
	}

	if (msg.getTargetItem().isValid() && (msg.getTargetItem() != msg.getRequestingPlayer()) && (msg.getTargetItem() != msg.getTargetPlayer()))
	{
		MessageToQueue::getInstance().sendMessageToC(msg.getTargetItem(),
			"C++AdjustAccountFeatureIdResponse",
			std::string(reinterpret_cast<const char *>(bs.getBuffer()), static_cast<size_t>(bs.getSize())),
			0,
			false);
	}
}

// ----------------------------------------------------------------------

void GameServerNamespace::handleAccountFeatureIdResponse(AccountFeatureIdResponse const & msg)
{
	// this message may arrive before the Client object has been attached
	// to the character object, or even before we have loaded the character
	// object into the game, so we'll just directly search for the Client
	// object associated with the character object and update the information
	bool needToNotifyRequester = (((msg.getRequestReason() == AccountFeatureIdRequest::RR_ConsoleCommandReloadRequest) || (msg.getRequestReason() == AccountFeatureIdRequest::RR_ReloadRewardCheck)) && msg.getRequester().isValid());

	if (msg.getResultCode() == RESULT_SUCCESS)
	{
		Client * client = GameServer::getInstance().getClient(msg.getTarget());
		if (client)
		{
			if (msg.getGameCode() == PlatformGameCode::SWG)
				client->setAccountFeatureIds(msg.getFeatureIds());
		}
		else
		{
			// character object may have moved to a different game server,
			// so forward the message to the character object for handling
			Archive::ByteStream bs;
			msg.pack(bs);

			MessageToQueue::getInstance().sendMessageToC(msg.getTarget(),
				"C++AccountFeatureIdResponse",
				std::string(reinterpret_cast<const char *>(bs.getBuffer()), static_cast<size_t>(bs.getSize())),
				0,
				false);

			// don't need to send another messageTo to the requester
			if (needToNotifyRequester && (msg.getRequester() == msg.getTarget()))
				needToNotifyRequester = false;
		}
	}

	if (needToNotifyRequester)
	{
		Archive::ByteStream bs;
		msg.pack(bs);

		MessageToQueue::getInstance().sendMessageToC(msg.getRequester(),
			"C++AccountFeatureIdResponse",
			std::string(reinterpret_cast<const char *>(bs.getBuffer()), static_cast<size_t>(bs.getSize())),
			0,
			false);
	}
}

// ----------------------------------------------------------------------

std::string GameServer::getRetroactiveCtsHistory(std::string const & clusterName, NetworkId const & characterId)
{
	std::string result;
	std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > const * clusterCtsHistory = nullptr;

#ifdef _DEBUG
	std::map<std::string, std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > >::const_iterator const iterFindCluster = s_retroactiveCtsHistoryList.find(clusterName);
	if (iterFindCluster != s_retroactiveCtsHistoryList.end())
		clusterCtsHistory = &(iterFindCluster->second);
#else
	if (clusterName == GameServer::getInstance().getClusterName())
		clusterCtsHistory = &s_retroactiveCtsHistoryListThisCluster;
#endif

	if (!clusterCtsHistory)
		return result;

	std::map<NetworkId, std::vector<CtsSourceCharacterInfo> >::const_iterator const iterFindCharacter = clusterCtsHistory->find(characterId);
	if (iterFindCharacter == clusterCtsHistory->end())
		return result;

	int count = 0;
	for (std::vector<CtsSourceCharacterInfo>::const_reverse_iterator iterCtsHistory = iterFindCharacter->second.rbegin(); iterCtsHistory != iterFindCharacter->second.rend(); ++iterCtsHistory)
	{
		if (!result.empty())
			result += "\n";

		++count;

		if (iterCtsHistory->sourceCharacterBornDate <= 0)
			result += FormattedString<1024>().sprintf("%2d) transferred from (%s, %s, %s) on (%ld, %s)", count, iterCtsHistory->sourceCluster.c_str(), iterCtsHistory->sourceCharacterName.c_str(), iterCtsHistory->sourceCharacterId.getValueString().c_str(), iterCtsHistory->transferTime, CalendarTime::convertEpochToTimeStringLocal(iterCtsHistory->transferTime).c_str());
		else
			result += FormattedString<1024>().sprintf("%2d) transferred from (%s, %s, %s, born date=%d, %s) on (%ld, %s)", count, iterCtsHistory->sourceCluster.c_str(), iterCtsHistory->sourceCharacterName.c_str(), iterCtsHistory->sourceCharacterId.getValueString().c_str(), iterCtsHistory->sourceCharacterBornDate, CalendarTime::getCharacerBirthDateString(iterCtsHistory->sourceCharacterBornDate).c_str(), iterCtsHistory->transferTime, CalendarTime::convertEpochToTimeStringLocal(iterCtsHistory->transferTime).c_str());

		for (std::vector<std::pair<std::string, DynamicVariable> >::const_iterator iterRetroactiveObjvar = iterCtsHistory->sourceCharacterRetroactiveObjvars.begin(); iterRetroactiveObjvar != iterCtsHistory->sourceCharacterRetroactiveObjvars.end(); ++iterRetroactiveObjvar)
			result += FormattedString<1024>().sprintf("\n\t(%s, %s)", iterRetroactiveObjvar->first.c_str(), Unicode::wideToNarrow(iterRetroactiveObjvar->second.getPackedValueString()).c_str());
	}

	return result;
}

// ----------------------------------------------------------------------

void GameServer::setRetroactiveCtsHistory(CreatureObject & player)
{
	if (!player.isAuthoritative())
		return;

	if (player.getObjVars().hasItem("ctsHistory"))
		return;

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(&player);
	if (!playerObject)
		return;

	if (!playerObject->isAuthoritative())
		return;

	std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > const * clusterCtsHistory = nullptr;

#ifdef _DEBUG
	std::map<std::string, std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > >::const_iterator const iterFindCluster = s_retroactiveCtsHistoryList.find(GameServer::getInstance().getClusterName());
	if (iterFindCluster != s_retroactiveCtsHistoryList.end())
		clusterCtsHistory = &(iterFindCluster->second);
#else
	clusterCtsHistory = &s_retroactiveCtsHistoryListThisCluster;
#endif

	if (!clusterCtsHistory)
		return;

	std::map<NetworkId, std::vector<CtsSourceCharacterInfo> >::const_iterator const iterFindCharacter = clusterCtsHistory->find(player.getNetworkId());
	if (iterFindCharacter == clusterCtsHistory->end())
		return;

	int count = 0;
	int rootSourceCharacterBornDate = 0;
	std::vector<std::string> ctsSourceGalaxies;
	for (std::vector<CtsSourceCharacterInfo>::const_reverse_iterator iterCtsHistory = iterFindCharacter->second.rbegin(); iterCtsHistory != iterFindCharacter->second.rend(); ++iterCtsHistory)
	{
		++count;
		LOG("CustomerService", ("CharacterTransferRetroactiveHistory: %s, setting objvar ctsHistory.%d = \"%ld %s %s %s\", transfer time (%s)", playerObject->getAccountDescription().c_str(), count, iterCtsHistory->transferTime, iterCtsHistory->sourceCluster.c_str(), iterCtsHistory->sourceCharacterId.getValueString().c_str(), iterCtsHistory->sourceCharacterName.c_str(), CalendarTime::convertEpochToTimeStringLocal(iterCtsHistory->transferTime).c_str()));
		IGNORE_RETURN(player.setObjVarItem(FormattedString<1024>().sprintf("ctsHistory.%d", count), FormattedString<1024>().sprintf("%ld %s %s %s", iterCtsHistory->transferTime, iterCtsHistory->sourceCluster.c_str(), iterCtsHistory->sourceCharacterId.getValueString().c_str(), iterCtsHistory->sourceCharacterName.c_str())));

		if (iterCtsHistory->sourceCharacterBornDate > 0)
		{
			if ((rootSourceCharacterBornDate == 0) || (iterCtsHistory->sourceCharacterBornDate < rootSourceCharacterBornDate))
				rootSourceCharacterBornDate = iterCtsHistory->sourceCharacterBornDate;
		}

		ctsSourceGalaxies.push_back(iterCtsHistory->sourceCluster);
	}

	if (rootSourceCharacterBornDate > 0)
	{
		// initialize the character's born date, if necessary
		playerObject->setBornDate();

		// fix up the character's born date, but only if the "fixup" date is earlier than the current date
		int const characterBornDate = playerObject->getBornDate();
		if (rootSourceCharacterBornDate < characterBornDate)
		{
			playerObject->adjustBornDate(rootSourceCharacterBornDate - characterBornDate);
			LOG("CustomerService", ("CharacterTransferRetroactiveHistory: %s, fixing born date from (%d, %s) to (%d, %s)", playerObject->getAccountDescription().c_str(), characterBornDate, CalendarTime::getCharacerBirthDateString(characterBornDate).c_str(), rootSourceCharacterBornDate, CalendarTime::getCharacerBirthDateString(rootSourceCharacterBornDate).c_str()));

			// send mail to player notifying of born date change
			Chat::sendPersistentMessage("system", Unicode::wideToNarrow(Unicode::toLower(player.getAssignedObjectFirstName())), Unicode::narrowToWide("Birth Date Fixup"), Unicode::narrowToWide(FormattedString<1024>().sprintf("This character's birth date has been adjusted from %s to %s.", CalendarTime::getCharacerBirthDateString(characterBornDate).c_str(), CalendarTime::getCharacerBirthDateString(rootSourceCharacterBornDate).c_str())), Unicode::emptyString);
		}
	}

	// grant CTS titles for transfers that occurred between different galaxies
	if (!ctsSourceGalaxies.empty())
	{
		ctsSourceGalaxies.push_back(GameServer::getInstance().getClusterName());
		for (size_t i = 0, stopIndex = (ctsSourceGalaxies.size() - 2); i <= stopIndex; ++i)
		{
			if (ctsSourceGalaxies[i] != ctsSourceGalaxies[i+1])
			{
				// lower case source cluster name and replace
				// - with _ for use to grant CTS title
				std::string collectionSlotName = std::string("cts_from_") + Unicode::toLower(ctsSourceGalaxies[i]);

				std::string::size_type pos = collectionSlotName.find('-');
				while (pos != std::string::npos)
				{
					collectionSlotName.replace(pos, 1, 1, '_');
					pos = collectionSlotName.find('-');
				}

				CollectionsDataTable::CollectionInfoSlot const * collectionSlot = CollectionsDataTable::getSlotByName(collectionSlotName);
				if (collectionSlot && !playerObject->hasCompletedCollectionSlot(*collectionSlot))
				{
					LOG("CustomerService", ("CharacterTransferRetroactiveHistory: %s, granting CTS title %s", playerObject->getAccountDescription().c_str(), collectionSlotName.c_str()));	
					IGNORE_RETURN(playerObject->modifyCollectionSlotValue(collectionSlotName, 1ll));
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

std::vector<std::vector<std::pair<std::string, DynamicVariable> > const *> const & GameServer::getRetroactiveCtsHistoryObjvars(NetworkId const & characterId)
{
	static std::vector<std::vector<std::pair<std::string, DynamicVariable> > const *> returnValue;
	returnValue.clear();

	std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > const * clusterCtsHistory = nullptr;

#ifdef _DEBUG
	std::map<std::string, std::map<NetworkId, std::vector<CtsSourceCharacterInfo> > >::const_iterator const iterFindCluster = s_retroactiveCtsHistoryList.find(GameServer::getInstance().getClusterName());
	if (iterFindCluster != s_retroactiveCtsHistoryList.end())
		clusterCtsHistory = &(iterFindCluster->second);
#else
	clusterCtsHistory = &s_retroactiveCtsHistoryListThisCluster;
#endif

	if (!clusterCtsHistory)
		return returnValue;

	std::map<NetworkId, std::vector<CtsSourceCharacterInfo> >::const_iterator const iterFindCharacter = clusterCtsHistory->find(characterId);
	if (iterFindCharacter == clusterCtsHistory->end())
		return returnValue;

	int numberOfNonEmptyVectorsReturned = 0;
	for (std::vector<CtsSourceCharacterInfo>::const_reverse_iterator iterCtsHistory = iterFindCharacter->second.rbegin(); iterCtsHistory != iterFindCharacter->second.rend(); ++iterCtsHistory)
	{
		returnValue.push_back(&(iterCtsHistory->sourceCharacterRetroactiveObjvars));

		if (!iterCtsHistory->sourceCharacterRetroactiveObjvars.empty())
		{
			++numberOfNonEmptyVectorsReturned;
		}
	}

	if (numberOfNonEmptyVectorsReturned == 0)
		returnValue.clear();

	return returnValue;
}

// ----------------------------------------------------------------------

time_t GameServer::getRetroactivePlayerCityCreationTime(std::string const & clusterName, int cityId)
{
	std::map<int, time_t> const * clusterPlayerCityCreationTimeHistory = nullptr;

#ifdef _DEBUG
	std::map<std::string, std::map<int, time_t> >::const_iterator const iterFindCluster = s_retroactivePlayerCityCreationTime.find(clusterName);
	if (iterFindCluster != s_retroactivePlayerCityCreationTime.end())
		clusterPlayerCityCreationTimeHistory = &(iterFindCluster->second);
#else
	if (clusterName == GameServer::getInstance().getClusterName())
		clusterPlayerCityCreationTimeHistory = &s_retroactivePlayerCityCreationTimeThisCluster;
#endif

	if (!clusterPlayerCityCreationTimeHistory)
		return -1;

	std::map<int, time_t>::const_iterator const iterFindCity = clusterPlayerCityCreationTimeHistory->find(cityId);
	if (iterFindCity != clusterPlayerCityCreationTimeHistory->end())
		return iterFindCity->second;

	return -1;
}

// ----------------------------------------------------------------------

int GameServer::getServerSpawnLimit()
{
	static int serverSpawnLimit = -1;

	if (GameServer::getInstance().getPreloadAreaId() == 0)
	{
		return ConfigServerGame::getServerSpawnLimit();
	}

	if (serverSpawnLimit == -1)
	{
		serverSpawnLimit = ConfigServerGame::getServerSpawnLimit();

		// see if there's a spawn limit set for this particular game server
		char key[128];
		snprintf(key, sizeof(key)-1, "serverSpawnLimit_%s.%lu", ServerWorld::getSceneId().c_str(), GameServer::getInstance().getPreloadAreaId());
		key[sizeof(key)-1] = '\0';

		if (getConfigSetting("GameServer", key, serverSpawnLimit))
			return serverSpawnLimit;

		// see if there's a spawn limit set for this particular planet
		snprintf(key, sizeof(key)-1, "serverSpawnLimit_%s", ServerWorld::getSceneId().c_str());
		key[sizeof(key)-1] = '\0';

		if (getConfigSetting("GameServer", key, serverSpawnLimit))
			return serverSpawnLimit;

		// use default server spawn limit
		serverSpawnLimit = ConfigServerGame::getServerSpawnLimit();
	}

	return serverSpawnLimit;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void GameServer::setExtraDelayPerFrameMs(int ms) // to emulate long loop time
{
	s_extraDelayPerFrameMs = ms;
}
#endif

// ======================================================================
