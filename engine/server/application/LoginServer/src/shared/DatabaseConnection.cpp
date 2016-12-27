// ======================================================================
//
// DatabaseConnection.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "DatabaseConnection.h"

#include "ConfigLoginServer.h"
#include "LoginServer.h"
#include "TaskClaimRewards.h"
#include "TaskChangeStationId.h"
#include "TaskCreateCharacter.h"
#include "TaskDeleteCharacter.h"
#include "TaskEnableCharacter.h"
#include "TaskGetAccountForPurge.h"
#include "TaskGetAvatarList.h"
#include "TaskGetCharactersForDelete.h"
#include "TaskGetClusterList.h"
#include "TaskGetValidationData.h"
#include "TaskMapAccount.h"
#include "TaskRegisterNewCluster.h"
#include "TaskRenameCharacter.h"
#include "TaskRestoreCharacter.h"
#include "TaskSetPurgeStatus.h"
#include "TaskToggleCharacterDisable.h"
#include "TaskToggleCompletedTutorial.h"
#include "TaskUpdatePurgeAccountList.h"
#include "TaskUpgradeAccount.h"
#include "serverNetworkMessages/AvatarList.h"
#include "serverNetworkMessages/ClaimRewardsReplyMessage.h"
#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"

// ======================================================================

DatabaseConnection::DatabaseConnection() :
		Singleton<DatabaseConnection>(),
		m_databaseServer(0),
		m_taskQueue(0)
{
}

// ----------------------------------------------------------------------

DatabaseConnection::~DatabaseConnection()
{
	delete m_databaseServer;
	delete m_taskQueue;

	m_databaseServer=0;
	m_taskQueue=0;
}
	
// ----------------------------------------------------------------------

void DatabaseConnection::connect()
{
	DEBUG_FATAL(m_databaseServer,("Called DatabaseConnection::connect() when already connected.\n"));
	
	m_databaseServer = DB::Server::create(ConfigLoginServer::getDSN(),
										  ConfigLoginServer::getDatabaseUID(),
										  ConfigLoginServer::getDatabasePWD(),
										  DB::Server::getProtocolByName(ConfigLoginServer::getDatabaseProtocol()),
										  true);
	DB::Server::setDisconnectSleepTime(ConfigLoginServer::getDisconnectSleepTime());
	m_taskQueue=new DB::TaskQueue(static_cast<unsigned int>(ConfigLoginServer::getDatabaseThreads()),m_databaseServer,0);

	if (ConfigLoginServer::getEnableQueryProfile())
		DB::Server::enableProfiling();

	if (ConfigLoginServer::getVerboseQueryMode())
		DB::Server::enableVerboseMode();

	DB::TaskQueue::enableWorkerThreadsLogging(ConfigLoginServer::getLogWorkerThreads());
}

// ----------------------------------------------------------------------

void DatabaseConnection::disconnect()
{
	DEBUG_FATAL(!m_databaseServer,("Called DatabaseConnection::disconnect() when not connected.\n"));

	if (ConfigLoginServer::getEnableQueryProfile())
	{
		DB::Server::debugOutputProfile();
		DB::Server::endProfiling();
	}
	
	delete m_taskQueue;
	delete m_databaseServer;

	m_taskQueue=0;
	m_databaseServer=0;
}

// ----------------------------------------------------------------------

void DatabaseConnection::requestAvatarListForAccount(StationId stationId, const TransferCharacterData * characterData)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskGetAvatarList(stationId, ConfigLoginServer::getClusterGroup(), characterData));
}

// ----------------------------------------------------------------------

void DatabaseConnection::onAvatarListRetrieved(StationId stationId, int stationIdNumberJediSlot, const AvatarList & avatars, TransferCharacterData * const transferData) const
{
	LoginServer::getInstance().sendAvatarList(stationId, stationIdNumberJediSlot, avatars, transferData);
}

// ----------------------------------------------------------------------

/** 
 * Retreive the list of all known clusters from the database.
 * This list is in the database so that we know about clusters
 * that aren't currently running, for character lists, etc.
 */

void DatabaseConnection::requestClusterList()
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskGetClusterList(ConfigLoginServer::getClusterGroup()));
}

// ----------------------------------------------------------------------

