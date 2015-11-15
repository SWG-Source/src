// ======================================================================
//
// SaveTimestampQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/SaveTimestampQuery.h"

#include "serverDatabase/DatabaseProcess.h"

using namespace DBQuery;

// ======================================================================

SaveTimestampQuery::SaveTimestampQuery(int _timestamp) :
		timestamp(_timestamp)
{	
}

// ======================================================================

void SaveTimestampQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() + "persister.set_clock (:time); end;";
}

// ======================================================================

bool SaveTimestampQuery::bindParameters()
{
	if (!bindParameter(timestamp)) return false;
	return true;
}

// ======================================================================

bool SaveTimestampQuery::bindColumns()
{
	return true;
}

// ======================================================================
