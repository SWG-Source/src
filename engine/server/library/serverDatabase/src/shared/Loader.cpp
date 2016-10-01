// ======================================================================
//
// Loader.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/Loader.h"

#include "UnicodeUtils.h"
#include "serverDatabase/CharacterLocator.h"
#include "serverDatabase/CharacterNameLocator.h"
#include "serverDatabase/ChunkLocator.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/ContainedObjectLocator.h"
#include "serverDatabase/ContentsLocator.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/LoaderSnapshotGroup.h"
#include "serverDatabase/TaskCheckGoldVersionNumber.h"
#include "serverDatabase/TaskCheckVersionNumber.h"
#include "serverDatabase/TaskCSTasks.h"
#include "serverDatabase/TaskGetObjectIds.h"
#include "serverDatabase/TaskGetStructures.h"
#include "serverDatabase/TaskLoadClock.h"
#include "serverDatabase/TaskLoadSnapshots.h"
#include "serverDatabase/UniverseLocator.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/ChunkObjectListMessage.h"
#include "serverNetworkMessages/CSDBNetMessages.h"
#include "serverNetworkMessages/LoadContainedObjectMessage.h"
#include "serverNetworkMessages/LoadContentsMessage.h"
#include "serverNetworkMessages/LoadObjectMessage.h"
#include "serverNetworkMessages/LoadUniverseMessage.h"
#include "serverNetworkMessages/LocateStructureMessage.h"
#include "serverNetworkMessages/PlanetLoadCharacterMessage.h"
#include "serverNetworkMessages/PreloadRequestCompleteMessage.h"
#include "serverNetworkMessages/ReleaseAuthoritativeMessage.h"
#include "serverNetworkMessages/RequestChunkMessage.h"
#include "serverNetworkMessages/RequestObjectIdsMessage.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/ValidateCharacterForLoginMessage.h"
#include "serverUtility/ServerClock.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "sharedFoundation/CrcConstexpr.hpp"

#include <algorithm>

// ======================================================================

Loader *Loader::ms_instance = nullptr;

// ======================================================================

void Loader::installDerived(Loader *derivedInstance)
{
	DEBUG_FATAL(ms_instance,("Installed loader twice.\n"));
	ms_instance = derivedInstance;
	ExitChain::add(&remove, "Loader::remove");
}

// ----------------------------------------------------------------------

void Loader::remove()
{
	NOT_NULL(ms_instance);
	delete ms_instance;
	ms_instance = nullptr;
}

// ----------------------------------------------------------------------

Loader::Loader() :
		MessageDispatch::Receiver(),
		outgoingQueue(),
		m_serverDiscardList(),
		m_loadLock(),
		m_multipleLoginLock(),
		m_characterLoadCount(),
		m_chunkLoadCount(),
		m_nextLoadStartTime(),
		m_backlogged(false),
		m_currentTime(0),
		m_locatorList(),
		m_loadSerialNumber(0),
		m_unackedLoads(),
		m_unackedLoadsTime(),
		m_numQueuedLocators(0),
		taskQ(new DB::TaskQueue(static_cast<unsigned int>(ConfigServerDatabase::getLoaderThreads()),DatabaseProcess::getInstance().getDBServer(),0))
{
	connectToMessage("LoadAckMessage");
	connectToMessage("LoadContainedObjectMessage");
	connectToMessage("LoadContentsMessage");
	connectToMessage("LoadObjectMessage");
	connectToMessage("LoadUniverseMessage");
	connectToMessage("LocateStructureMessage");
	connectToMessage("PlanetLoadCharacterMessage");
	connectToMessage("PreloadRequestCompleteMessage");
	connectToMessage("RequestChunkMessage");
	connectToMessage("RequestOIDsMessage");
	connectToMessage("TransferGetLoginLocationData");
	connectToMessage("ValidateCharacterForLoginMessage");
	connectToMessage("CSGetCharactersRequestMessage");
	connectToMessage("CSGetDeletedItemsRequestMessage");
	connectToMessage("DBCSRequestMessage");
}

//-----------------------------------------------------------------------

Loader::~Loader()
{
	DEBUG_FATAL(taskQ,("Call shutdown() before deleting the Loader singleton.\n"));
}

// ----------------------------------------------------------------------

