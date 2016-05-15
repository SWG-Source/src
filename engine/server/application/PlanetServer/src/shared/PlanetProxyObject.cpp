// ======================================================================
//
// PlanetProxyObject.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstPlanetServer.h"
#include "PlanetProxyObject.h"

#include "ConfigPlanetServer.h"
#include "GameServerData.h"
#include "PlanetServer.h"
#include "QuadtreeNode.h"
#include "Scene.h"
#include "WatcherConnection.h"
#include "serverNetworkMessages/LoadObjectMessage.h"
#include "serverNetworkMessages/SetAuthoritativeMessage.h"
#include "serverNetworkMessages/UnloadProxyMessage.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Tag.h"
#include "sharedLog/Log.h"
#include "sharedMathArchive/TransformArchive.h"

#include <algorithm>

// ======================================================================

MemoryBlockManager* PlanetProxyObject::memoryBlockManager;

// ======================================================================

PlanetProxyObject::PlanetProxyObject (const NetworkId &objectId) :
		m_objectId(objectId),
		m_containedBy(NetworkId::cms_invalid),
		m_x(0),
		m_y(0),
		m_z(0),
		m_authoritativeServer(0),
		m_lastReportedServer(0),
		m_interestRadius(0),
		m_quadtreeNode(nullptr),
		m_authTransferTimeMs(Clock::timeMs()),
		m_contents(nullptr),
		m_level(0),
		m_hibernating(false),
		m_templateCrc(0),
		m_aiActivity(-1),
		m_creationType(-1)
{
}

// ----------------------------------------------------------------------

PlanetProxyObject::~PlanetProxyObject ()
{
	if (PlanetServer::getInstance().isWatcherPresent())
	{
		outputStatusToAll(true);
	}
	m_quadtreeNode=0;

	if (m_contents)
	{
		for (std::vector<NetworkId>::const_iterator i=m_contents->begin(); i!=m_contents->end(); ++i)
			WARNING(true,("Object %s was deleted without removing contained object %s first",m_objectId.getValueString().c_str(), i->getValueString().c_str()));
		delete m_contents;
		m_contents=0;
	}
}

// ----------------------------------------------------------------------

/**
 * Update an object with new data sent from a game server.
 * Checks to see what changed, then takes appropriate actions.
 */
