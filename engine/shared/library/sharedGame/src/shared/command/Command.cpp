// ======================================================================
//
// Command.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/Command.h"
#include "sharedFoundation/Crc.h"
#include "StringId.h"

// ======================================================================

namespace CommandNamespace
{
	const uint32 s_defaultAttackCoolGroup = 0x399ea293;   // "defaultattack"
};

using namespace CommandNamespace;

Command::Command() :
	m_commandName(),
	m_commandHash(Crc::crcNull),
	m_defaultPriority(CP_NumberOfPriorities),
	m_scriptHook(),
	m_failScriptHook(),
	m_cppHook(),
	m_failCppHook(),
	m_defaultTime(0.0f),
	m_characterAbility(),
	m_locomotionPermissions(),
	m_statePermissions(),
	m_stateRequired(),
	m_tempScript(),
	m_targetType(CTT_NumberOfTargetTypes),
	m_stringId(),
	m_visibleToClients(0),
	m_callOnTarget(false),
	m_commandGroup(0),
	m_maxRangeToTarget(0.0f),
	m_maxRangeToTargetSquared(0.0f),
	m_godLevel(0),
	m_displayGroup(0),
	m_addToCombatQueue(false),
	m_warmTime(0),
	m_execTime(0),
	m_coolTime(0),
	m_coolGroup(0),
	m_weaponTypesValid(0),
	m_weaponTypesInvalid(0),
	m_coolGroup2(0),
	m_coolTime2(0),
	m_toolbarOnly(false),
	m_fromServerOnly(false)
{
}

// ----------------------------------------------------------------------

Command::Command(Command const &rhs) :
	m_commandName(rhs.m_commandName),
	m_commandHash(rhs.m_commandHash),
	m_defaultPriority(rhs.m_defaultPriority),
	m_scriptHook(rhs.m_scriptHook),
	m_failScriptHook(rhs.m_failScriptHook),
	m_cppHook(rhs.m_cppHook),
	m_failCppHook(rhs.m_failCppHook),
	m_defaultTime(rhs.m_defaultTime),
	m_characterAbility(rhs.m_characterAbility),
	m_locomotionPermissions(rhs.m_locomotionPermissions),
	m_statePermissions(rhs.m_statePermissions),
	m_stateRequired(rhs.m_stateRequired),
	m_tempScript(rhs.m_tempScript),
	m_targetType(rhs.m_targetType),
	m_stringId(rhs.m_stringId),
	m_visibleToClients(rhs.m_visibleToClients),
	m_callOnTarget(rhs.m_callOnTarget),
	m_commandGroup(rhs.m_commandGroup),
	m_maxRangeToTarget(rhs.m_maxRangeToTarget),
	m_maxRangeToTargetSquared(rhs.m_maxRangeToTargetSquared),
	m_godLevel(rhs.m_godLevel),
	m_displayGroup(rhs.m_displayGroup),
	m_addToCombatQueue(rhs.m_addToCombatQueue),
	m_warmTime(rhs.m_warmTime),
	m_execTime(rhs.m_execTime),
	m_coolTime(rhs.m_coolTime),
	m_coolGroup(rhs.m_coolGroup),
	m_weaponTypesValid(rhs.m_weaponTypesValid),
	m_weaponTypesInvalid(rhs.m_weaponTypesInvalid),
	m_coolGroup2(rhs.m_coolGroup2),
	m_coolTime2(rhs.m_coolTime2),
	m_toolbarOnly(rhs.m_toolbarOnly),
	m_fromServerOnly(rhs.m_fromServerOnly)
{
}

// ----------------------------------------------------------------------

Command &Command::operator=(Command const &rhs)
{
	if (&rhs != this)
	{
		m_commandName = rhs.m_commandName;
		m_commandHash = rhs.m_commandHash;
		m_defaultPriority = rhs.m_defaultPriority;
		m_scriptHook = rhs.m_scriptHook;
		m_failScriptHook = rhs.m_failScriptHook;
		m_cppHook = rhs.m_cppHook;
		m_failCppHook = rhs.m_failCppHook;
		m_defaultTime = rhs.m_defaultTime;
		m_characterAbility = rhs.m_characterAbility;
		m_locomotionPermissions = rhs.m_locomotionPermissions;
		m_statePermissions = rhs.m_statePermissions;
		m_stateRequired = rhs.m_stateRequired;
		m_tempScript = rhs.m_tempScript;
		m_targetType = rhs.m_targetType;
		m_stringId = rhs.m_stringId;
		m_visibleToClients = rhs.m_visibleToClients;
		m_callOnTarget = rhs.m_callOnTarget;
		m_commandGroup = rhs.m_commandGroup;
		m_maxRangeToTarget = rhs.m_maxRangeToTarget;
		m_maxRangeToTargetSquared = rhs.m_maxRangeToTargetSquared;
		m_godLevel = rhs.m_godLevel;
		m_displayGroup = rhs.m_displayGroup;
		m_addToCombatQueue = rhs.m_addToCombatQueue;
		m_warmTime = rhs.m_warmTime;
		m_execTime = rhs.m_execTime;
		m_coolTime = rhs.m_coolTime;
		m_coolGroup = rhs.m_coolGroup;
		m_weaponTypesValid = rhs.m_weaponTypesValid;
		m_weaponTypesInvalid = rhs.m_weaponTypesInvalid;
		m_coolGroup2 = rhs.m_coolGroup2;
		m_coolTime2 = rhs.m_coolTime2;
		m_toolbarOnly = rhs.m_toolbarOnly;
		m_fromServerOnly = rhs.m_fromServerOnly;
	}
	return *this;
}

// ----------------------------------------------------------------------

bool Command::isNull() const
{
	return m_commandHash == Crc::crcNull;
}

// ----------------------------------------------------------------------

bool Command::isPrimaryCommand() const
{
	return m_coolGroup == s_defaultAttackCoolGroup;
}

//----------------------------------------------------------------------

const StringId & Command::getStringIdForErrorCode (const ErrorCode code)
{
	static const std::string table = "cmd_err";

	static const StringId ids [Command::CEC_Max] =
	{
		StringId (table, "success_prose"),
		StringId (table, "locomotion_prose"),
		StringId (table, "ability_prose"),
		StringId (table, "target_type_prose"),
		StringId (table, "target_range_prose"),
		StringId (table, "state_prose"),
		StringId (table, "state_must_have_prose"),
		StringId (table, "god_level_prose")
	};

	if (code < 0 || code >= Command::CEC_Max)
	{
		WARNING(true, ("Command::getStringIdForErrorCode received invalid code %d", static_cast<int>(code)));
		return ids [static_cast<size_t>(Command::CEC_Success)];
	}
	return ids [static_cast<size_t>(code)];
}

// ======================================================================

