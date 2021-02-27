// ======================================================================
//
// ConsoleCommandParserResource.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserResource.h"

#include "UnicodeUtils.h"
#include "serverGame/HarvesterInstallationObject.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/SurveySystem.h"
#include "serverGame/CreatureObject.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "swgServerNetworkMessages/MessageQueueActivateInstallation.h"
#include "swgServerNetworkMessages/MessageQueueDeactivateInstallation.h"
#include "swgServerNetworkMessages/MessageQueueInstallationHarvest.h"

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"activate",            2,"<harvester oid> <resource name>","Activates a harvester."},
	{"addtocontainer",      4,"<container oid> <resource name> <quantity> <source player id>","Add resources to a container."},
	{"fillcontainer",       1,"<container oid> [source player id]", "Fills a resource container with its resource."},
	{"recycle",             1,"<container oid>", "Convert a container of resource to the recycled version (for debugging)."},
	{"deactivate",          1,"<harvester oid>","Deactivates a harvester."},
	{"discardHopper",       1,"<harvester>","Empties the contents of a harvester's hopper."},
	{"getSurveyList",       2,"<resource class name> <surveytoolId>","For testing surveys, get the resource list for the survey screen."},
	{"harvest",             1,"<harvester oid>","Causes a harvester to collect resources."},
	{"name",                2,"<resource type oid> <name>","Set the name of a resource."},
	{"pools",               1,"<planet>","List all resource pools on a particular planet."},
	{"removefromcontainer", 3,"<container oid> <resource name> <quantity>","Remove resources from a container."},
	{"survey",              1,"<resource class name> <resource type name> <survey range> <number of points>","For testing surveys, take a survey."},
	{"tree",                0,"","List resource hierarchy."},
	{"viewcontainer",       1,"<container object id>","View the contents of a resource container."},
	{"deplete",             1,"<resource type oid>","Cause a resource type to deplete."},
	{"",                    0, "", ""} // this must be last
};


//-----------------------------------------------------------------

