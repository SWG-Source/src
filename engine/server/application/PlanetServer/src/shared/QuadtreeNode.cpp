// ======================================================================
//
// Node.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstPlanetServer.h"
#include "QuadtreeNode.h"

#include <stdio.h>

#include "ConfigPlanetServer.h"
#include "PlanetProxyObject.h"
#include "PlanetServer.h"
#include "PreloadManager.h"
#include "Scene.h"
#include "WatcherConnection.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/PlanetNodeStatusMessage.h"
#include "sharedLog/Log.h"

//===================================================================

MemoryBlockManager* Node::memoryBlockManager;

// ======================================================================

/** 
 * Create a new spatial subdivision node.
 */

Node::Node(int x, int z) :
		m_objectList(),
		m_subscriptionList(),
		m_preferredServer(0),
		m_preloadServerId(0),
		m_loaded(false),
		m_isBorder(false),
		m_x(x),
		m_z(z)
{
	WARNING_DEBUG_FATAL((x!=roundToNode(x)) || (z!=roundToNode(z)),("Creating node %s, coordinates not on a node boundary.",getDebugNodeString().c_str()));
}

// ----------------------------------------------------------------------

/** 
 * Places the object in this node and notifies servers as appropriate.
 * 
 * @param newObject The object to add
 */

void Node::addObject(PlanetProxyObject *newObject)
{
	if (ConfigPlanetServer::getLogObjectLoading())
		LOG("ObjectLoading",("Adding object %s to node %s",newObject->getObjectId().getValueString().c_str(),getDebugNodeString().c_str()));

	WARNING_DEBUG_FATAL(!m_loaded,("Attempt to place an object in an unloaded node."));
	WARNING_DEBUG_FATAL(newObject->getObjectId() == NetworkId::cms_invalid,("Attempt to place object 0 in a node."));
	
	m_objectList.push_back(newObject);
	// We don't send add proxy messages here, because PlanetProxyObject::move() does that.
}

// ----------------------------------------------------------------------

/** 
 * Remove an object from the node.
 */

void Node::removeObject(const PlanetProxyObject *oldObject)
{
	NOT_NULL(oldObject);
	if (ConfigPlanetServer::getLogObjectLoading())
		LOG("ObjectLoading",("Removing object %s from node %s",oldObject->getObjectId().getValueString().c_str(),getDebugNodeString().c_str()));
	ObjectListType::iterator i;
	for (i=m_objectList.begin();i!=m_objectList.end();++i)
		if (*i==oldObject)
			break;
	if (i==m_objectList.end())
	{
		WARNING_DEBUG_FATAL(true,("Attempted to remove object %s that was not in the node.", oldObject->getObjectId().getValueString().c_str()));
		return; //lint !e527 Unreachable (in debug mode only)
	}
	IGNORE_RETURN(m_objectList.erase(i));
}

// ----------------------------------------------------------------------

/** 
 * Subscribe a server to this node.
 * Makes sure that the specifed server has proxies of the objects in the
 * node and gets updates when new objects enter the node.
 * @param serverId the server
 * @param count How many objects are interested (Note: 0 is a legal value.
 * It means that we want the server to know about this node, but there
 * aren't any interested objects nearby yet.)
 */

void Node::subscribeServer(uint32 serverId, int count)
{
	WARNING_DEBUG_FATAL(serverId==0,("Programmer bug:  Server 0 passed to subscribeServer"));
	WARNING_DEBUG_FATAL(count < 0,("Programmer bug:  Passed negative count to subscribeServer"));
	
	bool newSub=false;
	
	if (!m_loaded)
	{
		load();
	}
	
	SubscriptionListType::iterator i=m_subscriptionList.find(serverId);
	if (i!=m_subscriptionList.end())
	{
		((*i).second)+=count;
	}
	else
	{
		newSub = true;
		m_subscriptionList[serverId]=count;

		for (ObjectListType::iterator j=m_objectList.begin(); j!=m_objectList.end(); ++j)
		{
			if (serverId!=(*j)->getAuthoritativeServer() && (*j)->getContainedBy() == NetworkId::cms_invalid)
			{
				(*j)->sendAddProxy(serverId);
			}
		}
	}

	if (PlanetServer::getInstance().isWatcherPresent())
	{
		outputStatusToAll();
	}

	if (ConfigPlanetServer::getLogChunkLoading())
	{
		if (newSub)
			LOG("ChunkLoading",("Node %s Server %lu new subscription, count %i",getDebugNodeString().c_str(), serverId, m_subscriptionList[serverId]));
		else
			LOG("ChunkLoading",("Node %s Server %lu subscription count changed by %i to %i",getDebugNodeString().c_str(), serverId, count, m_subscriptionList[serverId]));
	}
}

