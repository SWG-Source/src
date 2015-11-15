// ======================================================================
//
// ServerCommandTable.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerCommandTable.h"

#include "serverGame/ConfigServerGame.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedGame/CommandTable.h"
#include "sharedUtility/DataTable.h"

// ======================================================================

namespace ServerCommandTableNamespace
{
	void getCommandTableListFromDataTable(std::string const &tableListFilename, std::vector<std::string> &results);
}

using namespace ServerCommandTableNamespace;

// ======================================================================

void ServerCommandTableNamespace::getCommandTableListFromDataTable(std::string const &tableListFilename, std::vector<std::string> &results)
{
	Iff iff;
	if (iff.open(tableListFilename.c_str(), true))
	{
		DataTable dataTable;
		dataTable.load(iff);

		int numberOfRows = dataTable.getNumRows();
		for (int row = 0; row < numberOfRows; ++row)
		{
			std::string const tableFilename = dataTable.getStringValue("tableFilename", row);
			if (!tableFilename.empty())
			{
				if (!TreeFile::exists(tableFilename.c_str()))
					WARNING(true, ("'%s' from command table list '%s' is not a valid command table.", tableFilename.c_str(), tableListFilename.c_str()));
				else
					results.push_back(tableFilename);
			}
		}
	}
}

// ----------------------------------------------------------------------

void ServerCommandTable::load()
{
	std::vector<std::string> commandTablesToLoad;

	getCommandTableListFromDataTable("datatables/command/command_tables_shared.iff", commandTablesToLoad);
	getCommandTableListFromDataTable("datatables/command/command_tables_server.iff", commandTablesToLoad);

	if (!strncmp(ConfigServerGame::getSceneID(), "space_", 6))
	{
		getCommandTableListFromDataTable("datatables/command/command_tables_shared_space.iff", commandTablesToLoad);
		getCommandTableListFromDataTable("datatables/command/command_tables_server_space.iff", commandTablesToLoad);
	}
	else
	{
		getCommandTableListFromDataTable("datatables/command/command_tables_shared_ground.iff", commandTablesToLoad);
		getCommandTableListFromDataTable("datatables/command/command_tables_server_ground.iff", commandTablesToLoad);
	}

	CommandTable::loadCommandTables(commandTablesToLoad);
}

// ======================================================================

