// ======================================================================
//
// ObserveTracker.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ObserveTracker.h"

#include "serverGame/BaselineDistributionListPool.h"
#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DestroyMessageManager.h"
#include "serverGame/GroupObject.h"
#include "serverGame/GuildObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PortalTriggerVolume.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerObjectBaselinesManager.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipClientUpdateTracker.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceVisibilityManager.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/Scheduler.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include <map>
#include <deque>
#include <set>

// ======================================================================

namespace ObserveTrackerNamespace
{
	typedef std::pair<ServerObject *, Client *>                UnobserveCbInfo;
	typedef std::deque<UnobserveCbInfo>                        UnobserveCbInfoQueue;
	typedef std::map<Client *, UnobserveCbInfo *>              UnobserveCbInfoMapEntry;
	typedef std::map<ServerObject *, UnobserveCbInfoMapEntry>  UnobserveCbInfoMap;

	bool                  s_installed;
	Scheduler *           s_observeTrackerScheduler;
	UnobserveCbInfoQueue  s_unobserveCallbacksOrdered;
	UnobserveCbInfoMap    s_unobserveCallbackMap;

	bool observe(Client &client, ServerObject &obj, std::set<NetworkId> const *oldObserveList = 0);
	void unobserve(Client &client, ServerObject &obj, bool sendDestroy);
	void setUnobserveCallback(Client &client, ServerObject &obj);
	void removeUnobserveCallback(Client *client, ServerObject &obj);
	bool observeContainmentChain(Client &client, ServerObject &obj, std::set<NetworkId> const *oldObserveList);
	void observeContents(Client &client, ServerObject &obj, std::set<NetworkId> const *oldObserveList, bool recursive);
	void observeContentsIfObservedWithParent(Client &client, ServerObject &obj, std::set<NetworkId> const *oldObserveList);
	bool shouldObserveCellContentsDueToClientContainmentChain(Client const &client, ServerObject const &portallizedObject);
	void observeMissionCriticalObjects(Client &client, CreatureObject const &clientCreature);
	void handleUnobserveCallback(void const *);
}
using namespace ObserveTrackerNamespace;

// ======================================================================

void ObserveTracker::install()
{
	FATAL(s_installed, ("ObserveTracker::install - already installed"));
	s_installed = true;
	s_observeTrackerScheduler = new Scheduler;
}

// ----------------------------------------------------------------------

void ObserveTracker::remove()
{
	FATAL(!s_installed, ("ObserveTracker::remove - not installed"));
	delete s_observeTrackerScheduler;
	s_observeTrackerScheduler = 0;
	s_unobserveCallbacksOrdered.clear();
	s_unobserveCallbackMap.clear();
	s_installed = false;
}

// ----------------------------------------------------------------------

void ObserveTracker::update()
{
	NOT_NULL(s_observeTrackerScheduler);
	s_observeTrackerScheduler->update(Clock::timeMs());
}

// ----------------------------------------------------------------------

bool ObserveTracker::isObserving(Client const &client, ServerObject const &obj)
{
	return obj.getObservers().count(const_cast<Client *>(&client)) == 1;
}

// ----------------------------------------------------------------------

void ObserveTracker::onObjectControlled(Client &client, ServerObject &obj, std::set<NetworkId> const &oldObserveList)
{
	if (ConfigServerGame::getLogObservers())
		LOG("ObserveTracker", ("onObjectControlled begin: client %s object %s", client.getCharacterObjectId().getValueString().c_str(), obj.getNetworkId().getValueString().c_str()));

	// When an object is controlled, if it was already being observed,
	// and baselines are not waiting to be sent, then we need to push
	// out baselines for the auth* packages to that client.
	// Note: firstParent* packages still do not behave properly for
	// objects contained by newly controlled objects, but that
	// functionality is not currently needed.
	if (isObserving(client, obj))
	{
		std::vector<Watcher<Client> > * const baselineDistributionList = BaselineDistributionListPool::get(obj, false);
		if (!baselineDistributionList || std::find(baselineDistributionList->begin(), baselineDistributionList->end(), static_cast<Client const *>(&client)) == baselineDistributionList->end())
			obj.sendAuthClientBaselines(client);
	}
	
	// clients should observe the containment chain of their controlled objects
	FATAL(!observeContainmentChain(client, obj, &oldObserveList), ("Client for object %s could not observe its own containment chain.", obj.getDebugInformation().c_str()));

	// clients should observe the master guild object
	GuildObject * const masterGuildObject = ServerUniverse::getInstance().getMasterGuildObject();
	if (masterGuildObject)
		IGNORE_RETURN(observe(client, *masterGuildObject, &oldObserveList));

	CreatureObject * const objAsCreature = obj.asCreatureObject();
	if (objAsCreature)
	{
		// clients with controlled creatures should observe all their contents and indirect contents
		observeContents(client, obj, &oldObserveList, true);
		// clients with controlled creatures should observe their group object
		GroupObject * const group = objAsCreature->getGroup();
		if (group)
			IGNORE_RETURN(observe(client, *group, &oldObserveList));
	}

	if (!ServerWorld::isSpaceScene())
	{
		if (obj.isInWorld())
		{
			// trigger any observation due to network or portal trigger volumes
			if (obj.getTriggerVolumeEntered())
			{
				const std::set<TriggerVolume *> triggerVolumeEntered = *(obj.getTriggerVolumeEntered());
				for (std::set<TriggerVolume *>::const_iterator i = triggerVolumeEntered.begin(); i != triggerVolumeEntered.end(); ++i)
				{
					if ((*i)->isNetworkTriggerVolume())
						onClientEnteredNetworkTriggerVolume(client, **i, &oldObserveList);
					else if ((*i)->isPortalTriggerVolume())
						onClientEnteredPortalTriggerVolume(client, **i, &oldObserveList);
				}
			}
		}
	}
	else
	{
		// observe any mission critical objects
		CreatureObject const * const clientCreature = obj.asCreatureObject();
		if (clientCreature)
			observeMissionCriticalObjects(client, *clientCreature);
	}

	if (ConfigServerGame::getLogObservers())
		LOG("ObserveTracker", ("onObjectControlled end: client %s object %s", client.getCharacterObjectId().getValueString().c_str(), obj.getNetworkId().getValueString().c_str()));
}

