// ======================================================================
//
// Persister.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/Persister.h"

#include "serverDatabase/CentralServerConnection.h"
#include "serverDatabase/CommoditiesServerConnection.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/CreateCharacterCustomPersistStep.h"
#include "serverDatabase/DataLookup.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/DeleteCharacterCustomPersistStep.h"
#include "serverDatabase/FixLoadWithCustomPersistStep.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/MessageToManager.h"
#include "serverDatabase/MoveToPlayerCustomPersistStep.h"
#include "serverDatabase/PurgeCompleteCustomPersistStep.h"
#include "serverDatabase/RenameCharacterCustomPersistStep.h"
#include "serverDatabase/Snapshot.h"
#include "serverDatabase/TaskCSTasks.h"
#include "serverDatabase/TaskSaveSnapshot.h"
#include "serverDatabase/TaskChangeStationId.h"
#include "serverDatabase/UnloadCharacterCustomPersistStep.h"
#include "serverNetworkMessages/AddCharacterMessage.h"
#include "serverNetworkMessages/AddResourceTypeMessage.h"
#include "serverNetworkMessages/BountyHunterTargetMessage.h"
#include "serverNetworkMessages/CMCreateAuctionBidMessage.h"
#include "serverNetworkMessages/CMCreateAuctionMessage.h"
#include "serverNetworkMessages/CMCreateLocationMessage.h"
#include "serverNetworkMessages/CMDeleteAuctionMessage.h"
#include "serverNetworkMessages/CMDeleteLocationMessage.h"
#include "serverNetworkMessages/CMUpdateAuctionMessage.h"
#include "serverNetworkMessages/CMUpdateLocationMessage.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/CreateObjectMessage.h"
#include "serverNetworkMessages/CSDBNetMessages.h"
#include "serverNetworkMessages/EndBaselinesMessage.h"
#include "serverNetworkMessages/FlagObjectForDeleteMessage.h"
#include "serverNetworkMessages/GetMoneyFromOfflineObjectMessage.h"
#include "serverNetworkMessages/MessageToAckMessage.h"
#include "serverNetworkMessages/MessageToMessage.h"
#include "serverNetworkMessages/RenameCharacterMessage.h"
#include "serverNetworkMessages/ServerDeleteCharacterMessage.h"
#include "serverNetworkMessages/TransferAccountData.h"
#include "serverNetworkMessages/UnloadedPlayerMessage.h"
#include "serverNetworkMessages/UpdateObjectPositionMessage.h"
#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"
#include "sharedNetworkMessages/DeleteCharacterMessage.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetworkMessages/DeltasMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"

#include "sharedFoundation/CrcConstexpr.hpp"

#include <utility>
#include <algorithm>

// ======================================================================

Persister *Persister::ms_instance=nullptr;

// ======================================================================

void Persister::installDerived(Persister *derivedInstance)
{
	DEBUG_FATAL(ms_instance,("Installed Persister twice\n"));
	ms_instance = derivedInstance;
	
	ExitChain::add(&remove, "Persister::remove");
}

// ----------------------------------------------------------------------

void Persister::remove()
{
	NOT_NULL(ms_instance);
	delete ms_instance;
	ms_instance = nullptr;
}

//-----------------------------------------------------------------------

Persister::Persister() :
		MessageDispatch::Receiver(),
		taskQueue(new DB::TaskQueue(ConfigServerDatabase::getPersisterThreads(),DatabaseProcess::getInstance().getDBServer(),1)),
		m_newCharacterTaskQueue(new DB::TaskQueue(ConfigServerDatabase::getNewCharacterThreads(),DatabaseProcess::getInstance().getDBServer(),3)),
		m_currentSnapshots(),
		m_newObjectSnapshots(),
		m_newCharacterSnapshots(),
		m_objectSnapshotMap(),
		m_pendingCharacters(),
		m_savingSnapshots(),
		m_savingCharacterSnapshots(),
		m_newCharacterLock(),
		m_charactersToDeleteThisSaveCycle(new CharactersToDeleteType),
		m_charactersToDeleteNextSaveCycle(new CharactersToDeleteType),
		m_timeSinceLastSave(0),
		m_messageSnapshot(nullptr),
		m_commoditiesSnapshot(nullptr),
		m_arbitraryGameDataSnapshot(nullptr),
		m_saveStartTime(0),
		m_totalSaveTime(0),
		m_maxSaveTime(0),
		m_saveCount(0),
		m_newObjectCount(0),
		m_lastSaveTime(0),
		m_lastSaveTotalObjectCount(0),
		m_lastSaveNewObjectCount(0),
		m_lastSaveCompletionTime(),
		m_saveCounter(0),
		m_startSaveWhenPossible(false),
		m_inMagicMinute(false),
		m_clusterShuttingDown(false)
{
	connectToMessage("AddCharacterMessage");
	connectToMessage("AddResourceTypeMessage");
	connectToMessage("BaselinesMessage");
	connectToMessage("BountyHunterTargetMessage");
	connectToMessage("CentralRequestSave");
	connectToMessage("ClusterShutdownMessage");
	connectToMessage("CMCreateAuctionBidMessage");
	connectToMessage("CMCreateAuctionMessage");
	connectToMessage("CMCreateLocationMessage");
	connectToMessage("CMDeleteAuctionMessage");
	connectToMessage("CMDeleteLocationMessage");
	connectToMessage("CMUpdateAuctionMessage");
	connectToMessage("CMUpdateLocationMessage");
	connectToMessage("CreateObjectByCrcMessage");
	connectToMessage("DeltasMessage");
	connectToMessage("EndBaselinesMessage");
	connectToMessage("FixLoadWith");
	connectToMessage("FlagObjectForDeleteMessage");
	connectToMessage("GetMoneyFromOfflineObjectMessage");
	connectToMessage("LoadCommodities");
	connectToMessage("MessageToAckMessage");
	connectToMessage("MessageToMessage");
	connectToMessage("MoveToPlayer");
	connectToMessage("MoveToPlayerBankMessage");
	connectToMessage("MoveToPlayerDatapadMessage");
	connectToMessage("MoveToPlayerMessage");
	connectToMessage("PlanetRequestSave");
	connectToMessage("PurgeCompleteMessage");
	connectToMessage("RenameCharacterMessageEx");
	connectToMessage("RestoreCharacterMessage");
	connectToMessage("RestoreHouseMessage");
	connectToMessage("ServerDeleteCharacterMessage");
	connectToMessage("StartSaveMessage");
	connectToMessage("UndeleteItemMessage");
	connectToMessage("UnloadObjectMessage");
	connectToMessage("UnloadedPlayerMessage");
	connectToMessage("UpdateObjectPositionMessage");
	connectToMessage("DBCSRequestMessage");
	connectToMessage("UndeleteItemForCsMessage");
}