/** 
 * Called when a previously unknown cluster connects.  Adds the cluster
 * to the database and assigns it the next available cluster ID.
 */

void DatabaseConnection::registerNewCluster(const std::string &clusterName, const std::string &remoteAddress)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskRegisterNewCluster(clusterName, remoteAddress));	
}

// ----------------------------------------------------------------------

void DatabaseConnection::update()
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->update(ConfigLoginServer::getDefaultDBQueueUpdateTimeLimit());
}

// ----------------------------------------------------------------------

void DatabaseConnection::getAccountValidationData(StationId stationId, uint32 clusterId, unsigned int track, uint32 subscriptionBits)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskGetValidationData(stationId, ConfigLoginServer::getClusterGroup(), clusterId, track, subscriptionBits));
}

// ----------------------------------------------------------------------

void DatabaseConnection::getAccountValidationData(const TransferRequestMoveValidation & request, uint32 clusterId)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskGetValidationData(request, ConfigLoginServer::getClusterGroup(), clusterId));
}

// ----------------------------------------------------------------------

void DatabaseConnection::upsertAccountRelationship(StationId parentID, StationId childID) {
	m_taskQueue->asyncRequest(new TaskMapAccount(parentID, childID));
}

// ----------------------------------------------------------------------

void DatabaseConnection::deleteCharacter(uint32 clusterId, const NetworkId &characterId, StationId stationId)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskDeleteCharacter(clusterId,characterId,stationId));
}

// ----------------------------------------------------------------------

void DatabaseConnection::deleteAllCharacters(StationId stationId)
{
	NON_NULL(m_taskQueue)->asyncRequest(new TaskGetCharactersForDelete(stationId,ConfigLoginServer::getClusterGroup()));
}

// ----------------------------------------------------------------------

void DatabaseConnection::renameCharacter(uint32 clusterId, const NetworkId &characterId, const Unicode::String &newName, const TransferCharacterData * requestData)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskRenameCharacter(clusterId, characterId, newName, requestData));
}

// ----------------------------------------------------------------------

void DatabaseConnection::requestAvatarListAccountTransfer(const TransferAccountData * requestData)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskGetAvatarList(ConfigLoginServer::getClusterGroup(), requestData));
}

// ----------------------------------------------------------------------

void DatabaseConnection::onAvatarListRetrievedAccountTransfer(const AvatarList &avatars, TransferAccountData * transferAccountData)
{
	NOT_NULL(m_taskQueue);
	LoginServer::getInstance().performAccountTransfer(avatars, transferAccountData);
}

// ----------------------------------------------------------------------

void DatabaseConnection::changeStationId(StationId sourceStationId, StationId destinationStationId, const TransferAccountData * const transferAccountData)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskChangeStationId(sourceStationId, destinationStationId, transferAccountData));
}

// ----------------------------------------------------------------------

void DatabaseConnection::createCharacter(uint32 clusterId, StationId stationId, const Unicode::String &characterName, const NetworkId &characterObjectId, int templateId, bool jedi)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskCreateCharacter(clusterId, stationId, characterName, characterObjectId, templateId, jedi));
}

// ----------------------------------------------------------------------

void DatabaseConnection::restoreCharacter(uint32 clusterId, const std::string &whoRequested, StationId stationId, const Unicode::String &characterName, const NetworkId &characterObjectId, int templateId, bool jedi)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskRestoreCharacter(clusterId, whoRequested, stationId, characterName, characterObjectId, templateId, jedi));
}

// ----------------------------------------------------------------------

void DatabaseConnection::enableCharacter(StationId stationId, const NetworkId &characterId, const std::string &whoRequested, bool enabled, uint32 clusterId)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskEnableCharacter(stationId, characterId, whoRequested, enabled, clusterId));
}

// ----------------------------------------------------------------------

void DatabaseConnection::upgradeAccount(LoginUpgradeAccountMessage *msg, uint32 clusterId)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskUpgradeAccount(msg,clusterId));
}

// ----------------------------------------------------------------------

void DatabaseConnection::toggleDisableCharacter(uint32 clusterId, const NetworkId &characterId, StationId stationId, bool enabled)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskToggleCharacterDisable(clusterId,characterId,stationId,enabled));
}

// ----------------------------------------------------------------------

