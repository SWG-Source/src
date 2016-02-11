// ======================================================================
//
// SwgLoader.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgLoader.h"

#include "SwgDatabaseServer/SwgLoaderSnapshotGroup.h"
#include "SwgDatabaseServer/TaskGetCharacters.h"
#include "SwgDatabaseServer/TaskLoadObjvarNames.h"
#include "SwgDatabaseServer/TaskLocateStructure.h"
#include "SwgDatabaseServer/TaskVerifyCharacter.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"
#include "sharedDebug/Profiler.h"

// ======================================================================

void SwgLoader::install()
{
	Loader::installDerived(new SwgLoader);
}

// ----------------------------------------------------------------------

SwgLoader::SwgLoader() :
		Loader(),
		m_pendingTaskVerifyCharacter(nullptr),
		m_loadingTaskVerifyCharacter(nullptr),
		m_verifyCharacterTaskQ(nullptr)
{
	m_verifyCharacterTaskQ = new DB::TaskQueue(1,DatabaseProcess::getInstance().getDBServer(),4);
}

// ----------------------------------------------------------------------

SwgLoader:: ~SwgLoader()
{
	m_verifyCharacterTaskQ->cancel();
	delete m_verifyCharacterTaskQ;
	delete m_pendingTaskVerifyCharacter;
	m_loadingTaskVerifyCharacter = 0; // owned by a db task queue, freed elsewhere
	m_verifyCharacterTaskQ = 0;
}

// ----------------------------------------------------------------------

LoaderSnapshotGroup *SwgLoader::makeLoaderSnapshotGroup(uint32 processId)
{
	PROFILER_AUTO_BLOCK_DEFINE("SwgLoader::makeLoaderSnapshotGroup");
	return new SwgLoaderSnapshotGroup(processId);
}

// ----------------------------------------------------------------------

// /**
//  * Called to request that the Loader get the list of characters for
//  * an account and send them to central.
//  */
// void SwgLoader::getCharacters(uint32 accountNumber)
// {
// 	TaskGetCharacters *task=new TaskGetCharacters(accountNumber);
// 	taskQ->asyncRequest(task);
// }

// ----------------------------------------------------------------------

/**
 * Called to verify that a player can use a particular character,
 * and to get location data needed for login.
 */
void SwgLoader::verifyCharacter(StationId suid, const NetworkId &characterId, const TransferCharacterData * transferCharacterData)
{
	if (!m_pendingTaskVerifyCharacter)
	{
		if (ConfigServerDatabase::getEnableGoldDatabase())
			m_pendingTaskVerifyCharacter = new TaskVerifyCharacter(DatabaseProcess::getInstance().getGoldSchemaQualifier());
		else
		{
			static std::string nullSchema;
			m_pendingTaskVerifyCharacter = new TaskVerifyCharacter(nullSchema);
		}
	}
	m_pendingTaskVerifyCharacter->addCharacter(suid,characterId, transferCharacterData);
}

// ----------------------------------------------------------------------

void SwgLoader::update(real updateTime)
{
	if (m_pendingTaskVerifyCharacter && !m_loadingTaskVerifyCharacter)
	{
		m_loadingTaskVerifyCharacter = m_pendingTaskVerifyCharacter;
		m_pendingTaskVerifyCharacter = nullptr;
		m_verifyCharacterTaskQ->asyncRequest(m_loadingTaskVerifyCharacter);		
	}

	m_verifyCharacterTaskQ->update(ConfigServerDatabase::getDefaultQueueUpdateTimeLimit());
	Loader::update(updateTime);
}

// ----------------------------------------------------------------------

void SwgLoader::verifyCharacterFinished (TaskVerifyCharacter *task)
{
	UNREF(task);
	DEBUG_FATAL(task!=m_loadingTaskVerifyCharacter,("Programmer bug:  wrong TaskVerifyCharacter finished.\n"));
	m_loadingTaskVerifyCharacter = nullptr;
}

// ----------------------------------------------------------------------

void SwgLoader::locateStructure(const NetworkId &structureId, const std::string whoRequested)
{
	TaskLocateStructure *task = new TaskLocateStructure(structureId, whoRequested);
	taskQ->asyncRequest(task);
}

// ----------------------------------------------------------------------

void SwgLoader::loadStartupData()
{
	taskQ->asyncRequest(new TaskLoadObjvarNames);
}

// ======================================================================