//-----------------------------------------------------------------------

Persister::~Persister()
{
	DEBUG_FATAL(taskQueue,("Call shutdown() before deleting Persister.\n"));

	ServerSnapshotMap::iterator i;
	for (i=m_currentSnapshots.begin(); i!=m_currentSnapshots.end(); ++i)
		delete i->second;
	for (i=m_newObjectSnapshots.begin(); i!=m_newObjectSnapshots.end(); ++i)
		delete i->second;

	m_currentSnapshots.clear();
	m_newObjectSnapshots.clear();
	m_objectSnapshotMap.clear();
	m_messageSnapshot = nullptr;
	m_commoditiesSnapshot = nullptr;
	m_arbitraryGameDataSnapshot = nullptr;

	delete m_charactersToDeleteThisSaveCycle;
	m_charactersToDeleteThisSaveCycle = nullptr;

	delete m_charactersToDeleteNextSaveCycle;
	m_charactersToDeleteNextSaveCycle = nullptr;
}

// ----------------------------------------------------------------------

void Persister::update(real updateTime)
{
	m_timeSinceLastSave += updateTime;
		
	{
		PROFILER_AUTO_BLOCK_DEFINE("taskQueue->update");
		taskQueue->update(ConfigServerDatabase::getDefaultQueueUpdateTimeLimit());
	}
	{
		PROFILER_AUTO_BLOCK_DEFINE("m_newCharacterTaskQueue->update");
		m_newCharacterTaskQueue->update(ConfigServerDatabase::getDefaultQueueUpdateTimeLimit());
	}
}

// ----------------------------------------------------------------------

