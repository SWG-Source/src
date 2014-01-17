// ======================================================================
//
// ResourceTypeQuery.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/ResourceTypeQuery.h"

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

namespace ResourceTypeQueryNamespace
{
	const int ms_fetchBatchSize = 1000;
}

using namespace ResourceTypeQueryNamespace;
using namespace DBQuery;

// ======================================================================

ResourceTypeQuery::ResourceTypeQuery() :
	m_numItems(0)
{
}

// ----------------------------------------------------------------------

bool ResourceTypeQuery::setupData(DB::Session *session)
{
	if (!m_resource_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_resource_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_resource_classes.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_attributes.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_fractal_seeds.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_depleted_timestamps.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
	return true;
}

// ----------------------------------------------------------------------

bool ResourceTypeQuery::addData(DBSchema::ResourceTypeRow const & data)
{
	if (!m_resource_ids.push_back(data.resource_id.getValueASCII())) return false;
	if (!m_resource_names.push_back(data.resource_name.getValueASCII())) return false;
	if (!m_resource_classes.push_back(data.resource_class.getValueASCII())) return false;
	if (!m_attributes.push_back(data.attributes.getValueASCII())) return false;
	if (!m_fractal_seeds.push_back(data.fractal_seeds.getValueASCII())) return false;
	if (!m_depleted_timestamps.push_back(data.depleted_timestamp.isNull(), data.depleted_timestamp.getValue())) return false;

	m_numItems=m_numItems.getValue() + 1;
	return true;
}

// ----------------------------------------------------------------------

int ResourceTypeQuery::getNumItems() const
{
	return m_numItems.getValue();
}

// ----------------------------------------------------------------------

void ResourceTypeQuery::clearData()
{
	m_resource_ids.clear();
	m_resource_names.clear();
	m_resource_classes.clear();
	m_attributes.clear();
	m_fractal_seeds.clear();
	m_depleted_timestamps.clear();

	m_numItems=0;
}

// ----------------------------------------------------------------------

void ResourceTypeQuery::freeData()
{
	m_resource_ids.free();
	m_resource_names.free();
	m_resource_classes.free();
	m_attributes.free();
	m_fractal_seeds.free();
	m_depleted_timestamps.free();
}

// ----------------------------------------------------------------------


bool ResourceTypeQuery::bindParameters()
{
	if (!bindParameter(m_resource_ids)) return false;
	if (!bindParameter(m_resource_names)) return false;
	if (!bindParameter(m_resource_classes)) return false;
	if (!bindParameter(m_attributes)) return false;
	if (!bindParameter(m_fractal_seeds)) return false;
	if (!bindParameter(m_depleted_timestamps)) return false;
	if (!bindParameter(m_numItems)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool ResourceTypeQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

void ResourceTypeQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_resource_type (:resource_id, :resource_name, :resource_class, :attributes, :fractal_seeds, :depleted_timestamp, :chunk_size); end;";		
}

// ======================================================================

ResourceTypeQuerySelect::ResourceTypeQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

// ----------------------------------------------------------------------

bool ResourceTypeQuerySelect::bindParameters ()
{
	return true;
}

// ----------------------------------------------------------------------

bool ResourceTypeQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].resource_id)) return false;
	if (!bindCol(m_data[0].resource_name)) return false;
	if (!bindCol(m_data[0].resource_class)) return false;
	if (!bindCol(m_data[0].attributes)) return false;
	if (!bindCol(m_data[0].fractal_seeds)) return false;
	if (!bindCol(m_data[0].depleted_timestamp)) return false;
	return true;
}

// ----------------------------------------------------------------------

const std::vector<DBSchema::ResourceTypeRow> & ResourceTypeQuerySelect::getData() const
{
	return m_data;
}

// ----------------------------------------------------------------------

void ResourceTypeQuerySelect::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"loader.load_resource_types; end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode ResourceTypeQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================
