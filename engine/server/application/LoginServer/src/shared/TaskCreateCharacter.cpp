// ======================================================================
//
// TaskCreateCharacter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskCreateCharacter.h"

#include "LoginServer.h"
#include "DatabaseConnection.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "serverNetworkMessages/LoginCreateCharacterAckMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/NetworkIdArchive.h"

// ======================================================================

TaskCreateCharacter::TaskCreateCharacter(uint32 clusterId, StationId stationId, const Unicode::String &characterName, const NetworkId &characterId, int templateId, bool jedi) :
		TaskRequest(),
		m_clusterId(clusterId),
		m_stationId(stationId),
		m_characterName(characterName),
		m_characterId(characterId),
		m_templateId(templateId),
		m_jedi(jedi)
{
}

// ----------------------------------------------------------------------

bool TaskCreateCharacter::process(DB::Session *session)
{
	CreateCharacterQuery qry;
	qry.cluster_id = static_cast<long>(m_clusterId);
	qry.station_id = static_cast<long>(m_stationId);
	qry.character_name = m_characterName;
	qry.character_id = m_characterId;
	qry.template_id = m_templateId;
	if (m_jedi)
		qry.character_type = 2; //TODO: Make an enum somewhere
	else
		qry.character_type = 1;
	
	bool rval = session->exec(&qry);
	
	qry.done();
	return rval;
}

// ----------------------------------------------------------------------

void TaskCreateCharacter::onComplete()
{
	LoginCreateCharacterAckMessage ackMessage(m_stationId,m_characterId);
	LoginServer::getInstance().sendToCluster(m_clusterId,ackMessage);

	// let all other galaxies know that a new character has been created for the station account
	GenericValueTypeMessage<StationId> const ncc("NewCharacterCreated", m_stationId);
	LoginServer::getInstance().sendToAllClusters(ncc, nullptr, m_clusterId);
}

// ======================================================================

TaskCreateCharacter::CreateCharacterQuery::CreateCharacterQuery() :
		Query(),
		cluster_id(),
		station_id(),
		character_name(),
		character_id(),
		template_id(),
		character_type()
{
}

// ----------------------------------------------------------------------

void TaskCreateCharacter::CreateCharacterQuery::getSQL(std::string &sql)
{
	sql = std::string("begin ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.create_character(:cluster_id, :station_id, :character_name, :character_id, :template_name, :character_type); end;";
	// DEBUG_REPORT_LOG(true, ("TaskCreateCharacter SQL: %s\n", sql.c_str()));
}

// ----------------------------------------------------------------------

bool TaskCreateCharacter::CreateCharacterQuery::bindParameters()
{
	if (!bindParameter(cluster_id)) return false;
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(character_name)) return false;
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(template_id)) return false;
	if (!bindParameter(character_type)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskCreateCharacter::CreateCharacterQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskCreateCharacter::CreateCharacterQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
