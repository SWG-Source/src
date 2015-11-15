// ======================================================================
//
// TaskRestoreCharacter.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskRestoreCharacter.h"

#include "DatabaseConnection.h"
#include "LoginServer.h"
#include "serverNetworkMessages/LoginCreateCharacterAckMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

TaskRestoreCharacter::TaskRestoreCharacter(uint32 clusterId, const std::string &whoRequested, StationId stationId, const Unicode::String &characterName, const NetworkId &characterId, int templateId, bool jedi) :
		TaskRequest(),
		m_clusterId(clusterId),
		m_whoRequested(whoRequested),
		m_stationId(stationId),
		m_characterName(characterName),
		m_characterId(characterId),
		m_templateId(templateId),
		m_jedi(jedi),
		m_result(0)
{
}

// ----------------------------------------------------------------------

bool TaskRestoreCharacter::process(DB::Session *session)
{
	RestoreCharacterQuery qry;
	qry.cluster_id = static_cast<long>(m_clusterId);
	qry.station_id = static_cast<long>(m_stationId);
	qry.character_name = m_characterName;
	qry.character_id = m_characterId;
	qry.template_id = m_templateId;
	if (m_jedi)
		qry.character_type = 2;
	else
		qry.character_type = 1;
	
	bool rval = session->exec(&qry);
	qry.done();

	qry.result.getValue(m_result);
	return rval;
}

// ----------------------------------------------------------------------

void TaskRestoreCharacter::onComplete()
{
	std::string message;
	switch (m_result)
	{
		case 1:
			message = "Character restored.";
			break;

		case 2:
			message = "Character restored.  The player now has more characters on this cluster than the normal limit.";
			break;

		case 3:
			message = "There was an error in the database while attempting to restore the character.";
			break;

		default:
			message = "The database returned an unknown code in response to the request to restore the character.";
			break;
	}
		
	GenericValueTypeMessage<std::pair<std::string, std::string> > reply("DatabaseConsoleReplyMessage",
																		std::make_pair(m_whoRequested, message));
	LoginServer::getInstance().sendToCluster(m_clusterId, reply);
}

// ======================================================================

TaskRestoreCharacter::RestoreCharacterQuery::RestoreCharacterQuery() :
		Query(),
		cluster_id(),
		station_id(),
		character_name(),
		character_id(),
		template_id(),
		character_type(),
		result()
{
}

// ----------------------------------------------------------------------

void TaskRestoreCharacter::RestoreCharacterQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.restore_character(:cluster_id, :station_id, :character_name, :character_id, :template_name, :character_type); end;";
}

// ----------------------------------------------------------------------

bool TaskRestoreCharacter::RestoreCharacterQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(cluster_id)) return false;
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(character_name)) return false;
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(template_id)) return false;
	if (!bindParameter(character_type)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskRestoreCharacter::RestoreCharacterQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskRestoreCharacter::RestoreCharacterQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