void PlanetProxyObject::update(int x, int y, int z, NetworkId containedBy, uint32 authoritativeServer, int interestRadius, int objectTypeTag, int const level, bool const hibernating, uint32 const templateCrc, int const aiActivity, int const creationType)
{
	WARNING_DEBUG_FATAL(m_objectId==NetworkId::cms_invalid,("Object ID was 0."));
	bool watcherUpdateNeeded = false;

	m_level = level;
	m_hibernating = hibernating;
	m_templateCrc = templateCrc;
	m_aiActivity = aiActivity;
	m_creationType = creationType;

	if ((m_authoritativeServer != 0 ) && (authoritativeServer != m_authoritativeServer) && (containedBy == NetworkId::cms_invalid))
	{
		static unsigned long authTransferSanityCheckTimeMs = ConfigPlanetServer::getAuthTransferSanityCheckTimeMs();
		if (Clock::timeMs()-m_authTransferTimeMs > authTransferSanityCheckTimeMs)
		{
			DEBUG_WARNING(true, ("Resending auth transfer for %s to %lu due to receiving a stale position update.", m_objectId.getValueString().c_str(), authoritativeServer));
			sendAuthorityChange(authoritativeServer, m_authoritativeServer, false);
			return;
		}
		if (ConfigPlanetServer::getLogObjectLoading())
			LOG("ObjectLoading",("Ignoring update for %s from %lu because server was not authoritative.",m_objectId.getValueString().c_str(),authoritativeServer));
		return;
	}
	
	if (interestRadius>ConfigPlanetServer::getMaxInterestRadius())
		interestRadius = ConfigPlanetServer::getMaxInterestRadius();
	
	if (interestRadius>0 && PlanetServer::getInstance().isInTutorialMode())
		interestRadius = 1;

	if (containedBy!=NetworkId::cms_invalid)
	{
		// get coordinates from container
		PlanetProxyObject *container = Scene::getInstance().findObjectByID(containedBy);
		if (container)
		{
			x=container->getX();
			y=container->getY();
			z=container->getZ();
		}
		else
		{
			LOG("PlanetUpdate",("Game server said object %s was contained by object %s, but we could not find the container.",m_objectId.getValueString().c_str(),containedBy.getValueString().c_str()));
			containedBy = NetworkId::cms_invalid; // so that we'll try again when the object moves again, in case we just got objects out-of-order from the game server
		}
	}
	
	std::vector<uint32> oldProxyList;

	Node *newNode=Scene::getInstance().findNodeByPosition(x,z);
	NOT_NULL(newNode);
	if (newNode!=m_quadtreeNode || m_interestRadius != interestRadius || m_authoritativeServer != authoritativeServer || m_containedBy != containedBy)
	{
		watcherUpdateNeeded = true;

		// For debugging, update the contents of the containers
		if (ConfigPlanetServer::getEnableContentsChecking() && (m_containedBy!=containedBy))
			updateContentsTracking(containedBy);
		
		bool const firstUpdate = (m_quadtreeNode == nullptr);

		if (m_quadtreeNode!=nullptr) // if this is the first update for this object, m_quadtreeNode will be nullptr because it hasn't been placed anywhere yet
		{
			removeServerStatistics();

			// remember old list of proxies
			getServers(oldProxyList);
			
			// remove from the old location
			m_quadtreeNode->removeObject(this);

			if (ConfigPlanetServer::getLogObjectLoading())
				LOG("ObjectLoading", ("unsubscribing nodes for object id=[%s]", m_objectId.getValueString().c_str()));
			unsubscribeSurroundingNodes(false);
		}
		
		// update data
		m_quadtreeNode=newNode;
		m_x=x;
		m_y=y;
		m_z=z;
		m_interestRadius=interestRadius;
		m_objectTypeTag = objectTypeTag;
		m_authoritativeServer=authoritativeServer;
		m_containedBy=containedBy;

		// make sure the destination is loaded
		if (!newNode->isLoaded())
		{
			if (ConfigPlanetServer::getLogObjectLoading())
				LOG("ObjectLoading",("Loading node %s because object %s entered it.",newNode->getDebugNodeString().c_str(), m_objectId.getValueString().c_str()));
			newNode->load();
		}
			
		// check if we're on the right server
		if (!isAuthorityOk())
		{
			uint32 preferredServer = m_quadtreeNode->getPreferredServer();
			if (m_containedBy==NetworkId::cms_invalid)
			{
				if (ConfigPlanetServer::getLogObjectLoading())
					LOG("ObjectLoading",("Changing authority for object %s from %lu to %lu because node %s is authoritative on that server.",
										 m_objectId.getValueString().c_str(),m_authoritativeServer,preferredServer,m_quadtreeNode->getDebugNodeString().c_str()));
				changeAuthority(preferredServer, false, false); // changes m_authoritativeServer and sends message

				// we received the object for the first time, and we had to
				// transfer the object's authority, so we need to make sure
				// to unload the object on the current authoritative server,
				// if necessary
				if (firstUpdate)
					oldProxyList.push_back(authoritativeServer);
			}
			else
			{
				WARNING(true,("Programmer bug:  Object %s is in container %s, which is node %s.  The object should be authoritative on server %i but it is authoritative on server %i.",
							  m_objectId.getValueString().c_str(),m_containedBy.getValueString().c_str(),m_quadtreeNode->getDebugNodeString().c_str(), m_authoritativeServer, preferredServer));
			}
		}

		// place in the new location
		newNode->addObject(this);

		if (interestRadius > 0)
		{
			if (ConfigPlanetServer::getLogObjectLoading())
				LOG("ObjectLoading", ("subscribing nodes for object id=[%s]", m_objectId.getValueString().c_str()));
			subscribeSurroundingNodes();		
		}
		
		// check against new interested servers, and send proxy/unproxy as needed
		if (m_containedBy==NetworkId::cms_invalid) // we don't manage proxies for containers
		{
			std::vector<uint32> newProxyList;
			getServers(newProxyList);
			changeProxies(oldProxyList,newProxyList);
		}
		
		addServerStatistics();
	}  
	else
	{
		if (m_interestRadius > 0 || (abs(m_x - x) > 25) || (abs(m_z - z) > 25))
			watcherUpdateNeeded = true;
	}
		
	if (PlanetServer::getInstance().isWatcherPresent() && watcherUpdateNeeded)
	{
		m_x=x;
		m_y=y;
		m_z=z;
		m_objectTypeTag = objectTypeTag;

		outputStatusToAll(false);
	}
}