// ----------------------------------------------------------------------


/** 
 * Decrease the count of objects on a server that are interested in this
 * node.  If the count reaches 0, the subscription is kept, but
 * the load balancing algorithms may remove it later.
 */

void Node::unsubscribeServer(uint32 serverId, int count, bool clearIfZero)
{
	WARNING_DEBUG_FATAL(serverId==0,("Programmer bug:  Server 0 passed to unsubscribeServer"));
	WARNING_DEBUG_FATAL(count < 0,("Programmer bug:  Passed negative count to unsubscribeServer"));

	bool cleared=false;
	
	SubscriptionListType::iterator i=m_subscriptionList.find(serverId);
	WARNING(i==m_subscriptionList.end(),("Attempted to unsubscribe server %i that was not subscribed to node %s.",serverId,getDebugNodeString().c_str()));

	if (i!=m_subscriptionList.end())
	{
		i->second-=count;
		
		if (i->second < 0)
		{
			WARNING(true,("Programmer bug:  Subscription count for server %i on node %s went negative.",serverId,getDebugNodeString().c_str()));
			i->second = 0;
		}

		if (clearIfZero && i->second == 0 && m_preferredServer != serverId)
		{
			cleared=true;
			unproxyAllObjects(serverId);
			m_subscriptionList.erase(i);

			if (PlanetServer::getInstance().isWatcherPresent())
			{
				outputStatusToAll();
			}
		}
	}

	if (ConfigPlanetServer::getLogChunkLoading())
	{
		if (cleared)
			LOG("ChunkLoading",("Node %s Server %lu unsubscribed",getDebugNodeString().c_str(), serverId));
		else
			LOG("ChunkLoading",("Node %s Server %lu subscription count changed by -%i to %i",getDebugNodeString().c_str(), serverId, count, m_subscriptionList[serverId]));
	}
}

// ----------------------------------------------------------------------

/**
 * Helper function.  Removes the proxies on the specified server.
 * Does not update the subscription list.
 */
void Node::unproxyAllObjects(uint32 serverId)
{
	for (ObjectListType::iterator j=m_objectList.begin(); j!=m_objectList.end(); ++j)
		(*j)->sendRemoveProxy(serverId);	
}

// ----------------------------------------------------------------------

/**
 * Load a node from the database.
 * Marks the node as loaded, and instructs the database to put the objects
 * in the node on a particular game server.  The game server will report
 * the objects back to us after they get loaded.
 */

void Node::load()
{
	if (m_loaded)
		return;

	if (m_preferredServer == 0)
		pickPreferredServer();

	if (m_preferredServer == 0)
	{
		WARNING_DEBUG_FATAL(true, ("load failed for node %s because m_preferredServer is 0",getDebugNodeString().c_str()));
		return;
	}

	m_loaded=true;

	bool sendRequest=true; // in certain modes, we don't need to check the DB for objects
	if (PlanetServer::getInstance().isInTutorialMode())
		sendRequest=false;
	if (PlanetServer::getInstance().isInSpaceMode() && !(m_x == 0 && m_z == 0)) // only load (0,0) in space
		sendRequest=false;

	if (sendRequest)
	{
		PlanetServer::getInstance().sendChunkRequest(m_preferredServer, m_x,m_z);
		if (ConfigPlanetServer::getLogChunkLoading())
			LOG("ChunkLoading", ("RequestChunk: Node %s Server %i",getDebugNodeString().c_str(),m_preferredServer));
	}

	subscribeServer(m_preferredServer,0);
}

// ----------------------------------------------------------------------

/**
 * Helper function:  picks the best server for this node and puts it
 * into m_preferredServer.
 */
void Node::pickPreferredServer()
{
	m_preferredServer = PreloadManager::getInstance().getRealServerId(m_preloadServerId);
	if (m_preferredServer == 0)
		m_preferredServer = PreloadManager::getInstance().getClosestGameServer(m_x, m_z);
	WARNING_DEBUG_FATAL(m_preferredServer==0,("Programmer bug:  Node::pickPreferredServer could not pick a server"));
}

// ----------------------------------------------------------------------

/**
 * Get a string listing the coordinates of this node.
 * For debugging, FATALs, etc.
 */
std::string Node::getDebugNodeString() const
{
	char buffer[100];
	snprintf(buffer,sizeof(buffer)-1,"%s (%i,%i)",Scene::getInstance().getSceneId().c_str(),m_x,m_z);
	buffer[sizeof(buffer)-1]='\0';
	return std::string(buffer);
}

