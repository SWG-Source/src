// ======================================================================
//
// Scene.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstPlanetServer.h"
#include "Scene.h"

#include "ConfigPlanetServer.h"
#include "GameServerConnection.h"
#include "GameServerData.h"
#include "PlanetProxyObject.h"
#include "PreloadManager.h"
#include "QuadtreeNode.h"
#include "serverNetworkMessages/PlanetRemoveObject.h"
#include "serverNetworkMessages/PopulationListMessage.h"
#include "serverNetworkMessages/UpdateObjectOnPlanetMessage.h"
#include "serverUtility/PopulationList.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedUtility/LocationManager.h"

#include "sharedFoundation/CrcConstexpr.hpp"

// ======================================================================

Scene::Scene() :
		Singleton2<Scene>(),
		MessageDispatch::Receiver(),
		m_sceneId(),
		m_objects(),
		m_deletedObjects(),
		m_nodeMap(),
		m_populationList(new PopulationList),
		m_populationCountTimer(static_cast<float>(ConfigPlanetServer::getPopulationCountTime())),
		m_maxCoordinate(0)
{
	ExitChain::add(&remove, "Scene::remove");

	connectToMessage("GameConnectionClosed");
	connectToMessage("UpdateObjectOnPlanetMessage");
	connectToMessage("PlanetRemoveObject");
	connectToMessage("ForceLoadArea");
}

// ----------------------------------------------------------------------

/**
 * Set the scene ID.  Make this part of the constructor if we ever make
 * this a non-singleton class.
 */
void Scene::setSceneId(const std::string &sceneId)
{
	m_sceneId=sceneId;

	//-- tell the LocationManager what planet we're on
	LocationManager::setPlanetName (sceneId.c_str ());
}

// ----------------------------------------------------------------------

/**
 * Called to add an object to the scene.  Scene takes ownership
 * of the object and will be resposible for deleting it.
 */

void Scene::addObject(PlanetProxyObject *newObject)
{
	m_objects[newObject->getObjectId()]=newObject;
	newObject->addServerStatistics();
}

// ----------------------------------------------------------------------

PlanetProxyObject *Scene::findObjectByID(const NetworkId &objectID) const
{
	ObjectMapType::const_iterator i=m_objects.find(objectID);
	if (i==m_objects.end())
		return nullptr;
	else
		return ((*i).second);
}

// ----------------------------------------------------------------------

bool Scene::isObjectLoaded(const NetworkId &networkId) const
{
	return m_objects.find(networkId) != m_objects.end();
}

// ----------------------------------------------------------------------

void Scene::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	const GameServerConnection *gameServer=dynamic_cast<const GameServerConnection*>(&source);
	WARNING_DEBUG_FATAL(gameServer==0,("Source was nullptr or source was not a GameServerConnection."));

	const uint32 messageType = message.getType();
	
	switch(messageType) {
		case constcrc("GameConnectionClosed") :
		{
			DEBUG_REPORT_LOG(true, ("Handling a game server crash for server %lu\n", gameServer->getProcessId()));
			handleCrash(gameServer->getProcessId());
			break;
		}
		case constcrc("UpdateObjectOnPlanetMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			UpdateObjectOnPlanetMessage msg(ri);

			int interestRadius = msg.getInterestRadius();

			if(msg.getWatched() && interestRadius == 0)
				interestRadius = 1;

			if (m_deletedObjects.find(msg.getObjectId())!=m_deletedObjects.end())
			{
				DEBUG_REPORT_LOG(true,("Ignoring update for object %s which has been removed.\n",msg.getObjectId().getValueString().c_str()));
			}
			else
			{
				NetworkId container(msg.getTopmostContainer());
				if (container==msg.getObjectId()) // game server sends container == objectId if the object is not contained
					container=NetworkId::cms_invalid;

				bool processUpdate = true;
				if (container != NetworkId::cms_invalid)
				{
					PlanetProxyObject const *const containerObject = findObjectByID(container);
					if (containerObject)
					{
						if (containerObject->getAuthoritativeServer() != gameServer->getProcessId())
						{
							// This update message came from a server that is not
							// authoritative for the container of this object.  This
							// can happen when the planet server is in control of
							// switching the container's authority but is not in
							// charge of changing the container content's authority.
							// This happens to the rider of a mount when the mount
							// switches authority.
							processUpdate = false;
						}
					}
				}

				if (processUpdate)
				{
					PlanetProxyObject *theObject=findObjectByID(msg.getObjectId());
					if (!theObject)
					{
						// this is a new object
						theObject=new PlanetProxyObject(msg.getObjectId());
						addObject(theObject);
					}
					
					theObject->onReceivedMessageFromServer(gameServer->getProcessId());
					theObject->update(msg.getX(), msg.getY(), msg.getZ(), container, gameServer->getProcessId(), interestRadius, msg.getObjectTypeTag(), msg.getLevel(), msg.getHibernating(), msg.getTemplateCrc(), msg.getAiActivity(), msg.getCreationType());

					//-- handle updating LocationManager
					if (msg.getLocationReservationRadius () > 0.f)
						LocationManager::updateObject (msg.getObjectId (), msg.getX (), msg.getZ (), msg.getLocationReservationRadius ());
				}
			}
			break;
		}
		case constcrc("PlanetRemoveObject") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			PlanetRemoveObject msg(ri);
			handleRemoveObject(msg.getObjectId());

			//-- handle updating LocationManager
			LocationManager::removeObject (msg.getObjectId ());
			break;
		}
		case constcrc("ForceLoadArea") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<std::pair<uint32, std::pair<std::pair<float, float>, std::pair<float, float> > > > const msg(ri);

			handleForceLoadArea(
				msg.getValue().first,
				static_cast<int>(msg.getValue().second.first.first),
				static_cast<int>(msg.getValue().second.first.second),
				static_cast<int>(msg.getValue().second.second.first),
				static_cast<int>(msg.getValue().second.second.second)
			);
				
			break;
		}
		default :
		{
			DEBUG_REPORT_LOG(true,("Scene.cpp received an unidentified message.\n"));
			break;
		}
	}
}