// ----------------------------------------------------------------------

void ObserveTracker::onClientControlComplete(Client &client, std::set<NetworkId> const &oldObserveList)
{
	// Anything in the old observe list that either doesn't exist on this server or that we
	// are not observing at this point, we need to send destroy messages for.
	for (std::set<NetworkId>::const_iterator j = oldObserveList.begin(); j != oldObserveList.end(); ++j)
	{
		ServerObject const * const so = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(*j));
		if (!so || (so->getSendToClient() && !isObserving(client, *so)))
		{
			DestroyMessageManager::add(&client, *j, false);
			if (ConfigServerGame::getLogObservers())
				LOG("ObserveTracker", ("onClientControlComplete sending destroy: client %s object %s", client.getCharacterObjectId().getValueString().c_str(), (*j).getValueString().c_str()));
		}
	}
}

// ----------------------------------------------------------------------

void ObserveTracker::onClientAddedVisibleObjectSpace(Client &client, ServerObject &visibleObject)
{
	DEBUG_REPORT_LOG(ConfigServerGame::getDebugSpaceVisibilityManager(),("ObserveTracker::onClientAddedVisibleObjectSpace(Client &client, %s);\n",visibleObject.getNetworkId().getValueString().c_str()));

	IGNORE_RETURN(observe(client, visibleObject));
}

// ----------------------------------------------------------------------

void ObserveTracker::onClientLostVisibleObjectSpace(Client &client, ServerObject &visibleObject)
{
	DEBUG_REPORT_LOG(ConfigServerGame::getDebugSpaceVisibilityManager(),("ObserveTracker::onClientLostVisibleObjectSpace(Client &client, %s);\n",visibleObject.getNetworkId().getValueString().c_str()));
	//NOTE:  This will not work with multiple controlled objects that are not contained by each other

	// do not unobserve mission critical objects
	CreatureObject const * const creatureObject = safe_cast<CreatureObject const *>(client.getCharacterObject());
	if (!creatureObject || !creatureObject->isMissionCriticalObject(visibleObject.getNetworkId()))
		setUnobserveCallback(client, visibleObject);
}

// ----------------------------------------------------------------------

void ObserveTracker::onClientEnteredNetworkTriggerVolume(Client &client, TriggerVolume &triggerVolume, std::set<NetworkId> const *oldObserveList)
{
	IGNORE_RETURN(observe(client, triggerVolume.getOwner(), oldObserveList));
}

// ----------------------------------------------------------------------

void ObserveTracker::onClientExitedNetworkTriggerVolume(Client &client, TriggerVolume &triggerVolume)
{
	// If the trigger volume no longer contains any controlled objects of this client, set an unobserve callback for its owner

	std::vector<ServerObject *> const &controlledObjects = client.getControlledObjects();
	for (std::vector<ServerObject *>::const_iterator i = controlledObjects.begin(); i != controlledObjects.end(); ++i)
		if (triggerVolume.hasObject(**i))
			return;

	setUnobserveCallback(client, triggerVolume.getOwner());
}

// ----------------------------------------------------------------------

void ObserveTracker::onClientEnteredPortalTriggerVolume(Client &client, TriggerVolume &triggerVolume, std::set<NetworkId> const *oldObserveList)
{
	observeContents(client, triggerVolume.getOwner(), oldObserveList, false);
}