// ----------------------------------------------------------------------

/**
 * Output the status of the node to the PlanetWatch tool.
 */
void Node::outputStatus(WatcherConnection &conn) const
{
	std::vector<uint32> servers;
	std::vector<int> subscriptionCount;
	for (SubscriptionListType::const_iterator i=m_subscriptionList.begin(); i!=m_subscriptionList.end(); ++i)
	{
		servers.push_back((*i).first);
		subscriptionCount.push_back((*i).second);

		WARNING_DEBUG_FATAL((*i).first==0,("Found a 0 in subscrption list (where did it come from?)"));
	}

	conn.addNodeUpdate(m_x,m_z,m_loaded, servers, subscriptionCount);
}

// ----------------------------------------------------------------------

void Node::outputStatusToAll() const
{
	const PlanetServer::WatcherList &connections = PlanetServer::getInstance().getWatchers();
	for (PlanetServer::WatcherList::const_iterator i= connections.begin(); i!=connections.end(); ++i)
	{
		outputStatus(**i);
	}
}

// ----------------------------------------------------------------------

/**
 * Set up a memory pool for QuadTreeNodes.
 */

void Node::install()
{
	memoryBlockManager = new MemoryBlockManager("Node::memoryBlockManager", true, sizeof(Node), 0, 0, 0);

	ExitChain::add(&remove, "Node::remove");	
}

// ----------------------------------------------------------------------

