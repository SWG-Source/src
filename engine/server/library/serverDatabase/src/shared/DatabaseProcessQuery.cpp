// ======================================================================
//
// DatabaseProcessQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/DatabaseProcessQuery.h"

// ======================================================================

DatabaseProcessQuery::DatabaseProcessQuery(DB::Row *derivedRow) :
		DB::ModeQuery(derivedRow)
{
}

// ----------------------------------------------------------------------

DB::Query::QueryMode DatabaseProcessQuery::getExecutionMode() const
{
	if ((mode==mode_SELECT) && (getProtocol()==DB::PROTOCOL_OCI))
		return (MODE_PLSQL_REFCURSOR);
	else
		return (MODE_SQL);
}

// ======================================================================
