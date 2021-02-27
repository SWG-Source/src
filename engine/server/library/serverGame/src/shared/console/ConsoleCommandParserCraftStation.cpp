// ======================================================================
//
// ConsoleCommandParserCraftStation.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserCraftStation.h"

#include "UnicodeUtils.h"
#include "serverGame/CreatureObject.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"


// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
//	{"add",      2, "<station oid> <ingredient oid>",  "Add an ingredient to a station"},
//	{"remove",   2, "<station oid> <ingredient oid>",  "Remove an ingredient from a station"},
//	{"list",     1, "<station oid>",                   "List a station's ingredients"},
    {"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------

ConsoleCommandParserCraftStation::ConsoleCommandParserCraftStation (void) :
CommandParser ("station", 0, "...", "Crafting station related commands.", 0)
{
    createDelegateCommands (cmds);
}

//-----------------------------------------------------------------


bool ConsoleCommandParserCraftStation::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
    NOT_NULL (node);

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

//	CreatureObject * playerObject = safe_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));

	//-----------------------------------------------------------------

    if (isAbbrev( argv [0], "add"))
    {
/*		// get the station
		TangibleObject * station = getStation(argv, result);
		if (station == nullptr)
			return true;

		// get the ingredient
		NetworkId ingredientId(Unicode::wideToNarrow (argv[2]));
		ServerObject * object = ServerWorld::findObjectByNetworkId(ingredientId);
		if (object == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_INGREDIENT);
			return true;
		}
		TangibleObject * ingredient = dynamic_cast<TangibleObject *>(object);
		if (ingredient == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_INGREDIENT);
			return true;
		}

		station->addIngredientToHopper(*ingredient, playerObject);
	    result += getErrorMessage (argv[0], ERR_SUCCESS);
*/		
    }

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "remove"))
    {
/*		// get the station
		TangibleObject * station = getStation(argv, result);
		if (station == nullptr)
			return true;

		// get the ingredient
		NetworkId ingredientId(Unicode::wideToNarrow (argv[2]));
		ServerObject * object = ServerWorld::findObjectByNetworkId(ingredientId);
		if (object == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_INGREDIENT);
			return true;
		}
		TangibleObject * ingredient = dynamic_cast<TangibleObject *>(object);
		if (ingredient == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_INGREDIENT);
			return true;
		}

		station->removeIngredientFromHopper(*ingredient);
        result += getErrorMessage (argv[0], ERR_SUCCESS);
*/
    }

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "list"))
    {
/*		// get the station
		NetworkId id(Unicode::wideToNarrow (argv[1]));
		ServerObject * object = ServerWorld::findObjectByNetworkId(id);
		if (object == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}
		TangibleObject * station = dynamic_cast<TangibleObject *>(object);
		if (station == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}

		station->listIngredientsInHopper(*playerObject);
        result += getErrorMessage (argv[0], ERR_SUCCESS);
*/
    }

	//-----------------------------------------------------------------

    else
    {
        result += getErrorMessage(argv[0], ERR_NO_HANDLER);
    }

    return true;
}

//-----------------------------------------------------------------

/**
 * Finds the crafting station for a given id.
 *
 * @return the station or nullptr on error
 */
TangibleObject * ConsoleCommandParserCraftStation::getStation(const StringVector_t & argv, String_t & result)
{
	NetworkId id(Unicode::wideToNarrow (argv[1]));
	ServerObject * object = ServerWorld::findObjectByNetworkId(id);
	if (object == nullptr)
	{
		result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
		return nullptr;
	}
	TangibleObject * station = dynamic_cast<TangibleObject *>(object);
	if (station == nullptr)
	{
		result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
		return nullptr;
	}
	// make sure the station is a atation and that it isn't in use
	if (!station->getObjVars().hasItem("crafting.station"))
	{
		result += getErrorMessage (argv [0], ERR_INVALID_STATION);
		return nullptr;
	}
	if (station->getObjVars().hasItem("crafting.crafter"))
	{
		result += getErrorMessage (argv [0], ERR_STATION_IN_USE);
		return nullptr;
	}
	return station;
}	// ConsoleCommandParserCraftStation::getStation


// ======================================================================





