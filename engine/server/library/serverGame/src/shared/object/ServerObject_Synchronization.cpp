// ======================================================================
//
// ServerObject_Synchronization.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerObject.h"

#include "serverGame/BaselineDistributionListPool.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConnectionServerConnection.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/ContainmentMessageManager.h"
#include "serverGame/DestroyMessageManager.h"
#include "serverGame/DistributionListStack.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObjectBaselinesManager.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/CreateObjectMessage.h"
#include "serverNetworkMessages/EndBaselinesMessage.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverNetworkMessages/SynchronizeScriptVarsMessage.h"
#include "serverNetworkMessages/UnloadObjectMessage.h"
#include "serverNetworkMessages/UpdateObjectPositionMessage.h"
#include "serverScript/GameScriptObject.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedLog/Log.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetworkMessages/DeltasMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/SceneChannelMessages.h"
#include "sharedNetworkMessages/UpdateContainmentMessage.h"

// ======================================================================

namespace ServerObject_SynchronizationNamespace
{

	// ----------------------------------------------------------------------

	Client const *getParentPlayerClient(ServerObject const *o)
	{
		while (o && !o->isInWorld() && !o->isPlayerControlled())
			o = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*o));
		return o ? o->getClient() : 0;
	}

	// ----------------------------------------------------------------------

	void logBaselines(NetworkId const &senderId, char const *messageType, std::vector<NetworkId> const & clients)
	{
		if (!ConfigServerGame::getLogBaselines())
			return;

		for (std::vector<NetworkId>::const_iterator i = clients.begin(); i != clients.end(); ++i)
		{
			DEBUG_REPORT_LOG(true, ("Baselines: %s for object %s to client %s\n", messageType, senderId.getValueString().c_str(), (*i).getValueString().c_str()));
			LOG(
				"Baselines",
				(
					"%s for object %s to client %s",
					messageType,
					senderId.getValueString().c_str(),
					(*i).getValueString().c_str()));
		}
	}

	// ----------------------------------------------------------------------

}
using namespace ServerObject_SynchronizationNamespace;

// ======================================================================

void ServerObject::synchronizationOnConstruct()
{
	m_dirtyArchiveCallback.set(*this, &ServerObject::onAutoDeltaChanged);
	m_authClientServerPackage.addOnDirtyCallback(&m_dirtyArchiveCallback);
	m_authClientServerPackage_np.addOnDirtyCallback(&m_dirtyArchiveCallback);
	m_firstParentAuthClientServerPackage.addOnDirtyCallback(&m_dirtyArchiveCallback);
	m_firstParentAuthClientServerPackage_np.addOnDirtyCallback(&m_dirtyArchiveCallback);
	m_serverPackage.addOnDirtyCallback(&m_dirtyArchiveCallback);
	m_serverPackage_np.addOnDirtyCallback(&m_dirtyArchiveCallback);
	m_sharedPackage.addOnDirtyCallback(&m_dirtyArchiveCallback);
	m_sharedPackage_np.addOnDirtyCallback(&m_dirtyArchiveCallback);
}

// ----------------------------------------------------------------------

void ServerObject::synchronizationOnDestruct()
{
	BaselineDistributionListPool::free(*this);

	m_authClientServerPackage.removeOnDirtyCallback();
	m_authClientServerPackage_np.removeOnDirtyCallback();
	m_firstParentAuthClientServerPackage.removeOnDirtyCallback();
	m_firstParentAuthClientServerPackage_np.removeOnDirtyCallback();
	m_serverPackage.removeOnDirtyCallback();
	m_serverPackage_np.removeOnDirtyCallback();
	m_sharedPackage.removeOnDirtyCallback();
	m_sharedPackage_np.removeOnDirtyCallback();

	ServerObjectBaselinesManager::serverObjDestroyed(*this);
}

// ----------------------------------------------------------------------

void ServerObject::synchronizationOnContainerChange(ServerObject const *oldContainer, ServerObject const *newContainer) const
{
	ServerObjectBaselinesManager::onContainerChanged(*this, oldContainer, newContainer);
}

// ----------------------------------------------------------------------

