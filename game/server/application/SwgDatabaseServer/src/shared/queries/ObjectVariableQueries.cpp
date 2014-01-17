// ======================================================================
//
// ObjectVariableQueries.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "ObjectVariableQueries.h"

#include "serverDatabase/DatabaseProcess.h"

using namespace DBSchema;
using namespace DBQuery;

// ======================================================================

namespace ObjectVariableQueriesNamespace
{
	static const size_t ms_fetchBatchSize=1000;
}
using namespace ObjectVariableQueriesNamespace;

// ======================================================================

GenericObjectVariableQuery::GenericObjectVariableQuery() :
		DatabaseProcessQuery(new ObjectVariableRow),
		m_numItems(0)
{
}

// ----------------------------------------------------------------------

bool GenericObjectVariableQuery::setupData(DB::Session *session)
{
	if (!m_objectIds.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_nameIds.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
	if (!m_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
	if (!m_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool GenericObjectVariableQuery::addData(const NetworkId &objectId, int nameId, int typeId, const std::string &value)
{
	if (!m_objectIds.push_back(objectId.getValueString())) return false;
	if (!m_nameIds.push_back(nameId)) return false;
	if (!m_types.push_back(typeId)) return false;
	if (!m_values.push_back(value)) return false;

	m_numItems=m_numItems.getValue() + 1;
	return true;
}

// ----------------------------------------------------------------------

int GenericObjectVariableQuery::getNumItems() const
{
	return m_numItems.getValue();
}

// ----------------------------------------------------------------------

void GenericObjectVariableQuery::clearData()
{
	m_objectIds.clear();
	m_nameIds.clear();
	m_types.clear();
	m_values.clear();

	m_numItems=0;
}

// ----------------------------------------------------------------------

void GenericObjectVariableQuery::freeData()
{
	m_objectIds.free();
	m_nameIds.free();
	m_types.free();
	m_values.free();
}

// ----------------------------------------------------------------------

bool GenericObjectVariableQuery::bindParameters()
{
	if (!bindParameter(m_objectIds)) return false;
	if (!bindParameter(m_nameIds)) return false;
	if (!bindParameter(m_types)) return false;
	if (!bindParameter(m_values)) return false;
	if (!bindParameter(m_numItems)) return false;
	
	return true;
}

// ----------------------------------------------------------------------

bool GenericObjectVariableQuery::bindColumns()
{
	return true;
}

// ======================================================================

AddObjectVariableQuery::AddObjectVariableQuery()
{
}

// ----------------------------------------------------------------------

void AddObjectVariableQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_object_variable_batch (:object_id, :name_id, :type, :value, :count); end;";
}

// ======================================================================

UpdateObjectVariableQuery::UpdateObjectVariableQuery()
{
}

// ----------------------------------------------------------------------

void UpdateObjectVariableQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.update_object_variable_batch (:object_id, :name_id, :type, :value, :count); end;";
}

// ======================================================================

RemoveObjectVariableQuery::RemoveObjectVariableQuery() :
		DatabaseProcessQuery(new ObjectVariableRow),
		m_numItems(0)
{
}

// ----------------------------------------------------------------------

bool RemoveObjectVariableQuery::setupData(DB::Session *session)
{
	if (!m_objectIds.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_nameIds.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
	return true;
}

// ----------------------------------------------------------------------

bool RemoveObjectVariableQuery::addData(const NetworkId &objectId, int nameId)
{
	if (!m_objectIds.push_back(objectId.getValueString())) return false;
	if (!m_nameIds.push_back(nameId)) return false;
	m_numItems = m_numItems.getValue() + 1;
	return true;
}

// ----------------------------------------------------------------------

void RemoveObjectVariableQuery::clearData()
{
	m_objectIds.clear();
	m_nameIds.clear();
	m_numItems=0;
}

// ----------------------------------------------------------------------

void RemoveObjectVariableQuery::freeData()
{
	m_objectIds.free();
	m_nameIds.free();
}

// ----------------------------------------------------------------------

int RemoveObjectVariableQuery::getNumItems() const
{
	return m_numItems.getValue();
}
	
// ----------------------------------------------------------------------

bool RemoveObjectVariableQuery::bindParameters()
{
	if (!bindParameter(m_objectIds)) return false;
	if (!bindParameter(m_nameIds)) return false;
	if (!bindParameter(m_numItems)) return false;
	
	return true;
}

// ----------------------------------------------------------------------

bool RemoveObjectVariableQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

void RemoveObjectVariableQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_object_variable_batch (:object_id, :name_id, :count); end;";
}

// ======================================================================

GetAllObjectVariables::GetAllObjectVariables(const std::string &schema) :
		m_data(ms_fetchBatchSize),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

bool GetAllObjectVariables::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool GetAllObjectVariables::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));

	setColArrayMode(skipSize, ms_fetchBatchSize);
	
	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].name_id)) return false;
	if (!bindCol(m_data[0].type)) return false;
	if (!bindCol(m_data[0].value)) return false;
	return true;
}

// ----------------------------------------------------------------------

void GetAllObjectVariables::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"loader.load_object_variables; end;";
	//sql=std::string("select /*+ ORDERED USE_NL(T)*/ t.object_id, t.name_id, t.type, t.value from object_list l,object_variables t where l.object_id = t.object_id and nvl(t.detached,0) = 0");
}

// ----------------------------------------------------------------------

const std::vector<DBSchema::ObjectVariableRow> &GetAllObjectVariables::getData() const
{
	return m_data;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetAllObjectVariables::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
	//return (MODE_SQL);
}

// ======================================================================

GetObjectVariableOverrides::GetObjectVariableOverrides(const std::string &schema, const std::string &goldSchema) :
		GetAllObjectVariables(schema),
		gold_schema(goldSchema)
{
}

// ----------------------------------------------------------------------

void GetObjectVariableOverrides::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"gold_override.load_objvar_overrides(:gold_schema); end;";
}

// ----------------------------------------------------------------------

bool GetObjectVariableOverrides::bindParameters()
{
	if (!bindParameter(gold_schema)) return false;
	return true;
}

// ======================================================================

