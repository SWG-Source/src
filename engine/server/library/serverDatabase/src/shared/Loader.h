// ======================================================================
//
// Loader.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Loader_H
#define INCLUDED_Loader_H

// ======================================================================

#include <list>
#include <set>
#include <map>
#include <vector>

#include "Singleton/Singleton2.h"
#include "Unicode.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/StationId.h"
#include "sharedMessageDispatch/Receiver.h"

namespace DB
{
	class TaskQueue;
	class Server;
}
class LoaderSnapshotGroup;
class ObjectLocator;
class TransferCharacterData;
class CSGetCharactersRequestMessage;
class CSGetDeletedItemsRequestMessage;
class DBCSRequestMessage;

// ======================================================================

/**
 * Singleton that organizes and queues requests to load objects from the database.
 */
class Loader : public MessageDispatch::Receiver
{
public:
	static Loader &getInstance();
	virtual void update(real updateTime);
	
	void requestChunk(uint32 processId,int nodeX, int nodeZ, const std::string &sceneId);
	void loadUniverse(uint32 processId);

	void snapshotLoaded(LoaderSnapshotGroup *snapshot);

	void getObjectIds(int processId, int numWanted, bool logRequest);
	virtual void verifyCharacter(StationId suid, const NetworkId &characterId, const TransferCharacterData *) =0;
	void checkVersionNumber(int expectedVersion, bool fatalOnMismatch);
	void loadClock();
	void loadClockCompleted();
	void startupLoadCompleted() const;
	void discardPendingLoadsForServer(uint32 processId);
	void addLoadLock(const NetworkId &characterId);
	void removeLoadLock(const NetworkId &characterId);
	bool isIdle();
	bool isBacklogged();
	int  getLoadQueueTasksEstimate() const;
	int  getNumPreloads() const;
	int  getAgeOldestUnackedLoad() const;
	uint32 getServerOldestUnackedLoad() const;
	void onCharacterLocatorDeleted(const NetworkId &characterId);
	virtual void locateStructure(const NetworkId &structureId, const std::string whoRequested) =0;
	
	// CS DB requests
	void handleCSGetCharacters( const CSGetCharactersRequestMessage & msg );
	void handleCSGetDeletedItems( const CSGetDeletedItemsRequestMessage & msg );
	void handleCSRequest( const DBCSRequestMessage & msg );
	void handleCSGetCharacters( const DBCSRequestMessage & msg );
	void handleCSGetDeletedItems( const DBCSRequestMessage & msg );
	void handleCSGetStructures( const DBCSRequestMessage & msg );
	void handleCSGetPlayerId( const DBCSRequestMessage & msg );
	
	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	void shutdown();
	
private:
	void                          queueOutgoingSnapshot   (LoaderSnapshotGroup *outgoingSnapshot);
	void                          requestCharacter        (const NetworkId &characterId, uint32 gameServerId);
	void                          loadContainedObject     (const NetworkId &containerId, const NetworkId &objectId, uint32 gameServerId);
	void                          loadContents            (const NetworkId &containerId, uint32 gameServerId);
	virtual LoaderSnapshotGroup * makeLoaderSnapshotGroup (uint32 processId) =0;
	void                          preloadRequestComplete  (uint32 processId, uint32 preloadAreaId);
	virtual void                  loadStartupData         () =0;
	void                          startLoad               ();
	void                          startLoadForServer      (uint32 serverId);
	unsigned int                  getUnackedLoadCount     (uint32 serverId) const;
	void                          addLocatorsForServer    (uint32 serverId, ObjectLocator * regularLocator, ObjectLocator * goldLocator);
	void                          handleLoadAck           (uint32 serverId, int serialNumber);
	
private:
	typedef std::list<LoaderSnapshotGroup*>    OutgoingQueueType;
	typedef std::set<uint32>                   ServerDiscardListType;
	typedef std::map<NetworkId, uint32>        LoadLockType; // map of character -> server with a pending load for that character
	typedef std::map<uint32, int>              LoadCountType;
	typedef std::map<uint32, float>            NextLoadStartTimeType;
	typedef std::map<uint32, std::list<std::pair<ObjectLocator*, ObjectLocator*> >* > LocatorListType; // map of server -> list of locators requested for that server
	typedef std::map<uint32, std::vector<int> > UnackedLoadsType; // map of server -> last load sent to server
	typedef std::multimap<time_t, std::pair<uint32, int> > UnackedLoadsTimeType; // how long we've been waiting for each ack
	typedef std::map<uint32,uint32>            PreloadingProcessesType; // map of server -> numerical ID for area preloaded

	OutgoingQueueType     outgoingQueue;
	ServerDiscardListType m_serverDiscardList;
	LoadLockType          m_loadLock; // prevent loading objects that are being saved
	LoadLockType          m_multipleLoginLock; // prevent loading the same character multiple times
	LoadCountType         m_characterLoadCount;
	LoadCountType         m_chunkLoadCount;
	NextLoadStartTimeType m_nextLoadStartTime;
	bool                  m_backlogged;
	float                 m_currentTime;
	LocatorListType       m_locatorList;
	int                   m_loadSerialNumber;
	UnackedLoadsType      m_unackedLoads;
	UnackedLoadsTimeType  m_unackedLoadsTime;
	int                   m_numQueuedLocators;
	PreloadingProcessesType m_preloadingProcesses;
	
  protected:
	DB::TaskQueue *taskQ;

  protected:
	Loader();
	virtual ~Loader();
	Loader(const Loader &);
	Loader &  operator =  (const Loader &);
	static void installDerived(Loader *derivedInstance);

  private:
	static void remove();
	static Loader *ms_instance;	
};

// ----------------------------------------------------------------------

inline Loader &Loader::getInstance()
{
	NOT_NULL(ms_instance);
	return *ms_instance;
}

// ----------------------------------------------------------------------

inline bool Loader::isBacklogged()
{
	return m_backlogged;
}

// ======================================================================

#endif