ConsoleCommandParserResource::ConsoleCommandParserResource (void) :
CommandParser ("resource", 0, "...", "Resource related commands.", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------


bool ConsoleCommandParserResource::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
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

	if (isCommand( argv [0], "activate"))
	{
		CachedNetworkId harvester(Unicode::wideToNarrow (argv[1]));

		ServerObject* harvesterObject = safe_cast<ServerObject*>(harvester.getObject());
		if (harvesterObject == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}

		Controller *const controller = harvesterObject->getController();
		NOT_NULL(controller);
		controller->appendMessage(static_cast<int>(CM_activateInstallation),0.0f,new MessageQueueActivateInstallation(), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
		
	else if (isCommand( argv [0], "harvest"))
	{
		CachedNetworkId harvester(Unicode::wideToNarrow (argv[1]));

		ServerObject* harvesterObject = safe_cast<ServerObject*>(harvester.getObject());
		if (harvesterObject == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}
			//TODO:  Make all of these into function calls instead of sending messages

		Controller *const controller = harvesterObject->getController();
		NOT_NULL(controller);
		controller->appendMessage(static_cast<int>(CM_installationHarvest),0.0f,new MessageQueueInstallationHarvest(), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER); 
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	
	else if (isCommand( argv [0], "discardHopper"))
	{
		CachedNetworkId harvester(Unicode::wideToNarrow (argv[1]));
		
		HarvesterInstallationObject* harvesterObject = dynamic_cast<HarvesterInstallationObject*>(harvester.getObject());
		if (!harvesterObject)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}
		harvesterObject->discardAllHopperContents ();

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	else if (isCommand( argv [0], "deactivate"))
	{
		CachedNetworkId harvester(Unicode::wideToNarrow (argv[1]));

		ServerObject* harvesterObject = safe_cast<ServerObject*>(harvester.getObject());
		if (harvesterObject == nullptr)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}
		Controller *const controller = harvesterObject->getController();
		NOT_NULL(controller);
		controller->appendMessage(static_cast<int>(CM_deactivateInstallation),0.0f,new MessageQueueDeactivateInstallation(), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	else if (isCommand( argv [0], "tree"))
	{
		ServerUniverse & universe = ServerUniverse::getInstance();
		ResourceClassObject * root = 0;

		if (argv.size () > 1)
			root = universe.getResourceClassByName (Unicode::wideToNarrow (argv [1]));
		else
			root = universe.getResourceTreeRoot();

		if (root)
		{
			std::string temp;
			root->debugOutput(temp);
			result += Unicode::narrowToWide(temp);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
		
	}
	
	else if (isCommand( argv [0], "pools"))
	{
		PlanetObject* planet=ServerUniverse::getInstance().getPlanetByName(Unicode::wideToNarrow(argv[1]));
		if (planet)
		{
			std::string temp;
			planet->debugOutputPools(temp);
			result += Unicode::narrowToWide(temp);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);	
		}
	}

	else if (isCommand( argv [0], "name"))
	{
		NetworkId resourceId(Unicode::wideToNarrow (argv[1]));
		ResourceTypeObject * const resource = ServerUniverse::getInstance().getResourceTypeById(resourceId);
		if (resource)
		{
			resource->setName(Unicode::wideToNarrow(argv[2]));
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);	
		}
	}

	else if (isCommand( argv [0], "viewcontainer"))
	{
		CachedNetworkId contId (Unicode::wideToNarrow (argv[1]));
		ResourceContainerObject* container=dynamic_cast<ResourceContainerObject*>(contId.getObject());
		if (container)
		{
			result += Unicode::narrowToWide(container->debugPrint()+'\n');
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);	
		}
	}

	else if (isCommand( argv [0], "addtocontainer"))
	{
		NetworkId const contId (Unicode::wideToNarrow (argv[1]));
		ResourceContainerObject * const container = dynamic_cast<ResourceContainerObject*>(NetworkIdManager::getObjectById(contId));
		std::string const & resourcePath = Unicode::wideToNarrow(argv[2]);
		ResourceTypeObject * const resType = ServerUniverse::getInstance().getResourceTypeByName(resourcePath);
		int const amount = strtol(Unicode::wideToNarrow (argv[3]).c_str (), nullptr, 10);
		NetworkId const source(Unicode::wideToNarrow (argv[4]));
		
		if (container && resType)
		{
			if (container->addResource(CachedNetworkId(resType->getNetworkId()),amount, source))
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			else
				result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);
		}
		else
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);	
		}
	}

	else if (isCommand( argv [0], "fillcontainer"))
	{
		NetworkId contId (Unicode::wideToNarrow (argv[1]));
		ResourceContainerObject* container=dynamic_cast<ResourceContainerObject*>(NetworkIdManager::getObjectById(contId));
		if (container != nullptr)
		{
			NetworkId sourceId;
			if (argv.size() >= 3)
				sourceId = NetworkId(Unicode::wideToNarrow (argv[2]));

			if (container->addResource(container->getResourceType(), 
				container->getMaxQuantity() - container->getQuantity(), 
				sourceId))
			{
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
			else
				result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);
		}
		else
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);	
	}

	else if (isCommand( argv [0], "recycle"))
	{
		NetworkId contId (Unicode::wideToNarrow (argv[1]));
		ResourceContainerObject* container=dynamic_cast<ResourceContainerObject*>(NetworkIdManager::getObjectById(contId));
		if (container != nullptr)
		{
			if (container->debugRecycle())
					result += getErrorMessage(argv[0], ERR_SUCCESS);
			else
				result += Unicode::narrowToWide("The resource in the container could not be recycled.\n");
		}
		else
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);	
	}

	else if (isCommand( argv [0], "removefromcontainer"))
	{
		CachedNetworkId contId (Unicode::wideToNarrow (argv[1]));
		ResourceContainerObject* container=dynamic_cast<ResourceContainerObject*>(contId.getObject());
		ResourceTypeObject *resType=ServerUniverse::getInstance().getResourceTypeByName(Unicode::wideToNarrow(argv[2]));
		int amount=strtol(Unicode::wideToNarrow (argv[3]).c_str (), nullptr, 10);
		
		if (container && resType)
		{
			NetworkId sourcePlayer(NetworkId::cms_invalid);

			typedef std::vector<std::pair<NetworkId, int> > SourcesType;
			SourcesType sources;
			if (container->removeResource(resType->getNetworkId(),amount,&sources))
			{
				char buffer[50];
				for(SourcesType::iterator i=sources.begin(); i!=sources.end(); ++i)
				{
					IGNORE_RETURN(snprintf(buffer,50,"%i",i->second));
					result += Unicode::narrowToWide(std::string("Removed ") + buffer + " resources harvested by player " + i->first.getValueString() + '\n');
				}
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
			else
				result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);
		}
		else
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);	
		}
	}

	else if (isCommand( argv [0], "getSurveyList"))
	{
		std::string parentResourceClassName(Unicode::wideToNarrow(argv[1]));
		CachedNetworkId tool(Unicode::wideToNarrow (argv[2]));

		SurveySystem::getInstance().requestResourceListForSurvey(userId, tool, parentResourceClassName);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	
	else if (isCommand( argv [0], "survey"))
	{
		const std::string       parentResourceClassName   (Unicode::wideToNarrow(argv[1]));
		const std::string       resourceTypeName          (Unicode::wideToNarrow(argv[2]));
		const int               surveyRange             = strtol(Unicode::wideToNarrow(argv[3]).c_str(),nullptr,10);
		const int               numPoints               = strtol(Unicode::wideToNarrow(argv[4]).c_str(),nullptr,10);
		const Object *          player                  = NetworkIdManager::getObjectById(userId);

		if (player)
		{
			SurveySystem::getInstance().requestSurvey(userId, parentResourceClassName, resourceTypeName, player->getPosition_w(), surveyRange, numPoints);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage(argv [0], ERR_INVALID_OBJECT);
	}

	else if (isCommand( argv [0], "deplete"))
	{
		NetworkId resourceId (Unicode::wideToNarrow (argv[1]));
		if (ServerUniverse::getInstance().manualDepleteResource(resourceId))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);	
	}
	
	return true;
}	// ConsoleCommandParserResource::performParsing

// ======================================================================
