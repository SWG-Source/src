// ======================================================================
//
// ManufactureSchematicAttributeQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/ManufactureSchematicAttributeQuery.h"

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;

// ======================================================================

ManufactureSchematicAttributeQuery::ManufactureSchematicAttributeQuery() :
		ModeQuery(new DBSchema::ManufactureSchematicAttributeRow) //lint !e1524 // We don't delete the new row (base class does)
{
}

// ----------------------------------------------------------------------

bool ManufactureSchematicAttributeQuery::bindParameters()
{
	DBSchema::ManufactureSchematicAttributeRow *myData=dynamic_cast<DBSchema::ManufactureSchematicAttributeRow*>(data);
	NOT_NULL(myData);

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(myData->action)) return false;
			if (!bindParameter(myData->object_id)) return false;
			if (!bindParameter(myData->attribute_type)) return false;
			if (!bindParameter(myData->value)) return false;
			break;

		case mode_SELECT:
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	} //lint !e788 //not all modes have an explicit case in the switch
	
	return true;
}

bool ManufactureSchematicAttributeQuery::bindColumns()
{
	DBSchema::ManufactureSchematicAttributeRow *myData=dynamic_cast<DBSchema::ManufactureSchematicAttributeRow*>(data);
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

void ManufactureSchematicAttributeQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_manf_schem_attribute (:action, :object_id, :attribute_type, :value); end;";
			break;
			
		case mode_SELECT:
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	} //lint !e788 //not all modes have an explicit case in the switch
}

// ======================================================================

DBQuery::GetAllManufactureSchematicAttributes::GetAllManufactureSchematicAttributes(const std::string &schema) :
		Query(),
		m_data(ConfigServerDatabase::getDefaultFetchBatchSize()),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

bool GetAllManufactureSchematicAttributes::bindParameters()
{

	return true;
}

bool GetAllManufactureSchematicAttributes::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());
	
	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].attribute_type)) return false;
	if (!bindCol(m_data[0].value)) return false;

	return true;
}

// ----------------------------------------------------------------------

void GetAllManufactureSchematicAttributes::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"loader.load_manf_schematic_attributes (); end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetAllManufactureSchematicAttributes::getExecutionMode() const
{
	return (MODE_PLSQL_REFCURSOR);
}

// ======================================================================
