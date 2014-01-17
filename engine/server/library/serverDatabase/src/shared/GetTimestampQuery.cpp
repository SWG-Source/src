// ======================================================================
//
// GetTimestampQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/GetTimestampQuery.h"

#include "serverDatabase/DatabaseProcess.h"

using namespace DBQuery;

// ======================================================================

GetTimestampQuery::GetTimestampQuery() :
		Query()
{
}

// ======================================================================

void GetTimestampQuery::getSQL(std::string &sql)
{
	if (getProtocol()==DB::PROTOCOL_ODBC)
		sql="select last_save_time from clock";
	else
		sql=std::string("begin :result:=")+DatabaseProcess::getInstance().getSchemaQualifier()+"serverclock.get_last_save_time(); end;";
}

// ======================================================================

bool GetTimestampQuery::bindParameters()
{
	if (getProtocol()!=DB::PROTOCOL_ODBC)
	{
		if (!bindParameter(timestamp)) return false;
	}
	return true;
}

// ======================================================================

bool GetTimestampQuery::bindColumns()
{
	if (getProtocol()==DB::PROTOCOL_ODBC)
	{
		if (!bindCol(timestamp)) return false;
	}
	return true;
}

// ======================================================================
