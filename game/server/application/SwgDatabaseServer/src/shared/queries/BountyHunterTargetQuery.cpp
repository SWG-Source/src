// ======================================================================
//
// BountyHunterTargetQuery.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/BountyHunterTargetQuery.h"

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

namespace BountyHunterTargetQueryNamespace
{
	const int ms_fetchBatchSize = 1000;
}

using namespace BountyHunterTargetQueryNamespace;
using namespace DBQuery;

// ======================================================================

BountyHunterTargetQuery::BountyHunterTargetQuery() :
	m_numItems(0)
{
}

// ----------------------------------------------------------------------

bool BountyHunterTargetQuery::setupData(DB::Session *session)
{
	if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_target_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool BountyHunterTargetQuery::addData(DBSchema::BountyHunterTargetRow const & data)
{
	if (!m_object_ids.push_back(data.object_id.getValueASCII())) return false;
	if (!m_target_ids.push_back(data.target_id.getValueASCII())) return false;

	m_numItems=m_numItems.getValue() + 1;
	return true;
}

// ----------------------------------------------------------------------

int BountyHunterTargetQuery::getNumItems() const
{
	return m_numItems.getValue();
}

// ----------------------------------------------------------------------

void BountyHunterTargetQuery::clearData()
{
	m_object_ids.clear();
	m_target_ids.clear();

	m_numItems=0;
}

// ----------------------------------------------------------------------

void BountyHunterTargetQuery::freeData()
{
	m_object_ids.free();
	m_target_ids.free();
}

// ----------------------------------------------------------------------


bool BountyHunterTargetQuery::bindParameters()
{
	if (!bindParameter(m_object_ids)) return false;
	if (!bindParameter(m_target_ids)) return false;
	if (!bindParameter(m_numItems)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool BountyHunterTargetQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

void BountyHunterTargetQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_bounty_hunter_target (:object_id, :target_id, :chunk_size); end;";		
}

// ======================================================================

BountyHunterTargetQuerySelect::BountyHunterTargetQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

// ----------------------------------------------------------------------

bool BountyHunterTargetQuerySelect::bindParameters ()
{
	return true;
}

// ----------------------------------------------------------------------

bool BountyHunterTargetQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].target_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

const std::vector<DBSchema::BountyHunterTargetRow> & BountyHunterTargetQuerySelect::getData() const
{
	return m_data;
}

// ----------------------------------------------------------------------

void BountyHunterTargetQuerySelect::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"loader.load_bounty_hunter_targets; end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode BountyHunterTargetQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================
