// ======================================================================
//
// ExperienceQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/ExperienceQuery.h"

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;

// ======================================================================

ExperienceQuery::ExperienceQuery() :
		ModeQuery(new DBSchema::ExperiencePointRow) //lint !e1524 // We don't delete the new row (base class does)
{
}

// ----------------------------------------------------------------------

bool ExperienceQuery::bindParameters()
{
	DBSchema::ExperiencePointRow *myData=dynamic_cast<DBSchema::ExperiencePointRow*>(data);
	NOT_NULL(myData);

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(myData->points)) return false;

			if (!bindParameter(myData->experience_type)) return false;
			if (!bindParameter(myData->object_id)) return false;
			break;

		case mode_SELECT:
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
	
	return true;
}

bool ExperienceQuery::bindColumns()
{
	DBSchema::ExperiencePointRow *myData=dynamic_cast<DBSchema::ExperiencePointRow*>(data);
	NOT_NULL(myData);

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			break;

		case mode_SELECT:
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	} //lint !e788 //not all modes have an explicit case in the switch
	
	return true;
}

// ----------------------------------------------------------------------

void ExperienceQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_experience (:points, :experience_type, :object_id); end;";
			break;
			
		case mode_SELECT:
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
}

// ======================================================================

DBQuery::GetAllExperience::GetAllExperience(const std::string &schema) :
		Query(),
		m_data(ConfigServerDatabase::getDefaultFetchBatchSize()),
		m_schema(schema)
{
}

bool GetAllExperience::bindParameters()
{
	return true;
}

bool GetAllExperience::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].experience_type)) return false;
	if (!bindCol(m_data[0].points)) return false;

	return true;
}

// ----------------------------------------------------------------------

void GetAllExperience::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"loader.load_experience(); end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetAllExperience::getExecutionMode() const
{
	if (getProtocol()==DB::PROTOCOL_OCI)
		return (MODE_PLSQL_REFCURSOR);
	else
	{
		FATAL(true,("ODBC mode is not supported for Experience\n"));
		return MODE_SQL; // return to avoid compiler warning
	}
}


// ======================================================================