// ----------------------------------------------------------------------

void Scene::subscribeServer(uint32 server, int x, int z, int interestRadius, int count)
{
	NodeListType nodelist;
	findIntersection(nodelist,x,z,interestRadius);

	for (NodeListType::iterator i=nodelist.begin(); i!=nodelist.end(); ++i)
	{
		(*i)->subscribeServer(server, count);
	}
}

// ----------------------------------------------------------------------

/**
 * Given coordinates, return the node that encloses those coordinates.
 * Creates the node if it doesn't exist already.
 */
Node *Scene::findNodeByPosition(int x, int z)
{
	return findNodeByRoundedPosition(Node::roundToNode(x),Node::roundToNode(z));
}

// ----------------------------------------------------------------------

/**
 * Given coordinates, return a const pointer to the node that encloses
 * those coordinates.  Will not create new nodes, so it may return nullptr.
 */
const Node *Scene::findNodeByPositionConst(int x, int z) const
{
	return findNodeByRoundedPositionConst(Node::roundToNode(x),Node::roundToNode(z));
}

// ----------------------------------------------------------------------

/**
 * Given coordinates that are known to be a node boundary, return the node.
 * Creates the node if it doesn't exist already.
 */
Node *Scene::findNodeByRoundedPosition(int x, int z)
{
	DEBUG_FATAL((x!=Node::roundToNode(x)) || (z!=Node::roundToNode(z)),("Parameters to findNodeByRoundedPosistion weren't rounded."));
	Node *result=m_nodeMap[Coordinates(x,z)];
	if (!result)
	{
		result=new Node(x,z);
		m_nodeMap[Coordinates(x,z)]=result;
	}
	return result;
}

// ----------------------------------------------------------------------

/**
 * Given coordinates that are known to be a node boundary, return a const pointer
 * to the node.  Does not create new nodes, so may return nullptr.
 */
const Node *Scene::findNodeByRoundedPositionConst(int x, int z) const
{
	WARNING_DEBUG_FATAL((x!=Node::roundToNode(x)) || (z!=Node::roundToNode(z)),("Parameters to findNodeByRoundedPosistionConst weren't rounded."));
	NodeMapType::const_iterator i=m_nodeMap.find(Coordinates(x,z));
	if (i!=m_nodeMap.end())
		return (*i).second;
	else
		return nullptr;
}

// ----------------------------------------------------------------------

/**
 * Given coordinates and a radius, find a set of nodes that contains the circle.
 *
 * May return some nodes that don't contain any part of the circle, but
 * is guaranteed to return at least every node that the circle intersects.
 * Will add nodes if they don't exist.
 */