void Persister::onFrameBarrierReached()
{
	if (m_newCharacterTaskQueue->getNumPendingTasks() == 0)
	{
		ServerSnapshotMap delayedSaves;
		
		for (auto i=m_newCharacterSnapshots.begin(); i!=m_newCharacterSnapshots.end(); ++i)
		{
			if (m_newCharacterLock.find(i->first)==m_newCharacterLock.end())
			{
				DEBUG_REPORT_LOG(ConfigServerDatabase::getReportSaveTimes(),("Starting new character save\n"));
				m_savingCharacterSnapshots.push_back(i->second);
				m_newCharacterTaskQueue->asyncRequest(new TaskSaveSnapshot(i->second));
				for (ObjectSnapshotMap::iterator obj=m_objectSnapshotMap.begin(); obj!=m_objectSnapshotMap.end();)
				{
					if (obj->second == i->second)
						obj = m_objectSnapshotMap.erase(obj);
					else
						++obj;
				}
			}
			else
			{
				delayedSaves[i->first]=i->second;
				DEBUG_REPORT_LOG(true,("New character persistence for server %lu delayed because the server split character baselines across a frame boundary.\n",i->first));
			}
		}
		m_newCharacterSnapshots.swap(delayedSaves);
	}

	// Check for whether it is time to save

	FATAL(m_timeSinceLastSave > ConfigServerDatabase::getMaxTimewarp(),("Save was not completed within %i seconds.  Shutting down to avoid a longer timewarp.\n",ConfigServerDatabase::getMaxTimewarp()));

	if (ConfigServerDatabase::getSaveAtModulus()!=-1)
	{
		// save based on being a certain # of minutes past the hour
		tm zulu;
		Os::convertTimeToGMT(Os::getRealSystemTime(),zulu);
		if ((zulu.tm_min % ConfigServerDatabase::getSaveAtDivisor())==ConfigServerDatabase::getSaveAtModulus())
		{
			if (!m_inMagicMinute)
			{
				m_inMagicMinute = true; // to make sure we don't start multiple saves in the same minute
				m_startSaveWhenPossible = true;
			}
		}
		else
			m_inMagicMinute = false;
	}

	if (m_timeSinceLastSave > ConfigServerDatabase::getSaveFrequencyLimit())
		m_startSaveWhenPossible = true;

	if (m_startSaveWhenPossible)
	{
		if (m_savingSnapshots.empty())
		{
			m_startSaveWhenPossible = false;
			startSave();
		}
		else if (ConfigServerDatabase::getReportSaveTimes())
		{
			DEBUG_REPORT_LOG(true,("Waiting for previous save.  %i objects queued (%i new).\n",m_objectSnapshotMap.size(),m_newObjectCount));
			taskQueue->report();
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Moves the current & new object snapshots onto the queue to be saved.
 *
 * Does nothing if these snapshots are nullptr.
 */

void Persister::startSave(void)
{
	DEBUG_REPORT_LOG(ConfigServerDatabase::getReportSaveTimes(),("Starting save with data for %i objects (%i new)\n",m_objectSnapshotMap.size(), m_newObjectCount));
	LOG("SaveTimes",("Starting save with data for %i objects (%i new)",m_objectSnapshotMap.size(), m_newObjectCount));
	// notify Central that a save is starting.  It needs this to determine when it can perform graceful shutdowns
	LOG("Database",("Sending DatabaseSaveStart network message to Central."));
	DatabaseSaveStart const startSaveMessage;
	DatabaseProcess::getInstance().sendToCentralServer(startSaveMessage, true);

	// record metrics data
	m_lastSaveTotalObjectCount=m_objectSnapshotMap.size();
	m_lastSaveNewObjectCount=m_newObjectCount;
	
	m_newObjectCount=0;

	// delete any characters for this save cycle
	if (m_charactersToDeleteNextSaveCycle && m_charactersToDeleteThisSaveCycle)
	{
		for (auto iter = m_charactersToDeleteThisSaveCycle->begin(); iter != m_charactersToDeleteThisSaveCycle->end(); ++iter)
		{
			DeleteCharacterCustomPersistStep *cps = new DeleteCharacterCustomPersistStep(iter->first, iter->second);
			getSnapshotForObject(iter->second, 0)->addCustomPersistStep(cps);
		}

		// if this is the final save before the cluster is brought down,
		// delete characters for the next save cycle as well since there
		// won't be a next save cycle
		if (m_clusterShuttingDown)
		{
			for (CharactersToDeleteType::const_iterator iter2 = m_charactersToDeleteNextSaveCycle->begin(); iter2 != m_charactersToDeleteNextSaveCycle->end(); ++iter2)
			{
				DeleteCharacterCustomPersistStep *cps = new DeleteCharacterCustomPersistStep(iter2->first, iter2->second);
				getSnapshotForObject(iter2->second, 0)->addCustomPersistStep(cps);
			}
		}
	}
	
	// pick an arbitrary snapshot to save the clock
	if (!m_currentSnapshots.empty())
		m_currentSnapshots.begin()->second->takeTimestamp();

	// queue up snapshots to be saved
	ServerSnapshotMap::iterator i;
	for (i=m_currentSnapshots.begin(); i!=m_currentSnapshots.end(); ++i)
	{
		m_savingSnapshots.push_back(i->second);
		taskQueue->asyncRequest(new TaskSaveSnapshot(i->second));
	}
	for (i=m_newObjectSnapshots.begin(); i!=m_newObjectSnapshots.end(); ++i)
	{
		m_savingSnapshots.push_back(i->second);
		taskQueue->asyncRequest(new TaskSaveSnapshot(i->second));
	}

	// nothing changed so send a complete message for the shutdown process
	if( m_savingSnapshots.empty() )
	{
		GenericValueTypeMessage<int> const saveCompleteMessage("DatabaseSaveComplete", ++m_saveCounter);
		DatabaseProcess::getInstance().sendToCentralServer(saveCompleteMessage, true);
		LOG("Database",("Sending DatabaseSaveComplete network message to Central."));
	}

	// clear the buffers
	m_currentSnapshots.clear();
	m_newObjectSnapshots.clear();
	m_objectSnapshotMap.clear();
	m_messageSnapshot = nullptr;
	m_commoditiesSnapshot = nullptr;
	m_arbitraryGameDataSnapshot = nullptr;

	// prepare the list of characters to delete during the next save cycle
	if (m_charactersToDeleteNextSaveCycle && m_charactersToDeleteThisSaveCycle)
	{
		m_charactersToDeleteThisSaveCycle->clear();

		if (m_clusterShuttingDown)
			m_charactersToDeleteNextSaveCycle->clear();
		else
			std::swap(m_charactersToDeleteThisSaveCycle, m_charactersToDeleteNextSaveCycle);
	}

	m_timeSinceLastSave = 0;

	if (ConfigServerDatabase::getReportSaveTimes())
		m_saveStartTime = Clock::timeMs();
}

// ----------------------------------------------------------------------

/**
 * Determine the appropriate snapshot to put an object in.  (If this is
 * the first time we've seen the object, remember which snapshot we
 * decided to use for it.)
 */
Snapshot * Persister::getSnapshotForObject(const NetworkId &networkId, uint32 serverId)
{
	auto i = m_objectSnapshotMap.find(networkId);
	if (i!=m_objectSnapshotMap.end())
	{
		NOT_NULL(i->second);
		return i->second;
	}
	else
	{
		Snapshot *snap = getSnapshotForServer(serverId);

		m_objectSnapshotMap[networkId]=snap;
		return snap;
	}
}

// ----------------------------------------------------------------------

bool Persister::hasDataForObject(const NetworkId &objectId) const
{
	auto i=m_objectSnapshotMap.find(objectId);
	return (i!=m_objectSnapshotMap.end());
}

// ----------------------------------------------------------------------

Snapshot *Persister::getSnapshotForServer(uint32 serverId)
{
	if (serverId==0)
	{
		if (!m_arbitraryGameDataSnapshot)
		{
			m_arbitraryGameDataSnapshot = makeSnapshot(DB::ModeQuery::mode_UPDATE);
			m_currentSnapshots[0] = m_arbitraryGameDataSnapshot;
		}
		return m_arbitraryGameDataSnapshot;
	}
	else
	{
		auto j=m_currentSnapshots.find(serverId);
	
		if (j==m_currentSnapshots.end())
		{
			Snapshot *snap = makeSnapshot(DB::ModeQuery::mode_UPDATE);
			m_currentSnapshots[serverId]=snap;
			
			if (!m_arbitraryGameDataSnapshot) {
				m_arbitraryGameDataSnapshot = snap;
			}
			return snap;

		}
		else
		{
			NOT_NULL (j->second);
			return j->second;
		}
	}
}

// ----------------------------------------------------------------------

Snapshot *Persister::getCommoditiesSnapshot(uint32 serverId)
{
	if (m_commoditiesSnapshot)
	{
		return m_commoditiesSnapshot;
	}
	else
	{
		auto j = m_currentSnapshots.find(serverId);
		if (j==m_currentSnapshots.end())
		{
			Snapshot *snap = makeCommoditiesSnapshot(DB::ModeQuery::mode_INSERT);
			m_currentSnapshots[serverId]=snap;
			m_commoditiesSnapshot = snap;
			return snap;
		}
		else
		{
			NOT_NULL (j->second);
			m_commoditiesSnapshot = j->second;
			return j->second;
		}
	}
}

// ----------------------------------------------------------------------

void Persister::handleDeltasMessage(uint32 serverId, const DeltasMessage &msg)
{
	NetworkId objectId=msg.getTarget();
//	DEBUG_REPORT_LOG(true,("Got deltas message for object %s.\n",objectId.getValueString().c_str()));
	getSnapshotForObject(objectId,serverId)->handleDeltasMessage(objectId, msg);
}

// ----------------------------------------------------------------------

void Persister::handleBaselinesMessage(uint32 serverId, const BaselinesMessage &msg)
{
//	DEBUG_REPORT_LOG(true,("Got baselines message.\n"));
	
	NetworkId objectId=msg.getTarget();
	getSnapshotForObject(objectId,serverId)->handleBaselinesMessage(objectId, msg);
}

// ----------------------------------------------------------------------
/** 
 * Called when we receive a delete message for an object.
 * @param  objectID The object to delete.
 */

void Persister::handleDeleteMessage(uint32 serverId, const NetworkId &objectId, int reasonCode, bool immediate, bool demandLoadedContainer, bool cascadeReason)
{
	getSnapshotForObject(objectId,serverId)->handleDeleteMessage(objectId, reasonCode, immediate, demandLoadedContainer, cascadeReason);
}

// ----------------------------------------------------------------------

/**
 * Invoked when we receive a CreateObject message.
 *
 * Creates a Snapshot to hold the new object and prepares to receive baselines.
 */

void Persister::newObject(uint32 serverId, const NetworkId &objectId, int templateId, Tag typeId, const NetworkId &container)
{
	UNREF(serverId);

	if (m_objectSnapshotMap.find(objectId)!=m_objectSnapshotMap.end())
	{
		DEBUG_WARNING(true,("Database received multiple new object messages for object %s",objectId.getValueString().c_str()));
		return;
	}

	Snapshot *snap=nullptr;

	auto chardata=m_pendingCharacters.find(objectId);
	if (chardata!=m_pendingCharacters.end())
	{
		// Object is a new character
		ServerSnapshotMap::iterator i=m_newCharacterSnapshots.find(serverId);
		if (i!=m_newCharacterSnapshots.end())
		{
			snap = i->second;
		}
		else
		{
			snap=makeSnapshot(DB::ModeQuery::mode_INSERT);
			m_newCharacterSnapshots[serverId]=snap;
		}

		snap->addCustomPersistStep(
			new CreateCharacterCustomPersistStep(chardata->second.stationId,
												 chardata->first,
												 chardata->second.name,
												 Unicode::wideToNarrow(DataLookup::getInstance().normalizeName(chardata->second.name)),
												 templateId,
												 chardata->second.special));
	}
	else
	{
		// Add the object to the appropriate snapshot
		snap=nullptr;
		{
			auto j = m_objectSnapshotMap.find(container);
			if (j!=m_objectSnapshotMap.end() && j->second->getMode() == DB::ModeQuery::mode_INSERT)
				snap = j->second;
			else
			{
				auto i = m_newObjectSnapshots.find(serverId);
				if (i==m_newObjectSnapshots.end())
				{
					snap=makeSnapshot(DB::ModeQuery::mode_INSERT);
					m_newObjectSnapshots[serverId]=snap;
				}
				else
					snap = i->second;
			}
		}
	}

	++m_newObjectCount;
	NOT_NULL(snap);
	snap->newObject(objectId, templateId, typeId);
	m_objectSnapshotMap[objectId]=snap;
}

// ----------------------------------------------------------------------

/**
 * Invoked when we receive an EndBaselines message.
 *
 * Starts saving the object to the database.
 */

void Persister::endBaselines(const NetworkId &objectId, uint32 serverId) 
{
	//TODO:  This is a hack until we remove frame boundaries and have "end frame" messages from the game server.  Apparently the game
	// server can split baselines across frame boundaries, so we can't assume we have all the data for a character when we hit a
	// frame bounday.
	auto chardata=m_pendingCharacters.find(objectId);
	if (chardata!=m_pendingCharacters.end())
	{
		m_pendingCharacters.erase(chardata);
		m_newCharacterLock.erase(serverId);
	}
}

// ----------------------------------------------------------------------

/**
 * Called by TaskSaveSnapshot when it finishes.
 */

void Persister::saveCompleted(Snapshot *completedSnapshot)
{
	bool found = false;
	for (auto i = m_savingSnapshots.begin(); i != m_savingSnapshots.end();) {
		if (*i == completedSnapshot) {
			i = m_savingSnapshots.erase(i);
			found = true;
		} else {
			++i;
		}
	}

	if (m_savingSnapshots.empty())
	{
		if (found && ConfigServerDatabase::getReportSaveTimes()) {
			int saveTime = Clock::timeMs() - m_saveStartTime;
			++m_saveCount;
			m_totalSaveTime += saveTime;
			if (saveTime > m_maxSaveTime)
				m_maxSaveTime = saveTime;

				DEBUG_REPORT_LOG(true,("Save completed in %i.  (Average %i, max %i)\n", saveTime, m_totalSaveTime/m_saveCount, m_maxSaveTime));
				LOG("SaveTimes",("Save completed in %i.  (Average %i, max %i)", saveTime, m_totalSaveTime/m_saveCount, m_maxSaveTime));
	
				m_lastSaveTime = saveTime;
		}

		LOG("Database",("Sending DatabaseSaveComplete network message to Central."));

		// TODO: so do we send this for the other snapshot type or not? hrmph
                // message Central Server that the current save cycle is complete
                GenericValueTypeMessage<int> const saveCompleteMessage("DatabaseSaveComplete", ++m_saveCounter);
                DatabaseProcess::getInstance().sendToCentralServer(saveCompleteMessage, true);
                LOG("Database",("Sending DatabaseSaveComplete network message to Central."));
	}
		
	if (!found) {		
		for (auto i = m_savingCharacterSnapshots.begin(); i != m_savingCharacterSnapshots.end();) {
			if (*i == completedSnapshot) {
				i = m_savingCharacterSnapshots.erase(i);
				found = true;
			} else {
				++i;
			}
		}
	
		DEBUG_REPORT_LOG(ConfigServerDatabase::getReportSaveTimes(),("New character save completed\n"));
	}

	if (found && completedSnapshot != nullptr) {
		delete completedSnapshot;
		completedSnapshot = nullptr;
	}
}


// ----------------------------------------------------------------------

void Persister::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	const GameServerConnection * gameConnection = dynamic_cast<const GameServerConnection *>(&source);
	uint32 sourceGameServer = 0;
	if (gameConnection)
		sourceGameServer = gameConnection->getProcessId();
	
	const uint32 messageType = message.getType();
	
	switch(messageType) {
		case constcrc("FlagObjectForDeleteMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			FlagObjectForDeleteMessage m(ri);
			handleDeleteMessage(sourceGameServer, m.getId(),m.getReason(),m.getImmediate(),m.getDemandLoadedContainer(),m.getCascadeReason());
			break;
		}
		case constcrc("CreateObjectByCrcMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CreateObjectByCrcMessage t(ri);
			// DEBUG_REPORT_LOG(true,("Got CreateObjectByCrcMessage for %s\n", t.getId().getValueString().c_str()));
			newObject(sourceGameServer, t.getId(), t.getCrc(), t.getObjectType(), t.getContainer());
			break;
		}
		case constcrc("EndBaselinesMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			EndBaselinesMessage t(ri);
			endBaselines(t.getId(),sourceGameServer);
			break;
		}
		case constcrc("DeltasMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			DeltasMessage msg(ri);

			handleDeltasMessage(sourceGameServer,msg);
			break;
		}
		case constcrc("BaselinesMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			BaselinesMessage msg(ri);

			handleBaselinesMessage(sourceGameServer,msg);
			break;
		}
		case constcrc("UpdateObjectPositionMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			UpdateObjectPositionMessage msg(ri);

			getSnapshotForObject(msg.getNetworkId(), sourceGameServer)->handleUpdateObjectPosition(msg);
			break;
		}
		case constcrc("AddCharacterMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			AddCharacterMessage ocm(ri);
			DEBUG_REPORT_LOG(true, ("Got AddCharacterMessage for object %s.\n",ocm.getObjectId().getValueString().c_str()));

			addCharacter(ocm.getAccountNumber(), ocm.getObjectId(), ocm.getProcess(), ocm.getName(), ocm.getSpecial());
			break;
		}
		case constcrc("UnloadObjectMessage") :
		{
			// TODO:  keep track of when it's done saving so we know when we can reload it
			break;
		}
		case constcrc("MessageToMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			MessageToMessage m(ri);

			handleMessageTo (sourceGameServer, m.getData());
			break;
		}
		case constcrc("MessageToAckMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			MessageToAckMessage m(ri);

			handleMessageToAck (sourceGameServer, m.getMessageId());
			break;
		}
		case constcrc("ServerDeleteCharacterMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			ServerDeleteCharacterMessage m(ri);

			deleteCharacter(m.getStationId(), m.getCharacterId());
			break;
		}
		case constcrc("RenameCharacterMessageEx") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			RenameCharacterMessageEx msg(ri);

			renameCharacter(sourceGameServer, static_cast<int8>(msg.getRenameCharacterMessageSource()), msg.getStationId(), msg.getCharacterId(), msg.getNewName(), msg.getOldName(), msg.getLastNameChangeOnly(), msg.getRequestedBy(), nullptr);
			break;
		}
		case constcrc("UnloadedPlayerMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			UnloadedPlayerMessage msg(ri);

			unloadCharacter(msg.getPlayerId(),sourceGameServer);
			break;
		}
		case constcrc("MoveToPlayer") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > msg(ri);

			moveToPlayer(sourceGameServer, msg.getValue().first, msg.getValue().second, -1, false, false);
			break;
		}
		case constcrc("MoveToPlayerBankMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > msg(ri);

			moveToPlayer(sourceGameServer, msg.getValue().first, msg.getValue().second, -1, true, false);
			break;
		}
		case constcrc("MoveToPlayerDatapadMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<std::pair<std::pair<NetworkId, NetworkId>, int> > msg(ri);

			moveToPlayer(sourceGameServer, msg.getValue().first.first, msg.getValue().first.second, msg.getValue().second, false, true);
			break;
		}
		case constcrc("FixLoadWith") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<std::pair<std::pair<NetworkId, NetworkId>, int> > msg(ri);

			fixLoadWith(sourceGameServer, msg.getValue().first.first, msg.getValue().first.second, msg.getValue().second);
			break;
		}
		case constcrc("ClusterShutdownMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<bool> msg(ri);

			m_clusterShuttingDown = msg.getValue();
			break;
		}
		case constcrc("StartSaveMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<NetworkId > msg(ri);

			userRequestedSave(sourceGameServer, msg.getValue());
			break;
		}
		case constcrc("RestoreHouseMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<std::pair<NetworkId, std::string> > msg(ri);

			restoreHouse(msg.getValue().first, msg.getValue().second);
			break;
		}
		case constcrc("RestoreCharacterMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<std::pair<NetworkId, std::string> > msg(ri);

			restoreCharacter(msg.getValue().first, msg.getValue().second);
			break;
		}
		case constcrc("UndeleteItemMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<std::pair<NetworkId, std::string> > msg(ri);

			LOG("CustomerService", ("undeleteItem %s request from %s", msg.getValue().first.getValueString().c_str(), msg.getValue().second.c_str()));
			undeleteItem(msg.getValue().first, msg.getValue().second);
			break;
		}
		case constcrc("MoveToPlayerMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<std::pair<std::pair<NetworkId, NetworkId>, std::string> > msg(ri);

			LOG("CustomerService", ("moveToPlayer %s, %s request from %s", msg.getValue().first.first.getValueString().c_str(), msg.getValue().first.second.getValueString().c_str(), msg.getValue().second.c_str()));
			moveToPlayer(msg.getValue().first.first, msg.getValue().first.second, msg.getValue().second);
			break;
		}
		case constcrc("CentralRequestSave") :
		{
			centralRequestedSave();
			break;
		}
		case constcrc("PlanetRequestSave") :
		{
			planetRequestedSave();
			break;
		}
		case constcrc("AddResourceTypeMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			AddResourceTypeMessage msg(ri);
			handleAddResourceTypeMessage(sourceGameServer, msg);
			break;
		}
		case constcrc("BountyHunterTargetMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			BountyHunterTargetMessage msg(ri);
			getSnapshotForServer(sourceGameServer)->handleBountyHunterTargetMessage(msg);
			break;
		}
		case constcrc("CMCreateAuctionMessage") :
		case constcrc("CMCreateAuctionBidMessage") :
		case constcrc("CMCreateLocationMessage") :
		case constcrc("CMUpdateAuctionMessage") :
		case constcrc("CMUpdateLocationMessage") :
		case constcrc("CMDeleteAuctionMessage") :
		case constcrc("CMDeleteLocationMessage") :
		{
			const CommoditiesServerConnection * commConnection = dynamic_cast<const CommoditiesServerConnection *>(&source);
			uint32 commServerId = 0;
			if (commConnection)
				commServerId = commConnection->getProcessId();
			
			getCommoditiesSnapshot(commServerId)->handleCommoditiesDataMessage(message);
			break;
		}
		case constcrc("LoadCommodities") :
		{
			if (m_commoditiesSnapshot)
			{
				m_commoditiesSnapshot->startLoadAfterSaveComplete();
				if (!isSaveInProgress())
					m_startSaveWhenPossible = true;
			}
			else
			{
				startLoadCommodities();
			}
			break;
		}
		case constcrc("GetMoneyFromOfflineObjectMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GetMoneyFromOfflineObjectMessage msg(ri);
			getMoneyFromOfflineObject(sourceGameServer, msg.getSourceObject(), msg.getAmount(), msg.getReplyTo(), msg.getSuccessCallback(), msg.getFailCallback(), msg.getPackedDictionary());
			break;
		}
		case constcrc("PurgeCompleteMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<StationId> msg(ri);
			handlePurgeCompleteMessage(sourceGameServer, msg.getValue());
			break;
		}
		case constcrc("DBCSRequestMessage" ) :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			DBCSRequestMessage const msg( ri );
			handleCSRequest( msg );
			break;
		}
		case constcrc("UndeleteItemForCsMessage") :
		{
			auto ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<std::pair<std::pair<std::pair<NetworkId, NetworkId>, std::string>, bool> > undeleteRequest(ri);
			handleCsUndelete(undeleteRequest.getValue().first.first.first, 
					 undeleteRequest.getValue().first.first.second, 
					 undeleteRequest.getValue().first.second,
					 undeleteRequest.getValue().second);
					 
			break;
		}
		default :
		{
			DEBUG_REPORT_LOG(true,("Unrecognized message recieved by Persister.\n"));
			break;
		}
	}
}

