// ======================================================================
//
// DatabaseConnection.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DatabaseConnection_H
#define INCLUDED_DatabaseConnection_H

// ======================================================================

#include "Singleton/Singleton.h"
#include "serverNetworkMessages/AvatarList.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/StationId.h"

namespace DB
{
	class Server;
	class TaskQueue;
}

class LoginUpgradeAccountMessage;
class TransferAccountData;
class TransferCharacterData;
class TransferRequestMoveValidation;

// ======================================================================

/** 
 * A class to manage the LoginServer's connection to the database.
 */

class DatabaseConnection : public Singleton<DatabaseConnection>
{
  public:
	DatabaseConnection();
	~DatabaseConnection();
	
	void connect                       ();
	void disconnect                    ();
	void requestAvatarListForAccount   (StationId stationId, const TransferCharacterData *);
	void requestClusterList            ();
	void registerNewCluster            (const std::string &clusterName, const std::string &remoteAddress);
	void onAvatarListRetrieved         (StationId stationId, int stationIdNumberJediSlot, const AvatarList &avatars, TransferCharacterData * const transferCharacterData) const;
	void update                        ();
	void getAccountValidationData      (StationId stationId, uint32 clusterId, unsigned int track, uint32 subscriptionBits);
	void getAccountValidationData      (const TransferRequestMoveValidation & request, uint32 clusterId);
	void upsertAccountRelationship	   (StationId parentID, StationId childID);
	void deleteCharacter               (uint32 clusterId, const NetworkId &characterId, StationId stationId);
	void deleteAllCharacters           (StationId stationId);
	void renameCharacter               (uint32 clusterId, const NetworkId &characterId, const Unicode::String &newName, const TransferCharacterData * requestData);
	void requestAvatarListAccountTransfer (const TransferAccountData * requestData);
	void onAvatarListRetrievedAccountTransfer (const AvatarList &avatars, TransferAccountData * transferAccountData);
	void changeStationId               (StationId sourceStationId, StationId destinationStationId, const TransferAccountData * const transferAccountData);
	void createCharacter               (uint32 clusterId, StationId stationId, const Unicode::String &characterName, const NetworkId &characterObjectId, int templateId, bool jedi);
	void restoreCharacter              (uint32 clusterId, const std::string &whoRequested, StationId stationId, const Unicode::String &characterName, const NetworkId &characterObjectId, int templateId, bool jedi);
	void enableCharacter               (StationId stationId, const NetworkId &characterId, const std::string &whoRequested, bool enabled, uint32 clusterId);
	void upgradeAccount                (LoginUpgradeAccountMessage *msg, uint32 clusterId);
	void toggleDisableCharacter        (uint32 clusterId, const NetworkId &characterId, StationId stationId, bool enabled);
	void toggleCompletedTutorial       (StationId stationId, bool newValue);
 	const std::string getSchemaQualifier() const;
	void claimRewards                  (uint32 clusterId, uint32 gameServerId, StationId stationId, NetworkId const & playerId, std::string const & rewardEvent, bool consumeEvent, std::string const & rewardItem, bool consumeItem, uint32 accountFeatureId, bool consumeAccountFeatureId, int previousAccountFeatureIdCount, int currentAccountFeatureIdCount);
	void getAccountForPurge            (int purgePhase);
	void setPurgeStatusAndRelease      (StationId account, int newPurgePhase);
	void updatePurgeAccountList        ();
	void occupyUnlockedSlot            (uint32 clusterId, StationId stationId, NetworkId const & characterId, uint32 replyGameServerId);
	void vacateUnlockedSlot            (uint32 clusterId, StationId stationId, NetworkId const & characterId, uint32 replyGameServerId);
	void swapUnlockedSlot              (uint32 clusterId, StationId stationId, NetworkId const & sourceCharacterId, NetworkId const & targetCharacterId, uint32 replyGameServerId);
	void featureIdTransactionRequest   (uint32 clusterId, StationId stationId, NetworkId const & characterId, uint32 replyGameServerId);
	void featureIdTransactionSyncUpdate(uint32 clusterId, StationId stationId, NetworkId const & characterId, std::string const & itemId, int adjustment);
		
  private:
	DB::Server *m_databaseServer;
	DB::TaskQueue *m_taskQueue;
};

// ======================================================================

#endif
