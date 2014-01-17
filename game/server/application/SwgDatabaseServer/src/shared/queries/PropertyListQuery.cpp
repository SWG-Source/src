// ======================================================================
//
// PropertyListQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/PropertyListQuery.h"

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;

// ======================================================================

PropertyListQuery::PropertyListQuery() :
		ModeQuery(new DBSchema::PropertyListQueryData), //lint !e1524 // We don't delete the new row (base class does)
		m_operations(-1), // init to -1 indicating an operation type has not yet been set for this query
		m_numItems(0),
		m_enableDatabaseLogging(ConfigServerDatabase::getEnableDatabaseErrorLogging())
{
}

// ----------------------------------------------------------------------

bool PropertyListQuery::setupData(DB::Session *session)
{
	if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_list_ids.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
	if (!m_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool PropertyListQuery::addData(const NetworkId &networkId, int listId, const std::string &value, int operation)
{
	if (!m_object_ids.push_back(networkId.getValueString())) return false;
	if (!m_list_ids.push_back(listId)) return false;
	if (!m_values.push_back(value)) return false;
 
	if (m_operations.getValue()  == -1) // if operation type for this query has not been initialized then do so
		m_operations = operation;

	m_numItems=m_numItems.getValue() + 1;
	return true;
}

// ----------------------------------------------------------------------

int PropertyListQuery::getNumItems() const
{
	return m_numItems.getValue();
}

// ----------------------------------------------------------------------

void PropertyListQuery::clearData()
{
	m_object_ids.clear();
	m_list_ids.clear();
	m_values.clear();

	m_numItems=0;
	m_operations= -1; // reset operation type to uninitialized in case someone tries to reuse same query for different operations
}

// ----------------------------------------------------------------------

void PropertyListQuery::freeData()
{
	m_object_ids.free();
	m_list_ids.free();
	m_values.free();
}

// ----------------------------------------------------------------------

bool PropertyListQuery::bindParameters()
{
	if (!bindParameter(m_object_ids)) return false;
	if (!bindParameter(m_list_ids)) return false;
	if (!bindParameter(m_operations)) return false;
	if (!bindParameter(m_values)) return false;
	if (!bindParameter(m_numItems)) return false;
	if (!bindParameter(m_enableDatabaseLogging)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool PropertyListQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

void PropertyListQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.update_property_list_batch (:object_id, :list_id, :operation, :value, :chunk_size, :enable_db_logging); end;";
}

// ======================================================================

DBQuery::GetAllPropertyLists::GetAllPropertyLists(const std::string &schema) :
		Query(),
		m_data(ConfigServerDatabase::getDefaultFetchBatchSize()),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

bool GetAllPropertyLists::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool GetAllPropertyLists::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].list_id)) return false;
	if (!bindCol(m_data[0].value)) return false;
	return true;
}

// ----------------------------------------------------------------------

void GetAllPropertyLists::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"loader.load_property_lists (); end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetAllPropertyLists::getExecutionMode() const
{
	DEBUG_FATAL (getProtocol()!=DB::PROTOCOL_OCI,("PropertyList persistance is not supported for PostgreSQL.\n"));
	return (MODE_PLSQL_REFCURSOR);
}

// ======================================================================

