// ======================================================================
//
// ConsoleCommandParserScript.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserScript.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/ReloadScriptMessage.h"
#include "serverScript/GameScriptObject.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedUtility/DataTableManager.h"
#include "UnicodeUtils.h"

#include <stdio.h>

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"attach",           2, "<name> <oid>",                 "Attach a script to an object."},
	{"detach",           2, "<name> <oid>",                 "Detach a script from an object."},
	{"list",             0, "",                             "List the scripts attached to an object."},
	{"pause",            1, "<on|off>",                     "Turn scripting on/off (globally)."},
	{"reload",           1, "<name>",                       "Reload a script that has changed."},
	{"triggerAll",       2, "<trigger> <oid> <...>",        "Run a trigger on all scripts attached to an object"},
	{"triggerOne",       3, "<trigger> <name> <oid> <...>", "Run a trigger on a script attached to an object"},
	{"scriptingEnabled", 0, "",                             "Checks if the scripting system is enabled or not"},
	{"enableLogging",    1, "<on|off>",                     "Turns script logging on or off"},
	{"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------

ConsoleCommandParserScript::ConsoleCommandParserScript (void) :
CommandParser ("script", 0, "...", "Script related commands.", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------


bool ConsoleCommandParserScript::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
	if (!playerObject)
	{
		WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
		return false;
	}

	if (!playerObject->getClient()->isGod()) {
		return false; // <3 you seefo
	}

	NOT_NULL (node);

	UNREF(originalCommand);
	UNREF(userId);

	//-----------------------------------------------------------------

	if (isAbbrev( argv [0], "attach"))
	{
		NetworkId oid(Unicode::wideToNarrow (argv[2]));
		ServerObject* object = ServerWorld::findObjectByNetworkId(oid);
		if (object == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}
		if (!object->getScriptObject()->attachScript(Unicode::wideToNarrow(argv[1]), true))
		{
			result += getErrorMessage (argv[0], ERR_INVALID_SCRIPT);
			return true;
		}
		result += getErrorMessage (argv[0], ERR_SUCCESS);
	}

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "detach"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[2]));
		ServerObject* object = ServerWorld::findObjectByNetworkId(oid);
		if (object == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		if (!object->getScriptObject()->detachScript(Unicode::wideToNarrow(argv[1])))
		{
			result += getErrorMessage(argv[0], ERR_INVALID_SCRIPT);
			return true;
		}
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	//-----------------------------------------------------------------

	else if (isAbbrev(argv[0], "list"))
	{
		ServerObject * const userServerObject = ServerWorld::findObjectByNetworkId(userId);

		if (userServerObject != nullptr)
		{
			NetworkId specifiedNetworkId;

			if (argv.size () > 1)
			{
				specifiedNetworkId = NetworkId(Unicode::wideToNarrow(argv[1]));
			}
			else
			{
				CreatureObject * const userCreatureObject = CreatureObject::asCreatureObject(userServerObject);

				if (userCreatureObject != nullptr)
				{
					specifiedNetworkId = userCreatureObject->getLookAtTarget();
				}
			}

			ServerObject * const specifiedServerObject = ServerObject::getServerObject(specifiedNetworkId);

			if (specifiedServerObject != nullptr)
			{
				ScriptList const & scripts = specifiedServerObject->getScriptObject()->getScripts();
				FormattedString<1024> fs;
				result += Unicode::narrowToWide(fs.sprintf("script list for object %s, num %d\n", specifiedServerObject->getNetworkId().getValueString().c_str(), scripts.size()));

				if (scripts.size() == 0)
				{
					result += Unicode::narrowToWide("none\n");
				}
				else
				{
					for (ScriptList::const_iterator i = scripts.begin(); i != scripts.end(); ++i)
					{
						result += Unicode::narrowToWide(fs.sprintf("%s\n", (*i).getScriptName().c_str()));
					}
				}

				result += getErrorMessage(argv[0], ERR_SUCCESS);
				return true;
			}
		}

		result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
	}

	//-----------------------------------------------------------------

	else if (isAbbrev(argv[0], "pause"))
	{
		bool flag;
		if (strcmp(Unicode::wideToNarrow(argv[1]).c_str(), "on") == 0)
			flag = true;
		else if (strcmp(Unicode::wideToNarrow(argv[1]).c_str(), "off") == 0)
			flag = false;
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_ARGUMENTS);
			return true;
		}
		GameScriptObject::pauseScripting(flag);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	//-----------------------------------------------------------------

	else if (isAbbrev(argv[0], "reload"))
	{
		std::string script(Unicode::wideToNarrow(argv[1]));
		if (GameScriptObject::reloadScript(script))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_SCRIPT);

		ServerMessageForwarding::beginBroadcast();

		ReloadScriptMessage const reloadScriptMessage(script);
		ServerMessageForwarding::send(reloadScriptMessage);

		ServerMessageForwarding::end();
	}

	//-----------------------------------------------------------------
	else if (isAbbrev(argv[0], "triggerAll") || isAbbrev(argv[0], "triggerOne"))
	{
		int i;
		bool all = isAbbrev(argv[0], "triggerAll");

		// get the trigger id
		int trigger = Scripting::TRIG_LAST_TRIGGER;
		std::string triggerName = Unicode::wideToNarrow(argv[1]).c_str();
		if (isdigit(triggerName[0]))
			trigger = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		else
		{
			Scripting::_ScriptFuncHashMap::const_iterator iter;
			for (iter = Scripting::ScriptFuncHashMap->begin();
				iter != Scripting::ScriptFuncHashMap->end();
				++iter)
			{
				if (std::string((*iter).second->name) == triggerName)
				{
					trigger = (*iter).first;
					break;
				}
			}
		}
		if (trigger < 0 || trigger >= Scripting::TRIG_LAST_TRIGGER)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_TRIGGER);
			return true;
		}

		int oidIndex = 2;
		int paramIndex = 3;
		std::string scriptName;
		if (!all)
		{
			oidIndex = 3;
			paramIndex = 4;
			scriptName = Unicode::wideToNarrow(argv[2]);
		}

		// get the object id
		NetworkId oid(Unicode::wideToNarrow(argv[oidIndex]));
		ServerObject* object = ServerWorld::findObjectByNetworkId(oid);
		if (object == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		// get the parameter template for the trigger
		Scripting::_ScriptFuncHashMap::const_iterator funcIter = Scripting::ScriptFuncHashMap->find(trigger);
		if (funcIter == Scripting::ScriptFuncHashMap->end())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_TRIGGER);
			return true;
		}
		const Scripting::ScriptFuncTable *func = (*funcIter).second;

		// make sure we have the correct parameter count
		int paramCount = argv.size() - paramIndex;
		int paramDesired = 0;
		for (i = 0; func->argList[i] != 0; ++i)
		{
			if (func->argList[i] != ']')
				++paramDesired;
			// don't let a trigger that takes a slotData or attribMod
			if (func->argList[i] == 'E' || func->argList[i] == 'A')
			{
				result += getErrorMessage(argv[0], ERR_UNSUPPORTED_TRIGGER);
				return true;
			}
		}
		if (paramCount != paramDesired)
		{
			char buffer[16];
			result += getErrorMessage(argv[0], ERR_INVALID_PARAMETER_COUNT);
			_itoa(paramDesired, buffer, 10);
			result += Unicode::narrowToWide(buffer);
			return true;
		}

		// copy the parameters
		StringVector_t params;
		for (i = 0; i < paramCount; ++i)
			params.push_back(argv[paramIndex + i]);
		

		// invoke the trigger
		int trigResult = object->getScriptObject()->trigScriptFromConsole(
			static_cast<Scripting::TrigId>(trigger),
			scriptName,
			params
			);

		if (trigResult == SCRIPT_CONTINUE)
			result += getErrorMessage(argv[0], ERR_SCRIPT_CONTINUE);
		else
			result += getErrorMessage(argv[0], ERR_SCRIPT_OVERRIDE);
	}

	//-----------------------------------------------------------------

	else if (isAbbrev(argv[0], "scriptingEnabled"))
	{
		if (GameScriptObject::isScriptingEnabled())
			result += getErrorMessage(argv[0], ERR_SCRIPTING_ENABLED);
		else
			result += getErrorMessage(argv[0], ERR_SCRIPTING_DISABLED);
	}

	//-----------------------------------------------------------------

	else if (isAbbrev(argv[0], "enableLogging"))
	{
		bool flag;
		if (strcmp(Unicode::wideToNarrow(argv[1]).c_str(), "on") == 0)
			flag = true;
		else if (strcmp(Unicode::wideToNarrow(argv[1]).c_str(), "off") == 0)
			flag = false;
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_ARGUMENTS);
			return true;
		}
		GameScriptObject::enableLogging(flag);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	//-----------------------------------------------------------------

	else
	{
		result += getErrorMessage(argv[0], ERR_NO_HANDLER);
	}

	return true;
}

// ======================================================================