void Node::remove()
{
	WARNING_DEBUG_FATAL(!memoryBlockManager,("Node is not installed"));

	delete memoryBlockManager;
	memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void * Node::operator new(size_t size)
{
	UNREF(size);

	WARNING_DEBUG_FATAL(!memoryBlockManager,("Node is not installed"));

	// do not try to alloc a descendant class with this allocator
	WARNING_DEBUG_FATAL(size != sizeof(Node),("bad size"));

	return memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void Node::operator delete(void* pointer)
{
	WARNING_DEBUG_FATAL(!memoryBlockManager,("Node is not installed"));
	memoryBlockManager->free(pointer);
}

// ----------------------------------------------------------------------

/**
 * Load the node onto the specified server.
 */
void Node::load(uint32 serverId)
{
	if (m_loaded)
	{
		WARNING_DEBUG_FATAL(m_loaded,("Programmer bug:  called load on Node %s,.which was already loaded",getDebugNodeString().c_str()));
		return;
	}
	m_preferredServer = serverId;
	load();
}

// ----------------------------------------------------------------------

void Node::getServers(std::vector<uint32> &serverList) const
{
	for (SubscriptionListType::const_iterator i=m_subscriptionList.begin(); i!=m_subscriptionList.end(); ++i)
	{
		serverList.push_back(i->first);
	}
}

// ----------------------------------------------------------------------

/**
 * Migrate the objects to the specfied server, and get rid of any subscribed
 * servers that aren't interested in the node anymore
 */
void Node::migrateToServer(uint32 newServer)
{
	WARNING_DEBUG_FATAL (!m_loaded,("Programmer bug: called migrateToServer(%i) on unloaded node %s",newServer,getDebugNodeString().c_str()));
	
	if (m_preferredServer == newServer)
		return;
	
	subscribeServer(newServer,0);
	m_preferredServer = newServer;

	for (ObjectListType::iterator i=m_objectList.begin(); i!=m_objectList.end(); ++i)
	{
		(*i)->unsubscribeSurroundingNodes(true);
		(*i)->changeAuthority(newServer, false, false);
		(*i)->subscribeSurroundingNodes();
	}

	// remove any zero subscriptions:
	
	std::vector<uint32> unsubscribeList;
	
	for (SubscriptionListType::iterator subscription=m_subscriptionList.begin(); subscription!=m_subscriptionList.end(); ++subscription)
		if (subscription->second == 0 && subscription->first != newServer)
			unsubscribeList.push_back(subscription->first);

	for (std::vector<uint32>::iterator s=unsubscribeList.begin(); s!=unsubscribeList.end(); ++s)
	{
		unproxyAllObjects(*s);
		IGNORE_RETURN(m_subscriptionList.erase(*s));
	}

	if (PlanetServer::getInstance().isWatcherPresent())
	{
		outputStatusToAll();
	}

	if (ConfigPlanetServer::getLogChunkLoading())
		LOG("ChunkLoading",("Node %s Migrate to server %lu",getDebugNodeString().c_str(), newServer));
}

// ----------------------------------------------------------------------

/**
 * Check that this node is assigned to the right server, and fix it if
 * it isn't.
 */
void Node::checkServerAssignment()
{
	if (!m_loaded)
		return;
	uint32 bestServer = PreloadManager::getInstance().getRealServerId(m_preloadServerId);
	if (bestServer==0)
		return;

	if (m_preferredServer != bestServer)
		migrateToServer(bestServer);
}

// ----------------------------------------------------------------------

void Node::handleCrash(uint32 crashedServer, std::vector<PlanetProxyObject*> &subscriptionFixups)
{
	bool wasSubscribed = false;

	{
		for (SubscriptionListType::iterator i=m_subscriptionList.begin(); i!=m_subscriptionList.end();)
		{
			if (i->first == crashedServer)
			{
				m_subscriptionList.erase(i++);
				wasSubscribed = true;
			}
			else
			{
				++i;
			}
		}
	}

	if (m_preferredServer == crashedServer)
	{
		if (!m_subscriptionList.empty())
		{
			// pick a new server for this node and move the objects

			uint32 newServer = m_subscriptionList.begin()->first;
			m_preferredServer = newServer;
			WARNING_DEBUG_FATAL(newServer == crashedServer,("Programmer bug:  crashed server was still in the subscription list after it was supposedly removed."));
		}
		else
		{
			// no other server has proxies.  Unload the node.
			m_loaded = false;
			m_preferredServer = 0;

			for (ObjectListType::iterator i=m_objectList.begin(); i!=m_objectList.end(); ++i)
			{
				if (ConfigPlanetServer::getLogObjectLoading())
					LOG("ObjectLoading",
						("Handling crash:  Dropping object %s because node %s was dropped",
						 (*i)->getObjectId().getValueString().c_str(),getDebugNodeString().c_str()));
				
				Scene::getInstance().removeObjectFromMap((*i)->getObjectId(),false);
				delete *i;
			}
			m_objectList.clear();			
		}
	}

	{
		for (ObjectListType::iterator i=m_objectList.begin(); i!=m_objectList.end(); ++i)
		{
			if ((*i)->getContainedBy() == NetworkId::cms_invalid)
			{
				if (crashedServer==(*i)->getAuthoritativeServer())
				{
					if (ConfigPlanetServer::getLogObjectLoading())
						LOG("ObjectLoading",
							("Handling crash:  moving object %s from server %lu to server %lu",
							 (*i)->getObjectId().getValueString().c_str(),crashedServer,m_preferredServer));

					(*i)->changeAuthority(m_preferredServer, true, false);
					(*i)->sendRemoveProxy(crashedServer);
					if ((*i)->getInterestRadius() > 0)
						subscriptionFixups.push_back(*i);
				}
				else
				{
					// tell objects they don't have a proxy on this server anymore
					if (wasSubscribed)
						(*i)->sendRemoveProxy(crashedServer);
				}
			}
		}
	}

	if (PlanetServer::getInstance().isWatcherPresent())
	{
		outputStatusToAll();
	}	
}

// ----------------------------------------------------------------------

bool Node::isBorderNode() const
{
	WARNING_DEBUG_FATAL(getPreferredServer()==0,("Programmer bug:  called isBorderNode() on node %s, which has 0 for its preferred server.",getDebugNodeString().c_str()));

	if (m_subscriptionList.size() < 2)
	{
		m_isBorder = false;
		return false;
	}
	
	if (m_isBorder)
		return true;

	int maxX=m_x+getNodeSize();
	int maxZ=m_z+getNodeSize();
	for (int x=m_x-getNodeSize();x<=maxX;x+=getNodeSize())
		for (int z=m_z-getNodeSize();z<=maxZ;z+=getNodeSize())
			if (!(x==m_x && z==m_z))
			{
				const Node *node = Scene::getInstance().findNodeByRoundedPositionConst(x,z);
				if (node && node->getPreferredServer()!=0 && node->getPreferredServer() != getPreferredServer())
				{
					m_isBorder = true;
					return true;
				}
			}
	
	return false;
}

// ----------------------------------------------------------------------

bool Node::isServerSubscribed(uint32 serverId) const
{
	return (m_subscriptionList.find(serverId)!=m_subscriptionList.end());
}

// ----------------------------------------------------------------------

void Node::setPreloadServerId(PreloadServerId preloadServerId)
{
	m_preloadServerId = preloadServerId;
}

// ----------------------------------------------------------------------

PreloadServerId Node::getPreloadServerId() const
{
	return m_preloadServerId;
}

// ======================================================================
