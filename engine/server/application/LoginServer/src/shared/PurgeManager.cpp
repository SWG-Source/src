// ======================================================================
//
// PurgeManager.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "PurgeManager.h"

#include "ConfigLoginServer.h"
#include "DatabaseConnection.h"
#include "LoginServer.h"
#include "SessionApiClient.h"
#include "sharedFoundation/Timer.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

namespace PurgeManagerNamespace
{
	enum PurgePhase
	{
		ppNoPurge, ppNotWarned, ppStructureWarningSent, ppStructuresPurged, ppCharacterWarningSent, ppPurged
	};
	bool m_gettingAccount=false;

	void removeFromPurgeList(StationId account);
	void releaseAccount(StationId account);
	PurgePhase getNextPurgePhase(PurgePhase previousPurgePhase);

	class PurgeRecord
	{
	public:
		PurgeRecord(StationId stationId, PurgePhase purgePhase);
		StationId getStationId() const;
		PurgePhase getPurgePhase() const;
		void grabClusterList();
		bool hasCluster(uint32 clusterId) const;
		void removeCluster(uint32 clusterId);
		bool hasClusters() const;
		void advancePurgePhase();
		
	private:
		StationId m_stationId;
		PurgePhase m_purgePhase;
		std::set<uint32> m_clusters;
	};

	typedef std::map<StationId, PurgeRecord> PurgeRecordsType;
	
	PurgeRecordsType m_purgeRecords;
	PurgePhase m_currentPurgePhase = ppNotWarned;
}

using namespace PurgeManagerNamespace;

// ======================================================================

/**
 * The PurgeManager picks a purge phase and finds accounts for that phase.
 * If no accounts can be found for that phase, it moves on to the next phase.
 * Once it has tried all purge phases without finding any accounts, it will
 * sleep for a while before starting again.
 */
void PurgeManager::update(float time)
{
	if (!(ConfigLoginServer::getEnableStructurePurge())||(ConfigLoginServer::getEnableCharacterPurge()))
		return;
	
	static Timer retryAccountsTimer(ConfigLoginServer::getPurgeSleepTime());	

	if ((m_currentPurgePhase==ppPurged) || // last purge phase, so wait a while then start over
		((m_currentPurgePhase>=ppStructuresPurged) && (!ConfigLoginServer::getEnableCharacterPurge())))
	{
		if (retryAccountsTimer.updateZero(time))
			m_currentPurgePhase=ppNotWarned;
	}
	else
	{
		if (LoginServer::getInstance().areAllClustersUp() && (!m_gettingAccount && m_purgeRecords.size() < static_cast<size_t>(ConfigLoginServer::getMaxSimultaneousPurgeAccounts())))
		{
			m_gettingAccount=true;
			DatabaseConnection::getInstance().getAccountForPurge(m_currentPurgePhase);
		}
	}

	if (ConfigLoginServer::getUpdatePurgeAccountListTime() != 0)
	{
		static Timer updateAccountListTimer(ConfigLoginServer::getUpdatePurgeAccountListTime());
		if (updateAccountListTimer.updateZero(time))
			DatabaseConnection::getInstance().updatePurgeAccountList();
	}
}

// ----------------------------------------------------------------------

void PurgeManager::onGetAccountForPurge(StationId account, int purgePhase)
{
	m_gettingAccount=false;
	if (account==0)
	{
		// No remaining accounts in the DB
		m_currentPurgePhase=getNextPurgePhase(m_currentPurgePhase);
	}
	else
	{
		if (purgePhase == ppNoPurge)
		{
			WARNING_DEBUG_FATAL(true,("Programmer bug:  onGetAccountForPurge(%i) -- account was in the \"No Purge\" phase.  The DB query shouldn't return accounts in this phase.", account));
			return;
		}
		if (purgePhase == ppPurged)
		{
			WARNING_DEBUG_FATAL(true,("Programmer bug:  onGetAccountForPurge(%i) -- account was in the \"Purged\" phase.  The DB query shouldn't return accounts in this phase.", account));
			return;
		}

		PurgeRecord record(account, static_cast<PurgePhase>(purgePhase));
		m_purgeRecords.insert(std::make_pair(account,record));
	
		onCheckStatusForPurge(account, false);
	}
}

