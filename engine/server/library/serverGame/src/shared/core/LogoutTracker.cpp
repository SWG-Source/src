// ======================================================================
//
// LogoutTracker.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/LogoutTracker.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/GameServer.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/UnloadedPlayerMessage.h"
#include "sharedFoundation/Scheduler.h"
#include "sharedFoundation/Watcher.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/Container.h"
#include <queue>
#include <map>
#include <unordered_map>

// ======================================================================

static bool s_installed;
static Scheduler *s_logoutTrackerScheduler;
static std::queue<NetworkId> s_logoutCallbacks;
static std::unordered_map<NetworkId, int, NetworkId::Hash> s_logoutCallbacksCount;

namespace LogoutTrackerNamespace
{
	std::map<NetworkId, Watcher<ServerObject> > *ms_pendingSaves;
};

using namespace LogoutTrackerNamespace;
// ======================================================================

void LogoutTracker::install()
{
	FATAL(s_installed, ("LogoutTracker::install - already installed"));
	s_installed = true;
	s_logoutTrackerScheduler = new Scheduler;
	ms_pendingSaves = new std::map<NetworkId, Watcher<ServerObject> >;
}

// ----------------------------------------------------------------------

void LogoutTracker::remove()
{
	FATAL(!s_installed, ("LogoutTracker::remove - not installed"));
	s_installed = false;
	delete s_logoutTrackerScheduler;
	s_logoutTrackerScheduler = 0;
	delete ms_pendingSaves;
	ms_pendingSaves=0;
}

// ----------------------------------------------------------------------

void LogoutTracker::add(NetworkId const &networkId)
{
	// create the callback info
	s_logoutCallbacks.push(networkId);

	// keep track of how many time the same networkId is put into the logout callback queue;
	// we will only process the *LAST* one that is put into the logout callback queue, meaning
	// if the player disconnects repeatedly, each time causing him to get put into the logout
	// callback queue, and a 3 minute timer set, that those previous 3 minute timers are ignored
	// when they go off, and only when the last 3 minute timer goes off do we remove him from the
	// world, which is the desired behavior of leaving a disconnected character in the world for
	// 3 minutes before removing him from the world
	std::unordered_map<NetworkId, int, NetworkId::Hash>::iterator iter = s_logoutCallbacksCount.find(networkId);
	if (iter == s_logoutCallbacksCount.end())
	{
		s_logoutCallbacksCount[networkId] = 1;
	}
	else
	{
		++(iter->second);
	}

	// set the callback
	getScheduler().setCallback(handleLogoutCallback, nullptr, ConfigServerGame::getUnsafeLogoutTimeMs());
}

// ----------------------------------------------------------------------

void LogoutTracker::handleLogoutCallback(const void *context)
{
	UNREF(context);

	NetworkId const &networkId = s_logoutCallbacks.front();
	std::unordered_map<NetworkId, int, NetworkId::Hash>::iterator iter = s_logoutCallbacksCount.find(networkId);

	// only process the *LAST* logout callback that is put into the logout callback
	// queue for this object as the previous ones are no longer applicable
	if ((iter == s_logoutCallbacksCount.end()) || (iter->second == 1))
	{
		ServerObject *obj = ServerWorld::findObjectByNetworkId(networkId);
		if (obj && obj->isAuthoritative() && !obj->getClient())
		{
			obj->handleDisconnect(true);
		}
	}

	if (iter != s_logoutCallbacksCount.end())
	{
		if (iter->second > 1)
		{
			--(iter->second);
		}
		else
		{
			s_logoutCallbacksCount.erase(iter);
		}
	}

	// ***MUST*** be done last, since we just have a reference to the front NetworkId
	s_logoutCallbacks.pop();
}

// ----------------------------------------------------------------------

bool LogoutTracker::isInLogoutCallbackList(NetworkId const &networkId)
{
	return (s_logoutCallbacksCount.count(networkId) > 0);
}

// ----------------------------------------------------------------------

Scheduler &LogoutTracker::getScheduler()
{
	FATAL(!s_logoutTrackerScheduler, ("No LogoutTracker scheduler found"));
	return *s_logoutTrackerScheduler;
}

// ----------------------------------------------------------------------

/**
 * The character will be removed from the world and hidden.  Ask the
 * database to send a confirmation when the character is saved, and then
 * we will delete him.  If the player returns in the mean time, we'll put
 * the character back in the world.  The PlanetServer also needs to know about this
 * so that it can send the login to the right server.
 */
void LogoutTracker::addPendingSave(ServerObject *character)
{
	NOT_NULL(ms_pendingSaves);
	(*ms_pendingSaves)[character->getNetworkId()]=character;
	UnloadedPlayerMessage const msg(character->getNetworkId());
	GameServer::getInstance().sendToPlanetServer(msg);
	GameServer::getInstance().sendToDatabaseServer(msg);
}

// ----------------------------------------------------------------------

void LogoutTracker::onPersisted(const NetworkId &character)
{
	NOT_NULL(ms_pendingSaves);
	std::map<NetworkId, Watcher<ServerObject> >::iterator i=ms_pendingSaves->find(character);
	if (i!=ms_pendingSaves->end())
	{
		if (i->second)
		{
			LOG("TRACE_LOGIN",("Deleting character %s who has been saved.",character.getValueString().c_str()));
			Object* containerObject = ContainerInterface::getContainedByObject(*(i->second));
			//If we're contained, remove ourself from the container before destruction
			if (containerObject)
			{
				Container* container = ContainerInterface::getContainer(*containerObject);
				NOT_NULL(container);
				container->internalItemRemoved(*(i->second));
			}
			delete i->second;
			ms_pendingSaves->erase(i);

			// Central doesn't need to track the scene anymore
			GenericValueTypeMessage<std::pair<NetworkId, std::pair<std::string, bool> > > const msg(
				"SetSceneForPlayer",
				std::make_pair(
					character,
					std::make_pair(std::string(), false)));
			
			GameServer::getInstance().sendToCentralServer(msg);

			// Send a message to all the other game servers to
			// unload any copy they still have of the character

			ServerMessageForwarding::beginBroadcast();

			GenericValueTypeMessage<NetworkId> const unloadPersistedCharacter("UnloadPersistedCharacter", character);
			ServerMessageForwarding::send(unloadPersistedCharacter);

			ServerMessageForwarding::end();

			ServerUniverse::removeConnectedCharacterBiographyData(character);
		}
		else
			LOG("TRACE_LOGIN",("Character %s was not deleted because they logged back on.",character.getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------

ServerObject *LogoutTracker::findPendingCharacterSave(const NetworkId &character)
{
	NOT_NULL(ms_pendingSaves);
	std::map<NetworkId, Watcher<ServerObject> >::iterator i=ms_pendingSaves->find(character);
	if (i!=ms_pendingSaves->end())
	{
		if (i->second)
			LOG("TRACE_LOGIN",("Reconnecting character %s who was logged out but waiting to be saved.",character.getValueString().c_str()));

		ServerObject *obj = i->second;
		ms_pendingSaves->erase(i);
		return obj;
	}
	else
		return nullptr;
}

// ======================================================================

