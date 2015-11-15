// ======================================================================
//
// ContentsLocator.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/ContentsLocator.h"

#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverNetworkMessages/LoadContentsMessage.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

ContentsLocator::ContentsLocator(const NetworkId &containerId) :
		ObjectLocator(),
		m_containerId(containerId)
{
}

// ----------------------------------------------------------------------

ContentsLocator::~ContentsLocator()
{
}

// ----------------------------------------------------------------------

bool ContentsLocator::locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated)
{
	NOT_NULL(session);

	LocateObjectQuery qry(m_containerId, schema);
	bool rval = session->exec(&qry);
	qry.done();

	objectsLocated = 1; //TODO:  counts
	return rval;
}

// ======================================================================

ContentsLocator::LocateObjectQuery::LocateObjectQuery(const NetworkId &containerId, const std::string &schema) :
		container_id(containerId),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

void ContentsLocator::LocateObjectQuery::getSQL(std::string &sql)
{
	sql = std::string("begin ") + m_schema + "loader.locate_contents (:container_id); end;";
}

// ----------------------------------------------------------------------

bool ContentsLocator::LocateObjectQuery::bindParameters()
{
	if (!bindParameter(container_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool ContentsLocator::LocateObjectQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode ContentsLocator::LocateObjectQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

void ContentsLocator::sendPostBaselinesCustomData (GameServerConnection &conn) const
{
	LoadContentsMessage msg(m_containerId);
	conn.send(msg,true);
}

// ======================================================================
