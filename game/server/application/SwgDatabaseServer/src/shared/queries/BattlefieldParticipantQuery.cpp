// ======================================================================
//
// BattlefieldParticipantQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/BattlefieldParticipantQuery.h"

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;

// ======================================================================

BattlefieldParticipantQuery::BattlefieldParticipantQuery() :
		ModeQuery(new DBSchema::BattlefieldParticipantRow) //lint !e1524 // We don't delete the new row (base class does)
{
}

// ----------------------------------------------------------------------

bool BattlefieldParticipantQuery::bindParameters()
{
	DBSchema::BattlefieldParticipantRow *myData=dynamic_cast<DBSchema::BattlefieldParticipantRow*>(data);
	NOT_NULL(myData);

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(myData->faction_id)) return false;

			if (!bindParameter(myData->character_object_id)) return false;
			if (!bindParameter(myData->region_object_id)) return false;
			break;

		case mode_SELECT:
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
	
	return true;
}

bool BattlefieldParticipantQuery::bindColumns()
{
	DBSchema::BattlefieldParticipantRow *myData=dynamic_cast<DBSchema::BattlefieldParticipantRow*>(data);
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

void BattlefieldParticipantQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_battlefield_participants (:faction_id, :character_object_id, :region_object_id); end;";
			break;
			
		case mode_SELECT:
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
}

// ======================================================================

DBQuery::GetAllBattlefieldParticipant::GetAllBattlefieldParticipant(const std::string &schema) :
		Query(),
		m_data(ConfigServerDatabase::getDefaultFetchBatchSize()),
		m_schema(schema)
{
}

bool GetAllBattlefieldParticipant::bindParameters()
{
	return true;
}

bool GetAllBattlefieldParticipant::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());
	
	if (!bindCol(m_data[0].region_object_id)) return false;
	if (!bindCol(m_data[0].character_object_id)) return false;
	if (!bindCol(m_data[0].faction_id)) return false;

	return true;
}

// ----------------------------------------------------------------------

void GetAllBattlefieldParticipant::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"loader.load_battlefield_participants(); end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetAllBattlefieldParticipant::getExecutionMode() const
{
	if (getProtocol()==DB::PROTOCOL_OCI)
		return (MODE_PLSQL_REFCURSOR);
	else
	{
		FATAL(true,("ODBC mode is not supported for BattlefieldParticipant\n"));
		return MODE_SQL; // return to avoid compiler warning
	}
}


// ======================================================================