// ----------------------------------------------------------------------
void Persister::handleCSRequest( const DBCSRequestMessage & msg )
{
	if( msg.getCommand() == "warp_player" )
	{
		handleCSMovePlayer( msg );
	}
}

void Persister::handleCsUndelete(const NetworkId & character, const NetworkId & item, const std::string & message, bool move)
{
	UNREF(message);
	DEBUG_REPORT_LOG(true,("Undeleting at db: %s %s\n", character.getValueString().c_str(), item.getValueString().c_str()));
	taskQueue->asyncRequest(new TaskCsUndeleteItem(character, item, move));
}

void Persister::handleCSMovePlayer( const DBCSRequestMessage & msg )
{
	char buffer[ 256 ];
	char scene_buf[ 32 ];
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	int num_params;
	if( ( num_params = sscanf( msg.getCommandLine().c_str(), "warp_player %s %s %f %f %f", buffer, scene_buf, &x, &y, &z ) ) == 5 )
	{
		NetworkId id( buffer );
		if( id.isValid() )
		{
			taskQueue->asyncRequest(new TaskMovePlayer(id, scene_buf, x, y, z));
		}
	}
}

// ----------------------------------------------------------------------

void Persister::addCharacter(uint32 stationId, const NetworkId &characterObject, uint32 creationGameServer, const Unicode::String &name, bool special)
{
	PendingCharacter temp;
	temp.stationId = stationId;
	temp.name = name;
	temp.special = special;
	m_pendingCharacters[characterObject]=temp;

	//TODO:  remove this hack: match up create and end messages because we can't count on having all the data at a frame bounday
	if (m_newCharacterLock.find(creationGameServer) != m_newCharacterLock.end()) {
		WARNING(true,("Programmer bug:  got an addCharacter from server %i before we received EndBaselines from the previous addCharacter.  Indicates we're getting network messages out of order.\n",creationGameServer));
	}

	m_newCharacterLock.insert(creationGameServer);
}