// ----------------------------------------------------------------------

void PurgeManager::onCheckStatusForPurge(StationId account, bool isActive)
{
	DEBUG_REPORT_LOG(true,("onCheckStatusForPurge(%li, %s)\n",account, isActive ? "true" : "false"));
	PurgeRecordsType::iterator i=m_purgeRecords.find(account);
	if (i==m_purgeRecords.end())
	{
		WARNING_DEBUG_FATAL(true,("Programmer bug:  onCheckStatusForPurge(%i) -- account was not in m_purgeRecords", account));
		return;
	}
	PurgeRecord & record = i->second;

	
	if (isActive)
	{
		removeFromPurgeList(account);
	}
	else
	{
		if (!LoginServer::getInstance().areAllClustersUp())
		{
			// Don't try to process any purges while clusters are down
			releaseAccount(account);
			return;
		}
		
		switch (record.getPurgePhase())
		{
			case ppNoPurge:
				WARNING_DEBUG_FATAL(true,("Programmer bug:  onCheckStatusForPurge(%i) -- account was in the \"No Purge\" phase.", account));
				break;
			case ppNotWarned:
			{
				LOG("CustomerService",( "Purge:  Sending structure purge warning emails for account %i", account));
				//TODO:  send to platform somehow for warning emails
				record.grabClusterList();
				GenericValueTypeMessage<StationId> msg("WarnStructuresAboutPurgeMessage",account);
				LoginServer::getInstance().sendToAllClusters(msg);

				break;
			}
			case ppStructureWarningSent:
			{
				LOG("CustomerService",( "Purge:  Starting structure and vendor purge for account %i", account));
				record.grabClusterList();
				GenericValueTypeMessage<StationId> msg("PurgeStructuresForAccountMessage",account);
				LoginServer::getInstance().sendToAllClusters(msg);
				break;
			}
			case ppStructuresPurged:
				LOG("CustomerService",( "Purge:  Sending character purge warning emails for account %i", account));
				//TODO:  send to platform somehow for warning emails
				record.advancePurgePhase();
				releaseAccount(record.getStationId());
				break;
			case ppCharacterWarningSent:
				LOG("CustomerService",( "Purge:  Deleting all characters for account %i", account));
				DatabaseConnection::getInstance().deleteAllCharacters(account);
				break;
			case ppPurged:
				WARNING_DEBUG_FATAL(true,("Programmer bug:  onCheckStatusForPurge(%i) -- account was in the \"Purged\" phase.  This should not be possible.", account));
				break;
			default:
				WARNING_DEBUG_FATAL(true,("Programmer bug:  onCheckStatusForPurge(%i) -- account was in an unhandled purge phase", account));
		}
	}
}

// ----------------------------------------------------------------------

void PurgeManagerNamespace::removeFromPurgeList(StationId account)
{
	DatabaseConnection::getInstance().setPurgeStatusAndRelease(account, ppNoPurge);
	m_purgeRecords.erase(account);
}

// ----------------------------------------------------------------------

void PurgeManagerNamespace::releaseAccount(StationId account)
{
	PurgeRecordsType::iterator record=m_purgeRecords.find(account);
	if (record==m_purgeRecords.end())
	{
		WARNING_DEBUG_FATAL(true,("Programmer bug:  releaseAccount(%i) -- account was not in m_purgeRecords", account));
		return;
	}

	DatabaseConnection::getInstance().setPurgeStatusAndRelease(account, record->second.getPurgePhase());
	m_purgeRecords.erase(record);
}

// ----------------------------------------------------------------------

/**
 * The specified cluster is no longer ready for players.  Indicates
 * a game server crashed or some other problem.  Assume any pending purges
 * on this cluster did not go through, retry them later.
 */
void PurgeManager::onClusterNoLongerReady(uint32 clusterId)
{
	for (PurgeRecordsType::iterator record=m_purgeRecords.begin(); record!=m_purgeRecords.end(); )
	{
		if (record->second.hasCluster(clusterId))
		{
			PurgeRecordsType::iterator nextRecord=record;
			++nextRecord;
			releaseAccount(record->second.getStationId());
			record=nextRecord;
		}
		else
			++record;
	}
}

