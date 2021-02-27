// ======================================================================
//
// ConsoleCommandParserNpc.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserNpc.h"

#include "UnicodeUtils.h"
#include "serverGame/CreatureObject.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerWorld.h"


// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"hail",    1, "<oid>",        "Hail an NPC"},
	{"goodbye", 0, "",             "End the current conversation"},
	{"respond", 1, "<response #>", "Select a conversation response"},
    {"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------

ConsoleCommandParserNpc::ConsoleCommandParserNpc (void) :
CommandParser ("npc", 0, "...", "NPC related commands.", 0)
{
    createDelegateCommands (cmds);
}

//-----------------------------------------------------------------


bool ConsoleCommandParserNpc::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
    NOT_NULL (node);
    UNREF (userId);

    UNREF(originalCommand);

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	//-----------------------------------------------------------------

    if (isAbbrev( argv [0], "hail"))
    {
		TangibleObject * const playerObject = safe_cast<TangibleObject *>(ServerWorld::findObjectByNetworkId(userId));

		NetworkId const oid(Unicode::wideToNarrow(argv[1]));
        ServerObject * const object = ServerWorld::findObjectByNetworkId(oid);
        if (object == nullptr)
        {
            result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
            return true;
        }
		TangibleObject * const npc = object->asTangibleObject();
		if (npc == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
            return true;
		}

		playerObject->startNpcConversation(*npc, nullptr, NpcConversationData::CS_Player, 0);
        result += getErrorMessage (argv[0], ERR_SUCCESS);
    }

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "goodbye"))
    {
		TangibleObject * const playerObject = safe_cast<TangibleObject *>(ServerWorld::findObjectByNetworkId(userId));
		playerObject->endNpcConversation();
        result += getErrorMessage (argv[0], ERR_SUCCESS);
    }

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "respond"))
    {
		TangibleObject * const playerObject = safe_cast<TangibleObject *>(ServerWorld::findObjectByNetworkId(userId));
		int const response = strtol(Unicode::wideToNarrow(argv[1]).c_str (), nullptr, 10);
		playerObject->respondToNpc(response);
        result += getErrorMessage (argv[0], ERR_SUCCESS);
    }

	//-----------------------------------------------------------------

    else
    {
        result += getErrorMessage(argv[0], ERR_NO_HANDLER);
    }

    return true;
}


// ======================================================================





