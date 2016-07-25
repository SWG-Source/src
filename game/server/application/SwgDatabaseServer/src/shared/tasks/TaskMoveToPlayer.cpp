// ======================================================================
//
// TaskMoveToPlayer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskMoveToPlayer.h"

#include "SwgLoader.h"
#include "Unicode.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

TaskMoveToPlayer::TaskMoveToPlayer(const NetworkId &oid, const NetworkId &player, const std::string &whoRequested) :
	TaskRequest(),
	m_oid(oid),
	m_player(player),
	m_whoRequested(whoRequested),
	m_result(0)
{
}

//-----------------------------------------------------------------------

TaskMoveToPlayer::~TaskMoveToPlayer()
{
}

//-----------------------------------------------------------------------

bool TaskMoveToPlayer::process(DB::Session *session)
{
	MoveToPlayerQuery query;
	query.oid = m_oid;
	query.player = m_player;

	if (!(session->exec(&query)))
		return false;
	query.done();

	m_result = query.result.getValue();

	LOG("DatabaseRestore", ("Move Item (%s) to Player (%s): result %i", m_oid.getValueString().c_str(), m_player.getValueString().c_str(), m_result));
	return true;
}

//-----------------------------------------------------------------------

void TaskMoveToPlayer::onComplete()
{
	std::string message;
	switch (m_result)
	{
	case 1:
		message = "Item moved.  (It may not reappear until the next server restart.)";
		break;

	case 2:
		message = "Could not find player's inventory";
		break;

	case 3:
		message = "Object id does not exist";
		break;

	case 4:
		message = "The database returned an unknown error code";
		break;

	case 5:
		message = "Object id cannot be a player";
		break;

	case 6:
		message = "Object id cannot be object/tangible/inventory/character_inventory.iff";
		break;

	case 7:
		message = "Object id cannot be object/tangible/mission_bag/mission_bag.iff";
		break;

	case 8:
		message = "Object id cannot be object/tangible/datapad/character_datapad.iff";
		break;

	case 9:
		message = "Object id cannot be object/tangible/bank/character_bank.iff";
		break;

	case 10:
		message = "Object id cannot be object/weapon/melee/unarmed/unarmed_default_player.iff";
		break;

	case 11:
		message = "Object id cannot be object/player/player.iff";
		break;

	case 12:
		message = "Object id cannot be object/cell/cell.iff";
		break;

	case 13:
		message = "Object id cannot be object/tangible/inventory/vendor_inventory.iff";
		break;

	case 14:
		message = "Object cannot be contained directly by a datapad";
		break;

	case 15:
		message = "Object cannot be a building";
		break;

	case 16:
		message = "Object cannot be an installation";
		break;

	case 17:
		message = "Object cannot be a ship";
		break;

	default:
		message = "admin.move_item_to_player() returned an unknown error code";
		break;
	}

	GenericValueTypeMessage<std::pair<std::string, std::string> > reply("DatabaseConsoleReplyMessage",
		std::make_pair(m_whoRequested, message));
	DatabaseProcess::getInstance().sendToAnyGameServer(reply);
}

// ======================================================================

void TaskMoveToPlayer::MoveToPlayerQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ") + DatabaseProcess::getInstance().getSchemaQualifier() + "admin.move_item_to_player (:item_id, :player_id); end;";
}

// ----------------------------------------------------------------------

bool TaskMoveToPlayer::MoveToPlayerQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(oid)) return false;
	if (!bindParameter(player)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool TaskMoveToPlayer::MoveToPlayerQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskMoveToPlayer::MoveToPlayerQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

TaskMoveToPlayer::MoveToPlayerQuery::MoveToPlayerQuery()
{
}

// ======================================================================