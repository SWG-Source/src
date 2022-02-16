// ======================================================================
//
// TaskGetValidationData.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskGetValidationData.h"

#include "ConfigLoginServer.h"
#include "DatabaseConnection.h"
#include "TaskGetAvatarList.h"
#include "TaskUpgradeAccount.h"
#include "serverNetworkMessages/AvatarList.h"
#include "serverNetworkMessages/TransferRequestMoveValidation.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

namespace TaskGetValidationDataNamespace
{
	class GetValidationDataQuery : public DB::Query
	{
	public:
		GetValidationDataQuery();

		DB::BindableLong station_id; //lint !e1925 // public data member
		DB::BindableLong cluster_id; //lint !e1925 // public data member
		DB::BindableLong character_type_id; //lint !e1925 // public data member
		DB::BindableLong num_open_slots; //lint !e1925 // public data member

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	private: //disable
		GetValidationDataQuery(const GetValidationDataQuery&);
		GetValidationDataQuery &operator=(const GetValidationDataQuery&);
	};

	class GetCompletedTutorialQuery : public DB::Query
	{
	public:
		GetCompletedTutorialQuery(StationId stationId);

		DB::BindableLong station_id;
		DB::BindableBool completed_tutorial;

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	private: //disable
		GetCompletedTutorialQuery(const GetCompletedTutorialQuery&);
		GetCompletedTutorialQuery &operator=(const GetCompletedTutorialQuery&);
	};

	class GetConsumedRewardEventsQuery : public DB::Query
	{
	public:
		explicit GetConsumedRewardEventsQuery(StationId stationId);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		std::string getEventId() const;
		uint32 getClusterId() const;
		NetworkId const getCharacterId() const;

	private:
		DB::BindableLong station_id;
		DB::BindableString<100> event_id;
		DB::BindableLong cluster_id;
		DB::BindableNetworkId character_id;

	private: //disable
		GetConsumedRewardEventsQuery();
		GetConsumedRewardEventsQuery(const GetConsumedRewardEventsQuery&);
		GetConsumedRewardEventsQuery &operator=(const GetConsumedRewardEventsQuery&);
	};

	class GetClaimedRewardItemsQuery : public DB::Query
	{
	public:
		explicit GetClaimedRewardItemsQuery(StationId stationId);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		std::string getItemId() const;
		uint32 getClusterId() const;
		NetworkId const getCharacterId() const;

	private:
		DB::BindableLong station_id;
		DB::BindableString<100> item_id;
		DB::BindableLong cluster_id;
		DB::BindableNetworkId character_id;

	private: //disable
		GetClaimedRewardItemsQuery();
		GetClaimedRewardItemsQuery(const GetClaimedRewardItemsQuery&);
		GetClaimedRewardItemsQuery &operator=(const GetClaimedRewardItemsQuery&);
	};

	class IsClusterAtLimitQuery : public DB::Query
	{
	public:
		IsClusterAtLimitQuery();

		DB::BindableLong         cluster_id; //lint !e1925 // public data member
		DB::BindableLong         result; //lint !e1925 // public data member

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	private: //disable
		IsClusterAtLimitQuery(const IsClusterAtLimitQuery&);
		IsClusterAtLimitQuery &operator=    (const IsClusterAtLimitQuery&);
	};
}

using namespace TaskGetValidationDataNamespace;

// ======================================================================

TaskGetValidationData::TaskGetValidationData(StationId stationId, int clusterGroupId, uint32 clusterId, unsigned int track, uint32 subscriptionBits) :
	TaskRequest(),
	m_stationId(stationId),
	m_clusterGroupId(clusterGroupId),
	m_clusterId(clusterId),
	m_openCharacterSlots(4, 0),
	m_canSkipTutorial(false),
	m_track(track),
	m_subscriptionBits(subscriptionBits),
	m_transferRequest(0),
	m_transferRequestSourceCharacterTemplateId(0),
	m_consumedRewardEvents(),
	m_claimedRewardItems()
{
}

