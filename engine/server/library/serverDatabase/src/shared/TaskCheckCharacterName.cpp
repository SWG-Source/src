// ======================================================================
//
// TaskCheckCharacterName.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/TaskCheckCharacterName.h"

#include "serverDatabase/DataLookup.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

TaskCheckCharacterName::TaskCheckCharacterName(uint32 stationId, const Unicode::String &name) :
	m_name(name),
	m_stationId(stationId),
	m_resultCode(0)
{
}

// ----------------------------------------------------------------------

bool TaskCheckCharacterName::process(DB::Session *session)
{
	CheckCharacterNameQuery qry(Unicode::wideToNarrow(m_name));

	bool rval = session->exec(&qry);
	qry.done();

	m_resultCode = qry.result.getValue();
	return rval;
}

// ----------------------------------------------------------------------

void TaskCheckCharacterName::onComplete()
{
	DataLookup::getInstance().onCharacterNameChecked(m_stationId, m_name, m_resultCode);
	LOG("TraceCharacterCreation", ("%d TaskCheckCharacterName(%s) complete with result code %d", m_stationId, Unicode::wideToNarrow(m_name).c_str(), m_resultCode));
}

// ======================================================================

TaskCheckCharacterName::CheckCharacterNameQuery::CheckCharacterNameQuery(const std::string &name) :
	character_name(name)
{
}

// ----------------------------------------------------------------------

void TaskCheckCharacterName::CheckCharacterNameQuery::getSQL(std::string &sql)
{
	sql = "begin :result := " + DatabaseProcess::getInstance().getSchemaQualifier() + "datalookup.check_character_name (:name); end;";
}

// ----------------------------------------------------------------------

bool TaskCheckCharacterName::CheckCharacterNameQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(character_name)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskCheckCharacterName::CheckCharacterNameQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskCheckCharacterName::CheckCharacterNameQuery::getExecutionMode() const
{
	return DB::Query::MODE_PROCEXEC;
}

// ======================================================================