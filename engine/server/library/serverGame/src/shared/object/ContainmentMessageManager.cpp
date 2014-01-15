// ======================================================================
//
// ContainmentMessageManager.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ContainmentMessageManager.h"

#include "serverGame/ConnectionServerConnection.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/DistributionListStack.h"
#include "serverGame/ServerObject.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "sharedNetworkMessages/UpdateContainmentMessage.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include <algorithm>

// ======================================================================
//
// Order of containment messages is preserved, and changes are sent for any objects that still exist.
// Containment states for each update are as of the time at which the message is added, to preserve causality.
//
// ======================================================================

namespace ContainmentMessageManagerNamespace
{
	// ----------------------------------------------------------------------

	class ContainmentMessageData
	{
	public:
		ContainmentMessageData();
		explicit ContainmentMessageData(ServerObject &object);

		void update() const;

	private:
		Watcher<ServerObject> m_watcher;
		NetworkId m_containedBy;
		int m_currentArrangement;
	};

	// ----------------------------------------------------------------------

	std::vector<ContainmentMessageData> s_containmentMessages;
	std::map<NetworkId, std::set<Client *> > s_frameBaselines;

	// ----------------------------------------------------------------------

	std::set<Client *> const *getClientBaselinesThisFrame(NetworkId const &networkId);

	// ----------------------------------------------------------------------

}
using namespace ContainmentMessageManagerNamespace;

// ======================================================================

void ContainmentMessageManager::addContainmentMessage(ServerObject &object)
{
	s_containmentMessages.push_back(ContainmentMessageData(object));
}

// ----------------------------------------------------------------------

void ContainmentMessageManager::addFrameBaselines(NetworkId const &networkId, std::vector<Watcher<Client> > const &distributionList)
{
	std::set<Client *>& baselines = s_frameBaselines[networkId];
	for (std::vector<Watcher<Client> >::const_iterator i = distributionList.begin(); i != distributionList.end(); ++i)
		IGNORE_RETURN(baselines.insert(*i));
}

// ----------------------------------------------------------------------

void ContainmentMessageManager::update()
{
	for (std::vector<ContainmentMessageData>::iterator i = s_containmentMessages.begin(); i != s_containmentMessages.end(); ++i)
		(*i).update();

	s_containmentMessages.clear();
	s_frameBaselines.clear();
}

// ----------------------------------------------------------------------

ContainmentMessageManagerNamespace::ContainmentMessageData::ContainmentMessageData() :
	m_watcher(),
	m_containedBy(NetworkId::cms_invalid),
	m_currentArrangement(-1)
{
}

// ----------------------------------------------------------------------

ContainmentMessageManagerNamespace::ContainmentMessageData::ContainmentMessageData(ServerObject &object) :
	m_watcher(&object),
	m_containedBy(ContainerInterface::getContainedByProperty(object)->getContainedByNetworkId()),
	m_currentArrangement(ContainerInterface::getSlottedContainmentProperty(object)->getCurrentArrangement())
{
}

// ----------------------------------------------------------------------

void ContainmentMessageManagerNamespace::ContainmentMessageData::update() const
{
	static std::set<Client *> clientResultSet;

	ServerObject * const obj = m_watcher.getPointer();
	if (obj)
	{
		std::set<Client *> const &observers = obj->getObservers();
		std::set<Client *> const * const baselines = getClientBaselinesThisFrame(obj->getNetworkId());
		std::set<Client *> const *clients;
		if (!baselines)
			clients = &observers;
		else
		{
			clientResultSet.clear();
			std::insert_iterator<std::set<Client *> > insertIter(clientResultSet, clientResultSet.begin());
			IGNORE_RETURN(std::set_difference(observers.begin(), observers.end(), baselines->begin(), baselines->end(), insertIter));
			clients = &clientResultSet;
		}

		std::map<ConnectionServerConnection *, std::vector<NetworkId> > &tmpDistributionList = DistributionListStack::alloc();
		for (std::set<Client *>::const_iterator i = clients->begin(); i != clients->end(); ++i)
			tmpDistributionList[(*i)->getConnection()].push_back((*i)->getCharacterObjectId());

		if (!tmpDistributionList.empty())
		{
			UpdateContainmentMessage const ucm(obj->getNetworkId(), m_containedBy, m_currentArrangement);
			for (std::map<ConnectionServerConnection *, std::vector<NetworkId> >::const_iterator j = tmpDistributionList.begin(); j != tmpDistributionList.end(); ++j)
			{
				GameClientMessage const gcm((*j).second, true, ucm);
				(*j).first->send(gcm, true);
			}
		}

		DistributionListStack::release();
		clientResultSet.clear();
	}
}

// ----------------------------------------------------------------------

std::set<Client *> const *ContainmentMessageManagerNamespace::getClientBaselinesThisFrame(NetworkId const &networkId)
{
	std::map<NetworkId, std::set<Client *> >::const_iterator i = s_frameBaselines.find(networkId);
	if (i != s_frameBaselines.end())
		return &((*i).second);
	return 0;
}

// ======================================================================

