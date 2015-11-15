// ======================================================================
//
// CommandTable.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CommandTable.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/Command.h"
#include "sharedUtility/DataTable.h"
#include "UnicodeUtils.h"
#include <map>

// ======================================================================

namespace CommandTableNamespace
{
	// ----------------------------------------------------------------------

	typedef std::map<std::string, CommandCppFunc::Type> CommandFunctionMap;

	CommandFunctionMap s_functionMap;
	CommandTable::CommandMap s_commandMap;
	Command const s_nullCommand;

	// ----------------------------------------------------------------------

	Command::CppFunc lookupCppFunc(std::string const &funcName);
	void loadCommandRow(DataTable const &t, int row);

	// ----------------------------------------------------------------------
}

using namespace CommandTableNamespace;

// ======================================================================

void CommandTable::loadCommandTables(std::vector<std::string> const &commandTableFilenames)
{
	// clear out all commands
	{
		for (CommandTable::CommandMap::iterator i = s_commandMap.begin(); i != s_commandMap.end(); ++i)
			(*i).second = s_nullCommand;
	}

	// load the command tables
	{
		for (std::vector<std::string>::const_iterator i = commandTableFilenames.begin(); i != commandTableFilenames.end(); ++i)
		{
			Iff iff;
			if (iff.open((*i).c_str(), true))
			{
				DataTable commandTable;
				commandTable.load(iff);

				for (int row = 0; row < commandTable.getNumRows(); ++row)
					loadCommandRow(commandTable, row);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CommandTable::addCppFunction(std::string const &funcName, Command::CppFunc func)
{
	std::map<std::string, Command::CppFunc>::iterator i = s_functionMap.find(funcName);
	if (i == s_functionMap.end())
		s_functionMap[funcName] = func;
	else
	{
		// attempted to add a c function more that once - this is an error if
		// the functions don't match
		FATAL((*i).second != func, ("Tried to add multiple cpp functions with identical names to the command table."));
	}
}

// ----------------------------------------------------------------------

Command const &CommandTable::getCommand(uint32 commandHash)
{
	std::pair<CommandMap::iterator, bool> const result = s_commandMap.insert(std::make_pair(commandHash, s_nullCommand));
	return (*result.first).second;
}

// ----------------------------------------------------------------------

CommandTable::CommandMap const &CommandTable::getCommandMap()
{
	return s_commandMap;
}

// ----------------------------------------------------------------------

Command const &CommandTable::getNullCommand()
{
	return s_nullCommand;
}

// ----------------------------------------------------------------------

Command::CppFunc CommandTableNamespace::lookupCppFunc(std::string const &funcName)
{
	std::map<std::string, Command::CppFunc>::iterator i = s_functionMap.find(funcName);
	if (i != s_functionMap.end())
		return (*i).second;
	return 0;
}

// ----------------------------------------------------------------------

void CommandTableNamespace::loadCommandRow(DataTable const &t, int row)
{
	std::string const &commandName = Unicode::getTrim(t.getStringValue("commandName", row));
	uint32 const commandHash = Crc::normalizeAndCalculate(commandName.c_str());

	Command &cmd = s_commandMap[commandHash];

	if (t.getIntValue("disabled", row))
	{
		cmd = s_nullCommand;
		return;
	}

	cmd.m_commandName = commandName;
	cmd.m_commandHash = commandHash;

	{
		int const priority = t.getIntValue("defaultPriority", row);
		if (priority < 0 || priority >= static_cast<int>(Command::CP_NumberOfPriorities))
			cmd.m_defaultPriority = Command::CP_Default;
		else
			cmd.m_defaultPriority = static_cast<Command::Priority>(priority);
	}

	cmd.m_scriptHook = Unicode::getTrim(t.getStringValue("scriptHook", row));
	cmd.m_failScriptHook = Unicode::getTrim(t.getStringValue("failScriptHook", row));

	std::string const cppHook = Unicode::getTrim(t.getStringValue("cppHook", row));
	cmd.m_cppHook = cppHook.empty() ? 0 : lookupCppFunc(cppHook);

	std::string const failCppHook = Unicode::getTrim(t.getStringValue("failCppHook", row));
	cmd.m_failCppHook = failCppHook.empty() ? 0 : lookupCppFunc(failCppHook);

	cmd.m_defaultTime = 0.f;//t.getFloatValue("defaultTime", row); // <-- defaultTime column is deprecated by combat upgrade - 3/16/05 jmatzen
	cmd.m_characterAbility = Unicode::getTrim(t.getStringValue("characterAbility", row));

	{
		for (Locomotions::Enumerator i = 0; i != Locomotions::NumberOfLocomotions; ++i)
		{
			char colName[128];
			IGNORE_RETURN(snprintf(colName, sizeof(colName)-1, "L:%s", Locomotions::getLocomotionName(i)));
			colName[sizeof(colName)-1] = '\0';
			int const colNumber = t.findColumnNumber(colName);
			if (colNumber >= 0)
				cmd.m_locomotionPermissions[i] = t.getIntValue(colNumber, row) ? true : false;
			else
				cmd.m_locomotionPermissions[i] = true;
		}
	}

	{
		for (States::Enumerator i = 0; i != States::NumberOfStates; ++i)
		{
			char colName[128];
			IGNORE_RETURN(snprintf(colName, sizeof(colName)-1, "S:%s", States::getStateName(i)));
			colName[sizeof(colName)-1] = '\0';
			int const colNumber = t.findColumnNumber(colName);
			if (colNumber >= 0)
				cmd.m_statePermissions[i] = t.getIntValue(colNumber, row) ? true : false;
			else
				cmd.m_statePermissions[i] = true;
		}
	}

	{
		for (States::Enumerator i = 0; i != States::NumberOfStates; ++i)
		{
			char colName[128];
			IGNORE_RETURN(snprintf(colName, sizeof(colName)-1, "R:%s", States::getStateName(i)));
			colName[sizeof(colName)-1] = '\0';
			int const colNumber = t.findColumnNumber(colName);
			if (colNumber >= 0)
				cmd.m_stateRequired[i] = t.getIntValue(colNumber, row) ? true : false;
			else
				cmd.m_stateRequired[i] = false;
		}
	}

	cmd.m_tempScript = Unicode::getTrim(t.getStringValue("tempScript", row));

	{
		cmd.m_target = Command::CT_Other;
		int column = t.findColumnNumber("target");
		if (column >= 0)
		{
			int const target = t.getIntValue(column, row);
			if (target >= 0 && target < static_cast<int>(Command::CT_NumberOfTargets))
				cmd.m_target = static_cast<Command::Target>(target);
		}
	}

	{
		int const targetType = t.getIntValue("targetType", row);
		if (targetType < 0 || targetType >= static_cast<int>(Command::CTT_NumberOfTargetTypes))
			cmd.m_targetType = Command::CTT_None;
		else
			cmd.m_targetType = static_cast<Command::TargetType>(targetType);
	}

	cmd.m_callOnTarget     = t.getIntValue("callOnTarget", row) ? true : false;
	cmd.m_maxRangeToTarget = t.getFloatValue("maxRangeToTarget", row);
	cmd.m_maxRangeToTargetSquared = sqr(cmd.m_maxRangeToTarget);
	cmd.m_godLevel         = t.getIntValue("godLevel", row);

	int queueColNumber = t.findColumnNumber("addToCombatQueue");
	if (queueColNumber >= 0)
		cmd.m_addToCombatQueue= t.getIntValue(queueColNumber, row) ? true : false;

	// The following preserve previously set values if the default value is in the new entry
	std::string const &stringId = Unicode::getTrim(t.getStringValue("stringId", row));
	if (!stringId.empty())
		cmd.m_stringId = stringId;
	int const visibleToClients = t.getIntValue("visible", row);
	if (visibleToClients)
		cmd.m_visibleToClients = visibleToClients;
	uint32 const commandGroup = static_cast<uint32>(t.getIntValue("commandGroup", row));
	if (commandGroup)
		cmd.m_commandGroup = commandGroup;
	uint32 const displayGroup = static_cast<uint32>(t.getIntValue("displayGroup", row));
	if (displayGroup)
		cmd.m_displayGroup = displayGroup;
		
	cmd.m_warmTime = t.getFloatValue( "warmupTime", row );
	cmd.m_execTime = t.getFloatValue( "executeTime", row );
	cmd.m_coolTime = t.getFloatValue( "cooldownTime", row );
	cmd.m_coolTime2 = t.getFloatValue( "cooldownTime2", row );
	
	const std::string cooldownGroupName( Unicode::getTrim( t.getStringValue( "cooldownGroup", row ) ) );
	if(cooldownGroupName.empty())
		cmd.m_coolGroup = static_cast<uint32>(-1);
	else
		cmd.m_coolGroup = Crc::normalizeAndCalculate( cooldownGroupName.c_str() );
	
	const std::string cooldownGroup2Name( Unicode::getTrim( t.getStringValue( "cooldownGroup2", row ) ) );
	if(cooldownGroup2Name.empty())
		cmd.m_coolGroup2 = static_cast<uint32>(-1);
	else
		cmd.m_coolGroup2 = Crc::normalizeAndCalculate( cooldownGroup2Name.c_str() );

	cmd.m_weaponTypesValid   = static_cast<uint32>(t.getIntValue( "validWeapon", row));
	cmd.m_weaponTypesInvalid = static_cast<uint32>(t.getIntValue( "invalidWeapon", row));

	cmd.m_toolbarOnly = t.getIntValue("toolbarOnly", row) ? true : false;
	cmd.m_fromServerOnly = t.getIntValue("fromServerOnly", row) ? true : false;
}

// ----------------------------------------------------------------------

bool CommandTable::isCommandQueueCommand(uint32 const hash)
{
	bool result = false;
	Command const & command = getCommand(hash);
	
	if (command.m_commandHash && command.m_addToCombatQueue)
	{
		result = true;
	}

	return result;
}

// ======================================================================
