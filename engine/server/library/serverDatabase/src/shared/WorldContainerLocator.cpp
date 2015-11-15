// ======================================================================
//
// WorldContainerLocator.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/WorldContainerLocator.h"

#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"
#include <vector>

// ======================================================================

namespace WorldContainerLocatorTablespace
{
	const int ms_maxItemsPerExec = 10000;
}

using namespace WorldContainerLocatorTablespace;

// ======================================================================

WorldContainerLocator::WorldContainerLocator(const std::vector<NetworkId> &containers) :
		ObjectLocator(),
		m_containers(new std::vector<NetworkId>(containers))
{
}

// ----------------------------------------------------------------------

WorldContainerLocator::~WorldContainerLocator()
{
	delete m_containers;
	m_containers=0;
}

// ======================================================================

bool WorldContainerLocator::locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated)
{
	NOT_NULL(session);
	int objectCount (0);
	bool rval = true;
	
	WorldContainerQuery qry(schema);
	if (!qry.setupData(session))
		return false;
	for (std::vector<NetworkId>::const_iterator i=m_containers->begin(); i!=m_containers->end(); ++i)
	{
		if (!qry.addContainer(*i))
			return false;
		if (qry.getNumItems() > ms_maxItemsPerExec)
		{
			rval=qry.exec();
			if (! rval)
				break;
			objectCount += qry.getObjectCount();
			qry.clearData();
		}
	}
	if (! rval)
		return false;

	if (qry.getNumItems() > 0)
	{
		rval = session->exec(&qry);
		objectCount += qry.getObjectCount();
	}
	
	qry.freeData();
	qry.done();

	objectsLocated = objectCount;
	return rval;
}

// ======================================================================

WorldContainerLocator::WorldContainerQuery::WorldContainerQuery(const std::string &schema) :
		num_containers(0),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

void WorldContainerLocator::WorldContainerQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :object_count := ")+m_schema+"loader.locate_by_loadwith_batch(:loadwiths, :chunk_size); end;";
}

// ----------------------------------------------------------------------

bool WorldContainerLocator::WorldContainerQuery::bindParameters()
{
	if (!bindParameter(num_objects_found)) return false;
	if (!bindParameter(container_ids)) return false;
	if (!bindParameter(num_containers)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool WorldContainerLocator::WorldContainerQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

bool WorldContainerLocator::WorldContainerQuery::setupData(DB::Session *session)
{
	return container_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000);
}

// ----------------------------------------------------------------------

void WorldContainerLocator::WorldContainerQuery::clearData()
{
	container_ids.clear();
}

// ----------------------------------------------------------------------

void WorldContainerLocator::WorldContainerQuery::freeData()
{
	container_ids.free();
}

// ----------------------------------------------------------------------

bool WorldContainerLocator::WorldContainerQuery::addContainer(const NetworkId &containerId)
{
	if (!container_ids.push_back(containerId))
		return false;
	num_containers = num_containers.getValue() + 1;
	return true;
}

// ----------------------------------------------------------------------

int WorldContainerLocator::WorldContainerQuery::getNumItems() const
{
	return num_containers.getValue();
}

// ----------------------------------------------------------------------

int WorldContainerLocator::WorldContainerQuery::getObjectCount() const
{
	return num_objects_found.getValue();
}

// ----------------------------------------------------------------------

DB::Query::QueryMode WorldContainerLocator::WorldContainerQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
