// ======================================================================
//
// WaypointQuery.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/WaypointQuery.h"

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;

// ======================================================================

WaypointQuery::WaypointQuery() :
		ModeQuery(new DBSchema::WaypointRow)
{
}

// ----------------------------------------------------------------------

bool WaypointQuery::bindParameters()
{
	DBSchema::WaypointRow *myData=dynamic_cast<DBSchema::WaypointRow*>(data);
	NOT_NULL(myData);

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(myData->object_id)) return false;
			if (!bindParameter(myData->waypoint_id)) return false;
			if (!bindParameter(myData->appearance_name_crc)) return false;
			if (!bindParameter(myData->location_x)) return false;
			if (!bindParameter(myData->location_y)) return false;
			if (!bindParameter(myData->location_z)) return false;
			if (!bindParameter(myData->location_cell)) return false;
			if (!bindParameter(myData->location_scene)) return false;
			if (!bindParameter(myData->name)) return false;
			if (!bindParameter(myData->color)) return false;
			if (!bindParameter(myData->active)) return false;
			if (!bindParameter(myData->detached)) return false;
			break;

		case mode_SELECT:
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
	
	return true;
}

bool WaypointQuery::bindColumns()
{
	DBSchema::WaypointRow *myData=dynamic_cast<DBSchema::WaypointRow*>(data);
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

void WaypointQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_waypoint (:object_id,:waypoint_id,:appearance_name_crc,:location_x,:location_y,:location_z,:location_cell,:location_scene,:name,:color,:active,:detached); end;";
			break;
			
		case mode_SELECT:
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
}

// ======================================================================

DBQuery::GetAllWaypoints::GetAllWaypoints(const std::string &schema) :
		Query(),
		m_data(ConfigServerDatabase::getDefaultFetchBatchSize()),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

bool GetAllWaypoints::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool GetAllWaypoints::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].waypoint_id)) return false;
	if (!bindCol(m_data[0].appearance_name_crc)) return false;
	if (!bindCol(m_data[0].location_x)) return false;
	if (!bindCol(m_data[0].location_y)) return false;
	if (!bindCol(m_data[0].location_z)) return false;
	if (!bindCol(m_data[0].location_cell)) return false;
	if (!bindCol(m_data[0].location_scene)) return false;
	if (!bindCol(m_data[0].name)) return false;
	if (!bindCol(m_data[0].color)) return false;
	if (!bindCol(m_data[0].active)) return false;

	return true;
}

// ----------------------------------------------------------------------

void GetAllWaypoints::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"loader.load_waypoint(); end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetAllWaypoints::getExecutionMode() const
{
	if (getProtocol()==DB::PROTOCOL_OCI)
		return (MODE_PLSQL_REFCURSOR);
	else
	{
		FATAL(true,("ODBC mode is not supported for Waypoint\n"));
		return MODE_SQL; // return to avoid compiler warning
	}
}


// ======================================================================