// ----------------------------------------------------------------------

void Persister::onNewCharacterSaved (uint32 stationId, const NetworkId &characterObject, const Unicode::String &characterName, const int templateId, bool special) const
{
	DEBUG_REPORT_LOG(true,("Sending  DatabaseCreateCharacterSuccess(%s)\n",characterObject.getValueString().c_str()));
	DatabaseCreateCharacterSuccess const successMessage(characterObject, stationId, characterName, templateId, special);
	DatabaseProcess::getInstance().sendToCentralServer(successMessage, true);
}

// ----------------------------------------------------------------------

/**
 * Save a delayed message in the database.
 * Will be delivered to the object when the object is loaded, or removed
 * from the database when it is acknowledged.
 */
void Persister::handleMessageTo(uint32 sourceServer, const MessageToPayload &data)
{
	if (data.getMessageId() == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true,("Database received a messageTo with serial number 0.  Message method:  %s, target:  %s\n", data.getMethod().c_str(), data.getNetworkId().getValueString().c_str()));
	}
	else
	{
		if (!m_messageSnapshot) 
			m_messageSnapshot = getSnapshotForServer(sourceServer);
		m_messageSnapshot->handleMessageTo(data);
	}
	MessageToManager::getInstance().handleMessageTo(data);
}

// ----------------------------------------------------------------------

