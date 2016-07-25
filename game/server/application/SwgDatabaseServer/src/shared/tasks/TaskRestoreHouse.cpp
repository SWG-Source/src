// ======================================================================
//
// TaskRestoreHouse.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskRestoreHouse.h"

#include "SwgLoader.h"
#include "Unicode.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

TaskRestoreHouse::TaskRestoreHouse(const NetworkId &houseId, const std::string &whoRequested) :
	TaskRequest(),
	m_houseId(houseId),
	m_whoRequested(whoRequested),
	m_result(0)
{
}

//-----------------------------------------------------------------------

TaskRestoreHouse::~TaskRestoreHouse()
{
}

//-----------------------------------------------------------------------

bool TaskRestoreHouse::process(DB::Session *session)
{
	RestoreHouseQuery query;
	query.house_id = m_houseId;

	if (!(session->exec(&query)))
		return false;
	query.done();

	m_result = query.result.getValue();

	LOG("DatabaseRestore", ("Restore house (%s): result %i", m_houseId.getValueString().c_str(), m_result));
	return true;
}

//-----------------------------------------------------------------------

void TaskRestoreHouse::onComplete()
{
	std::string message;
	switch (m_result)
	{
	case 1:
		message = "House restored.  (It may not reappear until the next server restart.)";
		break;

	case 2:
		message = "Object id was incorrect or house was not deleted";
		break;

	case 3:
		message = "There was an error in the database while attempting to restore the house.";
		break;

	default:
		message = "The database returned an unknown code in response to the request to restore the house.";
		break;
	}

	GenericValueTypeMessage<std::pair<std::string, std::string> > reply("DatabaseConsoleReplyMessage",
		std::make_pair(m_whoRequested, message));
	DatabaseProcess::getInstance().sendToAnyGameServer(reply);
}

// ======================================================================

void TaskRestoreHouse::RestoreHouseQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ") + DatabaseProcess::getInstance().getSchemaQualifier() + "admin.restore_house (:house_id); end;";
}

// ----------------------------------------------------------------------

bool TaskRestoreHouse::RestoreHouseQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(house_id)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool TaskRestoreHouse::RestoreHouseQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskRestoreHouse::RestoreHouseQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

TaskRestoreHouse::RestoreHouseQuery::RestoreHouseQuery()
{
}

// ======================================================================