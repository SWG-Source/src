// ======================================================================
//
// TaskUndeleteItem.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskUndeleteItem.h"

#include "SwgLoader.h"
#include "Unicode.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

TaskUndeleteItem::TaskUndeleteItem(const NetworkId &itemId, const std::string &whoRequested) :
	TaskRequest(),
	m_itemId(itemId),
	m_whoRequested(whoRequested),
	m_result(0)
{
}

//-----------------------------------------------------------------------

TaskUndeleteItem::~TaskUndeleteItem()
{
}

//-----------------------------------------------------------------------

bool TaskUndeleteItem::process(DB::Session *session)
{
	UndeleteItemQuery query;
	query.item_id = m_itemId;

	if (!(session->exec(&query)))
		return false;
	query.done();

	m_result = query.result.getValue();

	LOG("DatabaseRestore", ("Undelete Item (%s): result %i", m_itemId.getValueString().c_str(), m_result));
	return true;
}

//-----------------------------------------------------------------------

void TaskUndeleteItem::onComplete()
{
	std::string message;
	switch (m_result)
	{
	case 1:
		message = "Item restored.";
		break;

	case 2:
		message = "Object id was incorrect or item was not deleted";
		break;

	case 3:
		message = "There was an error in the database while attempting to undelete the item.";
		break;
	case 4:
		message = "Item restored.  Loading this item from database...";
		Loader::getInstance().locateStructure(m_itemId, m_whoRequested);
		break;

	default:
		message = "The database returned an unknown code in response to the request to undelete the item.";
		break;
	}

	GenericValueTypeMessage<std::pair<std::string, std::string> > reply("DatabaseConsoleReplyMessage",
		std::make_pair(m_whoRequested, message));
	DatabaseProcess::getInstance().sendToAnyGameServer(reply);
}

// ======================================================================

void TaskUndeleteItem::UndeleteItemQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ") + DatabaseProcess::getInstance().getSchemaQualifier() + "admin.undelete_item (:item_id); end;";
}

// ----------------------------------------------------------------------

bool TaskUndeleteItem::UndeleteItemQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(item_id)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool TaskUndeleteItem::UndeleteItemQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskUndeleteItem::UndeleteItemQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

TaskUndeleteItem::UndeleteItemQuery::UndeleteItemQuery()
{
}

// ======================================================================