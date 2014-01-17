// ======================================================================
//
// ContainedObjectLocator.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/ContainedObjectLocator.h"

#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverNetworkMessages/LoadContainedObjectMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

ContainedObjectLocator::ContainedObjectLocator(const NetworkId &containerId, const NetworkId &objectId) :
		ObjectLocator(),
		m_containerId(containerId),
		m_objectId(objectId)
{
}

// ----------------------------------------------------------------------

ContainedObjectLocator::~ContainedObjectLocator()
{
}

// ----------------------------------------------------------------------

bool ContainedObjectLocator::locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated)
{
	NOT_NULL(session);

	LocateObjectQuery qry(m_containerId, m_objectId, schema);
	bool rval = session->exec(&qry);
	qry.done();

	objectsLocated = 1; //TODO:  counts
	return rval;
}

// ======================================================================

ContainedObjectLocator::LocateObjectQuery::LocateObjectQuery(const NetworkId &containerId, const NetworkId &objectId, const std::string &schema) :
		container_id(containerId),
		object_id(objectId),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

void ContainedObjectLocator::LocateObjectQuery::getSQL(std::string &sql)
{
	LOG("AuctionRetrieval", ("Loader:: calling LocateObjectQuery::getSQL() for loading object %s for retrieval", object_id.getValue().getValueString().c_str()));
	sql = std::string("begin ") + m_schema + "loader.locate_contained_object (:container_id, :object_id); end;";
}

// ----------------------------------------------------------------------

bool ContainedObjectLocator::LocateObjectQuery::bindParameters()
{
	if (!bindParameter(container_id)) return false;
	if (!bindParameter(object_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool ContainedObjectLocator::LocateObjectQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode ContainedObjectLocator::LocateObjectQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

void ContainedObjectLocator::sendPostBaselinesCustomData (GameServerConnection &conn) const
{
	LOG("AuctionRetrieval", ("Loader:: sending post baselines data for loading object %s for retrieval", m_objectId.getValueString().c_str()));
	LoadContainedObjectMessage msg(m_containerId, m_objectId);
	conn.send(msg,true);
}

// ======================================================================
