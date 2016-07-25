// ======================================================================
//
// TaskEnableCharacter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskEnableCharacter.h"
#include "CentralServerConnection.h"

#include "ClientConnection.h"
#include "LoginServer.h"
#include "DatabaseConnection.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

TaskEnableCharacter::TaskEnableCharacter(StationId &stationId, const NetworkId &characterId, const std::string &whoRequested, bool enabled, uint32 clusterId) :
	TaskRequest(),
	m_stationId(stationId),
	m_characterId(characterId),
	m_whoRequested(whoRequested),
	m_enabled(enabled),
	m_clusterId(clusterId),
	m_result(0)
{
}

// ----------------------------------------------------------------------

bool TaskEnableCharacter::process(DB::Session *session)
{
	EnableCharacterQuery qry;
	qry.station_id = static_cast<long>(m_stationId);
	qry.character_id = m_characterId;
	qry.enabled.setValue(m_enabled);

	bool rval = session->exec(&qry);

	qry.done();
	m_result = qry.result.getValue();
	return rval;
}

// ----------------------------------------------------------------------

void TaskEnableCharacter::onComplete()
{
	std::string message;
	switch (m_result)
	{
	case 1:
		if (m_enabled)
			message = "Character enabled.";
		else
			message = "Character disabled.";
		break;

	case 2:
		message = "Could not find Character";
		break;

	default:
		message = "The database returned an unknown error code";
		break;
	}

	GenericValueTypeMessage<std::pair<std::string, std::string> > reply("EnableCharacterReplyMessage",
		std::make_pair(m_whoRequested, message));
	LoginServer::getInstance().sendToCluster(m_clusterId, reply);
}

// ----------------------------------------------------------------------

TaskEnableCharacter::EnableCharacterQuery::EnableCharacterQuery() :
	Query(),
	station_id(),
	character_id(),
	enabled()
{
}

// ----------------------------------------------------------------------

void TaskEnableCharacter::EnableCharacterQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ") + DatabaseConnection::getInstance().getSchemaQualifier() + "login.enable_disable_character(:station_id, :character_id, :enabled); end;";
}

// ----------------------------------------------------------------------

bool TaskEnableCharacter::EnableCharacterQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(enabled)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool TaskEnableCharacter::EnableCharacterQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskEnableCharacter::EnableCharacterQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================