// ----------------------------------------------------------------------

TaskGetValidationData::TaskGetValidationData(const TransferRequestMoveValidation & request, int clusterGroupId, uint32 clusterId) :
	TaskRequest(),
	m_stationId(request.getDestinationStationId()),
	m_clusterGroupId(clusterGroupId),
	m_clusterId(clusterId),
	m_openCharacterSlots(4, 0),
	m_canSkipTutorial(true),
	m_track(request.getTrack()),
	m_transferRequest(0),
	m_transferRequestSourceCharacterTemplateId(request.getSourceCharacterTemplateId()),
	m_consumedRewardEvents(),
	m_claimedRewardItems(),
	m_subscriptionBits(0)
{
	Archive::ByteStream bs;
	request.pack(bs);
	Archive::ReadIterator ri(bs);
	m_transferRequest = new TransferRequestMoveValidation(ri);
}

// ----------------------------------------------------------------------

TaskGetValidationData::~TaskGetValidationData()
{
	delete m_transferRequest;
}

// ----------------------------------------------------------------------

bool TaskGetValidationData::process(DB::Session *session)
{
	// if this is a same account transfer request, use a different check
	// for available character slots; specifically, don't enforce the max
	// characters per account rule because this is a same account transfer
	// and after everything is said and done, the number of characters on
	// the account will not change
	if (m_transferRequest && (m_transferRequest->getSourceStationId() == m_transferRequest->getDestinationStationId()))
	{
		IsClusterAtLimitQuery clusterLimitQry;
		clusterLimitQry.cluster_id = static_cast<long>(m_clusterId);

		if (!(session->exec(&clusterLimitQry)))
			return false;

		clusterLimitQry.done();

		if (clusterLimitQry.result.getValue() == 0)
		{
			// cluster is not at limit, check per account per cluster limit
			TaskVacateUnlockedSlot::GetOnlyOpenCharacterSlotsQuery qry;
			qry.station_id = static_cast<long>(m_stationId);
			qry.cluster_id = static_cast<long>(m_clusterId);

			int rowsFetched;
			if (!(session->exec(&qry)))
				return false;
			while ((rowsFetched = qry.fetch()) > 0)
				m_openCharacterSlots[static_cast<unsigned long>(qry.character_type_id.getValue())] = qry.num_open_slots.getValue();
			qry.done();
			if (rowsFetched < 0)
				return false;
		}
	}
	else
	{
		GetValidationDataQuery qry;
		qry.station_id = static_cast<long>(m_stationId);
		qry.cluster_id = static_cast<long>(m_clusterId);

		int rowsFetched;
		if (!(session->exec(&qry)))
			return false;
		while ((rowsFetched = qry.fetch()) > 0)
			m_openCharacterSlots[static_cast<unsigned long>(qry.character_type_id.getValue())] = qry.num_open_slots.getValue();
		qry.done();
		if (rowsFetched < 0)
			return false;
	}

	// if transfer request and the source character
	// template id hasn't been specified, retrieve it
	if (m_transferRequest && (m_openCharacterSlots[1] > 0) && (m_transferRequestSourceCharacterTemplateId == 0))
	{
		uint32 const sourceClusterId = LoginServer::getInstance().getClusterIDByName(m_transferRequest->getSourceGalaxy());

		TaskGetAvatarList::GetCharactersQuery qryGetCharacters;
		qryGetCharacters.station_id = m_transferRequest->getSourceStationId(); //lint !e713 // loss of precision unsigned long to long
		qryGetCharacters.cluster_group_id = m_clusterGroupId;

		if (!session->exec(&qryGetCharacters))
			return false;

		uint32 characterClusterId;
		std::string characterName;
		int rowsFetched;
		while ((rowsFetched = qryGetCharacters.fetch()) > 0)
		{
			qryGetCharacters.cluster_id.getValue(characterClusterId);
			if (characterClusterId == sourceClusterId)
			{
				qryGetCharacters.character_name.getValue(characterName);
				if (characterName == m_transferRequest->getSourceCharacter())
				{
					qryGetCharacters.object_template_id.getValue(m_transferRequestSourceCharacterTemplateId);
					break;
				}
			}
		}

		qryGetCharacters.done();
	}

	if (!m_transferRequest)
	{
		GetConsumedRewardEventsQuery eventQuery(m_stationId);
		int rowsFetched;
		if (!(session->exec(&eventQuery)))
			return false;
		while ((rowsFetched = eventQuery.fetch()) > 0)
		{
			// If the event was claimed on this cluster, tell the cluster the network ID of the character that claimed it.
			// If claimed on another cluster, hide the network ID because it's not meaningful
			NetworkId const characterId = eventQuery.getClusterId() == m_clusterId ? eventQuery.getCharacterId() : NetworkId::cms_invalid;
			m_consumedRewardEvents.push_back(std::make_pair(characterId, eventQuery.getEventId()));
		}
		eventQuery.done();
		if (rowsFetched < 0)
			return false;

		GetClaimedRewardItemsQuery itemQuery(m_stationId);
		if (!(session->exec(&itemQuery)))
			return false;
		while ((rowsFetched = itemQuery.fetch()) > 0)
		{
			NetworkId const characterId = itemQuery.getClusterId() == m_clusterId ? itemQuery.getCharacterId() : NetworkId::cms_invalid;
			m_claimedRewardItems.push_back(std::make_pair(characterId, itemQuery.getItemId()));
		}
		itemQuery.done();
		if (rowsFetched < 0)
			return false;

		// check to see if the account can skip the tutorial or not
		GetCompletedTutorialQuery tutorialQuery(m_stationId);
		if (!(session->exec(&tutorialQuery)))
			return false;
		while ((rowsFetched = tutorialQuery.fetch()) > 0)
		{
			tutorialQuery.completed_tutorial.getValue(m_canSkipTutorial);
		}
		tutorialQuery.done();
		if (rowsFetched < 0)
			return false;
	}
	return true;
}