void Loader::update(real updateTime)
{
	static float reportTime = 0;

	m_currentTime += updateTime;
	
	//TODO The outgoing queue isn't very efficient, since we check the 
	// whole thing every loop.  Maybe make something better if going 
	// through this queue turns out to be a big time waster.
	{
		PROFILER_AUTO_BLOCK_DEFINE("Outgoing Queue");
		for (OutgoingQueueType::iterator i=outgoingQueue.begin(); i!=outgoingQueue.end(); ++i)
		{
			if ((*i)->send())
			{
				DEBUG_REPORT_LOG(true,("Queued snapshot was sent.\n"));
				delete *i;
				i=outgoingQueue.erase(i);
			}
			else
			{
				if (m_serverDiscardList.find((*i)->getRequestingProcessId())!=m_serverDiscardList.end())
				{
					DEBUG_REPORT_LOG(true,("Discarding queued snapshot because server has disconnected.\n"));
					delete *i;
					i=outgoingQueue.erase(i);
				}
			}
		}
	}

	startLoad();

	{
		PROFILER_AUTO_BLOCK_DEFINE("Task Queue Update");
		taskQ->update(ConfigServerDatabase::getDefaultQueueUpdateTimeLimit());
		if (taskQ->getNumFinishedTasks() > ConfigServerDatabase::getMaxLoaderFinishedTasks())
			taskQ->pause();
		else
			taskQ->unpause();
	}
		
	reportTime+=updateTime;
	if (reportTime > 5)
	{
		reportTime=0;

		if ((m_numQueuedLocators != 0) || (!m_unackedLoadsTime.empty()) || (taskQ->getNumPendingTasks() != 0) || (taskQ->getNumActiveTasks() != 0) || (taskQ->getNumFinishedTasks() != 0))
		{
			if (m_unackedLoadsTime.empty())
			{
				LOG("LoadTimes",("Load queue status:  %i queued locators, 0 unacked loads, %i pending tasks, %i active threads, %i completed tasks", m_numQueuedLocators, taskQ->getNumPendingTasks(), taskQ->getNumActiveTasks(), taskQ->getNumFinishedTasks()));
			}
			else
			{
				UnackedLoadsTimeType::const_iterator i = m_unackedLoadsTime.begin();

				LOG("LoadTimes",("Load queue status:  %i queued locators, %i unacked loads (oldest is %i/%is), %i pending tasks, %i active threads, %i completed tasks", m_numQueuedLocators, m_unackedLoadsTime.size(), i->second.first, (time(0) - i->first), taskQ->getNumPendingTasks(), taskQ->getNumActiveTasks(), taskQ->getNumFinishedTasks()));
			}
		}

		bool updateNeeded = false;
		int loadQueueSize=taskQ->getNumPendingTasks();
		if (!m_backlogged)
		{
			if (loadQueueSize >= ConfigServerDatabase::getBackloggedQueueSize())
			{
				m_backlogged = true;
				updateNeeded = true;
			}
		}
		else
		{
			if (loadQueueSize <= ConfigServerDatabase::getBackloggedRecoveryQueueSize())
			{
				m_backlogged = false;
				updateNeeded = true;
			}
		}

		if (updateNeeded)
		{
			GenericValueTypeMessage<bool> backlogMessage("DatabaseBackloggedMessage",m_backlogged);
			DatabaseProcess::getInstance().sendToCentralServer(backlogMessage,true);
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Start asychronous loads if there is no backlog of load requests, or
 * if any of the early-start conditions have been met.
 */
void Loader::startLoad()
{
	PROFILER_AUTO_BLOCK_DEFINE("Loader::startLoad");

	if (static_cast<int>(m_unackedLoadsTime.size()) >= ConfigServerDatabase::getMaxUnackedLoadCount())
		return;

	for (LocatorListType::iterator i=m_locatorList.begin(); i!=m_locatorList.end(); )
	{
		if (getUnackedLoadCount(i->first) == 0)
		{
			LocatorListType::iterator next=i; // startLoadForServer() may erase the element we're looking at
			++next;
			startLoadForServer(i->first);
			i=next;
		}
		else
			++i;

		if (static_cast<int>(m_unackedLoadsTime.size()) >= ConfigServerDatabase::getMaxUnackedLoadCount())
			return;
	}

	if (static_cast<int>(m_unackedLoadsTime.size()) >= ConfigServerDatabase::getMaxUnackedLoadCount())
		return;

	// if task queue isn't busy, start additional loads for servers that
	// still have unacked pending loads; pick servers in round-robin
	// fashion, but don't pick servers that have too many unacked pending loads
	if (ConfigServerDatabase::getMaxUnackedLoadCountPerServer() <= 1)
		return;

	int numberOfIterationsWithoutMatch = 0;
	while (!m_unackedLoads.empty() && !m_locatorList.empty() && taskQ->getNumPendingTasks()<ConfigServerDatabase::getLoaderThreads() && numberOfIterationsWithoutMatch<2)
	{
		static uint32 lastEarlyServer=0;
		UnackedLoadsType::const_iterator i=m_unackedLoads.upper_bound(lastEarlyServer);
		if (i==m_unackedLoads.end())
		{
			++numberOfIterationsWithoutMatch;
			i=m_unackedLoads.begin();
		}
		lastEarlyServer=i->first;

		if ((static_cast<int>(i->second.size()) < ConfigServerDatabase::getMaxUnackedLoadCountPerServer()) && (m_locatorList.find(lastEarlyServer) != m_locatorList.end()))
		{
			numberOfIterationsWithoutMatch = 0;
			LOG("LoadTimes",("Early load (server %lu unacked loads=%i, all server unacked loads=%i)", lastEarlyServer, i->second.size(), m_unackedLoadsTime.size()));
			startLoadForServer(lastEarlyServer);

			if (static_cast<int>(m_unackedLoadsTime.size()) >= ConfigServerDatabase::getMaxUnackedLoadCount())
				return;
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Queue up a batch of loads for the specified server.  Include as many
 * locators as possible, up to the limits specified in the config file.
 */
void Loader::startLoadForServer(uint32 serverId)
{
	std::list<std::pair<ObjectLocator*, ObjectLocator*> > *locators = m_locatorList[serverId];
	DEBUG_FATAL(!locators,("Programmer bug:  called startLoadForServer(%lu), but the server had no pending ObjectLocators",serverId));
	if (locators)
	{
		if (!locators->empty())
		{
			// if server has been disconnected, toss out all pending load request for the server
			if (m_serverDiscardList.find(serverId) != m_serverDiscardList.end())
			{
				int discardCount = 0;
				while (!locators->empty())
				{
					delete locators->front().first;
					delete locators->front().second;

					locators->pop_front();
					--m_numQueuedLocators;
					++discardCount;
				}

				LOG("LoadTimes",("Discarding %d pending load request for disconnected server %lu", discardCount, serverId));
			}
			else
			{
				int chunkCount = 0;
				int characterCount = 0;
				LoaderSnapshotGroup * snapshot = makeLoaderSnapshotGroup(serverId);
				while (!locators->empty() && chunkCount < ConfigServerDatabase::getMaxChunksPerLoadRequest() && characterCount < ConfigServerDatabase::getMaxCharactersPerLoadRequest())
				{
					ObjectLocator * const regularLocator = NON_NULL(locators->front().first);
					ObjectLocator * const goldLocator = locators->front().second;
					if (dynamic_cast<ChunkLocator*>(regularLocator))
						chunkCount++;
					if (dynamic_cast<CharacterLocator*>(regularLocator))
						chunkCount++;
					snapshot->addLocator(regularLocator);
					if (goldLocator)
						snapshot->addGoldLocator(goldLocator);
					
					locators->pop_front();
					--m_numQueuedLocators;
				}

				snapshot->setLoadSerialNumber(++m_loadSerialNumber);
				m_unackedLoads[serverId].push_back(m_loadSerialNumber);
				IGNORE_RETURN(m_unackedLoadsTime.insert(std::make_pair(time(0), std::make_pair(serverId, m_loadSerialNumber))));

				TaskLoadSnapshots *task=new TaskLoadSnapshots(snapshot);
				taskQ->asyncRequest(task);
			}
		}

		if (locators->empty())
		{
			delete locators;
			IGNORE_RETURN(m_locatorList.erase(serverId));
		}
	}	
}

// ----------------------------------------------------------------------

/** 
 * Enqueue all the universe objects to be loaded.
 * This particular load is started right away, instead of being
 * queued in m_locatorsList (due to the setUniverseAuthHack() call)
 */
void Loader::loadUniverse(uint32 processId)
{
	LoaderSnapshotGroup * snapshot = makeLoaderSnapshotGroup(processId);
	snapshot->addLocator(new CharacterNameLocator);
	snapshot->addLocator(new UniverseLocator);
	snapshot->setUniverseAuthHack();
	
	TaskLoadSnapshots *task=new TaskLoadSnapshots(snapshot);
	taskQ->asyncRequest(task);	
}

// ----------------------------------------------------------------------

/** 
 * Called when a snapshot has been successfully loaded.
 *
 * Will send the snapshot to whichever server requested the object(s) it
 * contains.
 * @param snapshot The new snapshot.  The Loader takes ownership of the snapshot,
 * the caller does not need to delete it.
 * @see TaskLoadObjects
 */
void Loader::snapshotLoaded(LoaderSnapshotGroup *snapshot)
{
	PROFILER_AUTO_BLOCK_DEFINE("Loader::snapshotLoaded");

	if (snapshot->send())
	{
		PROFILER_AUTO_BLOCK_DEFINE("delete snapshot");
		delete snapshot;
	}
	else
	{
		if (m_serverDiscardList.find(snapshot->getRequestingProcessId())!=m_serverDiscardList.end())
		{
			DEBUG_REPORT_LOG(true,("Discarding pending snapshot because server has disconnected.\n"));
			delete snapshot;
		}
		else
		{
			DEBUG_REPORT_LOG(true,("Snapshot could not be sent, queueing for later.\n"));
			queueOutgoingSnapshot(snapshot);
		}
	}
}

// ----------------------------------------------------------------------

/** 
 * Save a Snapshot that couldn't be sent because there's no connection to the server.
 *
 * The Loader will attempt to send the Snapshot again later.
 *
 */

void Loader::queueOutgoingSnapshot(LoaderSnapshotGroup *outgoingSnapshot)
{
	outgoingQueue.push_back(outgoingSnapshot);
}

// ----------------------------------------------------------------------

void Loader::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);
	
	const uint32 messageType = message.getType();
	switch (messageType) {
		case constcrc("LoadObjectMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			LoadObjectMessage lom(ri);
			DEBUG_FATAL(true,("LoadObjectMessage is deprecated on the database process.\n"));
			//		requestObject(lom.getId(),lom.getProcess());
			break;
		}
		case constcrc("RequestChunkMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			RequestChunkMessage m(ri);
			
			std::vector<RequestChunkMessage::Chunk> const & chunks = m.getChunks();
			for (std::vector<RequestChunkMessage::Chunk>::const_iterator i=chunks.begin(); i!=chunks.end(); ++i)
				requestChunk(i->m_process, i->m_nodeX, i->m_nodeZ, m.getSceneId());
			
			break;
		}
		case constcrc("RequestOIDsMessage") :
		{
			//		DEBUG_REPORT_LOG(true,("Got RequestOIDsMessage.\n"));
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			RequestOIDsMessage m(ri);

			if (m.getLogRequest())
				LOG("ObjectIdManager", ("Received RequestOIDsMessage for %lu more object ids for pid %lu", m.getHowMany(), m.getServerId()));

			getObjectIds(static_cast<int>(m.getServerId()),static_cast<int>(m.getHowMany()), m.getLogRequest());
			break;
		}
		case constcrc("ValidateCharacterForLoginMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			ValidateCharacterForLoginMessage msg(ri);
			verifyCharacter(msg.getSuid(), msg.getCharacterId(), nullptr);
			break;
		}
		case constcrc("TransferGetLoginLocationData") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<TransferCharacterData> request(ri);
			TransferCharacterData * transferData = new TransferCharacterData(request.getValue());
			verifyCharacter(transferData->getSourceStationId(), transferData->getCharacterId(), transferData);
			break;
		}//lint !e429 not freed : suppressed because verifyCharacter will own this copy of the data
		case constcrc("LoadUniverseMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			LoadUniverseMessage m(ri);
			DEBUG_REPORT_LOG(true,("Got LoadUniverseMessage\n"));
			loadUniverse(m.getProcess());
			break;
		}
		case constcrc("PlanetLoadCharacterMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			PlanetLoadCharacterMessage m(ri);
			DEBUG_REPORT_LOG(true,("Got PlanetLoadCharacterMessage\n"));
			requestCharacter(m.getCharacterId(), m.getGameServerId());
			break;
		}
		case constcrc("LoadContainedObjectMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			LoadContainedObjectMessage msg(ri);

			const GameServerConnection * g = dynamic_cast<const GameServerConnection *>(&source);
			if (g)
				loadContainedObject(msg.getContainerId(), msg.getObjectId(), g->getProcessId());
			else
				WARNING_STRICT_FATAL(true,("Got LoadContainedObjectMessage for %s, but sender was not a GameServerConnection.\n",msg.getObjectId().getValueString().c_str()));
			
			break;
		}
		case constcrc("LoadContentsMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			LoadContentsMessage msg(ri);

			const GameServerConnection * g = dynamic_cast<const GameServerConnection *>(&source);
			if (g)
				loadContents(msg.getContainerId(), g->getProcessId());
			else
				WARNING_STRICT_FATAL(true,("Got LoadContentsMessage for container %s, but sender was not a GameServerConnection.\n",msg.getContainerId().getValueString().c_str()));
			
			break;
		}
		case constcrc("LocateStructureMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			LocateStructureMessage msg(ri);

			const GameServerConnection * g = dynamic_cast<const GameServerConnection *>(&source);
			if (g)
				locateStructure(msg.getStructureId(), msg.getWhoRequested());
			else
				WARNING_STRICT_FATAL(true,("Got LocateStructureMessage for %s, but sender was not a GameServerConnection.\n",msg.getStructureId().getValueString().c_str()));
			
			break;
		}
		case constcrc("PreloadRequestCompleteMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			PreloadRequestCompleteMessage msg(ri);

			preloadRequestComplete(msg.getGameServerId(), msg.getPreloadAreaId());
			
			break;
		}
		case constcrc("LoadAckMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<int> msg(ri);
			const GameServerConnection * g = dynamic_cast<const GameServerConnection *>(&source);
			if (g)
				handleLoadAck(g->getProcessId(), msg.getValue());
			else
				WARNING_STRICT_FATAL(true,("Got LoadAckMessage, but sender was not a GameServerConnection."));
			
			break;
		}
		case constcrc("CSGetCharactersRequestMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CSGetCharactersRequestMessage const msg( ri );
			handleCSGetCharacters( msg );	
			break;
		}
		case constcrc("CSGetDeletedItemsRequestMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CSGetDeletedItemsRequestMessage const msg( ri );
			handleCSGetDeletedItems( msg );
			break;			
		}
		case constcrc("DBCSRequestMessage" ) :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			DBCSRequestMessage const msg( ri );
			handleCSRequest( msg );
			break;
		}
	}
}

void Loader::handleCSRequest( const DBCSRequestMessage & msg )
{
	DEBUG_REPORT_LOG( true, ( "Message:%s\n", msg.getCommand().c_str() ) );
	
	std::string cmd = msg.getCommand();

	// TODO : maybe constexpr this one sometime
	if(cmd == "get_characters" )
	{
		handleCSGetCharacters( msg );
	}
	else if(cmd == "get_deleted_items" )
	{
		handleCSGetDeletedItems( msg );
	}
	else if(cmd == "list_structures" )
	{
		handleCSGetStructures( msg );
	}
	else if(cmd == "get_player_id" )
	{
		handleCSGetPlayerId( msg );
	}
}
// ----------------------------------------------------------------------

/**
 * Called to request that the Loader obtain more object ID's and send
 * them to the identified process.
 */
void Loader::getObjectIds(int processId, int numWanted, bool logRequest)
{
	TaskGetObjectIds *tgo=new TaskGetObjectIds(processId, numWanted, logRequest);
	taskQ->asyncRequest(tgo);
}

// ----------------------------------------------------------------------

void Loader::loadClock()
{
	taskQ->asyncRequest(new TaskLoadClock);
}

// ----------------------------------------------------------------------

/**
 * After the clock has been loaded, do any additional tasks needed
 */
void Loader::loadClockCompleted()
{
	DEBUG_FATAL(!ServerClock::getInstance().isSet(),("Clock was not set.\n"));
	loadStartupData();
}

// ----------------------------------------------------------------------

/**
 * After all initial data is loaded, tell Central we are ready.
 */
void Loader::startupLoadCompleted() const
{
	CentralGameServerDbProcessServerProcessId m(DatabaseProcess::getInstance().getProcessId(), ServerClock::getInstance().getSubtractInterval());
	DatabaseProcess::getInstance().sendToCentralServer(m,true);
}

// ----------------------------------------------------------------------

/**
 * @todo We'll need to add a map of game server -> load request at some point.
 * when we do, revisit this function.  The list of crashed game servers
 * can grow without bounds right now, because we can't tell when we're
 * all done loading data for a server.
 */
void Loader::discardPendingLoadsForServer(uint32 processId)
{
	IGNORE_RETURN(m_serverDiscardList.insert(processId));
	IGNORE_RETURN(m_preloadingProcesses.erase(processId));

	for (UnackedLoadsTimeType::iterator iter = m_unackedLoadsTime.begin(); iter != m_unackedLoadsTime.end();)
	{
		if (iter->second.first == processId)
			m_unackedLoadsTime.erase(iter++);
		else
			++iter;
	}

	UnackedLoadsType::iterator iterFind = m_unackedLoads.find(processId);

	if (iterFind != m_unackedLoads.end())
		m_unackedLoads.erase(iterFind);
}

// ----------------------------------------------------------------------

/**
 * Check that the database version number matches what this compile of the
 * code needs.
 */
void Loader::checkVersionNumber(int expectedVersion, bool fatalOnMismatch)
{
	taskQ->asyncRequest(new TaskCheckVersionNumber(expectedVersion, fatalOnMismatch));
	
	if (ConfigServerDatabase::getEnableGoldDatabase())
	{
		DEBUG_REPORT_LOG(true,("Checking GOLD database version.\n"));

		taskQ->asyncRequest(new TaskCheckGoldVersionNumber(expectedVersion, fatalOnMismatch));
	}
}

// ----------------------------------------------------------------------

void Loader::requestChunk(uint32 processId,int nodeX, int nodeZ, const std::string &sceneId)
{
	ObjectLocator * const regularLocator=new ChunkLocator(nodeX, nodeZ, sceneId, processId, true);
	ObjectLocator * goldLocator=nullptr;
	if (ConfigServerDatabase::getEnableGoldDatabase())
		goldLocator = new ChunkLocator(nodeX, nodeZ, sceneId, processId, false);
	addLocatorsForServer(processId, regularLocator, goldLocator);
}

// ----------------------------------------------------------------------

void Loader::requestCharacter(const NetworkId &characterId, uint32 gameServerId)
{
	if (ConfigServerDatabase::getEnableLoadLocks())
	{
		LoadLockType::iterator i=m_loadLock.find(characterId);
		if (i!=m_loadLock.end())
		{
			DEBUG_REPORT_LOG(true,("Delaying login for character %s because there is remaining data to be saved\n",characterId.getValueString().c_str()));
			DEBUG_REPORT_LOG(i->second!=0,("Character %s already had a pending login request -- replacing it with the new request.\n",characterId.getValueString().c_str()));
			i->second=gameServerId;
		}
		else
		{
			i=m_multipleLoginLock.find(characterId);
			if (i==m_multipleLoginLock.end())
			{
				addLocatorsForServer(gameServerId, new CharacterLocator(characterId), nullptr);

				DEBUG_REPORT_LOG(true,("Adding multipleLoginLock for %s\n",characterId.getValueString().c_str()));
				m_multipleLoginLock[characterId] = gameServerId;
			}
			else
			{
				LOG("TRACE_LOGIN",("Ignoring duplicate request to load character %s",characterId.getValueString().c_str()));
				DEBUG_REPORT_LOG(true,("Ignoring duplicate request to load character %s\n",characterId.getValueString().c_str()));
			}
		}
	}
	else
		addLocatorsForServer(gameServerId, new CharacterLocator(characterId), nullptr);
}

// ----------------------------------------------------------------------

void Loader::loadContainedObject(const NetworkId &containerId, const NetworkId &objectId, uint32 gameServerId)
{
	LOG("AuctionRetrieval", ("Loader::received loadContainedObject for loading object %s for retrieval", objectId.getValueString().c_str()));
	addLocatorsForServer(gameServerId, new ContainedObjectLocator(containerId,objectId), nullptr);
}


// ----------------------------------------------------------------------

void Loader::loadContents(const NetworkId &containerId, uint32 gameServerId)
{
	addLocatorsForServer(gameServerId, new ContentsLocator(containerId), nullptr);
}

// ----------------------------------------------------------------------

void Loader::shutdown()
{
	NOT_NULL(taskQ);
	taskQ->cancel();
	delete taskQ;
	taskQ = 0;
}

// ----------------------------------------------------------------------

void Loader::preloadRequestComplete(uint32 processId, uint32 preloadAreaId)
{
	LOG("Preload",("Received all preload requests for server %lu",processId));
	if (getUnackedLoadCount(processId) > 0 || m_locatorList.find(processId)!=m_locatorList.end())
	{
		if (m_serverDiscardList.find(processId) == m_serverDiscardList.end())
			m_preloadingProcesses.insert(std::make_pair(processId,preloadAreaId));
		else
			IGNORE_RETURN(m_preloadingProcesses.erase(processId));
	}
	else
	{
		LOG("Preload",("Done sending preload objects to server %lu.",processId));
		PreloadRequestCompleteMessage msg(processId, preloadAreaId);
		DatabaseProcess::getInstance().sendToGameServer(processId, msg);
	}
}

// ----------------------------------------------------------------------

void Loader::addLoadLock(const NetworkId &characterId)
{
	m_loadLock[characterId]=0;
}

void Loader::handleCSGetPlayerId( const DBCSRequestMessage & msg )
{
	std::string charNameString;
	std::string::size_type pos = msg.getCommandLine().find( ' ' );
	if( pos == std::string::npos )
		return;
		
	charNameString = msg.getCommandLine().substr( pos + 1 );
	TaskGetCharacterId * task = new TaskGetCharacterId( charNameString, msg.getLoginServerId(), msg.getToolId() );
	taskQ->asyncRequest( task );

}

void Loader::handleCSGetStructures( const DBCSRequestMessage & msg )
{
	std::string charIdString;
	std::string::size_type pos = msg.getCommandLine().find( ' ' );
	if( pos == std::string::npos )
		return; // bad parameters.
	charIdString = msg.getCommandLine().substr( pos + 1 );
	NetworkId characterId( charIdString );
	if( characterId.isValid() )
	{
		TaskGetStructures * req = new TaskGetStructures( characterId,  msg.getToolId(), msg.getLoginServerId() );
		taskQ->asyncRequest( req );
	}
	
}

void Loader::handleCSGetCharacters( const DBCSRequestMessage & msg )
{
	uint32 account_id = 0;
	sscanf( msg.getCommandLine().c_str(), "get_characters %lu", &account_id );
	TaskGetCharactersForAccount * req = new TaskGetCharactersForAccount( account_id, msg.getLoginServerId(), msg.getToolId() );
	taskQ->asyncRequest( req );
	
}

void Loader::handleCSGetCharacters( const CSGetCharactersRequestMessage & msg )
{
	TaskGetCharactersForAccount * req = new TaskGetCharactersForAccount( msg.getTargetAccountId(), msg.getLoginServerId(), msg.getToolId() );
	taskQ->asyncRequest( req );
	
}

void Loader::handleCSGetDeletedItems( const DBCSRequestMessage & msg )
{
	
	uint32 page_num;
	char buf [21];

	if (sscanf (msg.getCommandLine().c_str(), "get_deleted_items %20s %lu", buf, &page_num) == 2)
	{
		NetworkId netId(buf);
		if(!netId.isValid())
			return;
		TaskGetDeletedItems *req = new TaskGetDeletedItems(netId, msg.getLoginServerId(), msg.getToolId(), page_num);
		taskQ->asyncRequest( req );
	}
}

void Loader::handleCSGetDeletedItems( const CSGetDeletedItemsRequestMessage & msg )
{
	UNREF(msg);
//	TaskGetDeletedItems * req = new TaskGetDeletedItems( msg.getTargetAccountId(), msg.getLoginServerId(), msg.getToolId() );
//	taskQ->asyncRequest( req );
}
// ----------------------------------------------------------------------

void Loader::removeLoadLock(const NetworkId &characterId)
{
	LoadLockType::iterator i=m_loadLock.find(characterId);
	if (i==m_loadLock.end())
	{
		DEBUG_REPORT_LOG(true,("Tried to remove the load lock for character %s, but it wasn't locked.  Probably indicates the player logged out twice within the same save cycle.\n",characterId.getValueString().c_str()));
		return;
	}
	if (i->second!=0)
	{
		DEBUG_REPORT_LOG(true,("Now handling delayed login request for character %s\n",characterId.getValueString().c_str()));
		addLocatorsForServer(i->second, new CharacterLocator(characterId), nullptr);
	}

	m_loadLock.erase(i);
}

// ----------------------------------------------------------------------

bool Loader::isIdle()
{
	bool result = false;
	if(taskQ)
		result = (m_locatorList.empty() && taskQ->isIdle());
	return result;
}

// ----------------------------------------------------------------------

/**
 * Estimate the number of tasks waiting for the load queue.  This is the
 * number of actual tasks scheduled plus an estimate of how many tasks
 * the queued locators will produce.
 */
int Loader::getLoadQueueTasksEstimate() const
{
	int result = 0;
	if(taskQ)
		result = (taskQ->getNumPendingTasks() + taskQ->getNumActiveTasks()) + m_numQueuedLocators/ConfigServerDatabase::getMaxChunksPerLoadRequest();
	return result;
}

// ----------------------------------------------------------------------

void Loader::onCharacterLocatorDeleted(const NetworkId &characterId)
{
	DEBUG_REPORT_LOG(true,("Removing multipleLoginLock for %s\n",characterId.getValueString().c_str()));
	IGNORE_RETURN(m_multipleLoginLock.erase(characterId));
}

// ----------------------------------------------------------------------

unsigned int Loader::getUnackedLoadCount(uint32 serverId) const
{
	UnackedLoadsType::const_iterator iterFind = m_unackedLoads.find(serverId);

	if (iterFind == m_unackedLoads.end())
		return 0;

	return iterFind->second.size();
}

// ----------------------------------------------------------------------

/**
 * The game server has acknowledged a batch of objects we sent.  Remove the
 * record of the unacknowledged load.
 */
void Loader::handleLoadAck(uint32 serverId, int serialNumber)
{
	for (UnackedLoadsTimeType::iterator iter = m_unackedLoadsTime.begin(); iter != m_unackedLoadsTime.end(); ++iter)
	{
		if ((iter->second.first == serverId) && (iter->second.second == serialNumber))
		{
			m_unackedLoadsTime.erase(iter);
			break;
		}
	}

	UnackedLoadsType::iterator iterFind = m_unackedLoads.find(serverId);

	if (iterFind == m_unackedLoads.end())
		return;

	std::vector<int>::iterator iterFind2 = std::find(iterFind->second.begin(), iterFind->second.end(), serialNumber);

	if (iterFind2 ==  iterFind->second.end())
		return;

	IGNORE_RETURN(iterFind->second.erase(iterFind2));

	bool hasMoreUnacked = true;
	if (iterFind->second.empty())
	{
		m_unackedLoads.erase(iterFind);
		hasMoreUnacked = false;
	}

	PreloadingProcessesType::iterator j=m_preloadingProcesses.find(serverId);
	if (j != m_preloadingProcesses.end() &&
		!hasMoreUnacked &&
		m_locatorList.find(serverId) == m_locatorList.end())
	{
		// Server was preloading, and it has acknowledged all the requested loads
		// Tell it that preloading is done
		LOG("Preload",("Done sending preload objects to server %lu.",serverId));
		PreloadRequestCompleteMessage msg(serverId, j->second);
		DatabaseProcess::getInstance().sendToGameServer(serverId, msg);
		m_preloadingProcesses.erase(j);
	}
}

// ----------------------------------------------------------------------

void Loader::addLocatorsForServer(uint32 serverId, ObjectLocator * regularLocator, ObjectLocator * goldLocator)
{
	LocatorListType::iterator i=m_locatorList.find(serverId);
	if (i==m_locatorList.end())
		i=m_locatorList.insert(std::make_pair(serverId, new std::list<std::pair<ObjectLocator*, ObjectLocator*> >)).first;

	NON_NULL(i->second)->push_back(std::make_pair(regularLocator, goldLocator));
	++m_numQueuedLocators;
}

// ----------------------------------------------------------------------

int Loader::getNumPreloads() const
{
	return m_preloadingProcesses.size();
}

// ----------------------------------------------------------------------

int Loader::getAgeOldestUnackedLoad() const
{
	if (m_unackedLoadsTime.empty())
	{
		return 0;
	}
	else
	{
		UnackedLoadsTimeType::const_iterator i = m_unackedLoadsTime.begin();
		return static_cast<int>(time(0) - i->first);
	}
}

// ----------------------------------------------------------------------

uint32 Loader::getServerOldestUnackedLoad() const
{
	if (m_unackedLoadsTime.empty())
	{
		return 0;
	}
	else
	{
		UnackedLoadsTimeType::const_iterator i = m_unackedLoadsTime.begin();
		return i->second.first;
	}
}

// ======================================================================
