// ======================================================================
//
// Persister.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Persister_H
#define INCLUDED_Persister_H

// ======================================================================

#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <set> //TODO: remove when we clean up newCharacterLock hack

#include "Unicode.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "sharedDatabaseInterface/DbModeQuery.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/StationId.h"
#include "sharedFoundation/Tag.h"
#include "sharedMessageDispatch/Receiver.h"

class AddResourceTypeMessage;
class BaselinesMessage;
class BountyHunterTargetMessage;
class DeltasMessage;
class Snapshot;
class TransferAccountData;
class TransferCharacterData;
class DBCSRequestMessage;

namespace DB
{
	class TaskQueue;
}

// ======================================================================

/**
 * Singleton that organizes messages to be persisted to the database.
 *
 * For now, directs all changes to a "snapshot" object.  For the future,
 * may want to have a snapshot for each server so that we can do more in
 * parallel.  This might be implemented by having multiple instances
 * of Persister.
 *
 * This is an abstract base class.  A game-specific Persister should
 * be derived from it.
 */
class Persister : public MessageDispatch::Receiver
{
  public:
	static Persister &getInstance();
	
	void handleDeleteMessage                (uint32 serverId, const NetworkId &objectId, int reasonCode, bool immediate, bool demandLoadedContainer, bool cascadeReason);
	void handleMessageTo                    (uint32 sourceServer, const MessageToPayload &data);
	void handleMessageToAck                 (uint32 sourceServer, const MessageToId &messageId);

	void update(real updateTime);
	void onFrameBarrierReached();
	bool isIdle();
	bool isSaveInProgress();

	void newObject(uint32 serverId, const NetworkId &objectId, int templateId, Tag typeId, const NetworkId &container);
	void beginBaselines(const NetworkId &newObject) const;
	void endBaselines(const NetworkId &newObject, uint32 serverId);

	void saveCompleted       (Snapshot *completedSnapshot);
	void onNewCharacterSaved (uint32 stationId, const NetworkId &characterObject, const Unicode::String &characterName, const int templateId, bool special) const;

	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	void shutdown();

	int getTimeSinceLastSave();
	int getLastSaveTime();
	int getLastSaveTotalObjectCount();
	int getLastSaveNewObjectCount();
	std::string getLastSaveCompletionTime();

	void unloadCharacter(const NetworkId &characterId, uint32 sourceServer);
	bool hasDataForObject(const NetworkId &objectId) const;
	void renameCharacter(uint32 sourceServer, int8 renameCharacterMessageSource, uint32 stationId, const NetworkId &characterId, const Unicode::String &newName, const Unicode::String &oldName, bool lastNameChangeOnly, const NetworkId &requestedBy, const TransferCharacterData * renameRequest);
	void changeStationId(const TransferAccountData * transferRequest);
	
	void handleCSRequest( const DBCSRequestMessage & msg );
	void handleCSMovePlayer( const DBCSRequestMessage & msg );
	void handleCsUndelete(const NetworkId & character, const NetworkId & item, const std::string & message, bool move);
	
	virtual void startLoadCommodities() =0;
  protected:
	DB::TaskQueue *taskQueue;
	DB::TaskQueue *m_newCharacterTaskQueue;
	
  private:
	struct PendingCharacter
	{
		uint32 stationId;
		Unicode::String name;
		bool special;
	};

	typedef std::map<uint32,Snapshot*>            ServerSnapshotMap;
	typedef std::unordered_map<NetworkId,Snapshot*>    ObjectSnapshotMap; 
	typedef std::map<NetworkId,PendingCharacter>  PendingCharactersType;
	typedef std::vector<Snapshot*>                SnapshotListType;
	typedef std::set<uint32>                      NewCharacterLockType;
	typedef std::vector<std::pair<StationId, NetworkId> > CharactersToDeleteType;
	
	ServerSnapshotMap      m_currentSnapshots;
	ServerSnapshotMap      m_newObjectSnapshots;
	ServerSnapshotMap      m_newCharacterSnapshots;
	ObjectSnapshotMap      m_objectSnapshotMap;
	PendingCharactersType  m_pendingCharacters;
	SnapshotListType       m_savingSnapshots;
	SnapshotListType       m_savingCharacterSnapshots;
	NewCharacterLockType   m_newCharacterLock;
	CharactersToDeleteType * m_charactersToDeleteThisSaveCycle;
	CharactersToDeleteType * m_charactersToDeleteNextSaveCycle;
	real                   m_timeSinceLastSave;
	Snapshot *             m_messageSnapshot;
	Snapshot *             m_commoditiesSnapshot;
	Snapshot *             m_arbitraryGameDataSnapshot;
	int                    m_saveStartTime;
	int                    m_totalSaveTime;
	int                    m_maxSaveTime;
	int                    m_saveCount;
	int                    m_newObjectCount;

