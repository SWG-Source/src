// ======================================================================
//
// GetUniverseQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/GetUniverseQuery.h"

#include "serverDatabase/DatabaseProcess.h"

using namespace DBQuery;

// ======================================================================

GetUniverseQuery::GetUniverseQuery(const std::string &schema) :
		Query(),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

void GetUniverseQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ") + m_schema + "loader.locate_universe; end;";
}

// ----------------------------------------------------------------------

bool GetUniverseQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool GetUniverseQuery::bindColumns()
{
	return true;
}

// ======================================================================