void ServerObject::synchronizationOnLoadedInContainer()
{
	ServerObject * const containingObject = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*this));
	FATAL(!containingObject, ("ServerObject::synchronizationOnLoadedInContainer: obj %s with unresolvable container", getDebugInformation().c_str()));

	if (containingObject->isInitialized() && containingObject->isAuthoritative())
	{
		ProxyList const &proxyList = containingObject->getExposedProxyList();
		if (!proxyList.empty())
		{
			ServerMessageForwarding::begin(std::vector<uint32>(proxyList.begin(), proxyList.end()));

			forwardServerCreateAndBaselines();

			ServerMessageForwarding::end();
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::synchronizationOnUnload() const
{
	if (isAuthoritative())
	{
		ProxyList const &proxyList = getExposedProxyList();
		if (!proxyList.empty())
		{
			ServerMessageForwarding::begin(std::vector<uint32>(proxyList.begin(), proxyList.end()));

			UnloadObjectMessage const unloadMessage(getNetworkId());
			ServerMessageForwarding::send(unloadMessage);

			ServerMessageForwarding::end();
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::onAutoDeltaChanged()
{
	setLocalFlag(LocalObjectFlags::ServerObject_AutoDeltaChanged, true);
	addObjectToConcludeList();
}

// ----------------------------------------------------------------------

void ServerObject::addAuthClientServerVariable(Archive::AutoDeltaVariableBase &source)
{
	m_authClientServerPackage.addVariable(source);
}

// ----------------------------------------------------------------------

void ServerObject::addAuthClientServerVariable_np(Archive::AutoDeltaVariableBase &source)
{
	m_authClientServerPackage_np.addVariable(source);
}

// ----------------------------------------------------------------------

void ServerObject::addFirstParentAuthClientServerVariable(Archive::AutoDeltaVariableBase &source)
{
	m_firstParentAuthClientServerPackage.addVariable(source);
}

// ----------------------------------------------------------------------

void ServerObject::addFirstParentAuthClientServerVariable_np(Archive::AutoDeltaVariableBase &source)
{
	m_firstParentAuthClientServerPackage_np.addVariable(source);
}

// ----------------------------------------------------------------------

void ServerObject::addServerVariable(Archive::AutoDeltaVariableBase &source)
{
	m_serverPackage.addVariable(source);
}

// ----------------------------------------------------------------------

void ServerObject::addServerVariable_np(Archive::AutoDeltaVariableBase &source)
{
	m_serverPackage_np.addVariable(source);
}

// ----------------------------------------------------------------------

void ServerObject::addSharedVariable(Archive::AutoDeltaVariableBase &source)
{
	m_sharedPackage.addVariable(source);
}

// ----------------------------------------------------------------------

void ServerObject::addSharedVariable_np(Archive::AutoDeltaVariableBase &source)
{
	m_sharedPackage_np.addVariable(source);
}

// ----------------------------------------------------------------------

void ServerObject::sendCreateAndBaselinesToClient(Client &client) const
{
	if (getSendToClient() && !isBeingDestroyed())
	{
		if (!DestroyMessageManager::remove(&client, getNetworkId()))
		{
			std::vector<Watcher<Client> > *baselineDistributionList = BaselineDistributionListPool::get(*this, true);
			if (baselineDistributionList->empty())
				ServerObjectBaselinesManager::add(*this);
			baselineDistributionList->push_back(Watcher<Client>(&client));
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::sendCreateAndBaselinesToDatabaseServer() const
{
	if (isBeingDestroyed())
		return;

	FATAL(!isAuthoritative(), ("ServerObject::sendCreateAndBaselinesToDatabaseServer: obj %s, while non-auth", getDebugInformation().c_str()));
	FATAL(!isPersisted(), ("ServerObject::sendCreateAndBaselinesToDatabaseServer: obj %s, while nonpersisted", getDebugInformation().c_str()));

	GameServer::incrementTotalObjectCreatesSent();

	NetworkId const &containedById = ContainerInterface::getContainedByProperty(*this)->getContainedByNetworkId();
	NetworkId const &loadWithId = ContainerInterface::getLoadWithContainerId(*this);
	int const currentArrangement = ContainerInterface::getSlottedContainmentProperty(*this)->getCurrentArrangement();

	CreateObjectByCrcMessage const createMessage(getNetworkId(), getTemplateCrc(), getObjectType(), false, containedById);

	//Vector vp = getTransform_o2p().getPosition_p();
	//Vector vw = getTransform_o2w().getPosition_p();
	//LOG("ServerObject",("Baseline PS XYZ (%f %f %f)   WS XYZ (%f %f %f)",vp.x, vp.y, vp.z, vw.x, vw.y, vw.z));

	Transform worldspaceTransformForDatabase(getTransform_o2w());

	// if it's a player inside a container, the world space coordinate
	// that should be saved is the world space coordinate of the topmost
	// container; this way, player logging out of a house will get
	// loaded back into the same game server as the house, thus maximizing
	// that chance that the player will loaded back into the house
	if (isPlayerControlled() && asCreatureObject())
	{
		Object const *containerObj = ContainerInterface::getTopmostContainer(*this);

		if (containerObj && (containerObj != this))
		{
			worldspaceTransformForDatabase = containerObj->getTransform_o2w();
		}
	}

	UpdateObjectPositionMessage const uop(getNetworkId(), getTransform_o2p(), worldspaceTransformForDatabase, containedById, currentArrangement, loadWithId, isPlayerControlled(), asCreatureObject() ? true : false);
	BaselinesMessage const shared(getNetworkId(), getObjectType(), m_sharedPackage, BaselinesMessage::BASELINES_SHARED);
	BaselinesMessage const authClient(getNetworkId(), getObjectType(), m_authClientServerPackage, BaselinesMessage::BASELINES_CLIENT_SERVER);
	BaselinesMessage const servers(getNetworkId(), getObjectType(), m_serverPackage, BaselinesMessage::BASELINES_SERVER);
	BaselinesMessage const firstParentAuthClient(getNetworkId(), getObjectType(), m_firstParentAuthClientServerPackage, BaselinesMessage::BASELINES_FIRST_PARENT_CLIENT_SERVER);

	GameServer::getInstance().sendToDatabaseServer(createMessage);
	GameServer::getInstance().sendToDatabaseServer(uop);
	GameServer::getInstance().sendToDatabaseServer(shared);
	GameServer::getInstance().sendToDatabaseServer(authClient);
	GameServer::getInstance().sendToDatabaseServer(servers);
	GameServer::getInstance().sendToDatabaseServer(firstParentAuthClient);

	Container const * const container = ContainerInterface::getContainer(*this);
	if (container)
	{
		for (ContainerConstIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
		{
			ServerObject const * const containedObject = safe_cast<ServerObject const *>((*containerIterator).getObject());
			if (containedObject && containedObject->isPersisted())
				containedObject->sendCreateAndBaselinesToDatabaseServer();
		}
	}

	EndBaselinesMessage const endBaselinesMessage(getNetworkId());
	GameServer::getInstance().sendToDatabaseServer(endBaselinesMessage);
}

// ----------------------------------------------------------------------

void ServerObject::forwardServerCreateAndBaselines() const
{
	if (isBeingDestroyed())
		return;

	FATAL(!isAuthoritative(), ("ServerObject::forwardServerCreateAndBaselines: obj %s, while non-auth", getDebugInformation().c_str()));

	GameServer::incrementTotalObjectCreatesSent();

	NetworkId const &containedById = ContainerInterface::getContainedByProperty(*this)->getContainedByNetworkId();
	NetworkId const &loadWithId = ContainerInterface::getLoadWithContainerId(*this);
	int const currentArrangement = ContainerInterface::getSlottedContainmentProperty(*this)->getCurrentArrangement();

	CreateObjectByCrcMessage const createMessage(getNetworkId(), getTemplateCrc(), getObjectType(), false, containedById);

	//Vector vp = getTransform_o2p().getPosition_p();
	//Vector vw = getTransform_o2w().getPosition_p();
	//LOG("ServerObject",("forwardServerCreateAndBaselines PS XYZ (%f %f %f)   WS XYZ (%f %f %f)",vp.x, vp.y, vp.z, vw.x, vw.y, vw.z));

	UpdateObjectPositionMessage const uop(getNetworkId(), getTransform_o2p(), getTransform_o2w(), containedById, currentArrangement, loadWithId, isPlayerControlled(), asCreatureObject() ? true : false);
	BaselinesMessage const shared(getNetworkId(), getObjectType(), m_sharedPackage, BaselinesMessage::BASELINES_SHARED);
	BaselinesMessage const authClient(getNetworkId(), getObjectType(), m_authClientServerPackage, BaselinesMessage::BASELINES_CLIENT_SERVER);
	BaselinesMessage const servers(getNetworkId(), getObjectType(), m_serverPackage, BaselinesMessage::BASELINES_SERVER);
	BaselinesMessage const sharedNp(getNetworkId(), getObjectType(), m_sharedPackage_np, BaselinesMessage::BASELINES_SHARED_NP);
	BaselinesMessage const authClientNp(getNetworkId(), getObjectType(), m_authClientServerPackage_np, BaselinesMessage::BASELINES_CLIENT_SERVER_NP);
	BaselinesMessage const serversNp(getNetworkId(), getObjectType(), m_serverPackage_np, BaselinesMessage::BASELINES_SERVER_NP);
	BaselinesMessage const firstParentAuthClient(getNetworkId(), getObjectType(), m_firstParentAuthClientServerPackage, BaselinesMessage::BASELINES_FIRST_PARENT_CLIENT_SERVER);
	BaselinesMessage const firstParentAuthClientNp(getNetworkId(), getObjectType(), m_firstParentAuthClientServerPackage_np, BaselinesMessage::BASELINES_FIRST_PARENT_CLIENT_SERVER_NP);

	ServerMessageForwarding::send(createMessage);
	ServerMessageForwarding::send(uop);
	ServerMessageForwarding::send(shared);
	ServerMessageForwarding::send(authClient);
	ServerMessageForwarding::send(servers);
	ServerMessageForwarding::send(sharedNp);
	ServerMessageForwarding::send(authClientNp);
	ServerMessageForwarding::send(firstParentAuthClient);
	ServerMessageForwarding::send(firstParentAuthClientNp);
	ServerMessageForwarding::send(serversNp);

	if (m_scriptObject)
	{
		std::vector<int8> scriptVars;
		m_scriptObject->packScriptVars(scriptVars);
		if (!scriptVars.empty())
		{
			SynchronizeScriptVarsMessage const scriptVarsMessage(getNetworkId(), scriptVars);
			ServerMessageForwarding::send(scriptVarsMessage);
		}
	}

	forwardServerObjectSpecificBaselines();

	Container const * const container = ContainerInterface::getContainer(*this);
	if (container)
	{
		for (ContainerConstIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
		{
			ServerObject const * const containedObject = safe_cast<ServerObject const *>((*containerIterator).getObject());
			if (containedObject)
				containedObject->forwardServerCreateAndBaselines();
		}
	}

	EndBaselinesMessage const endBaselinesMessage(getNetworkId());
	ServerMessageForwarding::send(endBaselinesMessage);
}

// ----------------------------------------------------------------------

void ServerObject::sendDeltasForSelfAndContents(ProxyList const *proxyList) const
{
	sendDeltas(proxyList);

	Container const * const container = ContainerInterface::getContainer(*this);
	if (container)
	{
		for (ContainerConstIterator i = container->begin(); i != container->end(); ++i)
		{
			ServerObject const * const containedObject = safe_cast<ServerObject const *>((*i).getObject());
			if (containedObject)
				containedObject->sendDeltasForSelfAndContents(proxyList);
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::sendDeltas(ProxyList const *proxyList) const
{
	FATAL(!isInitialized(), ("ServerObject::sendDeltas: %s while uninitialized.", getDebugInformation().c_str()));

	if (!getLocalFlag(LocalObjectFlags::ServerObject_AutoDeltaChanged) || isBeingDestroyed())
		return;

	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::sendDeltas");

	ProxyList const * const proxyServers = proxyList ? proxyList : &getExposedProxyList();

	bool const sendServerDeltas = !proxyServers->empty();

	if (sendServerDeltas)
		ServerMessageForwarding::begin(std::vector<uint32>(proxyServers->begin(), proxyServers->end()));

	// sync auth client data (if we have an auth client)
	if (   m_authClientServerPackage.getItemCount() > 0
	    || m_authClientServerPackage_np.getItemCount() > 0)
	{
		static std::set<Client const *> authClients;

		{
			PROFILER_AUTO_BLOCK_DEFINE("getAuthClients");
			getAuthClients(authClients);
		}

		if (m_authClientServerPackage.getItemCount() > 0)
		{
			PROFILER_AUTO_BLOCK_DEFINE("authClientServerPackage");
			DeltasMessage const cdm(getNetworkId(), getObjectType(), m_authClientServerPackage, DeltasMessage::DELTAS_CLIENT_SERVER);
			for (std::set<Client const *>::const_iterator i = authClients.begin(); i != authClients.end(); ++i)
				(*i)->send(cdm, true);
			if (isAuthoritative())
			{
				if (sendServerDeltas)
					ServerMessageForwarding::send(cdm);
				if (isPersisted())
					GameServer::getInstance().sendToDatabaseServer(cdm);
			}
		}
		if (m_authClientServerPackage_np.getItemCount() > 0)
		{
			PROFILER_AUTO_BLOCK_DEFINE("authClientServerPackage_np");
			DeltasMessage const cdm(getNetworkId(), getObjectType(), m_authClientServerPackage_np, DeltasMessage::DELTAS_CLIENT_SERVER_NP);
			for (std::set<Client const *>::const_iterator i = authClients.begin(); i != authClients.end(); ++i)
				(*i)->send(cdm, true);
			if (isAuthoritative() && sendServerDeltas)
				ServerMessageForwarding::send(cdm);
		}
		authClients.clear();
	}

	// first parent auth client data
	{
		Client const * const parentPlayerClient = getParentPlayerClient(this);

		if (m_firstParentAuthClientServerPackage.getItemCount() > 0)
		{
			PROFILER_AUTO_BLOCK_DEFINE("firstParentAuthClientServerPackage");
			DeltasMessage const cdm(getNetworkId(), getObjectType(), m_firstParentAuthClientServerPackage, DeltasMessage::DELTAS_FIRST_PARENT_CLIENT_SERVER);
			if (parentPlayerClient)
				parentPlayerClient->send(cdm, true);
			if (isAuthoritative())
			{
				if (sendServerDeltas)
					ServerMessageForwarding::send(cdm);
				if (isPersisted())
					GameServer::getInstance().sendToDatabaseServer(cdm);
			}
		}

		if (m_firstParentAuthClientServerPackage_np.getItemCount() > 0)
		{
			PROFILER_AUTO_BLOCK_DEFINE("firstParentAuthClientServerPackage_np");
			DeltasMessage const cdm(getNetworkId(), getObjectType(), m_firstParentAuthClientServerPackage_np, DeltasMessage::DELTAS_FIRST_PARENT_CLIENT_SERVER_NP);
			if (parentPlayerClient)
				parentPlayerClient->send(cdm, true);
			if (isAuthoritative() && sendServerDeltas)
				ServerMessageForwarding::send(cdm);
		}
	}

	// sync shared data
	if (m_sharedPackage.getItemCount() > 0)
	{
		PROFILER_AUTO_BLOCK_DEFINE("sharedPackage");
		DeltasMessage const sharedPkg(getNetworkId(), getObjectType(), m_sharedPackage, DeltasMessage::DELTAS_SHARED);
		sendToClientsInUpdateRange(sharedPkg, true);
		if (isAuthoritative())
		{
			if (sendServerDeltas)
				ServerMessageForwarding::send(sharedPkg);
			if (isPersisted())
				GameServer::getInstance().sendToDatabaseServer(sharedPkg);
		}
	}
	if (m_sharedPackage_np.getItemCount() > 0)
	{
		PROFILER_AUTO_BLOCK_DEFINE("sharedPackage_np");
		DeltasMessage const sharedPkgNp(getNetworkId(), getObjectType(), m_sharedPackage_np, DeltasMessage::DELTAS_SHARED_NP);
		sendToClientsInUpdateRange(sharedPkgNp, true);
		if (isAuthoritative() && sendServerDeltas)
			ServerMessageForwarding::send(sharedPkgNp);
	}

	// sync server data (if we are authoritative)
	if (isAuthoritative())
	{
		if (m_serverPackage.getItemCount() > 0)
		{
			PROFILER_AUTO_BLOCK_DEFINE("serverPackage");
			DeltasMessage const server(getNetworkId(), getObjectType(), m_serverPackage, DeltasMessage::DELTAS_SERVER);
			if (sendServerDeltas)
				ServerMessageForwarding::send(server);
			if (isPersisted())
				GameServer::getInstance().sendToDatabaseServer(server);
		}
		if (sendServerDeltas && m_serverPackage_np.getItemCount() > 0)
		{
			PROFILER_AUTO_BLOCK_DEFINE("serverPackage_np");
			DeltasMessage const serverNp(getNetworkId(), getObjectType(), m_serverPackage_np, DeltasMessage::DELTAS_SERVER_NP);
			ServerMessageForwarding::send(serverNp);
		}
	}

	clearDeltas();

	if (sendServerDeltas)
		ServerMessageForwarding::end();

	// sync any real time UI
	if (m_synchronizedUi.getPointer())
		m_synchronizedUi->sendDeltas();
}

// ----------------------------------------------------------------------

void ServerObject::applyObjectPositionUpdate(UpdateObjectPositionMessage const &source)
{
	FATAL(isInitialized(), ("ServerObject::applyObjectPositionUpdate: obj %s, while initialized", getDebugInformation().c_str()));

	// catch the situation where we are trying to put the
	// (authoritative) object into a proxy container;
	// there are bad contained_by/load_with data in the DB
	// that can cause this situation
	ServerObject * const container = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(source.getContainerId()));
	if (container && !container->isAuthoritative() && isAuthoritative())
	{
		// it's OK if this happens to a character object;
		// fix it by moving the character outside of the container
		if (source.getPlayerControlled() && !ServerWorld::isSpaceScene())
		{
			DEBUG_REPORT_LOG(true,("Ejecting player %s from container %s, because the container is not authoritative\n",getNetworkId().getValueString().c_str(), container->getNetworkId().getValueString().c_str()));
			LOG("CustomerService",("Login:  Ejecting player %s from container %s, because the container is not authoritative",getNetworkId().getValueString().c_str(), container->getNetworkId().getValueString().c_str()));

			// attempt to put the character at the ejection point of the building
			Object * const topmostObject = ContainerInterface::getTopmostContainer(*container);
			if (topmostObject)
			{
				PortalProperty * const portalProperty = topmostObject->getPortalProperty();
				if (portalProperty)
				{
					Transform destinationTransform(Transform::IF_none);
					destinationTransform.multiply(topmostObject->getTransform_o2w(), portalProperty->getEjectionLocationTransform());
					setTransform_o2p(destinationTransform);
				}
				else
				{
					// put the character at the location of the building
					setTransform_o2p(topmostObject->getTransform_o2w());
				}
			}
			else
			{
				// last resort, just leave him where he is, out
				// in the world, which will most likely put him
				// near the origin
				setTransform_o2p(source.getTransform());
			}

			getContainedByProperty()->setContainedBy(NetworkId::cms_invalid, false);
			ContainerInterface::getSlottedContainmentProperty(*this)->setCurrentArrangement(-1, false);
		}
		else
		{
			ServerObject const * const loadWith = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(source.getLoadWith()));

			LOG("CustomerService", ("BadContainerDepersist: Tried to depersist auth object %s (load with %s) into non-auth container %s.", getDebugInformation().c_str(), (loadWith ? loadWith->getDebugInformation().c_str() : source.getLoadWith().getValueString().c_str()) , container->getDebugInformation().c_str()));
			unload();
		}

		return;
	}

	setTransform_o2p(source.getTransform());
	getContainedByProperty()->setContainedBy(source.getContainerId(), false);
	ContainerInterface::getSlottedContainmentProperty(*this)->setCurrentArrangement(source.getSlotArrangement(), false);
}

// ----------------------------------------------------------------------

void ServerObject::updateContainment(NetworkId const &containerId, int slotArrangement)
{
	FATAL(isAuthoritative(), ("ServerObject::updateContainment: obj %s, container %s, arrangement %d, while auth", getDebugInformation().c_str(), containerId.getValueString().c_str(), slotArrangement));

	getContainedByProperty()->setContainedBy(containerId, false);
	ContainerInterface::getSlottedContainmentProperty(*this)->setCurrentArrangement(slotArrangement, false);
	ContainmentMessageManager::addContainmentMessage(*this);
}

// ----------------------------------------------------------------------

void ServerObject::containerHandleUpdateProxies(ServerObject *oldContainer, ServerObject *newContainer)
{
	FATAL(!isAuthoritative(), ("ServerObject::containerHandleUpdateProxies: obj %s, while non-auth", getDebugInformation().c_str()));

	// Queue containment update to be sent to local clients - proxy clients will get the update from the change on their server.
	ContainmentMessageManager::addContainmentMessage(*this);

	// determine old and new topmost containers
	ServerObject * const oldTopmost = oldContainer ? NON_NULL(safe_cast<ServerObject *>(ContainerInterface::getTopmostContainer(*oldContainer))) : this;
	ServerObject * const newTopmost = newContainer ? NON_NULL(safe_cast<ServerObject *>(ContainerInterface::getTopmostContainer(*newContainer))) : this;

	// determine old and new proxy lists
	ProxyList const &oldProxyList = oldTopmost->getPrivateProxyList();
	ProxyList const &newProxyList = newContainer ? newTopmost->getPrivateProxyList() : oldProxyList;

	// send unloads to servers only in old proxy lists
	// send baselines to servers only in new proxy lists
	// send containment update to servers in both old and new proxy lists

	if (oldTopmost == newTopmost)
	{
		// If we have not changed topmost containers, our proxy list has not changed,
		// so send containment updates to everything in our proxy list.
		if (!newProxyList.empty())
		{
			ServerMessageForwarding::begin(std::vector<uint32>(newProxyList.begin(), newProxyList.end()));

			UpdateContainmentMessage const ucm(
				getNetworkId(),
				newContainer ? newContainer->getNetworkId() : NetworkId::cms_invalid,
				ContainerInterface::getSlottedContainmentProperty(*this)->getCurrentArrangement());
			ServerMessageForwarding::send(ucm);

			ServerMessageForwarding::end();
		}
	}
	else
	{
		std::vector<uint32> addedProxies;
		std::vector<uint32> removedProxies;
		std::vector<uint32> sustainedProxies;

		for (ProxyList::const_iterator i = oldProxyList.begin(); i != oldProxyList.end(); ++i)
		{
			if (!newProxyList.count(*i))
				removedProxies.push_back(*i);
			else
				sustainedProxies.push_back(*i);
		}

		for (ProxyList::const_iterator j = newProxyList.begin(); j != newProxyList.end(); ++j)
			if (!oldProxyList.count(*j))
				addedProxies.push_back(*j);

		if (!removedProxies.empty())
		{
			ServerMessageForwarding::begin(removedProxies);

			UnloadObjectMessage const unloadMessage(getNetworkId());
			ServerMessageForwarding::send(unloadMessage);

			ServerMessageForwarding::end();
		}

		if (!sustainedProxies.empty())
		{
			ServerMessageForwarding::begin(sustainedProxies);

			UpdateContainmentMessage const ucm(
				getNetworkId(),
				newContainer ? newContainer->getNetworkId() : NetworkId::cms_invalid,
				ContainerInterface::getSlottedContainmentProperty(*this)->getCurrentArrangement());
			ServerMessageForwarding::send(ucm);

			ServerMessageForwarding::end();
		}

		if (!addedProxies.empty())
		{
			ServerMessageForwarding::begin(addedProxies);

			forwardServerCreateAndBaselines();

			ServerMessageForwarding::end();
		}
	}

	if (newContainer)
		m_proxyServerProcessIds.clear();
	else
		copyProxyList(*oldTopmost);
}

// ----------------------------------------------------------------------

void ServerObject::applyBaselines(BaselinesMessage const &source)
{
	applyBaselines(source.getPackageId(), source.getPackage());
}

// ----------------------------------------------------------------------

void ServerObject::applyBaselines(char type, Archive::ByteStream const & package)
{
	Archive::ReadIterator ri = package.begin();
	switch (type)
	{
	case BaselinesMessage::BASELINES_CLIENT_SERVER:
		m_authClientServerPackage.unpack(ri);
		break;
	case BaselinesMessage::BASELINES_SERVER:
		m_serverPackage.unpack(ri);
		break;
	case BaselinesMessage::BASELINES_SHARED:
		m_sharedPackage.unpack(ri);
		break;
	case BaselinesMessage::BASELINES_CLIENT_SERVER_NP:
		m_authClientServerPackage_np.unpack(ri);
		break;
	case BaselinesMessage::BASELINES_SERVER_NP:
		m_serverPackage_np.unpack(ri);
		break;
	case BaselinesMessage::BASELINES_SHARED_NP:
		m_sharedPackage_np.unpack(ri);
		break;
	case BaselinesMessage::BASELINES_FIRST_PARENT_CLIENT_SERVER:
		m_firstParentAuthClientServerPackage.unpack(ri);
		break;
	case BaselinesMessage::BASELINES_FIRST_PARENT_CLIENT_SERVER_NP:
		m_firstParentAuthClientServerPackage_np.unpack(ri);
		break;
	case BaselinesMessage::BASELINES_UI:
		#ifdef _DEBUG
		WARNING_STRICT_FATAL(!m_synchronizedUi, ("Got UI package for %s with no UI object", getNetworkId().getValueString().c_str()));
		#endif
		if (m_synchronizedUi.getPointer())
			m_synchronizedUi->applyBaselines(ri);
		break;
	default:
		FATAL(true, ("ServerObject::applyBaselines on %s with unknown package %d", getDebugInformation().c_str(), static_cast<int>(type)));
		break;
	}
}

// ----------------------------------------------------------------------

void ServerObject::applyDeltas(DeltasMessage const &source)
{
	FATAL(!isInitialized(), ("ServerObject::applyDeltas on unitinialized object %s.", getDebugInformation().c_str()));
	WARNING(isAuthoritative(), ("ServerObject::applyDeltas on auth object %s.", getDebugInformation().c_str()));

	unsigned char const type = source.getPackageId();
	Archive::ReadIterator ri = source.getPackage().begin();
	switch (type)
	{
	case DeltasMessage::DELTAS_CLIENT_SERVER:
		m_authClientServerPackage.unpackDeltas(ri);
		break;
	case DeltasMessage::DELTAS_SERVER:
		m_serverPackage.unpackDeltas(ri);
		break;
	case DeltasMessage::DELTAS_SHARED:
		m_sharedPackage.unpackDeltas(ri);
		break;
	case DeltasMessage::DELTAS_CLIENT_SERVER_NP:
		m_authClientServerPackage_np.unpackDeltas(ri);
		break;
	case DeltasMessage::DELTAS_SERVER_NP:
		m_serverPackage_np.unpackDeltas(ri);
		break;
	case DeltasMessage::DELTAS_SHARED_NP:
		m_sharedPackage_np.unpackDeltas(ri);
		break;
	case DeltasMessage::DELTAS_FIRST_PARENT_CLIENT_SERVER:
		m_firstParentAuthClientServerPackage.unpackDeltas(ri);
		break;
	case DeltasMessage::DELTAS_FIRST_PARENT_CLIENT_SERVER_NP:
		m_firstParentAuthClientServerPackage_np.unpackDeltas(ri);
		break;
	case DeltasMessage::DELTAS_UI:
		#ifdef _DEBUG
		WARNING_STRICT_FATAL(!m_synchronizedUi, ("Got UI delta package for %s with no UI object", getNetworkId().getValueString().c_str()));
		#endif
		if (m_synchronizedUi.getPointer())
			m_synchronizedUi->applyDeltas(source);
		break;
	default:
		FATAL(true, ("ServerObject::applyDeltas on %s with unknown package %d", getDebugInformation().c_str(), static_cast<int>(type)));
		break;
	}
}

// ----------------------------------------------------------------------

bool ServerObject::removeFromBaselineDistributionList(Client const &client) const
{
	std::vector<Watcher<Client> > *baselineDistributionList = BaselineDistributionListPool::get(*this, false);
	if (baselineDistributionList)
	{
		std::vector<Watcher<Client> >::iterator i = std::find(baselineDistributionList->begin(), baselineDistributionList->end(), &client);
		if (i != baselineDistributionList->end())
		{
			IGNORE_RETURN(baselineDistributionList->erase(i));
			return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------

void ServerObject::sendAuthClientBaselines(Client const &client) const
{
	BaselinesMessage const authClientSvrPkg(getNetworkId(), getObjectType(), m_authClientServerPackage, BaselinesMessage::BASELINES_CLIENT_SERVER);
	BaselinesMessage const authClientSvrPkgNp(getNetworkId(), getObjectType(), m_authClientServerPackage_np, BaselinesMessage::BASELINES_CLIENT_SERVER_NP);
	client.send(authClientSvrPkg, true);
	client.send(authClientSvrPkgNp, true);
}

// ----------------------------------------------------------------------

void ServerObject::sendFirstParentBaselines(Client const &client) const
{
	BaselinesMessage const firstParentAuthClientSvrPkg(getNetworkId(), getObjectType(), m_firstParentAuthClientServerPackage, BaselinesMessage::BASELINES_FIRST_PARENT_CLIENT_SERVER);
	BaselinesMessage const firstParentAuthClientSvrPkgNp(getNetworkId(), getObjectType(), m_firstParentAuthClientServerPackage_np, BaselinesMessage::BASELINES_FIRST_PARENT_CLIENT_SERVER_NP);
	client.send(firstParentAuthClientSvrPkg, true);
	client.send(firstParentAuthClientSvrPkgNp, true);
}

// ----------------------------------------------------------------------

void ServerObject::flushCreateMessages() const
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::flushCreateMessages");

	// Validate this object's transform prior to sending baseline messages.
	checkAndLogInvalidTransform();

	std::map<ConnectionServerConnection *, std::vector<NetworkId> > &tmpDistributionList = DistributionListStack::alloc();

	std::vector<Watcher<Client> > * const baselineDistributionList = BaselineDistributionListPool::get(*this, false);

	if (baselineDistributionList && !baselineDistributionList->empty())
	{
		ContainmentMessageManager::addFrameBaselines(getNetworkId(), *baselineDistributionList);

		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerObject::flushCreateMessages.buildConnectionList");
			for (std::vector<Watcher<Client> >::const_iterator i = baselineDistributionList->begin(); i != baselineDistributionList->end(); ++i)
			{
				// build connection list
				Client * const c = *i;
				if (c)
				{
					ConnectionServerConnection * const n = c->getConnection();
					if (n)
					{
						std::vector<NetworkId> &list = tmpDistributionList[n];
						list.push_back(c->getCharacterObjectId());
					}
				}
			}
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerObject::flushCreateMessages.send");
			//-- build client object creation messages
			GameNetworkMessage *com = 0;
			ConstCharCrcString const clientSharedTemplateNameCrcString = ObjectTemplateList::lookUp(getClientSharedTemplateName());
			if (clientSharedTemplateNameCrcString.isEmpty())
				com = new SceneCreateObjectByName(getNetworkId(), getTransform_o2p(), getClientSharedTemplateName(), getLocalFlag(LocalObjectFlags::ServerObject_HyperspaceOnCreate));
			else
				com = new SceneCreateObjectByCrc(getNetworkId(), getTransform_o2p(), clientSharedTemplateNameCrcString.getCrc(), getLocalFlag(LocalObjectFlags::ServerObject_HyperspaceOnCreate));

			UpdateContainmentMessage const ucm(
				getNetworkId(),
				ContainerInterface::getContainedByProperty(*this)->getContainedByNetworkId(),
				ContainerInterface::getSlottedContainmentProperty(*this)->getCurrentArrangement());
			bool sendContainmentUpdate = (ucm.getContainerId() != NetworkId::cms_invalid || ucm.getSlotArrangement() != -1);

			BaselinesMessage const sharedPkg(getNetworkId(), getObjectType(), m_sharedPackage, BaselinesMessage::BASELINES_SHARED);
			BaselinesMessage const sharedPkgNp(getNetworkId(), getObjectType(), m_sharedPackage_np, BaselinesMessage::BASELINES_SHARED_NP);

			for (std::map<ConnectionServerConnection *, std::vector<NetworkId> >::const_iterator i = tmpDistributionList.begin(); i != tmpDistributionList.end(); ++i)
			{
				GameClientMessage const msgCom((*i).second, true, *com);
				(*i).first->send(msgCom, true);
				logBaselines(getNetworkId(), com->getCmdName().c_str(), (*i).second);

				// send containment updates
				if (sendContainmentUpdate)
				{
					GameClientMessage const msgUpdateContainment((*i).second, true, ucm);
					(*i).first->send(msgUpdateContainment, true);
				}

				// send shared data baselines
				GameClientMessage const msgSharedPkg((*i).second, true, sharedPkg);
				(*i).first->send(msgSharedPkg, true);

				GameClientMessage const msgSharedPkgNp((*i).second, true, sharedPkgNp);
				(*i).first->send(msgSharedPkgNp, true);
			}
			delete com;
		}

		// send auth client data
		{
			static std::set<Client const *> authClients;
			getAuthClients(authClients);
			for (std::set<Client const *>::const_iterator i = authClients.begin(); i != authClients.end(); ++i)
				if ((*i) && std::find(baselineDistributionList->begin(), baselineDistributionList->end(), (*i)) != baselineDistributionList->end())
					sendAuthClientBaselines(**i);
			authClients.clear();

			Client const * const parentPlayerClient = getParentPlayerClient(this);
			if (parentPlayerClient && std::find(baselineDistributionList->begin(), baselineDistributionList->end(), parentPlayerClient) != baselineDistributionList->end())
				sendFirstParentBaselines(*parentPlayerClient);
		}
	}

	// Flush create messages for contents
	Container const * const container = ContainerInterface::getContainer(*this);
	if (container)
	{
		PROFILER_AUTO_BLOCK_DEFINE("ServerObject::flushCreateMessages.flushContents");
		for (ContainerConstIterator i = container->begin(); i != container->end(); ++i)
		{
			ServerObject const * const containedObject = safe_cast<ServerObject const *>((*i).getObject());
			if (containedObject)
				containedObject->flushCreateMessages();
		}
	}

	if (baselineDistributionList && !baselineDistributionList->empty())
	{
		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerObject::flushCreateMessages.sendObjectSpecificBaselinesToClient");
			for (std::vector<Watcher<Client> >::const_iterator i = baselineDistributionList->begin(); i != baselineDistributionList->end(); ++i)
				if ((*i).getPointer())
					sendObjectSpecificBaselinesToClient(**i);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("ServerObject::flushCreateMessages.sendEndBaselines");
			SceneEndBaselines const ebm(getNetworkId());
			for (std::map<ConnectionServerConnection *, std::vector<NetworkId> >::const_iterator i = tmpDistributionList.begin(); i != tmpDistributionList.end(); ++i)
			{
				GameClientMessage const msgEbm((*i).second, true, ebm);
				(*i).first->send(msgEbm, true);
				logBaselines(getNetworkId(), "SceneEndBaselines", (*i).second);
			}
		}

		baselineDistributionList->clear();
	}

	if (baselineDistributionList)
		BaselineDistributionListPool::free(*this);

	DistributionListStack::release();
}

// ----------------------------------------------------------------------

void ServerObject::clearDeltas() const
{
	m_authClientServerPackage.clearDeltas();
	m_authClientServerPackage_np.clearDeltas();
	m_firstParentAuthClientServerPackage.clearDeltas();
	m_firstParentAuthClientServerPackage_np.clearDeltas();
	m_serverPackage.clearDeltas();
	m_serverPackage_np.clearDeltas();
	m_sharedPackage.clearDeltas();
	m_sharedPackage_np.clearDeltas();
}

// ----------------------------------------------------------------------

void ServerObject::addServerToProxyList(uint32 proxyServerProcessId)
{
	FATAL(!isAuthoritative(), ("ServerObject::addServerToProxyList: obj %s, processId %lu, while non-auth", getDebugInformation().c_str(), proxyServerProcessId));

	if (isProxiedOnServer(proxyServerProcessId))
		return;

	m_proxyServerProcessIds.insert(proxyServerProcessId);

	if (ContainerInterface::getTopmostContainer(*this) == this)
	{
		ServerMessageForwarding::begin(proxyServerProcessId);

		forwardServerCreateAndBaselines();

		ServerMessageForwarding::end();
	}
}

// ----------------------------------------------------------------------

void ServerObject::removeServerFromProxyList(uint32 proxyServerProcessId)
{
	if (!isProxiedOnServer(proxyServerProcessId))
		return;

	if (getAuthServerProcessId() != proxyServerProcessId)
	{
		FATAL(!isAuthoritative(), ("ServerObject::removeServerFromProxyList: obj %s, processId %lu, while non-auth", getDebugInformation().c_str(), proxyServerProcessId));

		ServerMessageForwarding::begin(proxyServerProcessId);

		UnloadObjectMessage const unloadMessage(getNetworkId());
		ServerMessageForwarding::send(unloadMessage);

		ServerMessageForwarding::end();
	}

	m_proxyServerProcessIds.erase(proxyServerProcessId);
}

// ----------------------------------------------------------------------

void ServerObject::clearProxyList()
{
	if (!m_proxyServerProcessIds.empty())
	{
		ProxyList const proxyList = m_proxyServerProcessIds.get();
		for (ProxyList::const_iterator i = proxyList.begin(); i != proxyList.end(); ++i)
			removeServerFromProxyList(*i);
	}
}

// ----------------------------------------------------------------------

void ServerObject::copyProxyList(ServerObject const &source)
{
	m_proxyServerProcessIds.clear();
	ProxyList const &sourceList = source.getPrivateProxyList();
	for (ProxyList::const_iterator i = sourceList.begin(); i != sourceList.end(); ++i)
		m_proxyServerProcessIds.insert(*i);
}

// ----------------------------------------------------------------------

size_t ServerObject::getProxyCount() const
{
	return m_proxyServerProcessIds.get().size();
}

// ----------------------------------------------------------------------

uint32 ServerObject::getAuthServerProcessId() const
{
	return m_authServerProcessId.get();
}

// ----------------------------------------------------------------------

void ServerObject::setAuthServerProcessId(uint32 processId)
{
	if (processId)
	{
		m_authServerProcessId = processId;
		removeServerFromProxyList(processId);

		Container * const container = ContainerInterface::getContainer(*this);
		if (container)
		{
			for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
			{
				ServerObject * const containedObject = safe_cast<ServerObject *>((*containerIterator).getObject());
				if (containedObject)
					containedObject->setAuthServerProcessId(processId);
			}
		}
	}
}

// ----------------------------------------------------------------------

ProxyList const &ServerObject::getExposedProxyList() const
{
	ServerObject const * const topmost = NON_NULL(safe_cast<ServerObject const *>(ContainerInterface::getTopmostContainer(*this)));
	return topmost->getPrivateProxyList();
}

// ----------------------------------------------------------------------

bool ServerObject::isProxiedOnServer(uint32 processId) const
{
	return m_proxyServerProcessIds.contains(processId);
}

// ======================================================================