// ----------------------------------------------------------------------

void TaskGetValidationData::onComplete()
{
	bool canCreateNormal = false;
	bool canCreateJedi = false;
	if (m_openCharacterSlots[1] > 0)
		canCreateNormal = true;
	if (m_openCharacterSlots[2] > 0 && m_openCharacterSlots[3] > 0)
		canCreateJedi = true;

	// check to see if character creation has been disabled for the cluster
	if (ConfigLoginServer::isCharacterCreationDisabled(LoginServer::getInstance().getClusterNameById(m_clusterId)))
	{
		canCreateNormal = false;
		canCreateJedi = false;
	}

	if (m_transferRequest)
	{
		LoginServer::getInstance().validateAccountForTransfer(*m_transferRequest, m_clusterId, m_transferRequestSourceCharacterTemplateId, canCreateNormal, false);
	}
	else
	{
		LoginServer::getInstance().validateAccount(m_stationId, m_clusterId, m_subscriptionBits, canCreateNormal, canCreateJedi, m_canSkipTutorial, m_track, m_consumedRewardEvents, m_claimedRewardItems);
	}
}

// ======================================================================

void GetValidationDataQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :rc := ") + DatabaseConnection::getInstance().getSchemaQualifier() + "login.get_open_character_slots(:station_id,:cluster_id); end;";
}

// ----------------------------------------------------------------------

