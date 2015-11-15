// ======================================================================
//
// ServerCommandPermissionManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// =====================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerCommandPermissionManager.h"

#include "serverGame/Client.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedCommandParser/CommandParser.h"
#include "sharedLog/Log.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "UnicodeUtils.h"

ServerCommandPermissionManager::ServerCommandPermissionManager() :
		CommandPermissionManager(),
		m_permissionTable(0)
{
	m_permissionTable = DataTableManager::getTable("datatables/admin/command_permissions.iff", true);
	DEBUG_FATAL(!m_permissionTable, ("Could not open command permissions table"));
	CommandParser::setPermissionManager(this);
}

//------------------------------------------------------------------------------------------

ServerCommandPermissionManager::~ServerCommandPermissionManager()
{
	DataTableManager::close("command_permissions.iff");
	CommandParser::setPermissionManager(0);
}


//------------------------------------------------------------------------------------------

bool ServerCommandPermissionManager::isCommandAllowed (const NetworkId & userId, const Unicode::String & commandPath) const
{
	// commands sent from the ServerConsole program don't have a client associated from them
	// although it is possible to  resolve to a ServerObject
	std::string cmd = Unicode::wideToNarrow(commandPath);
	if( cmd == "game" )
	{
		ServerObject * tmpu = ServerWorld::findObjectByNetworkId(userId);
		// a ServerConsole command sometimes won't have a ServerObject (first time a cluster receives a command)
		// a ServerConsole command will never have a Client associated with it
		if( !tmpu || !tmpu->getClient())
		{
			LOG("ServerCommandPermissionManager", ("Allowing permission to execute ServerConsole command.") );
			return true;
		}
		else
		{
			LOG("ServerCommandPermissionManager", ("Disallowing permission to execute ServerConsole command because it has a Client associated with it."));
			return false;
		}
	}

	ServerObject * user = ServerWorld::findObjectByNetworkId(userId);
	if (!user)
		return false;

	Client* client = user->getClient();
	if (!client)
		return false;

	int clientLevel = client->getGodLevel();
	std::string command = Unicode::wideToNarrow(commandPath);
	int row = m_permissionTable->searchColumnString( 0, command);
	int commandLevel = 5;
	
	if (row != -1)
		commandLevel = m_permissionTable->getIntValue(1, row);

	bool retval =  (commandLevel <= clientLevel);
	if (!retval)
	{
		LOG("CustomerService",("Avatar:%s denied command %s because the command level is %d and they are %d", PlayerObject::getAccountDescription(userId).c_str(), command.c_str(), commandLevel, clientLevel));
	}
	return retval;
}


//------------------------------------------------------------------------------------------