// ----------------------------------------------------------------------

void ObserveTracker::onClientExitedPortalTriggerVolume(Client &, TriggerVolume &)
{
	// We could set unobserve callbacks for contents of cells of the trigger volume owner if desired here,
	// but not doing so still results in correct observation semantics.
}

// ----------------------------------------------------------------------

void ObserveTracker::onClientJoinedGroup(Client &client, ServerObject &group)
{
	IGNORE_RETURN(observe(client, group));
}

// ----------------------------------------------------------------------

void ObserveTracker::onClientLeftGroup(Client &client, ServerObject &group)
{
	unobserve(client, group, true);
}

// ----------------------------------------------------------------------

bool ObserveTracker::onClientOpenedContainer(Client &client, ServerObject &obj, int sequence, std::string const &slotDesc, bool sendOpenContainerNotification)
{
	// Clients can only open containers that they are observing.
	if (isObserving(client, obj))
	{
		if (client.getOpenedContainers().insert(&obj).second)
		{
			// The container was not open for this client, so observe its contents.
			observeContents(client, obj, 0, false);
		}		

		if (sendOpenContainerNotification)
		{
			// Set up a container open notification to be send to the opening client.
			ServerObjectBaselinesManager::addOpenedContainer(client, sequence, obj.getNetworkId(), slotDesc);
		}

		return true;
	}

	DEBUG_WARNING(true, ("ObserveTracker::onClientOpenedContainer client %s tried to open a container %s that was not being observed.", client.getCharacterObjectId().getValueString().c_str(), obj.getNetworkId().getValueString().c_str()));
	return false;
}

// ----------------------------------------------------------------------

bool ObserveTracker::onClientClosedContainer(Client &client, ServerObject &obj)
{
	if (client.getOpenedContainers().erase(&obj))
	{
		// If we decide to use this message in the future, we can reenable it, but as it stands we don't use this for anything.
		//send(GenericValueTypeMessage<NetworkId>("ClosedContainerMessage", obj.getNetworkId()), true);
		return true;
	}
	return false;
} //lint !e1764 // obj used in a nonconst manner

// ----------------------------------------------------------------------

void ObserveTracker::onClientAboutToTransferAuthority(Client &client)
{
	// Since client is about to transfer authority, we need to purge out any
	// unsent pending baselines from its observation lists, since they may
	// depend on the state of the client which is about to go away.
	static std::vector<ServerObject*> objectsToUnobserve;
	objectsToUnobserve.clear();

	Client::ObservingList const &observing = client.getObserving();
	for (Client::ObservingList::const_iterator i = observing.begin(); i != observing.end(); ++i)
	{
		if ((*i)->isPatrolPathNode())
			(*i)->removePatrolPathObserver();
		if ((*i)->getSendToClient() && (*i)->removeFromBaselineDistributionList(client))
			objectsToUnobserve.push_back(*i);
	}

	for (std::vector<ServerObject*>::const_iterator i2 = objectsToUnobserve.begin(); i2 != objectsToUnobserve.end(); ++i2)
	{
		IGNORE_RETURN((*i2)->removeObserver(&client));
		client.removeObserving(*i2);
	}
}

// ----------------------------------------------------------------------

void ObserveTracker::onObjectContainerChanged(ServerObject &obj)
{
	ServerObject * const newContainer = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(obj));
	if (newContainer)
	{
		// If an object controlled by a client is entering a new container, they
		// should reobserve their containment chain, which will observe newly
		// exposed contents as appropriate.
		Client * const client = obj.getClient();
		if (client)
			FATAL(!observeContainmentChain(*client, obj, 0), ("Client for object %s could not observe its own containment chain.", obj.getDebugInformation().c_str()));

		{
			// all clients that were observing obj but are not observing its new container must unobserve obj
			std::set<Client *> const oldObservers = obj.getObservers();
			for (std::set<Client *>::const_iterator i = oldObservers.begin(); i != oldObservers.end(); ++i)
				if (!isObserving(**i, *newContainer))
					unobserve(**i, obj, true);
		}

		{
			// All clients which are observing the new container and must be observing
			// its contents must observe obj.
			bool isObservedWith = (newContainer->getContainerProperty() ? newContainer->getContainerProperty()->isContentItemObservedWith(obj) : false);
			CellProperty * newContainerCell = newContainer->getCellProperty();
			ServerObject const * newContainerContainer = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*newContainer));
			if (newContainerCell == nullptr || newContainerContainer != nullptr)
			{
				std::set<Client *> const &newObservers = newContainer->getObservers();
				for (std::set<Client *>::const_iterator i = newObservers.begin(); i != newObservers.end(); ++i)
				{
					if (isObservedWith ||
						(*i)->getOpenedContainers().count(newContainer) ||
						(newContainerCell != nullptr && shouldObserveCellContentsDueToClientContainmentChain(**i, *newContainerContainer)))
					{
						IGNORE_RETURN(observe(**i, obj));
					}
				}
			}
			else
			{
				WARNING(true, ("ObserveTracker::onObjectContainerChanged object %s inside cell %s that has no parent container",
					obj.getNetworkId().getValueString().c_str(), newContainer->getNetworkId().getValueString().c_str()));
			}
		}


		// The object should be observed by any clients on creatures which contain it.
		// Note: we skip observation if the client is not observing the new container,
		// which may be the case when container transfers occur in virtualOnSetClient.
		for (ServerObject *o = &obj; o; o = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*o)))
		{
			if (o->asCreatureObject())
			{
				Client * const containingClient = o->getClient();
				if (containingClient && isObserving(*containingClient, *newContainer))
					if (observe(*containingClient, obj))
						observeContents(*containingClient, obj, 0, true);
			}
		}
	}
}