// ----------------------------------------------------------------------

void PlanetProxyObject::unsubscribeSurroundingNodes(bool clearZeroSubscriptions)
{
	if (m_interestRadius <= 0)
		return;
	
	Scene::NodeListType nodelist;
	Scene::getInstance().findIntersection(nodelist,m_x,m_z,m_interestRadius);
	
	for (Scene::NodeListType::iterator i=nodelist.begin(); i!=nodelist.end(); ++i)
	{
		(*i)->unsubscribeServer(m_authoritativeServer, 1, clearZeroSubscriptions);
	}
}

// ----------------------------------------------------------------------

void PlanetProxyObject::subscribeSurroundingNodes()
{	
	if (m_interestRadius <= 0)
		return;

	NOT_NULL(m_quadtreeNode);
	Scene::NodeListType nodelist;
	Scene::getInstance().findIntersection(nodelist,m_x,m_z,m_interestRadius);
	
	for (Scene::NodeListType::iterator i=nodelist.begin(); i!=nodelist.end(); ++i)
	{
		(*i)->subscribeServer(m_authoritativeServer, 1);
		uint32 serverId = (*i)->getPreferredServer();
		if (serverId != m_authoritativeServer)
			m_quadtreeNode->subscribeServer(serverId,0); // make sure if we can see them, they can see us
	}
}

// ----------------------------------------------------------------------

/**
 * Restore the object's authority to the specified game server.
 */
void PlanetProxyObject::restoreAuthority(uint32 gameServer)
{
	m_authoritativeServer = gameServer;
	m_lastReportedServer = gameServer;

	// Send queued messages
	PlanetServer::getInstance().sendQueuedMessagesForObject(*this);
}

// ----------------------------------------------------------------------

/**
 * Make the object authortiative on a different game server.  Sends a
 * message to the game servers to change authority and updates
 * m_authoritativeServer.
 */
void PlanetProxyObject::changeAuthority(uint32 newGameServer, bool handlingCrash, bool forcedByGame)
{
	if (handlingCrash)
		sendAuthorityChange(newGameServer, newGameServer, true);
	else if (!forcedByGame)
		sendAuthorityChange(m_authoritativeServer, newGameServer, false);
	m_authoritativeServer = newGameServer;
}

// ----------------------------------------------------------------------

/**
 * Make the object authoritative on a different game server and adjust the
 * subscription counts.
 */
void PlanetProxyObject::changeAuthorityAndSubscriptions(uint32 newGameServer)
{
	unsubscribeSurroundingNodes(false);
	changeAuthority(newGameServer, false, false);
	subscribeSurroundingNodes();
}

// ----------------------------------------------------------------------

/**
 * Called when an object should be unloaded from the world.  The object
 * should be removed from the active game servers, but not deleted from
 * the database.
 *
 * This function also removes the PlanetProxyObject from all data structures.
 * On exit, it is legal to delete the PlanetProxyObject.
 *
 * @param sendToGameServer Set to true if ForceUnloadObjectMessage should be
 * sent to the game server.  In some cases there is no need to tell the game
 * server to unload the object (such as when handling a crash, for example).
 */