void Scene::findIntersection(NodeListType &results,int x, int z, int radius)
{
	int minX=Node::roundToNode(x-radius);
	int minZ=Node::roundToNode(z-radius);
	int maxX=Node::roundToNode(x+radius);
	int maxZ=Node::roundToNode(z+radius);
	int nodeSize=Node::getNodeSize();

	for (int loopX=minX; loopX <= maxX; loopX+=nodeSize)
		for (int loopZ=minZ; loopZ <= maxZ; loopZ+=nodeSize)
			results.push_back(findNodeByRoundedPosition(loopX,loopZ));
}

// ----------------------------------------------------------------------

/**
 * Removes an object from the list of objects.
 *
 * Places the object in a list of pending deletes.  Further updates to
 * this object will be ignored.
 * Doesn't remove the object from the quadtree or notify any servers.
 * This function should only be called by PlanetProxyObject::unload().
 *
 * @param objectId Object to remove.
 * @param addToDeletedList true if we should remember that we deleted the object,
 * false if not.  Generally, we don't need to put the object in the deleted map
 * if the game server initated the removal.  If we initated it, we remember
 * the object in the deleted map so that we ignore all updates to it until
 * the game server acknowledges the removal.
 */
void Scene::removeObjectFromMap(const NetworkId &objectId, bool addToDeletedList)
{
	ObjectMapType::iterator i=m_objects.find(objectId);
	if (i==m_objects.end())
	{
		DEBUG_REPORT_LOG(true,("Requested removing object %s, but it was not in the list.\n",objectId.getValueString().c_str()));
	}
	else
	{
		m_objects.erase(i);
	}

	if (addToDeletedList)
		IGNORE_RETURN(m_deletedObjects.insert(objectId));
}

// ----------------------------------------------------------------------

/**
 * Handler for the PlanetRemoveObject message.
 */

