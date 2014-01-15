// ======================================================================
//
// DestroyMessageManager.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/DestroyMessageManager.h"
#include "serverGame/Client.h"
#include "serverGame/ConnectionServerConnection.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/SceneChannelMessages.h"

#ifdef _DEBUG
#include "serverGame/ServerObject.h"
#include "sharedObject/NetworkIdManager.h"
#endif

// ======================================================================
//
// Destroy messages are in arbitrary order.
//
// ======================================================================

namespace DestroyMessageManagerNamespace
{
	std::map<NetworkId, std::pair<std::set<Watcher<Client> >, bool> > s_destroyMap;
}
using namespace DestroyMessageManagerNamespace;

// ======================================================================

void DestroyMessageManager::add(Client *client, NetworkId const &destroyId, bool const hyperspace)
{
	s_destroyMap[destroyId].first.insert(Watcher<Client>(client));
	s_destroyMap[destroyId].second = hyperspace;
}

// ----------------------------------------------------------------------

bool DestroyMessageManager::remove(Client const *client, NetworkId const &destroyId)
{
	std::map<NetworkId, std::pair<std::set<Watcher<Client> >, bool> >::iterator i = s_destroyMap.find(destroyId);
	if (i != s_destroyMap.end())
	{
		std::set<Watcher<Client> > &clients = (*i).second.first;
		std::set<Watcher<Client> >::iterator j = std::find(clients.begin(), clients.end(), client);
		if (j != clients.end())
		{
			if (clients.size() == 1)
				s_destroyMap.erase(i);
			else
				clients.erase(j);
			return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------

void DestroyMessageManager::update()
{
	for (std::map<NetworkId, std::pair<std::set<Watcher<Client> >, bool> >::const_iterator i = s_destroyMap.begin(); i != s_destroyMap.end(); ++i)
	{

#ifdef _DEBUG
		Object const * const o = NetworkIdManager::getObjectById((*i).first);
		DEBUG_FATAL(o && o->asServerObject() && o->asServerObject()->getCacheVersion() > 0, ("Tried to destroy cached object %s", (*i).first.getValueString().c_str()));
#endif

		std::set<Watcher<Client> > const &clientSet = (*i).second.first;
		bool const hyperspace = (*i).second.second;

		typedef std::map<ConnectionServerConnection *, std::vector<NetworkId> > DistributionList;
		DistributionList distributionList;

		for (std::set<Watcher<Client> >::const_iterator j = clientSet.begin(); j != clientSet.end(); ++j)
		{
			Client * const client = (*j).getPointer();
			if (client)
				distributionList[client->getConnection()].push_back(client->getCharacterObjectId());
		}

		if (!distributionList.empty())
		{
			SceneDestroyObject const message((*i).first, hyperspace);
			for (DistributionList::const_iterator k = distributionList.begin(); k != distributionList.end(); ++k)
			{
				GameClientMessage const gcm((*k).second, true, message);
				(*k).first->send(gcm, true);
			}
		}
	}

	s_destroyMap.clear();
}

// ======================================================================