// ----------------------------------------------------------------------

/**
 * Called when a cluster indicates it has done the purge for a particular
 * account and saved the results.
 */
void PurgeManager::handlePurgeCompleteOnCluster(StationId account, uint32 clusterId)
{
	PurgeRecordsType::iterator i=m_purgeRecords.find(account);
	if (i==m_purgeRecords.end())
	{
		DEBUG_REPORT_LOG(true,("Purge record could not be found in handlePurgeCompleteOnCluster(%li, %li).  Probably indicates the cluster and/or the login server were restarted while a purge was in progress.  The purge will be retried later.\n",account,clusterId));
		return;
	}

	PurgeRecord & record = i->second;
	record.removeCluster(clusterId);
	if (!record.hasClusters())
	{
		// No more clusters remaining.  Purge was a success
		record.advancePurgePhase();
		releaseAccount(record.getStationId());
	}
}

// ----------------------------------------------------------------------

void PurgeManager::onAllCharactersDeleted(StationId account, bool success)
{
	PurgeRecordsType::iterator i=m_purgeRecords.find(account);
	if (i==m_purgeRecords.end())
	{
		WARNING_DEBUG_FATAL(true,("Programmer bug:  onAllCharactersDeleted(%li) called with an account that wasn't in m_purgeRecords",account));
		return;
	}

	PurgeRecord & record = i->second;
	if (record.getPurgePhase()!=ppCharacterWarningSent)
	{
		WARNING_DEBUG_FATAL(true,("Programmer bug:  onAllCharactersDeleted(%li) called with an account that wasn't in the ppCharacterWarningSent phase",account));
		releaseAccount(record.getStationId());
		return;
	}
	
	if (success)
		record.advancePurgePhase();
	
	releaseAccount(record.getStationId());
}

// ----------------------------------------------------------------------

PurgePhase PurgeManagerNamespace::getNextPurgePhase(PurgePhase previousPurgePhase)
{
	switch (previousPurgePhase)
	{
		case ppNoPurge:
			return ppNoPurge;
		case ppNotWarned:
			return ppStructureWarningSent;
			break;
		case ppStructureWarningSent:
			return ppStructuresPurged;
			break;
		case ppStructuresPurged:
			return ppCharacterWarningSent;
			break;
		case ppCharacterWarningSent:
			return ppPurged;
			break;
		case ppPurged:
			return ppPurged;
		default:
			// This needs to FATAL, not WARN, because if the purge phase is fried it might mean we're going to purge accounts that aren't in the right phase
			FATAL(true,("Programmer bug:  getNextPurgePhase called with invalid purge phase %i",static_cast<int>(previousPurgePhase)));
			return ppNoPurge;
	}
}

// ======================================================================

PurgeRecord::PurgeRecord(StationId stationId, PurgePhase purgePhase) :
		m_stationId(stationId),
		m_purgePhase(purgePhase)
{
}
// ----------------------------------------------------------------------
	
StationId PurgeRecord::getStationId() const
{
	return m_stationId;
}

// ----------------------------------------------------------------------

PurgePhase PurgeRecord::getPurgePhase() const
{
	return m_purgePhase;
}

// ----------------------------------------------------------------------

void PurgeRecord::grabClusterList()
{
	std::vector<uint32> clusterList;
	LoginServer::getInstance().getClusterIds(clusterList);
	m_clusters.insert(clusterList.begin(), clusterList.end());
}

// ----------------------------------------------------------------------

bool PurgeRecord::hasCluster(uint32 clusterId) const
{
	return (m_clusters.find(clusterId)!=m_clusters.end());
}

// ----------------------------------------------------------------------

void PurgeRecord::removeCluster(uint32 clusterId)
{
	m_clusters.erase(clusterId);
}

// ----------------------------------------------------------------------

bool PurgeRecord::hasClusters() const
{
	return (!m_clusters.empty());
}

// ----------------------------------------------------------------------

/**
 * Move to the next phase in the purge sequence.
 */
void PurgeRecord::advancePurgePhase()
{
	m_purgePhase=getNextPurgePhase(m_purgePhase);
}

// ======================================================================
