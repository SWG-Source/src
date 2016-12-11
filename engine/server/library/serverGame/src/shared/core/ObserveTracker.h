// ======================================================================
//
// ObserveTracker.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObserveTracker_H
#define INCLUDED_ObserveTracker_H

// ======================================================================

class Client;
class NetworkId;
class ServerObject;
class TriggerVolume;

// ======================================================================

class ObserveTracker
{
public:
	static void install();
	static void remove();
	static void update();

	static bool isObserving(Client const &client, ServerObject const &obj);
	static void onObjectControlled(Client &client, ServerObject &obj, std::set<NetworkId> const &oldObserveList);
	static void onClientControlComplete(Client &client, std::set<NetworkId> const &oldObserveList);
	static void onClientEnteredNetworkTriggerVolume(Client &client, TriggerVolume &triggerVolume, std::set<NetworkId> const *oldObserveList = 0);
	static void onClientExitedNetworkTriggerVolume(Client &client, TriggerVolume &triggerVolume);
	static void onClientAddedVisibleObjectSpace(Client &client, ServerObject &visibleObject);
	static void onClientLostVisibleObjectSpace(Client &client, ServerObject &visibleObject);
	static void onClientEnteredPortalTriggerVolume(Client &client, TriggerVolume &triggerVolume, std::set<NetworkId> const *oldObserveList = 0);
	static void onClientExitedPortalTriggerVolume(Client &client, TriggerVolume &triggerVolume);
	static bool onClientOpenedContainer(Client &client, ServerObject &obj, int sequence, std::string const &slotDesc, bool sendOpenContainerNotification = true);
	static bool onClientClosedContainer(Client &client, ServerObject &obj);
	static void onClientJoinedGroup(Client &client, ServerObject &group);
	static void onClientLeftGroup(Client &client, ServerObject &group);
	static void onClientAboutToTransferAuthority(Client &client);
	static void onObjectContainerChanged(ServerObject &obj);
	static void onObjectArrangementChanged(ServerObject &obj);
	static void onObjectMadeInvisible(ServerObject &obj);
	static void onObjectMadeVisibleTo(ServerObject &obj, const std::vector<ServerObject *> & observers);
	static void onClientDestroyed(Client &client);
	static void onObjectDestroyed(ServerObject &obj, bool hyperspace);
	static void onGodModeChanged(Client &client);
	static void onCraftingPrototypeCreated(ServerObject const &objOwner, ServerObject &objPrototype);
	static void onCraftingEndCraftingSession(ServerObject const &objOwner, ServerObject &objPrototype);
	static void onMissionCriticalObjectAdded(ServerObject const &playerObject, ServerObject &criticalShip);
	static void onMakeVendorInventory(ServerObject &vendorInventory, std::set<Client *> const & oldInventoryObservers);
	static void onClientAboutToOpenPublicContainer(Client & client, ServerObject & container);
};

// ======================================================================

#endif // INCLUDED_ObserveTracker_H

