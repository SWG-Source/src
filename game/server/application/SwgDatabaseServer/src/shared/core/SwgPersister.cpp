// ======================================================================
//
// SwgPersister.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgPersister.h"

#include "sharedDatabaseInterface/DbTaskQueue.h"
#include "serverDatabase/CMLoader.h"
#include "SwgDatabaseServer/CommoditiesSnapshot.h"
#include "SwgDatabaseServer/ObjvarNameManager.h"
#include "SwgDatabaseServer/SwgSnapshot.h"
#include "SwgDatabaseServer/TaskRestoreCharacter.h"
#include "SwgDatabaseServer/TaskRestoreHouse.h"
#include "SwgDatabaseServer/TaskUndeleteItem.h"
#include "SwgDatabaseServer/TaskMoveToPlayer.h"

// ======================================================================

void SwgPersister::install()
{
	Persister::installDerived(new SwgPersister);
}

// ----------------------------------------------------------------------
	
SwgPersister::SwgPersister() :
		Persister()
{
}

// ----------------------------------------------------------------------

SwgPersister::~SwgPersister()
{
}

// ----------------------------------------------------------------------

Snapshot *SwgPersister::makeSnapshot(DB::ModeQuery::Mode mode) const
{
	return new SwgSnapshot(mode, false);
}

// ----------------------------------------------------------------------

Snapshot *SwgPersister::makeCommoditiesSnapshot(DB::ModeQuery::Mode mode) const
{
	return new CommoditiesSnapshot(mode);
}

// ----------------------------------------------------------------------

void SwgPersister::startLoadCommodities()
{
	CMLoader::getInstance().loadCommoditiesData();
}

// ----------------------------------------------------------------------

/**
 * Save new object variable names, if necessary, then call the base class'
 * startSave function.
 */
void SwgPersister::startSave()
{
	DB::TaskRequest *req = ObjvarNameManager::getInstance().saveNewNames();
	if (req)
		taskQueue->asyncRequest(req);

	Persister::startSave();
}

// ----------------------------------------------------------------------

void SwgPersister::restoreHouse(const NetworkId &houseId, const std::string &whoRequested)
{
	TaskRestoreHouse *task = new TaskRestoreHouse (houseId, whoRequested);
	taskQueue->asyncRequest(task);
}

// ----------------------------------------------------------------------

void SwgPersister::restoreCharacter(const NetworkId &characterId, const std::string &whoRequested)
{
	TaskRestoreCharacter *task = new TaskRestoreCharacter (characterId, whoRequested);
	taskQueue->asyncRequest(task);
}

// ----------------------------------------------------------------------

void SwgPersister::undeleteItem(const NetworkId &itemId, const std::string &whoRequested)
{
	TaskUndeleteItem *task = new TaskUndeleteItem (itemId, whoRequested);
	taskQueue->asyncRequest(task);
}

// ----------------------------------------------------------------------

void SwgPersister::moveToPlayer(const NetworkId &oid, const NetworkId &player, const std::string &whoRequested)
{
	TaskMoveToPlayer *task = new TaskMoveToPlayer (oid, player, whoRequested);
	taskQueue->asyncRequest(task);
}

// ----------------------------------------------------------------------

void SwgPersister::getMoneyFromOfflineObject(uint32 replyServer, NetworkId const & sourceObject, int amount, NetworkId const & replyTo, std::string const & successCallback, std::string const & failCallback, std::vector<int8> const & packedDictionary)
{
	SwgSnapshot * snapshot=nullptr;
	
	if (hasDataForObject(sourceObject))
		snapshot=safe_cast<SwgSnapshot*>(getSnapshotForObject(sourceObject, 0));
	else
		snapshot=safe_cast<SwgSnapshot*>(getSnapshotForServer(0));

	snapshot->getMoneyFromOfflineObject(replyServer, sourceObject, amount, replyTo, successCallback, failCallback, packedDictionary);
}

// ======================================================================