/**
 * Mark that a delayed message has been handled
 * Will be removed from the database.
 */
void Persister::handleMessageToAck(uint32 sourceServer, const MessageToId &messageId)
{
	if (!m_messageSnapshot)
		m_messageSnapshot = getSnapshotForServer(sourceServer);
	
	m_messageSnapshot->handleMessageToAck(messageId);
	MessageToManager::getInstance().handleMessageToAck(messageId);
}

// ----------------------------------------------------------------------

void Persister::deleteCharacter(StationId stationId, const NetworkId &characterId)
{
	// we cannot do the character delete in the current save cycle because
	// the SQL that deletes a character also deletes items contained inside
	// the character, but those items may have been transferred in the current
	// save cycle, and the individual snapshots are updated in the DB in a
	// non-deterministic order, possibly causing the delete character snapshot
	// to execute before the snapshot that contains the containment update for
	// the item, thus causing the item to get deleted incorrectly; to avoid
	// this problem, we will delete the character during the next save cycle
	if (m_charactersToDeleteNextSaveCycle && m_charactersToDeleteThisSaveCycle)
	{
		m_charactersToDeleteNextSaveCycle->push_back(std::make_pair(stationId, characterId));
	}
	else
	{
		DeleteCharacterCustomPersistStep *cps = new DeleteCharacterCustomPersistStep(stationId, characterId);
		getSnapshotForObject(characterId, 0)->addCustomPersistStep(cps);
	}

	// send delete character message to CommoditiesServer to delete auction items from this character
	DeleteCharacterMessage msg(0, characterId);
	DatabaseProcess::getInstance().sendToCommoditiesServer(msg, true);
}

