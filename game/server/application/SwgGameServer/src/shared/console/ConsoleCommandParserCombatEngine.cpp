// ======================================================================
//
// ConsoleCommandParserCombatEngine.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgGameServer.h"
#include "CombatEngine.h"

#include "UnicodeUtils.h"
#include "ConsoleCommandParserCombatEngine.h"
#include "ConsoleCommandParserCombatEngineQueue.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/WeaponObject.h"

#include <stdio.h>


// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"reload",       0, "",				 "Reloads the combat data from the config file."},
    {"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------

ConsoleCommandParserCombatEngine::ConsoleCommandParserCombatEngine (void) :
CommandParser ("combat", 0, "...", "Combat related commands.", 0)
{
    createDelegateCommands (cmds);
	IGNORE_RETURN(addSubCommand(new ConsoleCommandParserCombatEngineQueue()));//lint !e1524 (owned by the base class command parser)
}

//-----------------------------------------------------------------

bool ConsoleCommandParserCombatEngine::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL (node);
	UNREF (userId);

	UNREF(originalCommand);

    //-----------------------------------------------------------------
	if (isAbbrev( argv [0], "reload"))
	{	
		if (CombatEngine::reloadCombatData())
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_NO_ERROR_MSG);
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