void PlanetProxyObject::unload(bool tellGameServer)
{
	if (m_quadtreeNode)
		m_quadtreeNode->removeObject(this);

	Scene::getInstance().removeObjectFromMap(m_objectId,tellGameServer);

	if (tellGameServer)
	{
		PlanetServer::getInstance().unloadObject(m_objectId,m_authoritativeServer);
	}

	removeServerStatistics();

	if (ConfigPlanetServer::getEnableContentsChecking() && m_containedBy!=NetworkId::cms_invalid)
	{
		PlanetProxyObject *container = Scene::getInstance().findObjectByID(m_containedBy);
		if (container)
			container->removeContainedObject(m_objectId);
		else
			WARNING(true,("Unloading object %s in container %s, but the container could not be found.",m_objectId.getValueString().c_str(), m_containedBy.getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------

/**
 * Output the object's position to a watcher.
 * @param conn the watcher
 * @param deleteObject if true, tell the watcher to delete this object.
 */
void PlanetProxyObject::outputStatus(WatcherConnection &conn, bool deleteObject) const
{
	conn.addObjectUpdate(m_objectId, m_x, m_z, m_authoritativeServer, m_interestRadius, deleteObject, m_objectTypeTag, m_level, m_hibernating, m_templateCrc, m_aiActivity, m_creationType);
}

// ----------------------------------------------------------------------

void PlanetProxyObject::outputStatusToAll(bool deleteObject) const
{
	const PlanetServer::WatcherList &connections = PlanetServer::getInstance().getWatchers();
	for (PlanetServer::WatcherList::const_iterator i= connections.begin(); i!=connections.end(); ++i)
	{
		outputStatus(**i, deleteObject);
	}
}

// ----------------------------------------------------------------------

/** 
 * Update the GameServerData with statistics about this object.
 */
void PlanetProxyObject::addServerStatistics() const
{
	GameServerData *data=PlanetServer::getInstance().getGameServerData(m_authoritativeServer);
	if (data)
	{
		data->adjustObjectCount(1);
		if (m_interestRadius!=0)
		{
			data->adjustInterestObjectCount(1);
			if (isCreature())
				data->adjustInterestCreatureObjectCount(1);
		}		
	}
}

// ----------------------------------------------------------------------

/**
 * Remove statistics about this object from the GameServerData.
 * Should exactly undo addServerStatistics().
 */
void PlanetProxyObject::removeServerStatistics() const
{
	GameServerData *data=PlanetServer::getInstance().getGameServerData(m_authoritativeServer);
	if (data)
	{
		data->adjustObjectCount(-1);
		if (m_interestRadius!=0)
		{
			data->adjustInterestObjectCount(-1);
			if (isCreature())
				data->adjustInterestCreatureObjectCount(-1);				
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Set up memory pool
 */
void PlanetProxyObject::install()
{
	memoryBlockManager = new MemoryBlockManager("PlanetProxyObject::memoryBlockManager", true, sizeof(PlanetProxyObject), 0, 0, 0);
	ExitChain::add(&remove, "PlanetProxyObject::remove");	
}

// ----------------------------------------------------------------------

void PlanetProxyObject::remove()
{
	WARNING_DEBUG_FATAL(!memoryBlockManager, ("PlanetProxyObject is not installed"));
	delete memoryBlockManager;
	memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void * PlanetProxyObject::operator new(size_t size)
{
	UNREF(size);

	WARNING_DEBUG_FATAL(!memoryBlockManager, ("PlanetProxyObject is not installed"));

	// do not try to alloc a descendant class with this allocator
	WARNING_DEBUG_FATAL(size != sizeof(PlanetProxyObject), ("bad size"));

	return memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void PlanetProxyObject::operator delete(void* pointer)
{
	WARNING_DEBUG_FATAL(!memoryBlockManager, ("PlanetProxyObject is not installed"));
	memoryBlockManager->free(pointer);
}

// ----------------------------------------------------------------------

void PlanetProxyObject::sendAuthorityChange(uint32 currentAuthServer, uint32 newAuthServer, bool handlingCrash)
{
	DEBUG_WARNING(!isAuthorityClean(), ("Object %s attempted to change authority before previous authority change was complete.",getObjectId().getValueString().c_str()));

	SetAuthoritativeMessage const auth(getObjectId(), newAuthServer, false, handlingCrash, NetworkId::cms_invalid, Transform::identity, false);
	PlanetServer::getInstance().sendToGameServer(currentAuthServer, auth);
	m_authTransferTimeMs = Clock::timeMs();
}

// ----------------------------------------------------------------------

void PlanetProxyObject::sendAddProxy(uint32 proxyServer)
{
	if (ConfigPlanetServer::getLogObjectLoading())
		LOG("ObjectLoading",("Gameserver %lu needs a proxy of object %s",proxyServer,m_objectId.getValueString().c_str()));
	if (isAuthorityClean())
	{
		LoadObjectMessage const loadMsg(getObjectId(), proxyServer);
		PlanetServer::getInstance().sendToGameServer(m_authoritativeServer, loadMsg);
	}
	else
	{
		LoadObjectMessage * const loadMsg = new LoadObjectMessage(getObjectId(), proxyServer);
		PlanetServer::getInstance().queueMessageForObject(getObjectId(), loadMsg);
	}
}

// ----------------------------------------------------------------------

void PlanetProxyObject::sendRemoveProxy(uint32 proxyServer)
{
	if (ConfigPlanetServer::getLogObjectLoading())
		LOG("ObjectLoading",("Gameserver %lu no longer needs a proxy of object %s",proxyServer,m_objectId.getValueString().c_str()));
	if (isAuthorityClean())
	{
		UnloadProxyMessage msg(getObjectId(),proxyServer);
		PlanetServer::getInstance().sendToGameServer(m_authoritativeServer, msg);
	}
	else
	{
		UnloadProxyMessage *msg = new UnloadProxyMessage(getObjectId(),proxyServer);
		PlanetServer::getInstance().queueMessageForObject(getObjectId(), msg);
	}
}

// ----------------------------------------------------------------------

/**
 * Called when we receieve a message about the object from a game server.
 * Used to keep track of which server is sending us updates, and to send
 * any messages that were waiting for an authority change.
 */
void PlanetProxyObject::onReceivedMessageFromServer(uint32 server)
{
	if (m_lastReportedServer != m_authoritativeServer && server == m_authoritativeServer)
		// we've received a message from the new authoritative server.  Send queued messages:
		PlanetServer::getInstance().sendQueuedMessagesForObject(*this);
			
	m_lastReportedServer = server;
}

// ----------------------------------------------------------------------

/**
 * Get the list of servers that should have proxies of this object.
 */
void PlanetProxyObject::getServers(std::vector<uint32> &serverList) const
{
	Node *effectiveNode;
	if (m_containedBy == NetworkId::cms_invalid)
	{
		effectiveNode = m_quadtreeNode;
	}
	else
	{
		PlanetProxyObject *container=Scene::getInstance().findObjectByID(m_containedBy);
		WARNING_STRICT_FATAL(!container,("Object %s is in container %s, which couldn't be found",m_objectId.getValueString().c_str(),m_containedBy.getValueString().c_str()));
		if (container)
			effectiveNode = container->getNode();
		else
			effectiveNode = m_quadtreeNode;
	}

	if (effectiveNode)
		effectiveNode->getServers(serverList);
}

// ----------------------------------------------------------------------

/**
 * Given a list of servers that have a proxy and a list of servers
 * that should have a proxy, send messages to update the servers.
 * Note:  O(n^2), assumes both lists are small, otherwise it would be better
 * to sort them first.
 */
void PlanetProxyObject::changeProxies(const std::vector<uint32> &oldServers, const std::vector<uint32> &newServers)
{
	{
		for (std::vector<uint32>::const_iterator i=oldServers.begin(); i!=oldServers.end(); ++i)
		{
			if (*i != m_authoritativeServer)
			{
				bool removeNeeded = true;
				for (std::vector<uint32>::const_iterator j=newServers.begin(); j!=newServers.end(); ++j)
				{
					if (*i == *j)
					{
						removeNeeded = false;
						break;
					}
				}
				if (removeNeeded)
					sendRemoveProxy(*i);
			}
		}
	}
	{
		for (std::vector<uint32>::const_iterator i=newServers.begin(); i!=newServers.end(); ++i)
		{
			if (*i != m_authoritativeServer)
			{
				bool addNeeded = true;
				for (std::vector<uint32>::const_iterator j=oldServers.begin(); j!=oldServers.end(); ++j)
				{
					if (*i == *j)
					{
						addNeeded = false;
						break;
					}
				}
				if (addNeeded)
					sendAddProxy(*i);
			}
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Check whether we are on a server that's acceptable for where we are.
 */
bool PlanetProxyObject::isAuthorityOk() const
{
	if (m_authoritativeServer == m_quadtreeNode->getPreferredServer())
		return true;
	if (m_quadtreeNode->isBorderNode() && m_quadtreeNode->isServerSubscribed(m_authoritativeServer))
		return true;
	else
		return false;
}

// ----------------------------------------------------------------------

/**
 * Check for whether a desired server would be OK for where we are.
 * The server is OK if it is the preferred server.  If the object is
 * a creature, the server is also OK if the node is a border node and
 * the server has a subscription to the node.  Non-creatures must always
 * be on the preferred server.
 */
bool PlanetProxyObject::wouldAuthorityBeOk(uint32 possibleServer) const
{
	if (possibleServer == m_quadtreeNode->getPreferredServer())
		return true;
	if (isCreature() && m_quadtreeNode->isBorderNode() && m_quadtreeNode->isServerSubscribed(possibleServer))
		return true;
	else
		return false;
}

// ----------------------------------------------------------------------

void PlanetProxyObject::addContainedObject(const NetworkId &theObject)
{
	if (!m_contents)
	{
		m_contents = new std::vector<NetworkId>;
	}
	for (std::vector<NetworkId>::const_iterator i=m_contents->begin(); i!=m_contents->end(); ++i)
	{
		if (*i==theObject)
			WARNING(true,("Object %s was placed in container %s twice.", theObject.getValueString().c_str(), m_objectId.getValueString().c_str()));
	}
	m_contents->push_back(theObject);
}

// ----------------------------------------------------------------------

void PlanetProxyObject::removeContainedObject(const NetworkId &theObject)
{
	if (!m_contents)
	{
		WARNING(true,("Attempted to remove object %s from object %s, but that object has no contents.", theObject.getValueString().c_str(), m_objectId.getValueString().c_str()));
		return;
	}
	std::vector<NetworkId>::iterator newEnd=std::remove(m_contents->begin(), m_contents->end(), theObject);
	if (newEnd==m_contents->end())
	{
		WARNING(true,("Attempted to remove object %s from object %s, but it was not in the container.", theObject.getValueString().c_str(), m_objectId.getValueString().c_str()));
		return;
	}
	m_contents->erase(newEnd, m_contents->end());
}

// ----------------------------------------------------------------------

/**
 * Helper function to update the contents tracking lists, in order to
 * debug various authority problems
 */
void PlanetProxyObject::updateContentsTracking(const NetworkId &newContainedBy)
{
	if (m_containedBy!=NetworkId::cms_invalid)
	{
		PlanetProxyObject *container = Scene::getInstance().findObjectByID(m_containedBy);
		if (container)
			container->removeContainedObject(m_objectId);
		else
			WARNING(true,("Removing object %s from container %s, but the container could not be found.",m_objectId.getValueString().c_str(), m_containedBy.getValueString().c_str()));
	}

	if (newContainedBy!=NetworkId::cms_invalid)
	{
		PlanetProxyObject *container = Scene::getInstance().findObjectByID(newContainedBy);
		if (container)
			container->addContainedObject(m_objectId);
		else
			WARNING(true,("Adding object %s to container %s, but the container could not be found.",m_objectId.getValueString().c_str(), newContainedBy.getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------

bool PlanetProxyObject::isCreature() const
{
	return m_objectTypeTag==TAG(C,R,E,O);
}

// ======================================================================
