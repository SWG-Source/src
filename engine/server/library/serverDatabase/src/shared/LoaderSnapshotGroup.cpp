// ======================================================================
//
// LoaderSnapshotGroup.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/LoaderSnapshotGroup.h"

#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/Snapshot.h"
#include "serverDatabase/WorldContainerLocator.h"
#include "sharedDebug/Profiler.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

LoaderSnapshotGroup::LoaderSnapshotGroup(uint32 requestingProcess, Snapshot *snapshot) :
		m_requestingProcess(requestingProcess),
		m_snapshot(snapshot),
		m_goldSnapshot(0),
		m_loadSerialNumber(0)
{
	NOT_NULL(m_snapshot);
}

// ----------------------------------------------------------------------

LoaderSnapshotGroup::~LoaderSnapshotGroup()
{
	delete m_snapshot;
	m_snapshot=0;
	delete m_goldSnapshot;
	m_goldSnapshot=0;
}

// ----------------------------------------------------------------------

bool LoaderSnapshotGroup::load(DB::Session *session)
{
	if (m_goldSnapshot)
	{
		if (! (m_goldSnapshot->load(session)))
			return false;
		
		std::vector<NetworkId> containers;
		m_goldSnapshot->getWorldContainers(containers);
		WorldContainerLocator *loc = new WorldContainerLocator(containers);
		m_snapshot->addLocator(loc);
	}
	
	NOT_NULL(m_snapshot);
	return (m_snapshot->load(session));
}

// ----------------------------------------------------------------------

//TODO:  rather have Loader keep track of connections, pass them as parameter to send()
bool LoaderSnapshotGroup::send() const
{
	bool success = true;
	
	NOT_NULL(m_snapshot);
	GameServerConnection *connection = DatabaseProcess::getInstance().getConnectionByProcess(m_requestingProcess);

	if (!connection)
		return false;
	
	if (m_goldSnapshot)
	{
		if (!m_goldSnapshot->send(connection))
			success=false;
	}
	if (success && !m_snapshot->send(connection))
		success=false;

	if (success)
	{
		PROFILER_AUTO_BLOCK_DEFINE("send ChunkCompleteMessage");
	
		connection->sendChunkCompleteMessage();
			
		// Request an acknowledgement for this load
		if (m_loadSerialNumber != 0)
		{
			GenericValueTypeMessage<int> msg("RequestLoadAckMessage",m_loadSerialNumber);
			connection->send(msg,true);
		}
	}
	else
		connection->clearChunkCompleteQueue();

	return success;
}

// ----------------------------------------------------------------------

void LoaderSnapshotGroup::addLocator(ObjectLocator *newLocator)
{
	NOT_NULL(m_snapshot);
	m_snapshot->addLocator(newLocator);	
}

// ----------------------------------------------------------------------

void LoaderSnapshotGroup::addGoldLocator(ObjectLocator *newLocator)
{
	if (!m_goldSnapshot)
		m_goldSnapshot = makeGoldSnapshot();

	NOT_NULL(m_goldSnapshot);
	m_goldSnapshot->addLocator(newLocator);
}

// ----------------------------------------------------------------------

void LoaderSnapshotGroup::setUniverseAuthHack()
{
	NOT_NULL(m_snapshot);
	m_snapshot->setUniverseAuthHack();
}

// ----------------------------------------------------------------------

int LoaderSnapshotGroup::getLocatorCount() const
{
	int result=0;
	if (m_goldSnapshot)
		result = m_goldSnapshot->getLocatorCount();
	NOT_NULL(m_snapshot);
	return result + m_snapshot->getLocatorCount();
}

// ----------------------------------------------------------------------

void LoaderSnapshotGroup::setLoadSerialNumber(int loadSerialNumber)
{
	m_loadSerialNumber = loadSerialNumber;
}

// ======================================================================
