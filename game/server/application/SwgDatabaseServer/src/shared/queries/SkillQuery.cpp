// ======================================================================
//
// SkillQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/SkillQuery.h"

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;

// ======================================================================

SkillQuery::SkillQuery() :
		ModeQuery(new DBSchema::SkillQueryData) //lint !e1524 // We don't delete the new row (base class does)
{
}

// ----------------------------------------------------------------------

bool SkillQuery::bindParameters()
{
	DBSchema::SkillQueryData *myData=dynamic_cast<DBSchema::SkillQueryData*>(data);
	NOT_NULL(myData);

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(myData->object_id)) return false;
			if (!bindParameter(myData->index)) return false;
			if (!bindParameter(myData->operation)) return false;
			if (!bindParameter(myData->skill)) return false;
			break;

		case mode_SELECT:
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
	
	return true;
}

bool SkillQuery::bindColumns()
{
	DBSchema::SkillQueryData *myData=dynamic_cast<DBSchema::SkillQueryData*>(data);
	NOT_NULL(myData);

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:

			break;

		case mode_SELECT:
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
	
	return true;
}

// ----------------------------------------------------------------------

void SkillQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.update_skill (:object_id, :index, :operation, :skill); end;";
			break;
			
		case mode_SELECT:
		default:
			FATAL(true,("Bad query mode."));
	}
}

// ----------------------------------------------------------------------

bool GetAllSkills::bindParameters()
{
	return true;
}

bool GetAllSkills::bindColumns()
{
	if (!bindCol(m_data.object_id)) return false;
	if (!bindCol(m_data.index)) return false;
	if (!bindCol(m_data.skill)) return false;
	return true;
}

// ----------------------------------------------------------------------

void GetAllSkills::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+DatabaseProcess::getInstance().getSchemaQualifier()+"loader.load_skills (); end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetAllSkills::getExecutionMode() const
{
	DEBUG_FATAL (getProtocol()!=DB::PROTOCOL_OCI,("Skill persistance is not supported for PostgreSQL.\n"));
	return (MODE_PLSQL_REFCURSOR);
}


// ======================================================================
