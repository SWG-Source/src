// ======================================================================
//
// ScriptQueries.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/ScriptQueries.h"

#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;

// ======================================================================

void ScriptClearQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() + "persister.clear_scripts(:oid); end;";
}

// ----------------------------------------------------------------------

bool ScriptClearQuery::bindParameters()
{
	if (!bindParameter(object_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool ScriptClearQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

void ScriptClearQuery::setNetworkId(NetworkId objectID)
{
	object_id.setValue(objectID);
}

// ======================================================================

GetScripts::GetScripts(const std::string &schema) :
		m_data(ConfigServerDatabase::getDefaultFetchBatchSize()),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

void GetScripts::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ") + m_schema + "loader.load_scripts; end;";
}

// ----------------------------------------------------------------------

bool GetScripts::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool GetScripts::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].script_name)) return false;
	if (!bindCol(m_data[0].index)) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetScripts::getExecutionMode() const
{
	if (getProtocol()==DB::PROTOCOL_OCI)
		return (MODE_PLSQL_REFCURSOR);
	else
		return (MODE_SQL);
}

// ======================================================================
