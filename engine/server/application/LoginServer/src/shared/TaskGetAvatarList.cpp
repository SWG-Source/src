// ======================================================================
//
// TaskGetAvatarList.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskGetAvatarList.h"

#include "serverNetworkMessages/AvatarList.h"
#include "CentralServerConnection.h"
#include "DatabaseConnection.h"
#include "sharedLog/Log.h"
#include "serverNetworkMessages/TransferAccountData.h"
#include "serverNetworkMessages/TransferAccountDataArchive.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "TaskUpgradeAccount.h"
#include <vector>

// ======================================================================

TaskGetAvatarList::TaskGetAvatarList (StationId stationId, int clusterGroupId, const TransferCharacterData * transferCharacterData) :
		TaskRequest(),
		m_stationId(stationId),
		m_stationIdNumberJediSlot(0),
		m_clusterGroupId(clusterGroupId),
		m_avatars(),
		m_transferCharacterData(0),
		m_transferAccountData(0)
{
	if(transferCharacterData)
	{
		m_transferCharacterData = new TransferCharacterData(*transferCharacterData);
	}
}

// ----------------------------------------------------------------------

TaskGetAvatarList::TaskGetAvatarList (int clusterGroupId, const TransferAccountData * transferAccountData) :
		TaskRequest(),
		m_stationId(),
		m_stationIdNumberJediSlot(0),
		m_clusterGroupId(clusterGroupId),
		m_avatars(),
		m_transferCharacterData(0),
		m_transferAccountData(0)
{
	if(transferAccountData)
	{
		m_transferAccountData = new TransferAccountData(*transferAccountData);
		m_stationId = m_transferAccountData->getSourceStationId();
	}
}

// ----------------------------------------------------------------------

TaskGetAvatarList::~TaskGetAvatarList()
{
	delete m_transferCharacterData;
	m_transferCharacterData = 0;
	delete m_transferAccountData;
	m_transferAccountData = 0;
}

// ----------------------------------------------------------------------

bool TaskGetAvatarList::process(DB::Session *session)
{
	if (m_transferAccountData)
	{
		// if we are doing an account transfer, we need to also check the number of avatars in the destination station id 
		GetCharactersQuery qry;
		int rowsFetched;
		
		qry.station_id = m_transferAccountData->getDestinationStationId();
		qry.cluster_group_id = m_clusterGroupId;

		if (! (session->exec(&qry)))
			return false;

		bool hasAvatars = false;
		while ((rowsFetched = qry.fetch()) > 0)
		{
			hasAvatars = true;
		}

		m_transferAccountData->setDestinationHasAvatars(hasAvatars);
	}

	m_stationIdNumberJediSlot = 0;
	if (!m_transferAccountData)
	{
		TaskUpgradeAccount::QueryJediQuery qryHasJediSlot;
		qryHasJediSlot.station_id = m_stationId; //lint !e713 // loss of precision unsigned long to long
		qryHasJediSlot.character_type = 2;

		if (session->exec(&qryHasJediSlot))
		{
			m_stationIdNumberJediSlot = qryHasJediSlot.result.getValue();
		}

		qryHasJediSlot.done();
	}

	GetCharactersQuery qry;
	int rowsFetched;
	
	qry.station_id = m_stationId; //lint !e713 // loss of precision unsigned long to long
	qry.cluster_group_id = m_clusterGroupId;

	if (! (session->exec(&qry)))
		return false;

	AvatarRecord temp;
	
	while ((rowsFetched = qry.fetch()) > 0)
	{
		qry.character_name.getValue(temp.m_name);
		qry.object_template_id.getValue(temp.m_objectTemplateId);
		qry.object_id.getValue(temp.m_networkId);
		qry.cluster_id.getValue(temp.m_clusterId);
		qry.character_type.getValue(temp.m_characterType);

		m_avatars.push_back(temp);
	}
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void TaskGetAvatarList::onComplete()
{
	if (m_transferAccountData)
	{
		if (m_transferAccountData->getDestinationHasAvatars())
		{
			// send a message back to the transfer server
			LOG("CustomerService", ("CharacterTransfer: Cannot complete account transfer from %lu to %lu: destination stationId contains avatars\n", m_transferAccountData->getSourceStationId(), m_transferAccountData->getDestinationStationId()));
			const GenericValueTypeMessage<TransferAccountData> response("TransferAccountFailedDestinationNotEmpty", *m_transferAccountData);
			CentralServerConnection::sendToCentralServer(m_transferAccountData->getStartGalaxy(), response);
		}
		else
			DatabaseConnection::getInstance().onAvatarListRetrievedAccountTransfer(m_avatars, m_transferAccountData);

	}
	else
	{
		DatabaseConnection::getInstance().onAvatarListRetrieved(m_stationId, m_stationIdNumberJediSlot, m_avatars, m_transferCharacterData);
	}
}

// ----------------------------------------------------------------------

AvatarList const & TaskGetAvatarList::getAvatars() const
{
	return m_avatars;
}

// ----------------------------------------------------------------------

StationId TaskGetAvatarList::getStationId() const
{
	return m_stationId;
}

// ======================================================================

void TaskGetAvatarList::GetCharactersQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.get_avatar_list(:station_id, :cluster_group_id); end;";
 	// DEBUG_REPORT_LOG(true, ("TaskGetAvatarList SQL: %s\n", sql.c_str()));
}

// ----------------------------------------------------------------------

bool TaskGetAvatarList::GetCharactersQuery::bindParameters()
{
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(cluster_group_id)) return false;

	return true;	
}

// ----------------------------------------------------------------------

bool TaskGetAvatarList::GetCharactersQuery::bindColumns()
{
	if (!bindCol(character_name)) return false;
	if (!bindCol(object_template_id)) return false;
	if (!bindCol(object_id)) return false;
	if (!bindCol(cluster_id)) return false;
	if (!bindCol(character_type)) return false;

	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskGetAvatarList::GetCharactersQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ----------------------------------------------------------------------

TaskGetAvatarList::GetCharactersQuery::GetCharactersQuery() :
		Query(),
		station_id(),
		cluster_group_id(),
		character_name(),
		object_template_id(),
		object_id(),
		cluster_id(),
		character_type()
{
}

// ======================================================================
