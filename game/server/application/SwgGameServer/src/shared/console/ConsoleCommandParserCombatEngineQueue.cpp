// ======================================================================
//
// ConsoleCommandParserCombatEngineQueue.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgGameServer.h"
#include "ConsoleCommandParserCombatEngineQueue.h"

#include "CombatEngine.h"
#include "UnicodeUtils.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/TangibleObject.h"
#include "sharedFoundation/NetworkIdArchive.h"

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
    {"target",     1, "<oid>",               "Target an object (oid 0 = no target)."},
	{"attack",     0, "<weapon oid> <mode>", "Attack current target with weapon."},
	{"aim",        0, "",                    "Add aim."},
	{"attitude",   1, "<attitude>",          "Change combat attitude (queued)."},
	{"posture",    1, "<posture>",           "Change combat posture (queued)."},
    {"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------

ConsoleCommandParserCombatEngineQueue::ConsoleCommandParserCombatEngineQueue (void) :
CommandParser ("queue", 0, "...", "Action queue commands.", 0)
{
    createDelegateCommands (cmds);
}

//-----------------------------------------------------------------


bool ConsoleCommandParserCombatEngineQueue::performParsing (const NetworkId & userId, 
	const StringVector_t & argv, const String_t & originalCommand, String_t & result, 
	const CommandParser * node)
{
    NOT_NULL (node);
    UNREF (userId);

    UNREF(originalCommand);

	//-----------------------------------------------------------------
    if (isAbbrev( argv [0], "target"))
    {
		Object *object = ServerWorld::findObjectByNetworkId(userId);
		if (! object)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
			return true;
		}
		TangibleObject *attacker = dynamic_cast<TangibleObject *>(object);
		if (! attacker)
		{
			result += getErrorMessage(argv[0], ERR_BAD_ATTACKER);
			return true;
		}

		CombatEngineData::TargetIdList targets;
		CachedNetworkId oid((Unicode::wideToNarrow(argv[1]))); //lint !e747 Info: 747 - Significant prototype coercion (arg. no. 1) int to long long
		if (oid.getValue() != 0)
		{
			targets.push_back(oid);
		}

		if (CombatEngine::addTargetAction(*attacker, targets))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_QUEUE_COMMAND_FAIL);
    }
    
	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "attack"))
	{
		Object *object = ServerWorld::findObjectByNetworkId(userId);
		if (! object)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
			return true;
		}
		TangibleObject *attacker = dynamic_cast<TangibleObject *>(object);
		if (! attacker)
		{
			result += getErrorMessage(argv[0], ERR_BAD_ATTACKER);
			return true;
		}

		NetworkId weapon;
		unsigned int mode = 0;
		if (argv.size() >= 2)
			weapon = NetworkId(Unicode::wideToNarrow(argv[1]));
		if (argv.size() >= 3)
			mode = strtoul(Unicode::wideToNarrow(argv[2]).c_str (), 0, 10);
		if (CombatEngine::addAttackAction(*attacker, weapon, static_cast<int>(mode)))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_QUEUE_COMMAND_FAIL);
	}

	//-----------------------------------------------------------------
    
	else if (isAbbrev( argv [0], "aim"))
	{
		Object *object = ServerWorld::findObjectByNetworkId(userId);
		if (! object)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
			return true;
		}
		TangibleObject *attacker = dynamic_cast<TangibleObject *>(object);
		if (! attacker)
		{
			result += getErrorMessage(argv[0], ERR_BAD_ATTACKER);
			return true;
		}

		if (CombatEngine::addAimAction(*attacker))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_QUEUE_COMMAND_FAIL);
	}

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "attitude"))
	{
		Object *object = ServerWorld::findObjectByNetworkId(userId);
		if (! object)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
			return true;
		}
		TangibleObject *attacker = dynamic_cast<TangibleObject *>(object);
		if (! attacker)
		{
			result += getErrorMessage(argv[0], ERR_BAD_ATTACKER);
			return true;
		}

//		int attitude = strtoul(Unicode::wideToNarrow(argv[1]).c_str (), 0, 10);

//		if (CombatEngine::addAttitudeAction(*attacker, attitude))
//			result += getErrorMessage(argv[0], ERR_SUCCESS);
//		else
		result += getErrorMessage(argv[0], ERR_QUEUE_COMMAND_FAIL);
	}

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "posture"))
	{
		Object *object = ServerWorld::findObjectByNetworkId(userId);
		if (! object)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
			return true;
		}
		CreatureObject *attacker = dynamic_cast<CreatureObject *>(object);
		if (! attacker)
		{
			result += getErrorMessage(argv[0], ERR_BAD_ATTACKER);
			return true;
		}

//		Postures::Enumerator posture = static_cast<Postures::Enumerator>(strtoul(
//			Unicode::wideToNarrow(argv[1]).c_str (), 0, 10));

//		if (CombatEngine::addPostureAction(*attacker, posture))
//			result += getErrorMessage(argv[0], ERR_SUCCESS);
//		else
		result += getErrorMessage(argv[0], ERR_QUEUE_COMMAND_FAIL);
	}

	//-----------------------------------------------------------------

	else
	{
		result += getErrorMessage(argv[0], ERR_NO_HANDLER);
	}

	return true;
}

// ---------------------------------------------------------------------




// ======================================================================
