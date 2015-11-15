// ======================================================================
//
// GetContentsList.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/GetContentsList.h"

#include "sharedFoundation/NetworkId.h"

using namespace DBQuery;

// ======================================================================

GetContentsList::GetContentsList()
{
}

// ----------------------------------------------------------------------

GetContentsList::~GetContentsList()
{
}

// ----------------------------------------------------------------------

void GetContentsList::setContainerId(const NetworkId &id)
{
	containerId=id;
}

// ----------------------------------------------------------------------

NetworkId GetContentsList::getContainedId() const
{
	return containedId.getValue();
}

// ----------------------------------------------------------------------

bool GetContentsList::bindParameters()
{
	if (!bindParameter(containerId)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool GetContentsList::bindColumns()
{
	if (!bindCol(containedId)) return false;
	return true;
}

// ----------------------------------------------------------------------

void GetContentsList::getSQL(std::string &sql)
{
	//	return ("select object_id from objects where contained_by=? and deleted=0 and player_controlled='N'");
	sql="begin :result := loader.get_contents(:object_id); end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetContentsList::getExecutionMode() const
{
	return (MODE_PLSQL_REFCURSOR);
}

// ======================================================================
