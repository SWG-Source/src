// CentralCommandParserGame.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/CentralCommandParserGame.h"

#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverNetworkMessages/ReloadDatatableMessage.h"
#include "serverNetworkMessages/ReloadScriptMessage.h"
#include "serverNetworkMessages/ReloadTemplateMessage.h"
#include "serverNetworkMessages/SetConnectionServerPublic.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFile/TreeFile.h"
#include "sharedLog/Log.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/DataTable.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

CentralCommandParserGame::CentralCommandParserGame() :
CommandParser ("game", 0, "...", "game commands from the central server", 0)
{
}

//-----------------------------------------------------------------------

CentralCommandParserGame::~CentralCommandParserGame()
{
}

//-----------------------------------------------------------------------

bool CentralCommandParserGame::performParsing (const NetworkId &, const StringVector_t & argv, const String_t & originalMessage, String_t & result, const CommandParser *)
{
	LOG("ServerConsole", ("Parsing ServerConsole command."));
	bool retval = false;
	if(! argv.empty())
	{
		std::string cmd = Unicode::wideToNarrow(argv[2]);
		LOG("ServerConsole", ("Attempting to execute command '%s'.", cmd.c_str()) );
		if(cmd == "runScript")
		{
			// build arguments
			if(argv.size() > 4)
			{
				// get script name
				std::string scriptName = Unicode::wideToNarrow(argv[3]);
				std::string methodName = Unicode::wideToNarrow(argv[4]);

				// determine params
				Unicode::String strParam = originalMessage.substr(originalMessage.find(argv[4]) + argv[4].length() + 1);
				ScriptParams params;
				params.addParam(strParam, "strParam");
				std::string scriptResult = GameScriptObject::callScriptConsoleHandler(scriptName, methodName, "u", params);
				result += Unicode::narrowToWide(scriptResult);
				retval = true;
			}
		}
		else if(cmd == "systemMessage")
		{
			std::string targetId = "SWG." + GameServer::getInstance().getClusterName() + ".SYSTEM";
			static const Unicode::String oob;
			Unicode::String output = originalMessage.substr(originalMessage.find(argv[2]) + argv[2].length() + 1);
//			LOG("ServerConsole", ("Attempting to send system message."));
			Chat::sendToRoom("SYSTEM", targetId, output, oob);
			result += Unicode::narrowToWide("message \"");
			result += output;
			result += Unicode::narrowToWide("\" sent to chat system\n");
			retval = true;
		}
		// reloads a script on all game servers
		else if(cmd == "reloadScript")
		{
			std::string scriptName = Unicode::wideToNarrow(argv[3]);
			if( GameScriptObject::reloadScript(scriptName) )
			{
				LOG("ServerConsole", ("Successfully reloaded script '%s'.", scriptName.c_str()));
				result += Unicode::narrowToWide("reloadScript '");
				result += Unicode::narrowToWide(scriptName);
				result += Unicode::narrowToWide("' succeeded\n");
				retval = true;
			}
			else
			{
				LOG("ServerConsole", ("Failed to reload script '%s'.", scriptName.c_str()));
				result += Unicode::narrowToWide("reloadScript '");
				result += Unicode::narrowToWide(scriptName);
				result += Unicode::narrowToWide("' failed\n");
				retval = false;
			}

			ServerMessageForwarding::beginBroadcast();

			ReloadScriptMessage const reloadScriptMessage(scriptName);
			ServerMessageForwarding::send(reloadScriptMessage);

			ServerMessageForwarding::end();
		}
		else if( cmd == "reloadServerTemplate" )
		{
			std::string templateName = Unicode::wideToNarrow(argv[3]);
			Iff templateFile;
			if( !templateFile.open(templateName.c_str(), true))
			{
				result += Unicode::narrowToWide("Template '");
				result += Unicode::narrowToWide(templateName);
				result += Unicode::narrowToWide("' does not exist.\n");
				retval = false;
			}
			else
			{
				if( !ObjectTemplateList::isLoaded(templateName) )
				{
					result += Unicode::narrowToWide("Template '");
					result += Unicode::narrowToWide(templateName);
					result += Unicode::narrowToWide("' can not be reloaded because it is not currently loaded.\n");
					retval = false;
				}
				else
				{
					if( ObjectTemplateList::reload(templateFile) )
					{
						result += Unicode::narrowToWide("Template '");
						result += Unicode::narrowToWide(templateName);
						result += Unicode::narrowToWide("' reloaded.\n");
						retval = true;
					}
					else
					{
						result += Unicode::narrowToWide("Template '");
						result += Unicode::narrowToWide(templateName);
						result += Unicode::narrowToWide("' reload failed.\n");
						retval = false;
					}
				}
			}

			ServerMessageForwarding::beginBroadcast();

			ReloadTemplateMessage const reloadTemplateMessage(templateName);
			ServerMessageForwarding::send(reloadTemplateMessage);

			ServerMessageForwarding::end();
		}		
		else if( cmd == "reloadTable" )
		{
			std::string tableName = Unicode::wideToNarrow(argv[3]);
			if( !TreeFile::exists(tableName.c_str()) )
			{
				result += Unicode::narrowToWide("Table '");
				result += Unicode::narrowToWide(tableName);
				result += Unicode::narrowToWide("' does not exist.\n");
				retval = false;
			}
			else
			{
				if( DataTableManager::reload(tableName) )
				{
					result += Unicode::narrowToWide("Reload for datatable '");
					result += Unicode::narrowToWide(tableName);
					result += Unicode::narrowToWide("' succeeded.\n");
					retval = true;
				}
				else
				{
					result += Unicode::narrowToWide("Reload for datatable '");
					result += Unicode::narrowToWide(tableName);
					result += Unicode::narrowToWide("' failed.\n");
					retval = false;
				}
			}

			ServerMessageForwarding::beginBroadcast();

			ReloadDatatableMessage const reloadDatatableMessage(tableName);
			ServerMessageForwarding::send(reloadDatatableMessage);

			ServerMessageForwarding::end();
		}
		else if( cmd == "public" )
		{
			LOG("ServerConsole", ("Setting cluster to public."));
			SetConnectionServerPublic const p(true);
			GameServer::getInstance().sendToCentralServer(p);
			result += Unicode::narrowToWide("Cluster set to public.\n");
			retval = true;
		}
		else if( cmd == "private" )
		{
			LOG("ServerConsole", ("Setting cluster to private."));
			SetConnectionServerPublic const p(false);
			GameServer::getInstance().sendToCentralServer(p);
			result += Unicode::narrowToWide("Cluster set to private.\n");
			retval = true;
		}
		else if(cmd == "info")
		{
			char numBuf[64] = {"\0"};
			static const Unicode::String unl(Unicode::narrowToWide(std::string("\n")));


			result += Unicode::narrowToWide("PID: ");
			snprintf(numBuf, sizeof(numBuf), "%lu", GameServer::getInstance().getProcessId());
			result += Unicode::narrowToWide(std::string(numBuf)) + unl;

			result += Unicode::narrowToWide("Scene: ") + Unicode::narrowToWide(ConfigServerGame::getSceneID()) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumObjects());
			result += Unicode::narrowToWide(std::string("Number of Objects: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumAI());
			result += Unicode::narrowToWide(std::string("Number of AI: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumBuildings());
			result += Unicode::narrowToWide(std::string("Number of Buildings: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumCreatures());
			result += Unicode::narrowToWide(std::string("Number of Creatures: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumPlayers());
			result += Unicode::narrowToWide(std::string("Number of Players: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumTangibles());
			result += Unicode::narrowToWide(std::string("Number of Tangibles: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumUniverseObjects());
			result += Unicode::narrowToWide(std::string("Number of UniverseObjects: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumDynamicAI());
			result += Unicode::narrowToWide(std::string("Number of Dynamic AI: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumStaticAI());
			result += Unicode::narrowToWide(std::string("Number of Dynamic AI: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumIntangibles());
			result += Unicode::narrowToWide(std::string("Number of Intangibles: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumMissionDatas());
			result += Unicode::narrowToWide(std::string("Number of MissionDatas: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumMissionObjects());
			result += Unicode::narrowToWide(std::string("Number of MissionObjects: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumRunTimeRules());
			result += Unicode::narrowToWide(std::string("Number of Runtime Rules: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumGroupObjects());
			result += Unicode::narrowToWide(std::string("Number of GroupObjects: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumMissionListEntries());
			result += Unicode::narrowToWide(std::string("Number of MissionListEntries: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumWaypoints());
			result += Unicode::narrowToWide(std::string("Number of Waypoints: ") + std::string(numBuf)) + unl;

			snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumPlayerQuestObjects());
			result += Unicode::narrowToWide(std::string("Number of PlayerQuestObjects: ") + std::string(numBuf)) + unl;
		}
	}
	LOG("ServerConsole", ("Command parsing %s.", retval==true?"succeeded":"failed"));
	return retval;
}

//-----------------------------------------------------------------------