// ----------------------------------------------------------------------

void ObserveTracker::onObjectArrangementChanged(ServerObject &obj)
{
	// If the object moved into an appearance slot of a creature, remove any
	// unobserve callbacks for it, because it is going into packed wearables
	// data and will still exist as an object on clients that we don't want to
	// send a destroy for, even though we are no longer actually observing it.
	ServerObject * const newContainer = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(obj));
	if (newContainer && newContainer->asCreatureObject())
	{
		SlottedContainmentProperty * const scp = ContainerInterface::getSlottedContainmentProperty(obj);
		if (scp && scp->isInAppearanceSlot())
			removeUnobserveCallback(0, obj);
	}	
}

// ----------------------------------------------------------------------

void ObserveTracker::onObjectMadeInvisible(ServerObject &obj)
{
	// obj became invisible to some clients, so run through all observers
	// and make any of them unobserve that should no longer see it.
	std::set<Client *> const observers = obj.getObservers();
	for (std::set<Client *>::const_iterator i = observers.begin(); i != observers.end(); ++i)
		if (!obj.isVisibleOnClient(**i))
			unobserve(**i, obj, true);
}

// ----------------------------------------------------------------------

void ObserveTracker::onObjectMadeVisibleTo(ServerObject &obj, const std::vector<ServerObject *> & observers)
{
	for (std::vector<ServerObject *>::const_iterator i = observers.begin(); i != observers.end(); ++i)
	{
		Client * const client = (*i)->getClient();
		if (client && obj.isVisibleOnClient(*client))
			IGNORE_RETURN(observe(*client, obj, 0));
	}
}

// ----------------------------------------------------------------------

void ObserveTracker::onClientDestroyed(Client &client)
{
	// client is being destroyed, so clean up observation related references
	// to it.
	Client::ObservingList const &observing = client.getObserving();
	for (Client::ObservingList::const_iterator i = observing.begin(); i != observing.end(); ++i)
	{
		ServerObject * const obj = *i;
		removeUnobserveCallback(&client, *obj);
		if (obj->isPatrolPathNode())
			obj->removePatrolPathObserver();
		if (obj->getSendToClient())
			IGNORE_RETURN(obj->removeFromBaselineDistributionList(client));
		IGNORE_RETURN(obj->removeObserver(&client));
	}
	client.removeAllObserving();
	PvpUpdateObserver::removeClientFromPvpStatusCache(&client);
}

// ----------------------------------------------------------------------

void ObserveTracker::onObjectDestroyed(ServerObject &obj, bool const hyperspace)
{
	// obj is being destroyed, so clean up observation related references to it,
	// and set up a destroy message to be sent to observers if appropriate.

	const std::set<Client *> &observers = obj.getObservers();
	for (std::set<Client *>::const_iterator i = observers.begin(); i != observers.end(); ++i)
	{
		(*i)->removeObserving(&obj);
		IGNORE_RETURN(onClientClosedContainer(**i, obj));
		if (obj.getSendToClient() && !obj.removeFromBaselineDistributionList(**i))
			DestroyMessageManager::add(*i, obj.getNetworkId(), hyperspace);
	}
	obj.clearObservers();

	removeUnobserveCallback(0, obj);
}

// ----------------------------------------------------------------------

