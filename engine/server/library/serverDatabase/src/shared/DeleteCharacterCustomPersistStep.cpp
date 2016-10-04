// ======================================================================
//
// DeleteCharacterCustomPersistStep.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/DeleteCharacterCustomPersistStep.h"

#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

DeleteCharacterCustomPersistStep::DeleteCharacterCustomPersistStep(uint32 stationId, const NetworkId &characterId) :
	m_characterId(characterId),
	m_stationId(stationId),
	m_resultCode(0)
{
}

// ----------------------------------------------------------------------

bool DeleteCharacterCustomPersistStep::beforePersist(DB::Session *)
{
	return true;
}

// ----------------------------------------------------------------------

bool DeleteCharacterCustomPersistStep::afterPersist(DB::Session *session)
{
	DeleteCharacterQuery qry(m_stationId, m_characterId);

	if (!(session->exec(&qry)))
		return false;
	qry.done();

	m_resultCode = qry.result.getValue();
	return true;
}

// ----------------------------------------------------------------------

void DeleteCharacterCustomPersistStep::onComplete()
{
	if (m_resultCode == 2)
	{
		GenericValueTypeMessage<NetworkId> msg("ReleaseCharacterNameByIdMessage", m_characterId);
		DatabaseProcess::getInstance().sendToAllGameServers(msg, true);
	}
}

// ======================================================================

DeleteCharacterCustomPersistStep::DeleteCharacterQuery::DeleteCharacterQuery(uint32 stationId, const NetworkId &characterId) :
	station_id(stationId),
	character_id(characterId),
	delete_minutes(ConfigServerDatabase::getCharacterImmediateDeleteMinutes()),
	result()
{
}

// ----------------------------------------------------------------------

void DeleteCharacterCustomPersistStep::DeleteCharacterQuery::getSQL(std::string &sql)
{
	sql = "begin :result := " + DatabaseProcess::getInstance().getSchemaQualifier() + "persister.delete_character (:station_id, :character_id, :delete_minutes); end;";
}

// ----------------------------------------------------------------------

bool DeleteCharacterCustomPersistStep::DeleteCharacterQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(delete_minutes)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool DeleteCharacterCustomPersistStep::DeleteCharacterQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode DeleteCharacterCustomPersistStep::DeleteCharacterQuery::getExecutionMode() const
{
	return DB::Query::MODE_PROCEXEC;
}

// ======================================================================