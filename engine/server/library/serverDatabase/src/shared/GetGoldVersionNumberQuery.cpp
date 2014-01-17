// ======================================================================
//
// GetGoldVersionNumberQuery.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/GetGoldVersionNumberQuery.h"

#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;

// ----------------------------------------------------------------------

GetGoldVersionNumberQuery::GetGoldVersionNumberQuery() :
		Query(),
		current_version_number(),
		min_version_number()
{
}

// ----------------------------------------------------------------------

void GetGoldVersionNumberQuery::getSQL(std::string &sql)
{
	if (getProtocol()==DB::PROTOCOL_ODBC)
		sql="select version_number from version_number;";
	else
		sql=std::string("begin ")+DatabaseProcess::getInstance().getGoldSchemaQualifier() + "loader.get_version_number(:current_version_number, :min_version_number); end;";
}

// ----------------------------------------------------------------------

bool GetGoldVersionNumberQuery::bindParameters()
{
	if (getProtocol()!=DB::PROTOCOL_ODBC)
	{
		if (!bindParameter(current_version_number)) return false;
		if (!bindParameter(min_version_number)) return false;
	}
	return true;
}

// ----------------------------------------------------------------------

bool GetGoldVersionNumberQuery::bindColumns()
{
	if (getProtocol()==DB::PROTOCOL_ODBC)
	{
		if (!bindCol(current_version_number)) return false;
	}
	return true;
}

// ======================================================================
