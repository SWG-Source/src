// ======================================================================
//
// PositionUpdateTracker.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PositionUpdateTracker.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverNetworkMessages/UpdateObjectPositionMessage.h"
#include "sharedFoundation/Scheduler.h"
#include "sharedLog/Log.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include <queue>
#include <map>

// ======================================================================

namespace PositionUpdateTrackerNamespace
{
	bool s_installed;
	Scheduler *s_positionUpdateTrackerScheduler;
	std::queue<ServerObject *> s_positionUpdateLongDelayCallbacksOrdered;
	std::queue<ServerObject *> s_positionUpdateShortDelayCallbacksOrdered;
	std::map<ServerObject *, ServerObject **> s_positionUpdateCallbackMap;

	bool isContainerConsideredPersisted(ServerObject const &obj, ServerObject const &container);
}
using namespace PositionUpdateTrackerNamespace;

// ======================================================================

void PositionUpdateTracker::install()
{
	FATAL(s_installed, ("PositionUpdateTracker::install - already installed"));
	s_installed = true;
	s_positionUpdateTrackerScheduler = new Scheduler;
}

// ----------------------------------------------------------------------

void PositionUpdateTracker::remove()
{
	FATAL(!s_installed, ("PositionUpdateTracker::remove - not installed"));
	delete s_positionUpdateTrackerScheduler;
	s_positionUpdateTrackerScheduler = 0;
	s_installed = false;
}

// ----------------------------------------------------------------------

void PositionUpdateTracker::positionChanged(ServerObject &obj)
{
	if (!s_installed)
		return;
	
// if we've already got a callback set, ignore the update
	std::map<ServerObject *, ServerObject **>::const_iterator i = s_positionUpdateCallbackMap.find(&obj);
	if (i != s_positionUpdateCallbackMap.end())
		return;

	if (!shouldSendPositionUpdate(obj))
		return;

	if (ConfigServerGame::getLogPositionUpdates())
	{
		LOG("PositionUpdate", ("Queueing %s for position update", obj.getNetworkId().getValueString().c_str()));
	}

	// Creature objects such as players may frequently update their position,
	// so we want to slow down how often we update the database
	if (obj.asCreatureObject())
	{
		// create the callback info
		s_positionUpdateLongDelayCallbacksOrdered.push(&obj);
		ServerObject *&p = s_positionUpdateLongDelayCallbacksOrdered.back();
		s_positionUpdateCallbackMap[&obj] = &p;

		// set the callback
		getScheduler().setCallback(handlePositionUpdateLongDelayCallback, 0, static_cast<unsigned long>(ConfigServerGame::getDatabasePositionUpdateLongDelayIntervalMs()));
	}
	else
	{
		// create the callback info
		s_positionUpdateShortDelayCallbacksOrdered.push(&obj);
		ServerObject *&p = s_positionUpdateShortDelayCallbacksOrdered.back();
		s_positionUpdateCallbackMap[&obj] = &p;

		// set the callback
		getScheduler().setCallback(handlePositionUpdateShortDelayCallback, 0, static_cast<unsigned long>(ConfigServerGame::getDatabasePositionUpdateShortDelayIntervalMs()));
	}
} //lint !e1764 // obj used in a nonconst manner

// ----------------------------------------------------------------------

void PositionUpdateTracker::flushPositionUpdate(ServerObject &obj)
{
	if (!s_installed)
		return;

	if (s_positionUpdateCallbackMap.find(&obj) != s_positionUpdateCallbackMap.end())
		sendPositionUpdate(obj);
}

// ----------------------------------------------------------------------

bool PositionUpdateTracker::shouldSendPositionUpdate(ServerObject const &obj)
{
	// position updates should be sent only for non-deleted authoritative objects that are persisted and whose immediate container, if any, is persisted

	if (!obj.isAuthoritative() || !obj.isPersisted() || obj.isBeingDestroyed())
		return false;

	ServerObject const   *containedByObj    = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(obj));
	CreatureObject const *creatureContainer = (containedByObj ? containedByObj->asCreatureObject() : nullptr);
	bool const objectIsRidingMount          = (creatureContainer && creatureContainer->isMountable());
	
	if (containedByObj && !isContainerConsideredPersisted(obj, *containedByObj) && !objectIsRidingMount)
		return false;

	return true;
}

