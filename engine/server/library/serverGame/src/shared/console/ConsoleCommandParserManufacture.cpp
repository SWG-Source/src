// ======================================================================
//
// ConsoleCommandParserManufacture.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserManufacture.h"

#include "UnicodeUtils.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ManufactureInstallationObject.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "sharedObject/VolumeContainer.h"
#include "serverGame/ServerWorld.h"

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"addIngredient",    2, "<station oid> <ingredient oid>", "add an ingredient to a station's input hopper"},
	{"removeIngredient", 2, "<station oid> <ingredient oid>", "remove an ingredient from a station's input hopper"},
	{"listIngredients",  1, "<oid>", "list the ingredients in a station's input hopper"},
	{"addSchematic",     2, "<station oid> <schematic oid>", "add a manufacturing schematic to a station"},
	{"removeSchematic",  1, "<station oid>", "remove the manufacturing schematic from a station"},
	{"listSchematic",    1, "<station oid>", "list the manufacturing schematic of a station"},
	{"start",            1, "<oid>", "starts a station manufacturing"},
	{"stop",             1, "<oid>", "stops a station manufacturing"},
	{"getObjects",       1, "<oid>", "puts all the objects in a station's output hopper in your inventory"},
	{"listObjects",      1, "<oid>", "lists all the objects in a station's output hopper"},
    {"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------

ConsoleCommandParserManufacture::ConsoleCommandParserManufacture (void) :
CommandParser ("manf", 0, "...", "Manufacture station commands.", 0)
{
    createDelegateCommands (cmds);
}

//-----------------------------------------------------------------


bool ConsoleCommandParserManufacture::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
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

	//-----------------------------------------------------------------

    if (isAbbrev( argv [0], "addIngredient"))
    {
		CachedNetworkId stationId(Unicode::wideToNarrow(argv[1]));
		CachedNetworkId ingredientId(Unicode::wideToNarrow(argv[2]));
		
		ManufactureInstallationObject * station = dynamic_cast<ManufactureInstallationObject *>(stationId.getObject());
		ServerObject * ingredient = safe_cast<ServerObject *>(ingredientId.getObject());

		if (station == nullptr || ingredient == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (station->isActive())
		{
			result += getErrorMessage(argv[0], ERR_ACTIVE_MANF_STATION);
			return true;
		}

		ServerObject * hopper = station->getInputHopper();
		if (hopper == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		if (ContainerInterface::transferItemToVolumeContainer(*hopper, *ingredient, playerObject, tmp))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);
    }

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "removeIngredient"))
    {
		CachedNetworkId stationId(Unicode::wideToNarrow(argv[1]));
		CachedNetworkId ingredientId(Unicode::wideToNarrow(argv[2]));

		ManufactureInstallationObject * station = dynamic_cast<ManufactureInstallationObject *>(stationId.getObject());
		ServerObject * ingredient = safe_cast<ServerObject *>(ingredientId.getObject());

		if (station == nullptr || ingredient == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (station->isActive())
		{
			result += getErrorMessage(argv[0], ERR_ACTIVE_MANF_STATION);
			return true;
		}

		ServerObject * hopper = station->getInputHopper();
		if (hopper == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (hopper != ContainerInterface::getContainedByObject(*ingredient))
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (playerObject->getInventory() == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		Container::ContainerErrorCode tmp = Container::CEC_Success;				
		if (ContainerInterface::transferItemToVolumeContainer(*playerObject->getInventory(), *ingredient, playerObject, tmp))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);
    }

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "listIngredients"))
	{
		CachedNetworkId stationId(Unicode::wideToNarrow(argv[1]));
		
		ManufactureInstallationObject * station = dynamic_cast<ManufactureInstallationObject *>(stationId.getObject());

		if (station == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (station->isActive())
		{
			result += getErrorMessage(argv[0], ERR_ACTIVE_MANF_STATION);
			return true;
		}

		ServerObject * hopper = station->getInputHopper();
		if (hopper == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		VolumeContainer * container = ContainerInterface::getVolumeContainer(*hopper);
		if (container == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		ContainerIterator iter = container->begin();
		while (iter != container->end())
		{
			const Container::ContainedItem & itemId = *iter;
			if (itemId.getObject() != nullptr)
			{
				const ServerObject * item = safe_cast<const ServerObject *>(itemId.getObject());
				result += Unicode::narrowToWide("(");
				result += Unicode::narrowToWide(itemId.getValueString());
				result += Unicode::narrowToWide(") - ");
				const ResourceContainerObject * crate = dynamic_cast<
					const ResourceContainerObject *>(item);
				if (crate != nullptr)
				{
					char buffer[32];
					_itoa(crate->getQuantity(), buffer, 10);
					result += Unicode::narrowToWide(buffer);
					result += Unicode::narrowToWide(" units of resource ");
					ResourceTypeObject const * const resourceType = crate->getResourceType();
					if (resourceType)
					{
						result += Unicode::narrowToWide(resourceType->getResourceName());
					}
					else
					{
						result += Unicode::narrowToWide("<unknown type>");
					}
				}
				else
				{
					result += item->getObjectName();
				}
				result += Unicode::narrowToWide("\n");
			}
			else
			{
				result += Unicode::narrowToWide("(");
				result += Unicode::narrowToWide(itemId.getValueString());
				result += Unicode::narrowToWide(") - unknown name\n");
			}

			++iter;
		}
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "addSchematic"))
    {
		CachedNetworkId stationId(Unicode::wideToNarrow(argv[1]));
		CachedNetworkId schematicId(Unicode::wideToNarrow(argv[2]));
		
		ManufactureInstallationObject * station = dynamic_cast<ManufactureInstallationObject *>(stationId.getObject());
		ManufactureSchematicObject * schematic = dynamic_cast<ManufactureSchematicObject *>(schematicId.getObject());

		if (station == nullptr || schematic == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (station->isActive())
		{
			result += getErrorMessage(argv[0], ERR_ACTIVE_MANF_STATION);
			return true;
		}

//		if (station->addSchematic(*schematic, playerObject))
		if (station->addSchematic(*schematic, nullptr))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);			
    }

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "removeSchematic"))
    {
		CachedNetworkId stationId(Unicode::wideToNarrow(argv[1]));
		
		ManufactureInstallationObject * station = dynamic_cast<ManufactureInstallationObject *>(stationId.getObject());

		if (station == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (station->isActive())
		{
			result += getErrorMessage(argv[0], ERR_ACTIVE_MANF_STATION);
			return true;
		}

		ManufactureSchematicObject * schematic = station->getSchematic();
		if (schematic != nullptr)
		{
			if (playerObject->getDatapad() == nullptr)
			{
				result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
				return true;
			}
			Container::ContainerErrorCode tmp = Container::CEC_Success;					
			if (ContainerInterface::transferItemToVolumeContainer(*playerObject->getDatapad(), *schematic, playerObject, tmp))
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			else
				result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);			
		}
		else
			result += getErrorMessage(argv[0], ERR_SUCCESS);
    }

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "listSchematic"))
    {
		CachedNetworkId stationId(Unicode::wideToNarrow(argv[1]));
		
		ManufactureInstallationObject * station = dynamic_cast<ManufactureInstallationObject *>(stationId.getObject());

		if (station == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (station->isActive())
		{
			result += getErrorMessage(argv[0], ERR_ACTIVE_MANF_STATION);
			return true;
		}

		ManufactureSchematicObject * schematic = station->getSchematic();
		if (schematic != nullptr)
		{
			result += Unicode::narrowToWide("(");
			result += Unicode::narrowToWide(schematic->getNetworkId().getValueString());
			result += Unicode::narrowToWide(") - ");
			result += schematic->getObjectName();
			result += Unicode::narrowToWide("\n");
		}
		else
			result += Unicode::narrowToWide("no schematic in station\n");
		result += getErrorMessage(argv[0], ERR_SUCCESS);
    }

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "start"))
    {
		CachedNetworkId stationId(Unicode::wideToNarrow(argv[1]));
		
		ManufactureInstallationObject * station = dynamic_cast<ManufactureInstallationObject *>(stationId.getObject());
		
		if (station == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			station->activate(NetworkId::cms_invalid);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
    }

	//-----------------------------------------------------------------

	else if (isAbbrev( argv [0], "stop"))
	{
		CachedNetworkId stationId(Unicode::wideToNarrow(argv[1]));

		ManufactureInstallationObject * station = dynamic_cast<ManufactureInstallationObject *>(stationId.getObject());

		if (station == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			station->deactivate();
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
	}

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "getObjects"))
    {
		ServerObject * myInventory = playerObject->getInventory();
		if (myInventory == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CachedNetworkId stationId(Unicode::wideToNarrow(argv[1]));

		ManufactureInstallationObject * station = dynamic_cast<ManufactureInstallationObject *>(stationId.getObject());

		if (station == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		ServerObject * hopper = station->getOutputHopper();
		if (hopper == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		VolumeContainer * container = ContainerInterface::getVolumeContainer(*hopper);
		if (container == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		ContainerIterator iter = container->begin();
		while (iter != container->end())
		{
			const Container::ContainedItem & itemId = *iter;
			if (itemId.getObject() != nullptr)
			{
				ContainerInterface::transferItemToVolumeContainer (*myInventory, *safe_cast<ServerObject *>(itemId.getObject()), playerObject, tmp);
			}
			else
			{
				result += Unicode::narrowToWide("No object for id ");
				result += Unicode::narrowToWide(itemId.getValueString());
				result += Unicode::narrowToWide("!\n");
			}

			++iter;
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
    }

	//-----------------------------------------------------------------

    else if (isAbbrev( argv [0], "listObjects"))
    {
		CachedNetworkId stationId(Unicode::wideToNarrow(argv[1]));

		ManufactureInstallationObject * station = dynamic_cast<ManufactureInstallationObject *>(stationId.getObject());

		if (station == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		ServerObject * hopper = station->getOutputHopper();
		if (hopper == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		VolumeContainer * container = ContainerInterface::getVolumeContainer(*hopper);
		if (container == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		ContainerIterator iter = container->begin();
		while (iter != container->end())
		{
			const Container::ContainedItem & itemId = *iter;
			if (itemId.getObject() != nullptr)
			{
				const ServerObject * item = safe_cast<const ServerObject *>(itemId.getObject());
				result += Unicode::narrowToWide("(");
				result += Unicode::narrowToWide(itemId.getValueString());
				result += Unicode::narrowToWide(") - ");
				result += item->getObjectName();
				result += Unicode::narrowToWide("\n");
			}
			else
			{
				result += Unicode::narrowToWide("(");
				result += Unicode::narrowToWide(itemId.getValueString());
				result += Unicode::narrowToWide(") - unknown name\n");
			}

			++iter;
		}		

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