// ----------------------------------------------------------------------

void Persister::shutdown()
{
	delete taskQueue;
	taskQueue=0;

	delete m_newCharacterTaskQueue;
	m_newCharacterTaskQueue=0;
}

// ----------------------------------------------------------------------

void Persister::renameCharacter(uint32 sourceServer, int8 renameCharacterMessageSource, uint32 stationId, const NetworkId &characterId, const Unicode::String &newName, const Unicode::String &oldName, bool lastNameChangeOnly, const NetworkId &requestedBy, const TransferCharacterData * renameRequest)
{
	// don't queue up another rename for the character
	if (RenameCharacterCustomPersistStep::hasPendingRenameCharacterCustomPersistStep(characterId))
	{
		if (static_cast<RenameCharacterMessageEx::RenameCharacterMessageSource>(renameCharacterMessageSource) == RenameCharacterMessageEx::RCMS_player_request)
		{
			// send "player friendly" message
			MessageToMessage const mtm(
				MessageToPayload(characterId, NetworkId::cms_invalid, "C++RenameFailed", "There is already an outstanding rename request for this character.", 0, false, MessageToPayload::DT_c, NetworkId::cms_invalid, std::string(), 0),
				DatabaseProcess::getInstance().getProcessId());

			if ((sourceServer > 0) && DatabaseProcess::getInstance().getConnectionByProcess(sourceServer))
				DatabaseProcess::getInstance().sendToGameServer(sourceServer, mtm);
			else
				DatabaseProcess::getInstance().sendToAnyGameServer(mtm);
		}
		else if (requestedBy.isValid())
		{
			// send GM message
			MessageToMessage const mtm(
				MessageToPayload(requestedBy, NetworkId::cms_invalid, "C++RenameFailed", FormattedString<512>().sprintf("Rename request for %s (%s) failed because the character currently has a pending rename request, which will be performed during the next save cycle.", characterId.getValueString().c_str(), Unicode::wideToNarrow(newName).c_str()), 0, false, MessageToPayload::DT_c, NetworkId::cms_invalid, std::string(), 0),
				DatabaseProcess::getInstance().getProcessId());

			if ((sourceServer > 0) && DatabaseProcess::getInstance().getConnectionByProcess(sourceServer))
				DatabaseProcess::getInstance().sendToGameServer(sourceServer, mtm);
			else
				DatabaseProcess::getInstance().sendToAnyGameServer(mtm);
		}

		return;
	}

	RenameCharacterCustomPersistStep *cps = new RenameCharacterCustomPersistStep(renameCharacterMessageSource, stationId, characterId, newName, oldName, requestedBy, renameRequest);
	getSnapshotForServer(sourceServer)->addCustomPersistStep(cps);

	if (static_cast<RenameCharacterMessageEx::RenameCharacterMessageSource>(renameCharacterMessageSource) == RenameCharacterMessageEx::RCMS_player_request)
	{
		// if player requested, send confirmation message back to player character
		MessageToMessage const mtm(
			MessageToPayload(characterId, NetworkId::cms_invalid, (lastNameChangeOnly ? "C++PlayerLastNameRenameRequestSubmitted" : "C++PlayerRenameRequestSubmitted"), Unicode::wideToNarrow(newName), 0, false, MessageToPayload::DT_c, NetworkId::cms_invalid, std::string(), 0),
			DatabaseProcess::getInstance().getProcessId());

		if ((sourceServer > 0) && DatabaseProcess::getInstance().getConnectionByProcess(sourceServer))
			DatabaseProcess::getInstance().sendToGameServer(sourceServer, mtm);
		else
			DatabaseProcess::getInstance().sendToAnyGameServer(mtm);
	}
	else if (requestedBy.isValid())
	{
		// if GM requested, send confirmation message to GM
		MessageToMessage const mtm(
			MessageToPayload(requestedBy, NetworkId::cms_invalid, "C++GmRenameRequestSubmitted", FormattedString<512>().sprintf("Rename request for %s (%s) has been submitted, and will be performed during the next save cycle.", characterId.getValueString().c_str(), Unicode::wideToNarrow(newName).c_str()), 0, false, MessageToPayload::DT_c, NetworkId::cms_invalid, std::string(), 0),
			DatabaseProcess::getInstance().getProcessId());

		if ((sourceServer > 0) && DatabaseProcess::getInstance().getConnectionByProcess(sourceServer))
			DatabaseProcess::getInstance().sendToGameServer(sourceServer, mtm);
		else
			DatabaseProcess::getInstance().sendToAnyGameServer(mtm);
	}

	if (requestedBy.isValid())
		LOG("CustomerService", ("Player:rename character request submitted for %s (%s -> %s) for stationId %lu by %s", characterId.getValueString().c_str(), Unicode::wideToNarrow(oldName).c_str(), Unicode::wideToNarrow(newName).c_str(), stationId, requestedBy.getValueString().c_str()));
	else
		LOG("CustomerService", ("Player:rename character request submitted for %s (%s -> %s) for stationId %lu", characterId.getValueString().c_str(), Unicode::wideToNarrow(oldName).c_str(), Unicode::wideToNarrow(newName).c_str(), stationId));
}

