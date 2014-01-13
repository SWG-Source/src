// ======================================================================
//
// TaskClaimRewards.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskClaimRewards.h"

#include "DatabaseConnection.h"
#include "serverNetworkMessages/ClaimRewardsReplyMessage.h"
#include "serverNetworkMessages/FeatureIdTransactionResponse.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

namespace TaskClaimRewardsNamespace
{
	class ConsumeEventQuery : public DB::Query
	{
	public:
		ConsumeEventQuery(StationId stationId, NetworkId const & characterId, uint32 clusterId, std::string const & eventId);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		bool getResult() const;
		
	private:
		DB::BindableLong station_id;
		DB::BindableNetworkId character_id;
		DB::BindableLong cluster_id;
		DB::BindableString<255> event_id;
		DB::BindableLong result;

	private: //disable
		ConsumeEventQuery();
		ConsumeEventQuery(const ConsumeEventQuery&);
		ConsumeEventQuery &operator=(const ConsumeEventQuery&);
	};

	class ClaimItemQuery : public DB::Query
	{
	public:
		ClaimItemQuery(StationId stationId, NetworkId const & characterId, uint32 clusterId, std::string const & itemId);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		bool getResult() const;
		
	private:
		DB::BindableLong station_id;
		DB::BindableNetworkId character_id;
		DB::BindableLong cluster_id;
		DB::BindableString<255> item_id;
		DB::BindableLong result;

	private: //disable
		ClaimItemQuery();
		ClaimItemQuery(const ClaimItemQuery&);
		ClaimItemQuery &operator=(const ClaimItemQuery&);
	};

	class UpdateFeatureIdTransactionQuery : public DB::Query
	{
	public:
		UpdateFeatureIdTransactionQuery(StationId stationId, NetworkId const & characterId, uint32 clusterId, std::string const & itemId, int countAdjustment);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		bool getResult() const;

	private:
		DB::BindableLong station_id;
		DB::BindableNetworkId character_id;
		DB::BindableLong cluster_id;
		DB::BindableString<255> item_id;
		DB::BindableLong count_adjustment;
		DB::BindableLong result;

	private: //disable
		UpdateFeatureIdTransactionQuery();
		UpdateFeatureIdTransactionQuery(const UpdateFeatureIdTransactionQuery&);
		UpdateFeatureIdTransactionQuery &operator=(const UpdateFeatureIdTransactionQuery&);
	};

	class GetFeatureIdTransactionsQuery : public DB::Query
	{
	public:
		GetFeatureIdTransactionsQuery(StationId stationId, NetworkId const & characterId, uint32 clusterId);

		DB::BindableString<255> item_id;
		DB::BindableLong count;

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	private:
		DB::BindableLong station_id;
		DB::BindableNetworkId character_id;
		DB::BindableLong cluster_id;

	private:  //disable
		GetFeatureIdTransactionsQuery(const GetFeatureIdTransactionsQuery&);
		GetFeatureIdTransactionsQuery& operator=(const GetFeatureIdTransactionsQuery&);
	};
}

using namespace TaskClaimRewardsNamespace;

// ======================================================================

TaskClaimRewards::TaskClaimRewards(uint32 clusterId, uint32 gameServerId, StationId stationId, NetworkId const & playerId, std::string const & rewardEvent, bool consumeEvent, std::string const & rewardItem, bool consumeItem, uint32 accountFeatureId, bool consumeAccountFeatureId, int previousAccountFeatureIdCount, int currentAccountFeatureIdCount) :
		DB::TaskRequest(),
		m_clusterId(clusterId),
		m_gameServerId(gameServerId),
		m_stationId(stationId),
		m_playerId(playerId),
		m_rewardEvent(rewardEvent),
		m_consumeEvent(consumeEvent),
		m_rewardItem(rewardItem),
		m_consumeItem(consumeItem),
		m_accountFeatureId(accountFeatureId),
		m_consumeAccountFeatureId(consumeAccountFeatureId),
		m_previousAccountFeatureIdCount(previousAccountFeatureIdCount),
		m_currentAccountFeatureIdCount(currentAccountFeatureIdCount),
		m_result(false)
{
}

// ----------------------------------------------------------------------

