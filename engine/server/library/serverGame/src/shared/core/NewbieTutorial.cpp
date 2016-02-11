// ======================================================================
//
// NewbieTutorial.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/NewbieTutorial.h"

#include "serverGame/ContainerInterface.h"
#include "serverGame/ServerObject.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedFile/FileManifest.h"
#include "sharedFile/TreeFile.h"
#include "sharedLog/Log.h"
#include "sharedObject/Container.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/PortalProperty.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"

// ======================================================================

namespace NewbieTutorialNamespace
{
	const std::string        s_sceneId("tutorial");
	const std::string        s_tutorialTemplate("object/building/general/npe_hangar_1.iff");

	const Vector             s_startCoords(-12.5f, 0.0f, 19.5f);
	const std::string        s_startCellName("medicalroom");

	const float              s_tutorialMapWidth(16384.0f);
	const float              s_tutorialSpacing(512.0f);
	const int                s_sqrtMaxTutorials(static_cast<int>(s_tutorialMapWidth/s_tutorialSpacing));

	const std::string        s_tutorialObjVar("npe.phase_number");
	const std::string        s_skipTutorialObjVar("npe.skippingTutorial");
	const int                s_tutorialResetThreshold(2);

	const std::string        s_freeTrialPlanets [] = {"tutorial", "space_npe_falcon", "space_ord_mantell"};
	const int                s_numFreeTrialPlanets = sizeof (s_freeTrialPlanets) / sizeof (s_freeTrialPlanets[0]);

	const std::string        s_freeTrialBuildoutPlanet = "dungeon1";

	const std::string        s_freeTrialBuildoutAreas [] = {"npe_shared_station", "npe_dungeon"};
	const int                s_numFreeTrialBuildoutAreas = sizeof (s_freeTrialBuildoutAreas) / sizeof (s_freeTrialBuildoutAreas[0]);
}

using namespace NewbieTutorialNamespace;

std::string const &NewbieTutorial::getSceneId()
{
	return s_sceneId;
}

// ----------------------------------------------------------------------

std::string const &NewbieTutorial::getTutorialTemplateName()
{
	return s_tutorialTemplate;
}

// ----------------------------------------------------------------------

Vector NewbieTutorial::getTutorialLocation()
{
	// Pick a random spot, keep them 512m apart, and don't let them get too close to either axis because we are
	// now using them as server boundaries for multi-server
	float x, z;
	do
	{
		x = s_tutorialSpacing*Random::random(s_sqrtMaxTutorials-1) - s_tutorialMapWidth/2.0f;
		z = s_tutorialSpacing*Random::random(s_sqrtMaxTutorials-1) - s_tutorialMapWidth/2.0f;
	} while (std::abs(x) < 300.0f || std::abs(z) < 300.0f);
	return Vector(x, 0.0f, z);
}

// ----------------------------------------------------------------------

ServerObject *NewbieTutorial::createTutorial(Vector const &location)
{
	FATAL(ServerWorld::getSceneId() != s_sceneId, ("Tried to create a character on a non-tutorial server."));

	Transform tr;
	tr.setPosition_p(location);
	ServerObject *tutorial = ServerWorld::createNewObject(
		s_tutorialTemplate,
		tr,
		0,
		false);
	if (tutorial)
		tutorial->addToWorld();
	return tutorial;
}

// ----------------------------------------------------------------------

Vector const &NewbieTutorial::getStartCoords()
{
	return s_startCoords;
}

// ----------------------------------------------------------------------

std::string NewbieTutorial::getStartCellName()
{
	return s_startCellName;
}

// ----------------------------------------------------------------------

void NewbieTutorial::setupCharacterForTutorial(ServerObject* character)
{
	if (character)
		character->setObjVarItem(s_tutorialObjVar, 1);
}

// ----------------------------------------------------------------------

void NewbieTutorial::setupCharacterToSkipTutorial(ServerObject* character)
{
	if (character)
		character->setObjVarItem(s_skipTutorialObjVar, 1);
}

// ----------------------------------------------------------------------