// ----------------------------------------------------------------------

void PositionUpdateTracker::sendPositionUpdate(ServerObject &obj)
{
	if (!s_installed)
		return;

	std::map<ServerObject *, ServerObject **>::iterator i = s_positionUpdateCallbackMap.find(&obj);
	if (i != s_positionUpdateCallbackMap.end())
	{
		*((*i).second) = 0;
		s_positionUpdateCallbackMap.erase(i);
	}

	if (!shouldSendPositionUpdate(obj))
		return;

	if (ConfigServerGame::getLogPositionUpdates())
	{
		LOG("PositionUpdate", ("Sending position update for %s", obj.getNetworkId().getValueString().c_str()));
	}

	NetworkId const loadWithId = ContainerInterface::getLoadWithContainerId(obj);

	//-- Determine Object container's network id and Object's arrangement as far as the Database is concerned.
	NetworkId containerNetworkIdForDatabase = ContainerInterface::getContainedByProperty(obj)->getContainedByNetworkId();
	int objectArrangementForDatabase = ContainerInterface::getSlottedContainmentProperty(obj)->getCurrentArrangement();
	Transform transformForDatabase(obj.getTransform_o2p());
	Transform worldspaceTransformForDatabase(obj.getTransform_o2w());

	// if it's a player inside a container, the world space coordinate
	// that should be saved is the world space coordinate of the topmost
	// container; this way, player logging out of a house will get
	// loaded back into the same game server as the house, thus maximizing
	// that chance that the player will loaded back into the house
	if (obj.isPlayerControlled() && obj.asCreatureObject())
	{
		Object const *containerObj = ContainerInterface::getTopmostContainer(obj);
		if (containerObj && (containerObj != &obj))
		{
			worldspaceTransformForDatabase = containerObj->getTransform_o2w();
		}
	}

	// Ships should be persisted in their ship control device if they have one
	ShipObject const * const ship = obj.asShipObject();
	if (ship)
	{
		ServerObject const * const shipControlDevice = ship->getControlDevice();
		if (shipControlDevice)
		{
			containerNetworkIdForDatabase = shipControlDevice->getNetworkId();
			objectArrangementForDatabase = -1;
			transformForDatabase.resetRotateTranslate_l2p();
		}
	}
	else
	{
		ShipObject const * const containingShip = ShipObject::getContainingShipObject(&obj);

		if (obj.isPlayerControlled())
		{
			if (containingShip)
			{
				// Player controlled objects in ships persist at the location of the ship.
				// Note: we need a more general solution if we decide to allow ships in interiors of ships.
				containerNetworkIdForDatabase = ContainerInterface::getContainedByProperty(*containingShip)->getContainedByNetworkId();
				objectArrangementForDatabase = -1;
				transformForDatabase = containingShip->getTransform_o2p();
			}
			else
			{
				ServerObject const * const container = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(obj));
				CreatureObject const * const creatureContainer = (container ? container->asCreatureObject() : nullptr);
				if (creatureContainer && creatureContainer->isMountable())
				{
					// Riders of mounts persist at the location of the mount
					containerNetworkIdForDatabase = ContainerInterface::getContainedByProperty(*creatureContainer)->getContainedByNetworkId();
					objectArrangementForDatabase = -1;
					transformForDatabase = creatureContainer->getTransform_o2p();
				}
			}
		}
	}

	// Make sure we aren't about to try to save an invalid containment/load_with state
	if (containerNetworkIdForDatabase == NetworkId::cms_invalid && loadWithId != obj.getNetworkId())
	{
		WARNING_DEBUG_FATAL(true, ("Tried to send a position update to the db for object %s with load_with=%s but contained_by=0!", obj.getNetworkId().getValueString().c_str(), loadWithId.getValueString().c_str()));
		obj.unload();
		return;
	}

	// Make sure we aren't about to try to save a persisted non-player in a space scene away from the origin
	FATAL(ServerWorld::isSpaceScene() && !obj.isPlayerControlled() && !containerNetworkIdForDatabase.isValid() && transformForDatabase.getPosition_p() != Vector::zero, ("Tried to send a non-origin persisted object position to the db in a space scene for non-player object %s", obj.getDebugInformation().c_str()));

	//Vector vp = transformForDatabase.getPosition_p();
	//Vector vw = obj.getTransform_o2w().getPosition_p();
	//LOG("PositionUpdateTracker",("sendPositionUpdate PS XYZ (%f %f %f)   WS XYZ (%f %f %f)",vp.x, vp.y, vp.z, vw.x, vw.y, vw.z));

	UpdateObjectPositionMessage const positionMessage(
		obj.getNetworkId(),
		transformForDatabase,
		worldspaceTransformForDatabase,
		containerNetworkIdForDatabase,
		objectArrangementForDatabase,
		loadWithId,
		obj.isPlayerControlled(),
		obj.asCreatureObject() ? true : false);

	GameServer::getInstance().sendToDatabaseServer(positionMessage);

	if (loadWithId != obj.getLoadWith())
	{
		obj.setLoadWith(loadWithId);
		Container * const container = ContainerInterface::getContainer(obj);
		if (container)
		{
			for (ContainerIterator j = container->begin(); j != container->end(); ++j)
			{
				ServerObject * const content = safe_cast<ServerObject *>((*j).getObject());
				if (content)
					sendPositionUpdate(*content);
			}
		}
	}
}

