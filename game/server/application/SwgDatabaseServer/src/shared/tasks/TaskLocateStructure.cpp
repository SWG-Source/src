// ======================================================================
//
// TaskLocateStructure.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskLocateStructure.h"

#include "SwgLoader.h"
#include "Unicode.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverNetworkMessages/LocateStructureMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

TaskLocateStructure::TaskLocateStructure(const NetworkId &itemId, const std::string &whoRequested) :
	TaskRequest(),
	m_itemId(itemId),
	m_whoRequested(whoRequested),
	m_x(0),
	m_z(0),
	m_found(false)
{
}

//-----------------------------------------------------------------------

TaskLocateStructure::~TaskLocateStructure()
{
}

//-----------------------------------------------------------------------

bool TaskLocateStructure::process(DB::Session *session)
{
	LocateStructureQuery query;
	query.item_id = m_itemId;

	if (!(session->exec(&query)))
		return false;
	query.done();

	m_found = query.found.getValue();
	if (m_found)
	{
		m_x = query.x.getValue();
		m_z = query.z.getValue();
		m_sceneId = query.sceneId.getValueASCII();
	}

	LOG("CustomerService", ("playerStructure: Locate structure (%s): result %s", m_itemId.getValueString().c_str(), (m_found ? "found" : "not found")));
	return true;
}

//-----------------------------------------------------------------------

void TaskLocateStructure::onComplete()
{
	std::string message;
	if (m_found)
	{
		LocateStructureMessage lsm(m_itemId, m_x, m_z, m_sceneId, m_whoRequested);
		DatabaseProcess::getInstance().sendToAnyGameServer(lsm);
	}
	else
	{
		message = "Could not find object " + m_itemId.getValueString() + " in database";
		GenericValueTypeMessage<std::pair<std::string, std::string> > reply("DatabaseConsoleReplyMessage",
			std::make_pair(m_whoRequested, message));
		DatabaseProcess::getInstance().sendToAnyGameServer(reply);
	}
}

// ======================================================================

void TaskLocateStructure::LocateStructureQuery::getSQL(std::string &sql)
{
	sql = std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() + "loader.locate_structure(:item_id, :x, :z, :scene_id, :found); end;";
}

// ----------------------------------------------------------------------

bool TaskLocateStructure::LocateStructureQuery::bindParameters()
{
	if (!bindParameter(item_id)) return false;
	if (!bindParameter(x)) return false;
	if (!bindParameter(z)) return false;
	if (!bindParameter(sceneId)) return false;
	if (!bindParameter(found)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool TaskLocateStructure::LocateStructureQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskLocateStructure::LocateStructureQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

TaskLocateStructure::LocateStructureQuery::LocateStructureQuery()
{
}

// ======================================================================