bool NewbieTutorial::shouldStartTutorial(const ServerObject* character)
{
	if (character)
	{
		if (character->getObjVars().hasItem(s_tutorialObjVar))
		{
			int npePhase = 0;
			character->getObjVars().getItem(s_tutorialObjVar, npePhase);

			// if the player is in the shared space station or farther, don't restart the tutorial for them
			if (npePhase <= s_tutorialResetThreshold)
				return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool NewbieTutorial::isInTutorial(const ServerObject* character)
{
	if (character)
		if (character->getObjVars().hasItem(s_tutorialObjVar))
			return true;

	return false;
}

// ----------------------------------------------------------------------

bool NewbieTutorial::isInTutorialArea(const ServerObject* character)
{
	if (character)
	{
		std::string scene = ServerWorld::getSceneId();

		if (isFreeTrialScene(scene))
			return true;

		Vector position = character->getPosition_w();

		if (isInFreeTrialBuildoutArea(scene, position))
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool NewbieTutorial::isFreeTrialScene(const std::string &scene)
{
	for (int i = 0; i < s_numFreeTrialPlanets; ++i)
	{
		// if the scene matches the name, return true
		if (scene.compare(s_freeTrialPlanets[i]) == 0)
			return true;
		//otherwise, if the scene name is <scene name>_<num>, we return true (multiple zones)
		else if (scene.find(s_freeTrialPlanets[i] + "_", 0) == 0 && scene.length() == (s_freeTrialPlanets[i].length() + 2))
			return true;
	}
	return false;
}

// ----------------------------------------------------------------------

bool NewbieTutorial::isInFreeTrialBuildoutArea(const std::string &scene, const Vector &location)
{
	if (scene.substr(0, s_freeTrialBuildoutPlanet.length()) == s_freeTrialBuildoutPlanet)
	{
		const BuildoutArea *currentBuildout = SharedBuildoutAreaManager::findBuildoutAreaAtPosition(scene.c_str(), location.x, location.z, false);

		if (currentBuildout)
		{
			std::string areaName = currentBuildout->areaName;
			// if the scene matches a buildout area we know about, return true
			for (int i = 0; i < s_numFreeTrialBuildoutAreas; ++i)
				if (areaName.compare(s_freeTrialBuildoutAreas[i]) == 0)
					return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

void NewbieTutorial::stripNonFreeAssetsFromPlayerInTutorial(const CreatureObject* character)
{
	if (!character)
		return;

	std::vector<ServerObject *> objectsToDelete;
	const SlottedContainer * const equipmentContainer = ContainerInterface::getSlottedContainer(*character);

	if (equipmentContainer)
		getNonFreeObjectsForDeletion(equipmentContainer, objectsToDelete, character);
	else
		LOG("npe", ("Scanning player %s to stripNonFreeAssets from because they are in a tutorial, but character had no equipment container\n", character->getNetworkId().getValueString().c_str()));

	const ServerObject * inventoryObject = character->getInventory();
	const VolumeContainer * const inventoryContainer = ContainerInterface::getVolumeContainer(*inventoryObject);

	if (inventoryContainer)
		getNonFreeObjectsForDeletion(inventoryContainer, objectsToDelete, character);
	else
		LOG("npe", ("Scanning player %s to stripNonFreeAssets from because they are in a tutorial, but character had no inventory container\n", character->getNetworkId().getValueString().c_str()));

	for (std::vector<ServerObject *>::iterator i = objectsToDelete.begin(); i != objectsToDelete.end(); ++i)
		if (*i)
			(*i)->permanentlyDestroy(DeleteReasons::Player);
}

// ----------------------------------------------------------------------

void NewbieTutorial::getNonFreeObjectsForDeletion(const Container* const container, std::vector<ServerObject *>& objectsToDelete, const CreatureObject* character)
{
	if (!container)
		return;

	for (ContainerConstIterator i(container->begin()); i != container->end(); ++i)
	{
		const CachedNetworkId & itemId = *i;
		ServerObject * item = safe_cast<ServerObject *>(itemId.getObject());
		if (item != nullptr)
		{
			TangibleObject *itemTangible = item->asTangibleObject();
			if (itemTangible != nullptr)
			{
				const char *templateName = itemTangible->getSharedTemplateName();
				if (!FileManifest::contains(templateName))
				{
					objectsToDelete.push_back(item);
					DEBUG_LOG("NewbieTutorialObjectFilter", ("Removing object %s (%s) from player %s because the template is not in the skufree manifest\n", templateName, item->getNetworkId().getValueString().c_str(), character->getNetworkId().getValueString().c_str()));
					LOG("CustomerService", ("Removing object %s (%s) from player %s because the template is not in the skufree manifest\n", templateName, item->getNetworkId().getValueString().c_str(), character->getNetworkId().getValueString().c_str()));
				}
				else
				{
					// see if the item is a container and go through its contents
					const Container * const itemContainer = ContainerInterface::getContainer(*item);
					if (itemContainer != nullptr)
						getNonFreeObjectsForDeletion(itemContainer, objectsToDelete, character);
				}
			}
		}
	}
}

// ======================================================================

