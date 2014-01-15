// ======================================================================
//
// ServerObjectBaselinesManager.cpp
//
// Copyright 2000-2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerObjectBaselinesManager.h"

#include "serverGame/ContainerInterface.h"
#include "serverGame/ServerObject.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

class ContainerOpenInfo
{
public:
	ContainerOpenInfo() :
		m_client(),
		m_sequence(0),
		m_containerId(NetworkId::cms_invalid),
		m_slotName()
	{
	}

	ContainerOpenInfo(Client &client, int sequence, NetworkId const &containerId, std::string const &slotName) :
		m_client(&client),
		m_sequence(sequence),
		m_containerId(containerId),
		m_slotName(slotName)
	{
	}

	void flushContainerOpen() const
	{
		Client * const client = m_client.getPointer();
		if (client)
		{
			GenericValueTypeMessage<std::pair<int, std::pair<NetworkId, std::string> > > const msg(
				"OpenedContainerMessage",
				std::make_pair(m_sequence, std::make_pair(m_containerId, m_slotName)));
			client->send(msg, true);
		}
	}

private:
	Watcher<Client> m_client;
	int m_sequence;
	NetworkId m_containerId;
	std::string m_slotName;
};

// ======================================================================

static std::set<ServerObject const *> s_topmostObjectsToFlush;
static std::vector<ContainerOpenInfo> s_containerOpens;

// ======================================================================

void ServerObjectBaselinesManager::add(ServerObject const &object)
{
	IGNORE_RETURN(s_topmostObjectsToFlush.insert(safe_cast<ServerObject const *>(ContainerInterface::getTopmostContainer(object))));
}

// ----------------------------------------------------------------------

void ServerObjectBaselinesManager::addOpenedContainer(Client &client, int sequence, NetworkId const &containerId, std::string const &slotName)
{
	s_containerOpens.push_back(ContainerOpenInfo(client, sequence, containerId, slotName));
}

// ----------------------------------------------------------------------

void ServerObjectBaselinesManager::serverObjDestroyed(ServerObject const &obj)
{
	IGNORE_RETURN(s_topmostObjectsToFlush.erase(&obj));
}

// ----------------------------------------------------------------------

void ServerObjectBaselinesManager::onContainerChanged(ServerObject const &object, ServerObject const *oldContainer, ServerObject const *newContainer)
{
	// object is changing containers, so we need to adjust which topmost
	// parents may potentially have pending baselines.

	ServerObject const * const oldTopmost = oldContainer ? safe_cast<ServerObject const *>(ContainerInterface::getTopmostContainer(*oldContainer)) : &object;
	ServerObject const * const newTopmost = newContainer ? safe_cast<ServerObject const *>(ContainerInterface::getTopmostContainer(*newContainer)) : &object;

	// if object keeps the same topmost parent, there are no updates to make
	if (oldTopmost == newTopmost)
		return;

	std::set<ServerObject const *>::iterator i = s_topmostObjectsToFlush.find(oldTopmost);
	if (i == s_topmostObjectsToFlush.end())
		return; // old topmost had no baselines pending, so nothing to update

	// if object was previously a top level object, remove from the set
	if (!oldContainer)
		s_topmostObjectsToFlush.erase(i);

	// There were baselines pending for the old topmost or its children,
	// which means possible for object or its contents, so we add the new
	// topmost to the set of pending baselines.
	IGNORE_RETURN(s_topmostObjectsToFlush.insert(newTopmost));
}

// ----------------------------------------------------------------------

void ServerObjectBaselinesManager::update()
{
	// flush baselines
	{
		for (std::set<ServerObject const *>::const_iterator i = s_topmostObjectsToFlush.begin(); i != s_topmostObjectsToFlush.end(); ++i)
			(*i)->flushCreateMessages();
		s_topmostObjectsToFlush.clear();
	}

	// flush container opens
	{
		for (std::vector<ContainerOpenInfo>::const_iterator i = s_containerOpens.begin(); i != s_containerOpens.end(); ++i)
			(*i).flushContainerOpen();
		s_containerOpens.clear();
	}
}

// ======================================================================