void ObserveTracker::onGodModeChanged(Client &client)
{
	static std::vector<ServerObject*> objectsToObserveOrUnobserve;
	objectsToObserveOrUnobserve.clear();

	if (client.isGod())
	{
		// If the client just entered god mode, redo any trigger volume related observation.
		if (ServerWorld::isSpaceScene())
		{
			std::vector<ServerObject *> results;
			SpaceVisibilityManager::getObjectsVisibleFromLocation(ContainerInterface::getTopmostContainer(*client.getCharacterObject())->getPosition_p(), results);
			for (std::vector<ServerObject *>::const_iterator i = results.begin(); i != results.end(); ++i)
				IGNORE_RETURN(observe(client, **i));
		}
		else
		{
			std::vector<ServerObject *> const &controlledObjects = client.getControlledObjects();
			for (std::vector<ServerObject *>::const_iterator i = controlledObjects.begin(); i != controlledObjects.end(); ++i)
			{
				if ((*i)->getTriggerVolumeEntered())
				{
					const std::set<TriggerVolume *> triggerVolumeEntered = *((*i)->getTriggerVolumeEntered());
					for (std::set<TriggerVolume *>::const_iterator j = triggerVolumeEntered.begin(); j != triggerVolumeEntered.end(); ++j)
					{
						if ((*j)->isNetworkTriggerVolume())
						{
							if ((*j)->getOwner().isPatrolPathNode())
							{
								// this is somewhat of a hack - the client will already 
								// be observing the node, so we need to unobserve it so 
								// its baselines will be sent to the client when 
								// onClientEnteredNetworkTriggerVolume is called
								unobserve(client, (*j)->getOwner(), false);
							}
							onClientEnteredNetworkTriggerVolume(client, **j);
						}
						else if ((*j)->isPortalTriggerVolume())
							onClientEnteredPortalTriggerVolume(client, **j);
					}
				}
			}
		}

		// have the client observe the datapad of any creatures he is observing
		SlotId datapadSlot = SlotIdManager::findSlotId(ConstCharCrcLowerString("datapad"));
		if (datapadSlot != SlotId::invalid)
		{
			Client::ObservingList const &observing = client.getObserving();
			for (Client::ObservingList::const_iterator i = observing.begin(); i != observing.end(); ++i)
			{
				// don't observe ourself
				if (*i != client.getCharacterObject())
				{
					const SlottedContainer * slottedContainer = (*i)->getSlottedContainerProperty();
					if (slottedContainer)
					{
						Container::ContainerErrorCode tmp = Container::CEC_Success;
						Container::ContainedItem itemId = slottedContainer->getObjectInSlot(datapadSlot, tmp);
						ServerObject * const datapadObject = safe_cast<ServerObject *>(itemId.getObject());
						if (datapadObject)
							objectsToObserveOrUnobserve.push_back(datapadObject);
					}
				}
			}

			for (std::vector<ServerObject*>::const_iterator i2 = objectsToObserveOrUnobserve.begin(); i2 != objectsToObserveOrUnobserve.end(); ++i2)
			{
				client.addObserving(*i2);
				IGNORE_RETURN((*i2)->addObserver(&client));
			}
		}
	}
	else
	{
		// If the client just left god mode, unobserve anything they should no longer be observing
		Client::ObservingList const &observing = client.getObserving();
		for (Client::ObservingList::const_iterator i = observing.begin(); i != observing.end(); ++i)
		{
			if (!(*i)->isVisibleOnClient(client))
			{
				if (!(*i)->isPatrolPathNode())
					objectsToObserveOrUnobserve.push_back(*i);
				else
				{
					// we want to remove the node from the client, but keep observing it
					DestroyMessageManager::add(&client, (*i)->getNetworkId(), false);
				}
			}
		}

		for (std::vector<ServerObject*>::const_iterator i2 = objectsToObserveOrUnobserve.begin(); i2 != objectsToObserveOrUnobserve.end(); ++i2)
			unobserve(client, **i2, true);
	}
}

// ----------------------------------------------------------------------

void ObserveTracker::onCraftingPrototypeCreated(ServerObject const &objOwner, ServerObject &objPrototype)
{
	// unobserve the prototype item, and also have the client destroy it;
	// we will observe the item again later when the crafting session
	// is done (see onCraftingEndCraftingSession() below); this is because
	// the prototype appearance may change as the player changes it during
	// crafting, and the client must recreate the item in order to get the
	// new appearance
	Client * const client = objOwner.getClient();
	if (client)
		unobserve(*client, objPrototype, true);
}

// ----------------------------------------------------------------------

void ObserveTracker::onCraftingEndCraftingSession(ServerObject const &objOwner, ServerObject &objPrototype)
{
	// observe the prototype item and its contents after crafting is done
	Client * const client = objOwner.getClient();
	if (client)
	{
		IGNORE_RETURN(observe(*client, objPrototype));
		observeContents(*client, objPrototype, 0, true);
	}
}

// ----------------------------------------------------------------------

void ObserveTracker::onMissionCriticalObjectAdded(ServerObject const &playerObject, ServerObject &criticalObject)
{
	Client * const client = playerObject.getClient();
	if (client)
		IGNORE_RETURN(observe(*client, criticalObject));
}

// ----------------------------------------------------------------------

