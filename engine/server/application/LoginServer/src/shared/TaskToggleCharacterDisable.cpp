// ======================================================================
//
// TaskToggleCharacterDisable.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskToggleCharacterDisable.h"

#include "ClientConnection.h"
#include "LoginServer.h"
#include "DatabaseConnection.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

TaskToggleCharacterDisable::TaskToggleCharacterDisable(uint32 clusterId, const NetworkId &characterId, StationId stationId, bool enabled) :
		TaskRequest(),
		m_clusterId(clusterId),
		m_characterId(characterId),
		m_stationId(stationId),
		m_enabled(enabled)
{
}

// ----------------------------------------------------------------------

bool TaskToggleCharacterDisable::process(DB::Session *session)
{
	ToggleCharacterDisableQuery qry;
	qry.cluster_id=static_cast<long>(m_clusterId);
	qry.character_id=m_characterId;
	qry.station_id=static_cast<long>(m_stationId);
	qry.enabled_flag=m_enabled;
	
	bool rval = session->exec(&qry);
	
	qry.done();
	return rval;
}

// ----------------------------------------------------------------------

void TaskToggleCharacterDisable::onComplete()
{
}

// ----------------------------------------------------------------------

TaskToggleCharacterDisable::ToggleCharacterDisableQuery::ToggleCharacterDisableQuery() :
		Query(),
		cluster_id(),
		character_id(),
		station_id(),
		enabled_flag()
{
}

// ----------------------------------------------------------------------

void TaskToggleCharacterDisable::ToggleCharacterDisableQuery::getSQL(std::string &sql)
{
	sql = std::string("begin ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.toggle_disable_character(:cluster_id, :character_id, :station_id, :enabled); end;";
	// DEBUG_REPORT_LOG(true, ("TaskToggleCharacterDisable SQL: %s\n", sql.c_str()));

}

// ----------------------------------------------------------------------

bool TaskToggleCharacterDisable::ToggleCharacterDisableQuery::bindParameters()
{
	if (!bindParameter(cluster_id)) return false;
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(enabled_flag)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool TaskToggleCharacterDisable::ToggleCharacterDisableQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskToggleCharacterDisable::ToggleCharacterDisableQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
