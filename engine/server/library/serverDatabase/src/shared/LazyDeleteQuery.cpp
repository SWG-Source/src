// ======================================================================
//
// LazyDeleteQuery.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/LazyDeleteQuery.h"

#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/ConfigServerDatabase.h"

// ======================================================================

bool LazyDeleteQuery::setupData(DB::Session *session)
{
	return m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000);

}

// ----------------------------------------------------------------------

bool LazyDeleteQuery::addData(const NetworkId &object)
{
	if (!m_object_ids.push_back(object.getValueString()))
		return false;

	m_numItems=m_numItems.getValue() + 1;

	return true;
}

// ----------------------------------------------------------------------

int LazyDeleteQuery::getNumItems() const
{
	return m_numItems.getValue();
}

// ----------------------------------------------------------------------

void LazyDeleteQuery::clearData()
{
	m_object_ids.clear();
	m_numItems=0;

}

// ----------------------------------------------------------------------

void LazyDeleteQuery::freeData()
{
	m_object_ids.free();
}

// ----------------------------------------------------------------------

void LazyDeleteQuery::getSQL(std::string &sql)
{
	sql = std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"lazy_deleter.purge_objects_bulk(:object_id, :chunk_size, :enable_db_logging ); end;";
}

// ----------------------------------------------------------------------

bool LazyDeleteQuery::bindParameters()
{

	if (!bindParameter(m_object_ids)) return false;
	if (!bindParameter(m_numItems)) return false;
	if (!bindParameter(m_enableDatabaseLogging)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool LazyDeleteQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

LazyDeleteQuery::LazyDeleteQuery() :
		m_object_ids(),
		m_numItems(0),
		m_enableDatabaseLogging(ConfigServerDatabase::getEnableDatabaseErrorLogging())
{
}

// ======================================================================
