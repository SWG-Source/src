// ======================================================================
//
// GetBiographyQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/GetBiographyQuery.h"

#include "serverDatabase/DatabaseProcess.h"

using namespace DBQuery;

// ======================================================================

GetBiographyQuery::GetBiographyQuery(const NetworkId &newOwner) :
		owner(newOwner)
{
}

// ----------------------------------------------------------------------
		
void GetBiographyQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :rc := ") + DatabaseProcess::getInstance().getSchemaQualifier() + "biography.get_biography(:owner); end;";
}

// ----------------------------------------------------------------------

bool GetBiographyQuery::bindParameters()
{
	if (!bindParameter(owner)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool GetBiographyQuery::bindColumns()
{
	if (!bindCol(bio)) return false;
	return true;
}

// ----------------------------------------------------------------------

void GetBiographyQuery::getBio(Unicode::String &buffer) const
{
	bio.getValue(buffer);
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetBiographyQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}
	
// ======================================================================