	int                    m_lastSaveTime;
	int                    m_lastSaveTotalObjectCount;
	int                    m_lastSaveNewObjectCount;
	std::string            m_lastSaveCompletionTime;
	int                    m_saveCounter;

	bool                   m_startSaveWhenPossible;
	bool                   m_inMagicMinute;
	bool                   m_clusterShuttingDown;

  protected:

	virtual void startSave() = 0;

	Snapshot & getSnapshotForObject(const NetworkId &networkId, uint32 serverId);
	Snapshot & getSnapshotForServer(uint32 serverId);

  private:
	
	void handleDeltasMessage     (uint32 serverId, const DeltasMessage &msg);
	void handleBaselinesMessage  (uint32 serverId, const BaselinesMessage &msg);
	void handleAddResourceTypeMessage(uint32 const serverId, AddResourceTypeMessage const & message);
	void handlePurgeCompleteMessage(uint32 const serverId, StationId stationId);
	void addCharacter            (uint32 stationId, const NetworkId &characterObject, uint32 creationGameServer, const Unicode::String &name, bool special);
	void deleteCharacter         (StationId stationId, const NetworkId &characterId);
	void recordMoneyTransaction  (uint32 sourceServer, const NetworkId &transactionId, const int transactionType, const NetworkId &sourceId, const std::string &sourceString, const NetworkId &targetId, const std::string &targetString, int amount);
	void moveToPlayer            (uint32 sourceServer, const NetworkId &objectId, const NetworkId &targetPlayer, int maxDepth, bool useBank, bool useDatapad);
	void fixLoadWith             (uint32 sourceServer, const NetworkId &topmostObject, const NetworkId &startingLoadWith, int maxDepth);
	void moveToContainer         (uint32 sourceServer, NetworkId const & objectId, NetworkId const & targetContainer);
	void userRequestedSave       (uint32 sourceGameServer, const NetworkId &whoRequested);
	void centralRequestedSave    ();
	void planetRequestedSave     ();

	Snapshot & getCommoditiesSnapshot(uint32 serverId);
	
    /**
	 * Derived class should override this to make a game-specific derived Snapshot.
	 */
	virtual Snapshot *makeSnapshot(DB::ModeQuery::Mode mode) const =0;
	virtual Snapshot *makeCommoditiesSnapshot(DB::ModeQuery::Mode mode) const =0;

	/**
	 * Admin functions
	 */
	virtual void restoreHouse(const NetworkId &houseId, const std::string &whoRequested)=0;
	virtual void restoreCharacter(const NetworkId &characterId, const std::string &whoRequested)=0;
	virtual void undeleteItem(const NetworkId &itemId, const std::string &whoRequested)=0;
	virtual void moveToPlayer(const NetworkId &oid, const NetworkId &player, const std::string &whoRequested)=0;

	/**
	 * Misc game-specific persistence steps
	 */
	virtual void getMoneyFromOfflineObject(uint32 replyServer, NetworkId const & sourceObject, int amount, NetworkId const & replyTo, std::string const & successCallback, std::string const & failCallback, stdvector<int8>::fwd const & packedDictionary)=0;

  protected:
	Persister();
	virtual ~Persister();
	static void installDerived(Persister *derivedInstance);

  private:
	Persister(const Persister&); //disable
	Persister &operator=(const Persister&); //disable
	
  private:
	static void remove();
	static Persister *ms_instance;
};

// ----------------------------------------------------------------------

inline Persister &Persister::getInstance()
{
	NOT_NULL(ms_instance);
	return *ms_instance;
}

// ----------------------------------------------------------------------

inline int Persister::getTimeSinceLastSave()
{
	return static_cast<int>(m_timeSinceLastSave);
}

// ----------------------------------------------------------------------

inline int Persister::getLastSaveTime()
{
	return m_lastSaveTime;
}

// ----------------------------------------------------------------------

inline int Persister::getLastSaveTotalObjectCount()
{
	return m_lastSaveTotalObjectCount;
}

// ----------------------------------------------------------------------

inline int Persister::getLastSaveNewObjectCount()
{
	return m_lastSaveNewObjectCount;
}

// ======================================================================

#endif
