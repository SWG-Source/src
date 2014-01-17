// ======================================================================
//
// GetVersionNumberQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/GetVersionNumberQuery.h"

#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;

// ----------------------------------------------------------------------

GetVersionNumberQuery::GetVersionNumberQuery() :
		Query(),
		current_version_number(),
		min_version_number()
{
}

// ----------------------------------------------------------------------

void GetVersionNumberQuery::getSQL(std::string &sql)
{
	if (getProtocol()==DB::PROTOCOL_ODBC)
		sql="select version_number from version_number;";
	else
		sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier() + "loader.get_version_number(:current_version_number, :min_version_number); end;";
}

// ----------------------------------------------------------------------

bool GetVersionNumberQuery::bindParameters()
{
	if (getProtocol()!=DB::PROTOCOL_ODBC)
	{
		if (!bindParameter(current_version_number)) return false;
		if (!bindParameter(min_version_number)) return false;
	}
	return true;
}

// ----------------------------------------------------------------------

bool GetVersionNumberQuery::bindColumns()
{
	if (getProtocol()==DB::PROTOCOL_ODBC)
	{
		if (!bindCol(current_version_number)) return false;
	}
	return true;
}

// ======================================================================
