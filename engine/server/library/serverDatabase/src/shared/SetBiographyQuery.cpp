// ======================================================================
//
// SetBiographyQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/SetBiographyQuery.h"

using namespace DBQuery;

#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

SetBiographyQuery::SetBiographyQuery(const NetworkId &newOwner, const Unicode::String &newBio) :
		owner(newOwner),
		bio(newBio)
{
}

// ----------------------------------------------------------------------
		
void SetBiographyQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() + "biography.set_biography(:owner, :bio); end;";
}

// ----------------------------------------------------------------------

bool SetBiographyQuery::bindParameters()
{
	if (!bindParameter(owner)) return false;
	if (!bindParameter(bio)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool SetBiographyQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode SetBiographyQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}
	
// ======================================================================
