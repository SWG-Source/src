// ============================================================================
//
// ServerAsteroidManager.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
//
// ============================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerAsteroidManager.h"

#include "sharedCollision/Extent.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/AsteroidGenerationManager.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedGame/AsteroidGenerationManager.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Controller.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/RotationDynamics.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerWorld.h"

#include <set>
#include <vector>

// ============================================================================

namespace ServerAsteroidManagerNamespace
{
	float const cms_timeBetweenUpdates = 10.0f;
	float ms_timeSinceLastUpate = 0.0f;
	bool ms_installed = false;
}

using namespace ServerAsteroidManagerNamespace;

// ============================================================================

ServerAsteroidManager::FieldHandle const ServerAsteroidManager::BAD_HANDLE = -1;
ServerAsteroidManager::FieldHandle ServerAsteroidManager::s_nextHandle;
std::set<NetworkId> ServerAsteroidManager::ms_playersListeningForServerAsteroidDebugData;
std::vector<NetworkId> ServerAsteroidManager::ms_asteroids;


// ============================================================================

void ServerAsteroidManager::install()
{
	DEBUG_FATAL(ms_installed, ("ServerAsteroidManager::install() - Already installed."));

	ms_installed = true;

	s_nextHandle = 0;

	AsteroidGenerationManager::registerGetExtentRadiusFunction(getExtentRadius);

	ms_playersListeningForServerAsteroidDebugData.clear();
	ms_asteroids.clear();

	ExitChain::add(ServerAsteroidManager::remove, "ServerAsteroidManager::remove", 0, false);
}

//-----------------------------------------------------------------------------

void ServerAsteroidManager::remove()
{
	ms_playersListeningForServerAsteroidDebugData.clear();
	ms_asteroids.clear();
	ms_installed = false;
}

//-----------------------------------------------------------------------------

ServerAsteroidManager::FieldHandle ServerAsteroidManager::getBadHandle()
{
	DEBUG_FATAL(!ms_installed, ("ServerAsteroidManager not installed"));
	return BAD_HANDLE;
}

//-----------------------------------------------------------------------------

/** Create a set of server objects for an asteroid field with the given seed data.
    The same seed data will always generate the same field
*/
ServerAsteroidManager::FieldHandle ServerAsteroidManager::generateField(AsteroidGenerationManager::AsteroidFieldData const & fieldData)
{
	if(!ms_installed)
	{
		DEBUG_FATAL (true, ("ServerAsteroidManager not installed"));
		return BAD_HANDLE;
	}

	std::vector<AsteroidGenerationManager::AsteroidData> asteroidDatas;
	bool const result = AsteroidGenerationManager::generateField(fieldData, asteroidDatas);

	if(!result)
	{
		DEBUG_FATAL(true, ("ServerAsteroidManager::generateField problem occured generating field"));
		return BAD_HANDLE;
	}

	if(asteroidDatas.size() != static_cast<unsigned int>(fieldData.count))
	{
		DEBUG_FATAL(true, ("ServerAsteroidManager::generateField should have generated %d asteroids, but we have %d instead", fieldData.count, asteroidDatas.size()));
		return BAD_HANDLE;
	}

	ServerObject * newAsteroid = nullptr;

	//TODO disable server-rotation for now, it apparently spams the client horribly
//	RotationDynamics * rotationDynamics = nullptr;

	for(std::vector<AsteroidGenerationManager::AsteroidData>::iterator i = asteroidDatas.begin(); i != asteroidDatas.end(); ++i)
	{
		Transform tr;
		tr.setPosition_p((*i).position);
		tr.yaw_l((*i).orientation.x);
		tr.pitch_l((*i).orientation.y);
		tr.roll_l((*i).orientation.z);
		newAsteroid = ServerWorld::createNewObject(i->templateCrc, tr, 0, false);

		if(!newAsteroid)
			continue;

		//world cell, so add to world
		newAsteroid->addToWorld();

		newAsteroid->setScale(Vector(i->scale, i->scale, i->scale));

//		rotationDynamics = new RotationDynamics (newAsteroid, i->rotationVector);
//		rotationDynamics->setState (true);
//		newAsteroid->setDynamics (rotationDynamics);

		IGNORE_RETURN(newAsteroid->setObjVarItem("intNoDump", 1));

		ms_asteroids.push_back(newAsteroid->getNetworkId());
	}

	return s_nextHandle++; //lint !e429 rotationDynamics not released (we don't own it)
}

//-----------------------------------------------------------------------------

/** Generate the static asteroid fields for a given scene
*/
void ServerAsteroidManager::setupStaticFields(std::string const & sceneName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL (true, ("ServerAsteroidManager not installed"));
		return;
	}

	AsteroidGenerationManager::loadStaticFieldDataForScene(sceneName, true);
	std::vector<AsteroidGenerationManager::AsteroidFieldData> const & data = AsteroidGenerationManager::getDataForScene(sceneName);
	for(std::vector<AsteroidGenerationManager::AsteroidFieldData>::const_iterator i = data.begin(); i != data.end(); ++i)
	{
		FieldHandle const handle = generateField(*i);
		UNREF(handle);
	}
}

//-----------------------------------------------------------------------------