void ObserveTracker::onMakeVendorInventory(ServerObject &vendorInventory, std::set<Client *> const & oldInventoryObservers)
{
	// when an NPC vendor is initialized, its default inventory is replaced with
	// a new "vendor" inventory; we need to make sure that any client that was
	// observing the default inventory now observe the new "vendor" inventory
	for (std::set<Client *>::const_iterator i = oldInventoryObservers.begin(); i != oldInventoryObservers.end(); ++i)
	{
		(*i)->addObserving(&vendorInventory);
		IGNORE_RETURN(vendorInventory.addObserver(*i));
	}
}

// ----------------------------------------------------------------------

void ObserveTracker::onClientAboutToOpenPublicContainer(Client & client, ServerObject & publicContainer)
{
	if (!isObserving(client, publicContainer))
		IGNORE_RETURN(observeContainmentChain(client, publicContainer, 0));
}

// ======================================================================

bool ObserveTrackerNamespace::observe(Client &client, ServerObject &obj, std::set<NetworkId> const *oldObserveList)
{
	// client is attempting to observe obj.  oldObserveList may contain a list
	// of objects which the client knows about from its previous server in the
	// case of authority transfers without a loading screen.

	ServerObject * const containedByObject = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(obj));
	if (containedByObject && !ObserveTracker::isObserving(client, *containedByObject))
	{
		DEBUG_WARNING(true, ("ObserveTracker::observe - tried to observe an object %s whose immediate container %s was not observed.", obj.getDebugInformation().c_str(), containedByObject->getDebugInformation().c_str()));

		// for now, recover by observing the container
		if (!observeContainmentChain(client, *containedByObject, oldObserveList))
			return false;
	}

	bool isVisible = obj.isVisibleOnClient(client);
	bool isPatrol = obj.isPatrolPathNode();
	if (!isVisible && !isPatrol)
		return false;

	if (ObserveTracker::isObserving(client, obj))
	{
		// If client is still observing obj, remove any pending unobserve callback for it
		removeUnobserveCallback(&client, obj);
	}
	else
	{
		if (ConfigServerGame::getLogObservers())
			LOG("ObserveTracker", ("observe: client %s object %s", client.getCharacterObjectId().getValueString().c_str(), obj.getNetworkId().getValueString().c_str()));

		client.addObserving(&obj);
		IGNORE_RETURN(obj.addObserver(&client));

		if (isPatrol)
			obj.addPatrolPathObserver();

		// If we're observing a ship that we're not controlling, we need to start receiving updates for it if it's capable of moving
		ShipObject const * const ship = obj.asShipObject();
		if (ship && !ship->isAlwaysStationary() && obj.getClient() != &client)
			ShipClientUpdateTracker::queueForUpdate(client, *ship);

		// If the client was not previously observing obj, set things in motion
		// for baselines to be sent.
		if (isVisible && (!oldObserveList || oldObserveList->find(obj.getNetworkId()) == oldObserveList->end()))
			obj.sendCreateAndBaselinesToClient(client);
	}

	CreatureObject * const creatureObj = obj.asCreatureObject();
	if (creatureObj)
	{
		SlottedContainer * const slottedContainer = obj.getSlottedContainerProperty();
		if (slottedContainer)
		{
			if (obj.isPlayerControlled())
			{
				// observe the player object for player controlled creatures
				static SlotId const ghostSlot = SlotIdManager::findSlotId(ConstCharCrcLowerString("ghost"));
				if (ghostSlot != SlotId::invalid)
				{
					Container::ContainerErrorCode tmp = Container::CEC_Success;
					Container::ContainedItem itemId = slottedContainer->getObjectInSlot(ghostSlot, tmp);
					ServerObject * const playerObject = safe_cast<ServerObject *>(itemId.getObject());
					if (playerObject)
						IGNORE_RETURN(observe(client, *playerObject, oldObserveList));
				}

				// observe the appearance tab object for player controlled creatures
				ServerObject * const appearanceTabObject = creatureObj->getAppearanceInventory();
				if (appearanceTabObject)
					IGNORE_RETURN(observe(client, *appearanceTabObject, oldObserveList));
			}

			if (obj.getClient() != &client)
			{
				// mark the inventory container as observed but don't send baselines for non-player creatures
				static SlotId const inventorySlot = SlotIdManager::findSlotId(ConstCharCrcLowerString("inventory"));
				if (inventorySlot != SlotId::invalid)
				{
					Container::ContainerErrorCode tmp = Container::CEC_Success;
					Container::ContainedItem itemId = slottedContainer->getObjectInSlot(inventorySlot, tmp);
					ServerObject * const inventoryObject = safe_cast<ServerObject *>(itemId.getObject());
					if (inventoryObject)
					{
						client.addObserving(inventoryObject);
						IGNORE_RETURN(inventoryObject->addObserver(&client));
					}
				}
				if (client.isGod())
				{
					// have god clients observe other player's datapad
					SlotId datapadSlot = SlotIdManager::findSlotId(ConstCharCrcLowerString("datapad"));
					if (datapadSlot != SlotId::invalid)
					{
						Container::ContainerErrorCode tmp = Container::CEC_Success;
						Container::ContainedItem itemId = slottedContainer->getObjectInSlot(datapadSlot, tmp);
						ServerObject * const datapadObject = safe_cast<ServerObject *>(itemId.getObject());
						if (datapadObject)
						{
							client.addObserving(datapadObject);
							IGNORE_RETURN(datapadObject->addObserver(&client));
						}
					}
				}
			}
		}
	}
	else if (client.isGod())
	{
		// god clients observe the contents of ships and ship control devices
		if (obj.asShipObject() || obj.getGameObjectType() == static_cast<int>(SharedObjectTemplate::GOT_data_ship_control_device))
			observeContents(client, obj, oldObserveList, false);
	}

	if (client.getOpenedContainers().count(&obj))
		observeContents(client, obj, oldObserveList, false);
	if (obj.getCellProperty() && shouldObserveCellContentsDueToClientContainmentChain(client, *safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(obj))))
		observeContents(client, obj, oldObserveList, false);
	else
	{
		Container const * const container = obj.getContainerProperty();
		if (container && container->canContentsBeObservedWith())
			observeContentsIfObservedWithParent(client, obj, oldObserveList);
	}

	return true;
}