bool TaskClaimRewards::process(DB::Session *session)
{
	if (!session->setAutoCommitMode(false))
		return false;

	// record the account feature id transaction
	if ((m_accountFeatureId > 0) && m_consumeAccountFeatureId)
	{
		UpdateFeatureIdTransactionQuery featureIdQry(m_stationId, m_playerId, m_clusterId, m_rewardItem, 1);
		if (!session->exec(&featureIdQry))
			return false;
		if (!featureIdQry.getResult())
		{
			if (!session->rollbackTransaction())
				return false;
			m_result = false;
			return true;
		}
	}
	else
	{
		// consume the event
		if (m_consumeEvent)
		{
			ConsumeEventQuery eventQry(m_stationId, m_playerId, m_clusterId, m_rewardEvent);
			if (!session->exec(&eventQry))
				return false;
			if (!eventQry.getResult())
			{
				if (!session->rollbackTransaction())
					return false;
				m_result = false;
				return true;
			}
		}
		
		// consume the item
		if (m_consumeItem)
		{
			ClaimItemQuery itemQuery(m_stationId, m_playerId, m_clusterId, m_rewardItem);
			if (!session->exec(&itemQuery))
				return false;
			if (!itemQuery.getResult())
			{
				if (!session->rollbackTransaction())
					return false;
				m_result = false;
				return true;
			}
		}
	}

	if (!session->commitTransaction())
		return false;
	m_result = true;
	return true;
}

// ----------------------------------------------------------------------

void TaskClaimRewards::onComplete()
{
	// if the reward used an account feature id, we have already updated the account feature id
	// on session/Platform, so regardless if whether we were able to record the transaction
	// in the LoginServer DB or not, always send SUCCESS back to the game server for the item
	// to be granted
	ClaimRewardsReplyMessage msg(m_gameServerId, m_stationId, m_playerId, m_rewardEvent, m_rewardItem, m_accountFeatureId, m_consumeAccountFeatureId, m_previousAccountFeatureIdCount, m_currentAccountFeatureIdCount, (((m_accountFeatureId > 0) && m_consumeAccountFeatureId) ? true : m_result));
	LoginServer::getInstance().sendToCluster(m_clusterId, msg);
}

// ======================================================================

TaskFeatureIdTransactionRequest::TaskFeatureIdTransactionRequest(uint32 clusterId, StationId stationId, NetworkId const & playerId, uint32 gameServerId) :
DB::TaskRequest(),
m_clusterId(clusterId),
m_stationId(stationId),
m_playerId(playerId),
m_gameServerId(gameServerId),
m_transactions()
{
}

// ----------------------------------------------------------------------

bool TaskFeatureIdTransactionRequest::process(DB::Session *session)
{
	GetFeatureIdTransactionsQuery qry(m_stationId, m_playerId, m_clusterId);
	if (!(session->exec(&qry)))
		return false;

	int rowsFetched;
	while ((rowsFetched = qry.fetch()) > 0)
		m_transactions[qry.item_id.getValueASCII()] = static_cast<int>(qry.count.getValue());

	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void TaskFeatureIdTransactionRequest::onComplete()
{
	FeatureIdTransactionResponse const fitr(m_gameServerId, m_playerId, m_transactions);
	LoginServer::getInstance().sendToCluster(m_clusterId, fitr);
}

// ======================================================================

TaskFeatureIdTransactionSyncUpdate::TaskFeatureIdTransactionSyncUpdate(uint32 clusterId, StationId stationId, NetworkId const & playerId, std::string const & itemId, int adjustment) :
DB::TaskRequest(),
m_clusterId(clusterId),
m_stationId(stationId),
m_playerId(playerId),
m_itemId(itemId),
m_adjustment(adjustment)
{
}

// ----------------------------------------------------------------------

bool TaskFeatureIdTransactionSyncUpdate::process(DB::Session *session)
{
	UpdateFeatureIdTransactionQuery featureIdQry(m_stationId, m_playerId, m_clusterId, m_itemId, m_adjustment);
	if (!session->exec(&featureIdQry))
		return false;

	return true;
}

// ----------------------------------------------------------------------

void TaskFeatureIdTransactionSyncUpdate::onComplete()
{
}

// ======================================================================

TaskClaimRewardsNamespace::ConsumeEventQuery::ConsumeEventQuery(StationId stationId, NetworkId const & characterId, uint32 clusterId, std::string const & eventId) :
		DB::Query(),
		station_id(static_cast<long>(stationId)),
		character_id(characterId),
		cluster_id(static_cast<long>(clusterId)),
		event_id(eventId),
		result(0)
{
}

// ----------------------------------------------------------------------

void TaskClaimRewardsNamespace::ConsumeEventQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.consume_reward_event(:station_id, :character_id, :cluster_id, :event_id); end;";
}

