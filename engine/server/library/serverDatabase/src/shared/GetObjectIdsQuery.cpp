// ======================================================================
//
// GetOIDsQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/GetObjectIdsQuery.h"

#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;

// ----------------------------------------------------------------------

GetOIDsQuery::GetOIDsQuery() :
		min_count(0)
{
}

// ----------------------------------------------------------------------

void GetOIDsQuery::getSQL(std::string &sql)
{
	if (getProtocol()==DB::PROTOCOL_ODBC)
		sql="select getOidBlock(?)";
	else
		sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() + "objectidmanager.get_ids(:desired_count, :start_id, :end_id); end;";
}

// ----------------------------------------------------------------------

bool GetOIDsQuery::bindParameters()
{
	if (!bindParameter(min_count)) return false;
	if (!bindParameter(start_id)) return false;
	if (!bindParameter(end_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool GetOIDsQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetOIDsQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