// ----------------------------------------------------------------------

void ObserveTrackerNamespace::unobserve(Client &client, ServerObject &obj, bool sendDestroy)
{
	if (ObserveTracker::isObserving(client, obj))
	{
		if (ConfigServerGame::getLogObservers())
			LOG("ObserveTracker", ("unobserve: client %s object %s", client.getCharacterObjectId().getValueString().c_str(), obj.getNetworkId().getValueString().c_str()));

		// clear observation
		IGNORE_RETURN(obj.removeObserver(&client));
		client.removeObserving(&obj);
		if (obj.isPatrolPathNode())
			obj.removePatrolPathObserver();

		// remove from open containers
		IGNORE_RETURN(ObserveTracker::onClientClosedContainer(client, obj));

		removeUnobserveCallback(&client, obj);

		// If we send a destroy at this level, we no longer need to send
		// it for contents being unobserved.
		if (   !obj.removeFromBaselineDistributionList(client)
		    && sendDestroy
		    && obj.getSendToClient())
		{
			DestroyMessageManager::add(&client, obj.getNetworkId(), false);
			sendDestroy = false;
		}

		// unobserve all contents
		Container * const container = ContainerInterface::getContainer(obj);
		if (container)
		{
			for (ContainerIterator i = container->begin(); i != container->end(); ++i)
			{
				ServerObject * const content = safe_cast<ServerObject *>((*i).getObject());
				if (content)
					unobserve(client, *content, sendDestroy);
			}
		}
	}
}

// ----------------------------------------------------------------------

void ObserveTrackerNamespace::setUnobserveCallback(Client &client, ServerObject &obj)
{
	// Set up a callback for client to unobserve obj at some point in the future.
	if (ObserveTracker::isObserving(client, obj))
	{
		// If baselines for obj are still pending for client, then we can
		// just unobserve it since the client doesn't have it yet.
		if (obj.removeFromBaselineDistributionList(client))
			unobserve(client, obj, false);
		else
		{
			removeUnobserveCallback(&client, obj);
			s_unobserveCallbacksOrdered.push_back(UnobserveCbInfo(&obj, &client));
			UnobserveCbInfo &p = s_unobserveCallbacksOrdered.back();
			s_unobserveCallbackMap[&obj][&client] = &p;
			s_observeTrackerScheduler->setCallback(handleUnobserveCallback, 0, static_cast<unsigned long>(ConfigServerGame::getClientOutOfRangeObjectCacheTimeMs()));

			if (ConfigServerGame::getLogObservers())
				LOG("ObserveTracker", ("setUnobserveCallback: client %s object %s", client.getCharacterObjectId().getValueString().c_str(), obj.getNetworkId().getValueString().c_str()));
		}
	}
}

// ----------------------------------------------------------------------

