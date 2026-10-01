// ======================================================================
//
// ConsoleCommandParserCraft.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserCraft.h"
#include "UnicodeUtils.h"
#include "serverGame/ConsoleCommandParserCraftStation.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/GameControllerMessage.h"


// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"draft",                  1, "<number>",                          "Select a draft schematic to craft with"},
	{"fillslot",               3, "<slot #> <slot option #> <ingredient oid>", "Fill a draft schematic slot with an ingredient"},
	{"nextstage",              0, "",                                  "Go to the next stage of crafting"},
	{"experiment",             3, "<attribute #> <experiment points> <core level>", "Experiment to improve an attribute"},
	{"customize",              2, "<custom #> <value>",                "Set a customization property"},
	{"makeprototype",          1, "<name>",                            "Create a prototype object"},
	{"makeschematic",          1, "<name>",                            "Create manufacturing schematic"},
	{"stop",                   0, "",                                  "Stops the current crafting session"},
	{"enableSchematicFilter",  0, "",                                  "Enables schematic filtering (god mode only)"},
	{"disableSchematicFilter", 0, "",                                  "Disables schematic filtering (god mode only)"},
    {"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------

ConsoleCommandParserCraft::ConsoleCommandParserCraft (void) :
CommandParser ("craft", 0, "...", "Crafting related commands.", 0)
{
    createDelegateCommands (cmds);
	addSubCommand(new ConsoleCommandParserCraftStation());
}

//-----------------------------------------------------------------


bool ConsoleCommandParserCraft::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
    NOT_NULL (node);
    UNREF(originalCommand);

	CreatureObject * creatureObject = safe_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
	PlayerObject * playerObject = const_cast<PlayerObject *>(PlayerCreatureController::getPlayerObject(creatureObject));

    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	//-----------------------------------------------------------------

    if (isAbbrev( argv [0], "draft"))
    {
		// 1-based on the console.
		int32 index = 0;
		if (!parseArg(argv[1], index) || index == INT32_MIN)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_ARGUMENTS);
			return true;
		}
		--index;

		playerObject->selectDraftSchematic(index);
        result += getErrorMessage (argv[0], ERR_SUCCESS);
    }

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "fillslot"))
    {
		int32 slot = 0;
		int32 option = 0;
		if (!parseArg(argv[1], slot) || slot == INT32_MIN || !parseArg(argv[2], option) || option == INT32_MIN)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_ARGUMENTS);
			return true;
		}
		NetworkId ingredient(Unicode::wideToNarrow(argv[3]));

		if (playerObject->fillSlot(slot - 1, option - 1, ingredient))
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage (argv[0], ERR_FILLSLOT_FAIL);
    }

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "nextstage"))
	{
		if (playerObject->goToNextCraftingStage())
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage (argv[0], ERR_FAIL);
	}

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "experiment"))
    {
		int32 attribute = 0;
		int32 points = 0;
		int32 core = 0;
		if (!parseArg(argv[1], attribute) || attribute == INT32_MIN || !parseArg(argv[2], points) || !parseArg(argv[3], core))
		{
			result += getErrorMessage(argv[0], ERR_INVALID_ARGUMENTS);
			return true;
		}

		std::vector<MessageQueueCraftExperiment::ExperimentInfo> experiment;
		experiment.push_back(MessageQueueCraftExperiment::ExperimentInfo(attribute - 1, points));
		if (playerObject->experiment(experiment, points, core))
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage (argv[0], ERR_FILLSLOT_FAIL);
    }

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "customize"))
	{
		int32 property = 0;
		int32 value = 0;
		if (!parseArg(argv[1], property) || property == INT32_MIN || !parseArg(argv[2], value))
		{
			result += getErrorMessage(argv[0], ERR_INVALID_ARGUMENTS);
			return true;
		}

		if (playerObject->customize(property - 1, value))
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage (argv[0], ERR_CUSTOMIZE_FAIL);
	}

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "makeprototype"))
    {
		std::vector<Crafting::CustomValue> customizations;
		if (playerObject->setCustomizationData(argv[1], -1, customizations, 1) == 
			Crafting::CE_success && playerObject->createPrototype(true))
		{
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage (argv[0], ERR_FAIL);
    }

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "makeschematic"))
    {
		result += getErrorMessage (argv[0], ERR_FAIL);
    }

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "stop"))
    {
		playerObject->stopCrafting(true);
		result += getErrorMessage (argv[0], ERR_SUCCESS);
    }

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "enableSchematicFilter"))
	{
		if (creatureObject->getClient() == nullptr || !creatureObject->getClient()->isGod())
			result += getErrorMessage (argv[0], ERR_FAIL);
		else
		{
			creatureObject->enableSchematicFiltering();
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
	}

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "disableSchematicFilter"))
	{
		if (creatureObject->getClient() == nullptr || !creatureObject->getClient()->isGod())
			result += getErrorMessage (argv[0], ERR_FAIL);
		else
		{
			creatureObject->disableSchematicFiltering();
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
	}

	//-----------------------------------------------------------------


    else
    {
        result += getErrorMessage(argv[0], ERR_NO_HANDLER);
    }

    return true;
}


// ======================================================================





