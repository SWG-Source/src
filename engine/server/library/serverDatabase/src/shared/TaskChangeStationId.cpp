// ======================================================================
//
// TaskChangeStationId.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "TaskChangeStationId.h"

#include <vector>
#include "sharedLog/Log.h"
#include "serverNetworkMessages/TransferAccountData.h"
#include "serverNetworkMessages/TransferAccountDataArchive.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

TaskChangeStationId::TaskChangeStationId(StationId sourceStationId, StationId destinationStationId, const TransferAccountData *requestData) :
	TaskRequest(),
	m_sourceStationId(sourceStationId),
	m_destinationStationId(destinationStationId),
	m_requestData(0),
	m_success(false)
{
	if(requestData)
	{
		m_requestData = new TransferAccountData(*requestData);
	}
}

// ----------------------------------------------------------------------

TaskChangeStationId::~TaskChangeStationId()
{
	delete m_requestData;
	m_requestData = 0;
}

// ----------------------------------------------------------------------

bool TaskChangeStationId::process(DB::Session *session)
{
	ChangeStationIdQuery qry;
	qry.source_station_id = m_sourceStationId;
	qry.destination_station_id = m_destinationStationId;

	m_success = session->exec(&qry);

	qry.done();
	return m_success;
}

// ----------------------------------------------------------------------

void TaskChangeStationId::onComplete()
{
	LOG("CustomerService", ("CharacterTransfer: Completed changing station id from %lu to %lu in game database", m_sourceStationId, m_destinationStationId));
}

// ======================================================================

TaskChangeStationId::ChangeStationIdQuery::ChangeStationIdQuery() :
		Query(),
		destination_station_id(0),
		source_station_id(0)
{
}

// ----------------------------------------------------------------------

void TaskChangeStationId::ChangeStationIdQuery::getSQL(std::string &sql)
{
	sql = std::string("begin Update ")+DatabaseProcess::getInstance().getSchemaQualifier()+"players Set station_id = :destination_station_id Where station_id = :source_station_id; end;";
//	DEBUG_REPORT_LOG(true, ("TaskChangeStationId SQL: %s\n", sql.c_str()));
}

// ----------------------------------------------------------------------

bool TaskChangeStationId::ChangeStationIdQuery::bindParameters()
{
	if (!bindParameter(destination_station_id)) return false;
	if (!bindParameter(source_station_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskChangeStationId::ChangeStationIdQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskChangeStationId::ChangeStationIdQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