void DatabaseConnection::toggleCompletedTutorial(StationId stationId, bool newValue)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskToggleCompletedTutorial(stationId, newValue));
}

// ----------------------------------------------------------------------

const std::string DatabaseConnection::getSchemaQualifier() const
{
	if (ConfigLoginServer::getSchemaOwner()[0]!='\0')
		return std::string(ConfigLoginServer::getSchemaOwner())+'.';
	else
		return std::string();
}

// ----------------------------------------------------------------------

void DatabaseConnection::claimRewards(uint32 clusterId, uint32 gameServerId, StationId stationId, NetworkId const & playerId, std::string const & rewardEvent, bool consumeEvent, std::string const & rewardItem, bool consumeItem, uint32 accountFeatureId, bool consumeAccountFeatureId, int previousAccountFeatureIdCount, int currentAccountFeatureIdCount)
{
	NOT_NULL(m_taskQueue);

	if (consumeEvent || consumeItem || ((accountFeatureId > 0) && consumeAccountFeatureId))
	{
		m_taskQueue->asyncRequest(new TaskClaimRewards(clusterId, gameServerId, stationId, playerId, rewardEvent, consumeEvent, rewardItem, consumeItem, accountFeatureId, consumeAccountFeatureId, previousAccountFeatureIdCount, currentAccountFeatureIdCount));
	}
	else
	{
		ClaimRewardsReplyMessage const msg(gameServerId, stationId, playerId, rewardEvent, rewardItem, accountFeatureId, consumeAccountFeatureId, previousAccountFeatureIdCount, currentAccountFeatureIdCount, true);
		LoginServer::getInstance().sendToCluster(clusterId, msg);
	}
}

// ----------------------------------------------------------------------

void DatabaseConnection::getAccountForPurge(int purgePhase)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskGetAccountForPurge(purgePhase));
}

// ----------------------------------------------------------------------

void DatabaseConnection::setPurgeStatusAndRelease(StationId account, int newPurgePhase)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskSetPurgeStatus(account, newPurgePhase));
}

// ----------------------------------------------------------------------

void DatabaseConnection::updatePurgeAccountList()
{
	FATAL(ConfigLoginServer::getDatabaseThreads() < 2,("Attempted to run updatePurgeAccountList with only 1 worker thread, which would mean logins would be blocked while the process is running.  Either disable updating the purge account list, or add more worker threads."));
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskUpdatePurgeAccountList);
}

// ----------------------------------------------------------------------

void DatabaseConnection::occupyUnlockedSlot(uint32 clusterId, StationId stationId, NetworkId const & characterId, uint32 replyGameServerId)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskOccupyUnlockedSlot(ConfigLoginServer::getClusterGroup(), clusterId, stationId, characterId, replyGameServerId));
}

// ----------------------------------------------------------------------

void DatabaseConnection::vacateUnlockedSlot(uint32 clusterId, StationId stationId, NetworkId const & characterId, uint32 replyGameServerId)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskVacateUnlockedSlot(ConfigLoginServer::getClusterGroup(), clusterId, stationId, characterId, replyGameServerId));
}

// ----------------------------------------------------------------------

void DatabaseConnection::swapUnlockedSlot(uint32 clusterId, StationId stationId, NetworkId const & sourceCharacterId, NetworkId const & targetCharacterId, uint32 replyGameServerId)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskSwapUnlockedSlot(ConfigLoginServer::getClusterGroup(), clusterId, stationId, sourceCharacterId, targetCharacterId, replyGameServerId));
}

// ----------------------------------------------------------------------

void DatabaseConnection::featureIdTransactionRequest(uint32 clusterId, StationId stationId, NetworkId const & characterId, uint32 replyGameServerId)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskFeatureIdTransactionRequest(clusterId, stationId, characterId, replyGameServerId));
}

// ----------------------------------------------------------------------

void DatabaseConnection::featureIdTransactionSyncUpdate(uint32 clusterId, StationId stationId, NetworkId const & characterId, std::string const & itemId, int adjustment)
{
	NOT_NULL(m_taskQueue);
	m_taskQueue->asyncRequest(new TaskFeatureIdTransactionSyncUpdate(clusterId, stationId, characterId, itemId, adjustment));
}

// ======================================================================