void Scene::handleRemoveObject(const NetworkId &objectId)
{
	DeletedSetType::iterator i=m_deletedObjects.find(objectId);
	if (i!=m_deletedObjects.end())
	{
		if (ConfigPlanetServer::getLogObjectLoading())
			LOG("ObjectLoading",("Removal of object %s confirmed.",objectId.getValueString().c_str()));
		m_deletedObjects.erase(i);
	}
	else
	{
		if (ConfigPlanetServer::getLogObjectLoading())
			LOG("ObjectLoading",("Removal of object %s initiated by GameServer.",objectId.getValueString().c_str()));
		PlanetProxyObject *theObject=findObjectByID(objectId);
		if (theObject)
		{
			theObject->unload(false);

			if (theObject->getInterestRadius()>0)
				theObject->unsubscribeSurroundingNodes(false);
			delete theObject;
		}
		else
		{
			if (ConfigPlanetServer::getLogObjectLoading())
				LOG("ObjectLoading",("Object %s was unknown.  Removal request ignored.",objectId.getValueString().c_str()));
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Given an object, find which server it is on.
 * Used by the login process, for example, to find where the building
 * a player is in is loaded.
 */
uint32 Scene::getGameServerForObject(const NetworkId &object) const
{
	ObjectMapType::const_iterator i=m_objects.find(object);
	if (i!=m_objects.end())
	{
		const PlanetProxyObject *obj=(*i).second;
		NOT_NULL(obj);
		return obj->getAuthoritativeServer();
	}
	else
		return 0;
}

// ----------------------------------------------------------------------

/**
 * Output the status of the scene to a Watcher.  (Call when a new watcher
 * connects, for example.)
 */
void Scene::outputStatus(WatcherConnection &conn) const
{
	for (NodeMapType::const_iterator i=m_nodeMap.begin(); i!=m_nodeMap.end(); ++i)
	{
		(*i).second->outputStatus(conn);
	}

	for (ObjectMapType::const_iterator j=m_objects.begin(); j!=m_objects.end(); ++j)
	{
		(*j).second->outputStatus(conn, false);
	}
}

// ----------------------------------------------------------------------

Scene::~Scene()
{
	for (NodeMapType::iterator i=m_nodeMap.begin(); i!=m_nodeMap.end(); ++i)
	{
		delete (*i).second;
		(*i).second=0;
	}

	for (ObjectMapType::iterator j=m_objects.begin(); j!=m_objects.end(); ++j)
	{
		delete (*j).second;
		(*j).second = 0;
	}
}

// ----------------------------------------------------------------------

/**
 * Every so often, check for nodes that can be unloaded.  If any are found,
 * unload one node each frame to avoid sending too many unloads to the
 * game servers at the same time.
 */
void Scene::update(float time)
{
	//-- Update population count
	if (m_populationCountTimer.updateSubtract(time))
	{
		PreloadManager::getInstance().updatePopulationList(*m_populationList);

		PopulationListMessage msg(*m_populationList);
		PlanetServer::getInstance().sendToCentral(msg, true);
	}
}

// ----------------------------------------------------------------------

/**
 * For each node, record the preload server id.
 */
void Scene::assignPreloadsToNodes()
{
	for (int x=-getMaxCoordinate(); x<= getMaxCoordinate(); x+=Node::getNodeSize())
	{
		for (int z=-getMaxCoordinate(); z<= getMaxCoordinate(); z+=Node::getNodeSize())
		{
			Node *node = findNodeByRoundedPosition(x,z);
			node->setPreloadServerId(PreloadManager::getInstance().getPreloadGameServer(x,z));
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Check the map for any nodes that belong to the specified server, and
 * load them.  If they are already loaded, move them to the right server.
 */
void Scene::loadAllNodesForServer(uint32 server, PreloadServerId preloadServerId)
{
	for (int x=-getMaxCoordinate(); x<= getMaxCoordinate(); x+=Node::getNodeSize())
	{
		for (int z=-getMaxCoordinate(); z<= getMaxCoordinate(); z+=Node::getNodeSize())
		{
			Node *node = findNodeByRoundedPosition(x,z);
			if (node->getPreloadServerId()==preloadServerId)
			{
				if (ConfigPlanetServer::getLogChunkLoading())
					LOG("ChunkLoading",("Node %s preload to server %lu",node->getDebugNodeString().c_str(), server));

				if (!node->isLoaded())
					node->load(server);
				else
					node->migrateToServer(server);
			}
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Make sure that all nodes are on the server that they should be.
 * Move any nodes that aren't.  They could be on the wrong server due
 * to server crashes, etc.
 */
void Scene::checkServerAssignments()
{
	for (NodeMapType::iterator i=m_nodeMap.begin(); i!=m_nodeMap.end(); ++i)
		i->second->checkServerAssignment();
}

// ----------------------------------------------------------------------

void Scene::handleCrash(uint32 gameServerId)
{
	// wait for the next save before allowing new loads
	if (ConfigPlanetServer::getRequestDbSaveOnGameServerCrash())
		PlanetServer::getInstance().requestAndWaitForSave();

	PreloadManager::getInstance().removeGameServer(gameServerId);

	std::vector<PlanetProxyObject*> subscriptionFixups;

	for (NodeMapType::iterator nodeMapIter = m_nodeMap.begin(); nodeMapIter != m_nodeMap.end(); ++nodeMapIter)
	{
		nodeMapIter->second->handleCrash(gameServerId, subscriptionFixups);
	}

	// These have to be done after all the nodes are examined, because otherwise it might think
	// a server has proxies of the objects on a crashed node when it really doesn't
	for (std::vector<PlanetProxyObject*>::iterator j=subscriptionFixups.begin(); j!=subscriptionFixups.end(); ++j)
		(*j)->subscribeSurroundingNodes();

}

// ----------------------------------------------------------------------

int Scene::getServerPopulationByLocation(int x, int z) const
{
	const Node *node=findNodeByPositionConst(x,z);
	if (node && node->isLoaded())
	{
		uint32 server = node->getPreferredServer();
		return PlanetServer::getInstance().getInterestCreatureObjectCountForServer(server);
	}
	else
		return 0;
}

// ----------------------------------------------------------------------

bool Scene::requestSameServer(const NetworkId &id1, const NetworkId &id2)
{
	PlanetProxyObject *obj1 = findObjectByID(id1);
	PlanetProxyObject *obj2 = findObjectByID(id2);
	if (!obj1 || !obj2)
		return false;

	if (!obj1->isAuthorityClean() || !obj2->isAuthorityClean())
		return false;

	uint32 server1 = obj1->getAuthoritativeServer();
	uint32 server2 = obj2->getAuthoritativeServer();
	if (server1 == server2)
		return true;

	if (!obj1->getNode() || !obj2->getNode())
		return false;
	
	uint32 preferredServer = obj1->getNode()->getPreferredServer();
	if (obj2->getNode()->getPreferredServer() == preferredServer)
	{
		if (server1 == preferredServer)
		{
			if (ConfigPlanetServer::getLogObjectLoading())
				LOG("ObjectLoading",("Moving object %s to server %lu, so that it is on the same server as %s",id2.getValueString().c_str(),server1,id1.getValueString().c_str()));
			obj2->changeAuthorityAndSubscriptions(server1);
			return true;
		}
		else if (server2 == preferredServer)
		{
			if (ConfigPlanetServer::getLogObjectLoading())
				LOG("ObjectLoading",("Moving object %s to server %lu, so that it is on the same server as %s",id1.getValueString().c_str(),server2,id2.getValueString().c_str()));
			obj1->changeAuthorityAndSubscriptions(server2);
			return true;
		}
	}

	if (obj2->wouldAuthorityBeOk(server1))
	{
		if (ConfigPlanetServer::getLogObjectLoading())
			LOG("ObjectLoading",("Moving object %s to server %lu, so that it is on the same server as %s",id2.getValueString().c_str(),server1,id1.getValueString().c_str()));
		obj2->changeAuthorityAndSubscriptions(server1);
		return true;
	}

	if (obj1->wouldAuthorityBeOk(server2))
	{
		if (ConfigPlanetServer::getLogObjectLoading())
			LOG("ObjectLoading",("Moving object %s to server %lu, so that it is on the same server as %s",id1.getValueString().c_str(),server2,id2.getValueString().c_str()));
		obj1->changeAuthorityAndSubscriptions(server2);
		return true;
	}

	// Neither server will work.
	return false;
}

//------------------------------------------------------------------------------------------

bool Scene::requestAuthTransfer(const NetworkId &id1, uint32 newServer)
{
	PlanetProxyObject *obj1 = findObjectByID(id1);
	if (!obj1)
		return false;

	uint32 server1 = obj1->getAuthoritativeServer();

	if (server1 == newServer)
		return true;

	if (obj1->wouldAuthorityBeOk(newServer))
	{
		obj1->changeAuthorityAndSubscriptions(newServer);
		return true;
	}

	// Requested server cannot be used.
	return false;
}

// ----------------------------------------------------------------------

void Scene::handleForceLoadArea(uint32 gameServerId, int x1, int z1, int x2, int z2)
{
	typedef Node QuadTreeNode;
	
	int const zStart = QuadTreeNode::roundToNode(static_cast<int>(z1));
	int const xStart = QuadTreeNode::roundToNode(static_cast<int>(x1));

	for (int nz = zStart; nz <= z2; nz += 100)
	{
		for (int nx = xStart; nx <= x2; nx += 100)
		{
			Node * const node = findNodeByPosition(nx, nz);
			node->subscribeServer(gameServerId, 1);
		}
	}
}

// ----------------------------------------------------------------------

int Scene::getMaxCoordinate() const
{
	if (m_maxCoordinate==0)
		return 8000; // Assume largest possible map until m_maxCoordiate is set
	return m_maxCoordinate;
}

// ----------------------------------------------------------------------

void Scene::setMapSize(int mapSize)
{
	int maxCoordinate = Node::roundToNode(mapSize/2) + Node::getNodeSize();
	if (m_maxCoordinate == 0)
		m_maxCoordinate = maxCoordinate;
	else
		FATAL(m_maxCoordinate!=maxCoordinate,("Game servers for planet %s sent different sizes for the maps.  Possible sizes are %i and %i", m_sceneId.c_str(), m_maxCoordinate, maxCoordinate));
}

// ----------------------------------------------------------------------

/**
 * Create all the proxies that would ever be needed, in a single pass
 */
void Scene::createAllProxies()
{
	static int const subscriptionRange = ConfigPlanetServer::getMaxInterestRadius() + Node::getNodeSize(); // The logic in PlanetProxyObject effectively creates an overlap area of interestRadius + nodeSize, so don't change this without changing the logic in PlanetProxyObject
	
	for (int x=-getMaxCoordinate(); x<= getMaxCoordinate(); x+=Node::getNodeSize())
	{
		for (int z=-getMaxCoordinate(); z<= getMaxCoordinate(); z+=Node::getNodeSize())
		{
			Node * const node = findNodeByRoundedPosition(x,z);
			uint32 serverId = NON_NULL(node)->getPreferredServer();

			if (serverId!=0)
			{
				for (int subX=x-subscriptionRange; subX<= x+subscriptionRange; subX+=Node::getNodeSize())
				{
					for (int subZ=z-subscriptionRange; subZ<= z+subscriptionRange; subZ+=Node::getNodeSize())
					{
						NON_NULL(findNodeByRoundedPosition(subX, subZ))->subscribeServer(serverId, 0);
					}
				}
			}
		}
	}	
}

// ======================================================================