// ----------------------------------------------------------------------

void Persister::changeStationId(const TransferAccountData * transferRequest)
{
	taskQueue->asyncRequest(new TaskChangeStationId(transferRequest->getSourceStationId(), transferRequest->getDestinationStationId(), transferRequest));
}

// ----------------------------------------------------------------------

void Persister::unloadCharacter(const NetworkId &characterId, uint32 sourceServer)
{
	UnloadCharacterCustomPersistStep *cps = new UnloadCharacterCustomPersistStep(characterId, sourceServer);
	getSnapshotForObject(characterId, sourceServer)->addCustomPersistStep(cps);
}

// ----------------------------------------------------------------------

bool Persister::isIdle()
{
	return (m_currentSnapshots.empty() && m_newObjectSnapshots.empty() && m_savingSnapshots.empty() && taskQueue->isIdle() && m_newCharacterTaskQueue->isIdle());
}

// ----------------------------------------------------------------------

bool Persister::isSaveInProgress()
{
	return (!m_savingSnapshots.empty());
}

// ----------------------------------------------------------------------

void Persister::moveToPlayer(uint32 sourceServer, const NetworkId &objectId, const NetworkId &targetPlayer, int maxDepth, bool useBank, bool useDatapad)
{
	MoveToPlayerCustomPersistStep *cps = new MoveToPlayerCustomPersistStep(objectId, targetPlayer, maxDepth, useBank, useDatapad);
	getSnapshotForObject(objectId, sourceServer)->addCustomPersistStep(cps);
}

// ----------------------------------------------------------------------

void Persister::fixLoadWith(uint32 sourceServer, const NetworkId &topmostObject, const NetworkId &startingLoadWith, int maxDepth)
{
	FixLoadWithCustomPersistStep *cps = new FixLoadWithCustomPersistStep(topmostObject, startingLoadWith, maxDepth);
	getSnapshotForObject(topmostObject, sourceServer)->addCustomPersistStep(cps);
}

// ----------------------------------------------------------------------

std::string Persister::getLastSaveCompletionTime()
{
	return m_lastSaveCompletionTime;
}

// ----------------------------------------------------------------------

void Persister::userRequestedSave(uint32 sourceGameServer, const NetworkId &whoRequested)
{
	bool started = false;
	if (!isSaveInProgress())
	{
		started = true;
		m_startSaveWhenPossible = true;
	}

	if (sourceGameServer != 0)
	{
		GenericValueTypeMessage<std::pair<NetworkId, bool> > reply("StartSaveReplyMessage", std::make_pair(whoRequested, started));
		DatabaseProcess::getInstance().sendToGameServer(sourceGameServer, reply);
	}
}

// ----------------------------------------------------------------------

void Persister::centralRequestedSave()
{
	m_startSaveWhenPossible = true;
}

// ----------------------------------------------------------------------

void Persister::planetRequestedSave()
{
	m_startSaveWhenPossible = true;
}

// ----------------------------------------------------------------------

void Persister::handleAddResourceTypeMessage(uint32 const serverId, AddResourceTypeMessage const & message)
{
	getSnapshotForServer(serverId)->handleAddResourceTypeMessage(message);
}

// ----------------------------------------------------------------------

void Persister::handlePurgeCompleteMessage(uint32 const serverId, StationId stationId)
{
	PurgeCompleteCustomPersistStep * cps = new PurgeCompleteCustomPersistStep(stationId);
	getSnapshotForServer(serverId)->addCustomPersistStep(cps);
}

// ======================================================================