// ----------------------------------------------------------------------

void PositionUpdateTracker::serverObjDestroyed(ServerObject &obj)
{
	if (!s_installed)
		return;

	std::map<ServerObject *, ServerObject **>::iterator i = s_positionUpdateCallbackMap.find(&obj);
	if (i != s_positionUpdateCallbackMap.end())
	{
		*((*i).second) = 0;
		s_positionUpdateCallbackMap.erase(i);
	}
} //lint !e1764 // obj used in a nonconst manner

// ----------------------------------------------------------------------

void PositionUpdateTracker::handlePositionUpdateLongDelayCallback(void const *)
{
	if (!s_installed)
		return;

	ServerObject *&obj = s_positionUpdateLongDelayCallbacksOrdered.front();
	if (obj)
		flushPositionUpdate(*obj);
	s_positionUpdateLongDelayCallbacksOrdered.pop();
}

// ----------------------------------------------------------------------

void PositionUpdateTracker::handlePositionUpdateShortDelayCallback(void const *)
{
	if (!s_installed)
		return;

	ServerObject *&obj = s_positionUpdateShortDelayCallbacksOrdered.front();
	if (obj)
		flushPositionUpdate(*obj);
	s_positionUpdateShortDelayCallbacksOrdered.pop();
}

// ----------------------------------------------------------------------

Scheduler &PositionUpdateTracker::getScheduler()
{

	FATAL(!s_positionUpdateTrackerScheduler, ("No PositionUpdateTracker scheduler found"));
	return *s_positionUpdateTrackerScheduler;
}

// ----------------------------------------------------------------------

bool PositionUpdateTrackerNamespace::isContainerConsideredPersisted(ServerObject const &obj, ServerObject const &container)
{
	// For purposes of the position update tracker, consider players in buildout area containers to be persisted,
	// and also consider all buildout bazaar terminals (crc 0x8c525205) to be persisted to handle the case of the
	// Restuss bazaar terminals which are buildout which are not persisted, so when a player places an item on those
	// bazaar terminals, we need to update the database that the items are now contained inside the bazaar terminals
	return container.isPersisted() || (obj.isPlayerControlled() && container.getNetworkId() < NetworkId::cms_invalid) || (container.getObjectTemplate() && (container.getObjectTemplate()->getCrcName().getCrc() == 0x8c525205));
}

// ======================================================================