// ----------------------------------------------------------------------

bool TaskClaimRewardsNamespace::ConsumeEventQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(cluster_id)) return false;
	if (!bindParameter(event_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskClaimRewardsNamespace::ConsumeEventQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

TaskClaimRewardsNamespace::ConsumeEventQuery::QueryMode TaskClaimRewardsNamespace::ConsumeEventQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

bool TaskClaimRewardsNamespace::ConsumeEventQuery::getResult() const
{
	return (result.getValue() != 0);
}

// ======================================================================

TaskClaimRewardsNamespace::ClaimItemQuery::ClaimItemQuery(StationId stationId, NetworkId const & characterId, uint32 clusterId, std::string const & itemId) :
		DB::Query(),
		station_id(static_cast<long>(stationId)),
		character_id(characterId),
		cluster_id(static_cast<long>(clusterId)),
		item_id(itemId),
		result(0)
{
}

// ----------------------------------------------------------------------

void TaskClaimRewardsNamespace::ClaimItemQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.claim_reward_item(:station_id, :character_id, :cluster_id, :item_id); end;";
}

// ----------------------------------------------------------------------

bool TaskClaimRewardsNamespace::ClaimItemQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(cluster_id)) return false;
	if (!bindParameter(item_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskClaimRewardsNamespace::ClaimItemQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

TaskClaimRewardsNamespace::ClaimItemQuery::QueryMode TaskClaimRewardsNamespace::ClaimItemQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

bool TaskClaimRewardsNamespace::ClaimItemQuery::getResult() const
{
	return (result.getValue() != 0);
}

// ======================================================================

TaskClaimRewardsNamespace::UpdateFeatureIdTransactionQuery::UpdateFeatureIdTransactionQuery(StationId stationId, NetworkId const & characterId, uint32 clusterId, std::string const & itemId, int countAdjustment) :
DB::Query(),
station_id(static_cast<long>(stationId)),
character_id(characterId),
cluster_id(static_cast<long>(clusterId)),
item_id(itemId),
count_adjustment(static_cast<long>(countAdjustment)),
result(0)
{
}

// ----------------------------------------------------------------------

void TaskClaimRewardsNamespace::UpdateFeatureIdTransactionQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.update_feature_id_transaction(:station_id, :cluster_id, :character_id, :item_id, :count_adjustment); end;";
}

// ----------------------------------------------------------------------

bool TaskClaimRewardsNamespace::UpdateFeatureIdTransactionQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(cluster_id)) return false;
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(item_id)) return false;
	if (!bindParameter(count_adjustment)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskClaimRewardsNamespace::UpdateFeatureIdTransactionQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

TaskClaimRewardsNamespace::UpdateFeatureIdTransactionQuery::QueryMode TaskClaimRewardsNamespace::UpdateFeatureIdTransactionQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

bool TaskClaimRewardsNamespace::UpdateFeatureIdTransactionQuery::getResult() const
{
	return (result.getValue() != 0);
}

// ======================================================================

TaskClaimRewardsNamespace::GetFeatureIdTransactionsQuery::GetFeatureIdTransactionsQuery(StationId stationId, NetworkId const & characterId, uint32 clusterId) :
DB::Query(),
item_id(),
count(0),
station_id(static_cast<long>(stationId)),
character_id(characterId),
cluster_id(static_cast<long>(clusterId))
{
}

// ----------------------------------------------------------------------

void TaskClaimRewardsNamespace::GetFeatureIdTransactionsQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.get_feature_id_transactions(:station_id, :cluster_id, :character_id); end;";
}

// ----------------------------------------------------------------------

bool TaskClaimRewardsNamespace::GetFeatureIdTransactionsQuery::bindParameters()
{
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(cluster_id)) return false;
	if (!bindParameter(character_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskClaimRewardsNamespace::GetFeatureIdTransactionsQuery::bindColumns()
{
	if (!bindCol(item_id)) return false;
	if (!bindCol(count)) return false;

	return true;
}

// ----------------------------------------------------------------------

TaskClaimRewardsNamespace::GetFeatureIdTransactionsQuery::QueryMode TaskClaimRewardsNamespace::GetFeatureIdTransactionsQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================