void ObserveTrackerNamespace::removeUnobserveCallback(Client *client, ServerObject &obj)
{
	// Clear out unobserve callback info for a specified client/object pair,
	// or for all pairs associated with a particular object if client is 0.
	// The callback will still go off, but the data associated with it is
	// cleared out here so that it will not be processed.

	UnobserveCbInfoMap::iterator i = s_unobserveCallbackMap.find(&obj);
	if (i != s_unobserveCallbackMap.end())
	{
		UnobserveCbInfoMapEntry &m = (*i).second;
		if (client)
		{
			UnobserveCbInfoMapEntry::iterator j = m.find(client);
			if (j != m.end())
			{
				if (ConfigServerGame::getLogObservers())
					LOG("ObserveTracker", ("removeUnobserveCallback: client %s object %s", client->getCharacterObjectId().getValueString().c_str(), obj.getNetworkId().getValueString().c_str()));

				(*j).second->first = 0;
				m.erase(j);
			}
			if (m.empty())
				s_unobserveCallbackMap.erase(i);
		}
		else
		{
			for (UnobserveCbInfoMapEntry::iterator j = m.begin(); j != m.end(); ++j)
			{
				if (ConfigServerGame::getLogObservers())
					LOG("ObserveTracker", ("removeUnobserveCallback: client %s object %s", (*j).first->getCharacterObjectId().getValueString().c_str(), obj.getNetworkId().getValueString().c_str()));
				(*j).second->first = 0;
			}
			s_unobserveCallbackMap.erase(i);
		}
	}
} //lint !e1764 // obj used in a nonconst manner

// ----------------------------------------------------------------------

void ObserveTrackerNamespace::handleUnobserveCallback(void const *)
{
	UnobserveCbInfo &p = s_unobserveCallbacksOrdered.front();
	ServerObject * const obj = p.first;
	if (obj)
	{
		Client * const client = NON_NULL(p.second);
		unobserve(*client, *obj, true);
	}
	s_unobserveCallbacksOrdered.pop_front();
}

// ----------------------------------------------------------------------

bool ObserveTrackerNamespace::observeContainmentChain(Client &client, ServerObject &obj, std::set<NetworkId> const *oldObserveList)
{
	// Observe obj, its containing object, etc. until reaching an uncontained
	// object, but do the actual observation starting at the topmost and working
	// down toward obj, so that we never attempt to observe an object without
	// its containing object having been observed first.
	std::vector<ServerObject *> containers;
	for (ServerObject *o = &obj; o; o = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*o)))
		containers.push_back(o);

	for (std::vector<ServerObject *>::reverse_iterator i = containers.rbegin(); i != containers.rend(); ++i)
	{
		if (!observe(client, **i, oldObserveList))
		{
			WARNING_DEBUG_FATAL(true, ("ObserveTracker::observeContainmentChain - object %s's containment chain has an unobservable object %s", obj.getDebugInformation().c_str(), (*i)->getDebugInformation().c_str()));
			return false;
		}
	}
	return true;
}

// ----------------------------------------------------------------------

void ObserveTrackerNamespace::observeContents(Client &client, ServerObject &obj, std::set<NetworkId> const *oldObserveList, bool recursive)
{
	Container * const container = ContainerInterface::getContainer(obj);
	if (container)
	{
		for (ContainerIterator i = container->begin(); i != container->end(); ++i)
		{
			ServerObject * const content = safe_cast<ServerObject *>((*i).getObject());
			if (content && observe(client, *content, oldObserveList) && recursive)
				observeContents(client, *content, oldObserveList, true);
		}
	}
}

// ----------------------------------------------------------------------

void ObserveTrackerNamespace::observeContentsIfObservedWithParent(Client &client, ServerObject &obj, std::set<NetworkId> const *oldObserveList)
{
	Container * const container = obj.getContainerProperty();
	if (container)
	{
		for (ContainerIterator i = container->begin(); i != container->end(); ++i)
		{
			ServerObject * const content = safe_cast<ServerObject *>((*i).getObject());
			if (content && container->isContentItemObservedWith(*content))
				IGNORE_RETURN(observe(client, *content, oldObserveList));
		}
	}
}

// ----------------------------------------------------------------------

bool ObserveTrackerNamespace::shouldObserveCellContentsDueToClientContainmentChain(Client const &client, ServerObject const &portallizedObject)
{
	TriggerVolume const * const portalTriggerVolume = portallizedObject.getTriggerVolume(PortalTriggerVolume::getName());
	std::vector<ServerObject *> const &controlledObjects = client.getControlledObjects();
	for (std::vector<ServerObject *>::const_iterator i = controlledObjects.begin(); i != controlledObjects.end(); ++i)
	{
		if (portalTriggerVolume && portalTriggerVolume->hasObject(**i))
			return true;
		for (ServerObject const *so = (*i); so; so = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*so)))
			if (so == &portallizedObject)
				return true;
	}
	return false;
}

// ----------------------------------------------------------------------

void ObserveTrackerNamespace::observeMissionCriticalObjects(Client &client, CreatureObject const &clientCreature)
{
	CreatureObject::MissionCriticalObjectSet const & missionCriticalObjects = clientCreature.getMissionCriticalObjects();
	for (CreatureObject::MissionCriticalObjectSet::const_iterator i = missionCriticalObjects.begin(); i != missionCriticalObjects.end(); ++i)
	{
		ServerObject * const obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(*i));
		if (obj)
			IGNORE_RETURN(observe(client, *obj));
	}
}

// ======================================================================