bool GetValidationDataQuery::bindParameters()
{
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(cluster_id)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool GetValidationDataQuery::bindColumns()
{
	if (!bindCol(character_type_id)) return false;
	if (!bindCol(num_open_slots)) return false;

	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetValidationDataQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ----------------------------------------------------------------------

GetValidationDataQuery::GetValidationDataQuery() :
	Query(),
	station_id(),
	cluster_id(),
	character_type_id(),
	num_open_slots()
{
}

// ======================================================================

void GetCompletedTutorialQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :rc := ") + DatabaseConnection::getInstance().getSchemaQualifier() + "login.get_completed_tutorial(:station_id); end;";
}

// ----------------------------------------------------------------------

bool GetCompletedTutorialQuery::bindParameters()
{
	if (!bindParameter(station_id)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool GetCompletedTutorialQuery::bindColumns()
{
	if (!bindCol(completed_tutorial)) return false;

	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetCompletedTutorialQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ----------------------------------------------------------------------

GetCompletedTutorialQuery::GetCompletedTutorialQuery(StationId stationId) :
	DB::Query(),
	station_id(stationId),
	completed_tutorial(false)
{
}

// ======================================================================

GetConsumedRewardEventsQuery::GetConsumedRewardEventsQuery(StationId stationId) :
	DB::Query(),
	station_id(static_cast<long>(stationId)),
	event_id(),
	cluster_id(),
	character_id()
{
}

// ----------------------------------------------------------------------

void GetConsumedRewardEventsQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ") + DatabaseConnection::getInstance().getSchemaQualifier() + "login.get_consumed_reward_events(:station_id); end;";
}

// ----------------------------------------------------------------------

bool GetConsumedRewardEventsQuery::bindParameters()
{
	if (!bindParameter(station_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool GetConsumedRewardEventsQuery::bindColumns()
{
	if (!bindCol(event_id)) return false;
	if (!bindCol(cluster_id)) return false;
	if (!bindCol(character_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetConsumedRewardEventsQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ----------------------------------------------------------------------

std::string GetConsumedRewardEventsQuery::getEventId() const
{
	return (event_id.getValueASCII());
}

// ----------------------------------------------------------------------

uint32 GetConsumedRewardEventsQuery::getClusterId() const
{
	return static_cast<uint32>(cluster_id.getValue());
}

// ----------------------------------------------------------------------

NetworkId const GetConsumedRewardEventsQuery::getCharacterId() const
{
	return character_id.getValue();
}

// ======================================================================

GetClaimedRewardItemsQuery::GetClaimedRewardItemsQuery(StationId stationId) :
	DB::Query(),
	station_id(static_cast<long>(stationId)),
	item_id(),
	cluster_id(),
	character_id()
{
}

// ----------------------------------------------------------------------

void GetClaimedRewardItemsQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ") + DatabaseConnection::getInstance().getSchemaQualifier() + "login.get_claimed_reward_items(:station_id); end;";
}

// ----------------------------------------------------------------------

bool GetClaimedRewardItemsQuery::bindParameters()
{
	if (!bindParameter(station_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool GetClaimedRewardItemsQuery::bindColumns()
{
	if (!bindCol(item_id)) return false;
	if (!bindCol(cluster_id)) return false;
	if (!bindCol(character_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetClaimedRewardItemsQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ----------------------------------------------------------------------

std::string GetClaimedRewardItemsQuery::getItemId() const
{
	return (item_id.getValueASCII());
}

// ----------------------------------------------------------------------

uint32 GetClaimedRewardItemsQuery::getClusterId() const
{
	return static_cast<uint32>(cluster_id.getValue());
}

// ----------------------------------------------------------------------

NetworkId const GetClaimedRewardItemsQuery::getCharacterId() const
{
	return character_id.getValue();
}

// ======================================================================

IsClusterAtLimitQuery::IsClusterAtLimitQuery() :
	Query(),
	cluster_id(),
	result()
{
}

// ----------------------------------------------------------------------

void IsClusterAtLimitQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ") + DatabaseConnection::getInstance().getSchemaQualifier() + "login.is_cluster_at_limit(:cluster_id); end;";
}

// ----------------------------------------------------------------------

bool IsClusterAtLimitQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(cluster_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool IsClusterAtLimitQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode IsClusterAtLimitQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================