/** Add the player to a list of players that gets send the server asteroid data.
    Send the player an immediate update.
*/
void ServerAsteroidManager::listenforServerAsteroidDebugData(NetworkId const & player)
{
	UNREF(player);

#ifdef _DEBUG
	if(!ms_installed)
	{
		DEBUG_FATAL (true, ("ServerAsteroidManager not installed"));
		return;
	}

	if(ConfigServerGame::getSendAsteroidExtents())
	{
		IGNORE_RETURN(ms_playersListeningForServerAsteroidDebugData.insert(player));
		//send an immediate update
		std::vector<Sphere> spheres;
		getServerAsteroidData(spheres);
		sendServerAsteroidDataToPlayer(player, spheres);
	}
#endif
}

//-----------------------------------------------------------------------------

/** Remove the player from a list of players that gets send the server asteroid data
*/
void ServerAsteroidManager::endListenforServerAsteroidDebugData(NetworkId const & player)
{
	UNREF(player);

#ifdef _DEBUG
	if(!ms_installed)
	{
		DEBUG_FATAL (true, ("ServerAsteroidManager not installed"));
		return;
	}

	if(ConfigServerGame::getSendAsteroidExtents())
	{
		IGNORE_RETURN(ms_playersListeningForServerAsteroidDebugData.erase(player));
		//send an empty update to the client to clear out their data
		std::vector<Sphere> spheres;
		sendServerAsteroidDataToPlayer(player, spheres);
	}
#endif
}

//-----------------------------------------------------------------------------

/** If enough time has elapsed, send an update of the server asteroid data to the appropriate players
*/
void ServerAsteroidManager::update(float elapsedTime)
{
	UNREF(elapsedTime);

#ifdef _DEBUG
	if(!ms_installed)
	{
		DEBUG_FATAL (true, ("ServerAsteroidManager not installed"));
		return;
	}

	if(ConfigServerGame::getSendAsteroidExtents())
	{
		ms_timeSinceLastUpate += elapsedTime;
		if(ms_timeSinceLastUpate >= cms_timeBetweenUpdates)
		{
			if(!ms_playersListeningForServerAsteroidDebugData.empty())
			{
				std::vector<Sphere> spheres;
				getServerAsteroidData(spheres);
				for(std::set<NetworkId>::iterator i2 = ms_playersListeningForServerAsteroidDebugData.begin(); i2 != ms_playersListeningForServerAsteroidDebugData.end(); ++i2)
				{
					sendServerAsteroidDataToPlayer(*i2, spheres);
				}
			}
			ms_timeSinceLastUpate = 0.0f;
		}
	}
#endif
}

//-----------------------------------------------------------------------------

/** Build and return a vector of spheres corresponding to the server asteroids
*/
void ServerAsteroidManager::getServerAsteroidData(std::vector<Sphere> & /*OUT*/ spheres)
{
	if(!ms_installed)
	{
		DEBUG_FATAL (true, ("ServerAsteroidManager not installed"));
		return;
	}

	spheres.clear();
	for(std::vector<NetworkId>::iterator i = ms_asteroids.begin(); i != ms_asteroids.end(); ++i)
	{
		Object const * const o = NetworkIdManager::getObjectById(*i);
		ServerObject const * const so = o ? o->asServerObject() : nullptr;
		if(so)
		{
			spheres.push_back(so->getSphereExtent());
		}
	}
}

//-----------------------------------------------------------------------------

/** Send a set of server asteroid data to the given player
*/
void ServerAsteroidManager::sendServerAsteroidDataToPlayer(NetworkId const & player, std::vector<Sphere> const & spheres)
{
	if(!ms_installed)
	{
		DEBUG_FATAL (true, ("ServerAsteroidManager not installed"));
		return;
	}

	MessageQueueGenericValueType<std::vector<Sphere> > * const msg = new MessageQueueGenericValueType<std::vector<Sphere> >(spheres);

	Object * const o = NetworkIdManager::getObjectById(player);
	ServerObject * const so = o ? o->asServerObject() : nullptr;
	CreatureObject * const co = so ? so->asCreatureObject() : nullptr;
	Client const * const client = co ? co->getClient() : nullptr;
	if(client && co && co->isAuthoritative())
	{
		co->getController()->appendMessage(static_cast<int>(CM_serverAsteroidDebugData), 0.0f, msg,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}


//-----------------------------------------------------------------------------

float ServerAsteroidManager::getExtentRadius(uint32 serverTemplateCrc)
{
	float radius = 0.0f;
	ObjectTemplate const * const ot = ObjectTemplateList::fetch(serverTemplateCrc);
	if(ot)
	{
		ServerObjectTemplate const * const serverOT = ot->asServerObjectTemplate();
		if(serverOT)
		{
			std::string const sharedTemplateName = serverOT->getSharedTemplate();
			ObjectTemplate const * const ot2 = ObjectTemplateList::fetch(sharedTemplateName);
			if(ot2)
			{
				SharedObjectTemplate const * const sharedOT = ot2->asSharedObjectTemplate();
				if(sharedOT)
				{
					AppearanceTemplate const * const at = AppearanceTemplateList::fetch(sharedOT->getAppearanceFilename().c_str());
					if(at)
					{
						Extent const * const extent = at->getCollisionExtent();
						if(extent)
						{
							radius = extent->getSphere().getRadius();
							if (radius==0.0f)
							{
								DEBUG_WARNING(true,("Data problem:  Object template %s has radius 0",
													ObjectTemplateList::lookUp(serverTemplateCrc).getString()));
								radius=5.0f;
							}
						}
						AppearanceTemplateList::release(at);
					}
				}
				ot2->releaseReference();
			}
		}
		ot->releaseReference();
	}

	return radius;
}

// ============================================================================
