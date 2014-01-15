// ======================================================================
//
// ObjectCommandParserGetAttributes.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ObjectCommandParserGetAttributes.h"

#include "serverScript/GameScriptObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ServerObject.h"


// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"hitpoints",  2, "<oid> <value>",                "View an object's hitpoints."},
	{"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------

ObjectCommandParserGetAttributes::ObjectCommandParserGetAttributes (void) :
CommandParser ("getAttribute", 0, "...", "Commands to set attributes on objects", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------


bool ObjectCommandParserGetAttributes::performParsing (NetworkId userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL (node);

	UNREF(originalCommand);
	UNREF (userId);

	//-----------------------------------------------------------------

	if (isAbbrev( argv [0], "hitpoints"))
	{
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
