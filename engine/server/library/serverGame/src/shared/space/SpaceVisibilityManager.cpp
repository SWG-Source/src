// ======================================================================
//
// SpaceVisibilityManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SpaceVisibilityManager.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/ServerObject.h"
#include "serverGame/VisibleObjectNotification.h"
#include "sharedDebug/Profiler.h"
#include "sharedObject/NetworkIdManager.h"

#include <algorithm>
#include <unordered_map>

// ======================================================================

namespace SpaceVisibilityManager_namespace
{
	// Constants

	const int ms_nodeSize      = 512;   // must be a power of 2
	const int ms_nodeSizeLn    = 9;     // base 2 log of the node size
	const int ms_maxCoordinate = 4096;  // maximum allowed coordinate for an object, positive or negative
	const int ms_maxNodeIndex  = (ms_maxCoordinate * 2) / ms_nodeSize; 

	// Typedefs

	class TrackedObject;
	class Node;

	typedef std::set<Client*>                            ClientSet;
	typedef std::set<TrackedObject*>                     ObjectSet;
	typedef std::map<Client*, TrackedObject*>            ClientToObjectMapType;
	typedef std::unordered_map<NetworkId, TrackedObject*>     TrackedObjectsType;

	// Classes

	class NodeId
	{
	public:
		int m_x;  //lint !e1925 public data member
		int m_y;  //lint !e1925 public data member
		int m_z;  //lint !e1925 public data member

	public:
		explicit NodeId (const Vector &location);
		NodeId          (int x, int y, int z);

		bool operator== (const NodeId &rhs) const;
		bool operator!= (const NodeId &rhs) const;

		int magnitudeBetweenSquared(const NodeId &rhs) const;

	private:
		NodeId(); //disable
	};

	class TrackedObject
	{
	public:
		TrackedObject  (const ServerObject &object, int updateRadius);
		~TrackedObject ();

		void                    addClient                 (Client &newClient);
		void                    allClientsObserveObject   (ServerObject &newObject) const;
		void                    allClientsUnobserveObject (ServerObject &object) const;
		const NodeId &          getCurrentLocation        () const;
		Node &                  getCurrentNode            () const;
		const CachedNetworkId & getNetworkId              () const;
		ServerObject &          getServerObject           () const;
		bool                    hasClients                () const;
		void                    moveTo                    (const NodeId &newLocation);
		void                    removeClient              (Client &client);
		int                     getUpdateRadius           () const;
		ClientSet const *       getClients                () const;

	private:
		CachedNetworkId m_object; 
		int             m_updateRadius;       
		NodeId          m_currentLocation; 
		ClientSet *     m_clients;     

	private:
		TrackedObject(); //disable
	};

	class Node
	{
	public:
		Node ();

		void addObservingObject             (TrackedObject &newObject, bool observeNow);
		void addVisibleObject               (TrackedObject &newObject);
		void getAllVisibleObjects           (std::vector<ServerObject*> & results) const;
		void removeObservingObject          (TrackedObject &newObject);
		void removeVisibleObject            (TrackedObject &newObject);
		void getVisibleObjectDifference     (const Node &otherNode, std::vector<TrackedObject*> & lostObjects, std::vector<TrackedObject*> & gainedObjects) const;
		ObjectSet const &getObservingObjects() const;
		
	private:
		ObjectSet   m_visibleObjects;
		ObjectSet   m_observingObjects;

	private:
		Node (const Node &); //lint -esym(754,Node::Node) // unreferenced private copy constructor
	};

	// Functions

	void            addToNodesInRange       (TrackedObject &object, const NodeId &location, int range);
	void            addToNodesInVolume      (TrackedObject & object, int minX, int maxX, int minY, int maxY, int minZ, int maxZ);
	int             coordinateToNodeIndex   (float coordinate);
	Node &          getNode                 (const NodeId &location); //lint !e1929 returning a reference
	TrackedObject * internalAddObject       (ServerObject & object, int updateRadius);
	void            removeFromNodesInRange  (TrackedObject &object, const NodeId &location, int range);
	void            removeFromNodesInVolume (TrackedObject & object, int minX, int maxX, int minY, int maxY, int minZ, int maxZ);
	int             roundToNodeBoundary     (int unroundedLocation);
	int             trimIndexToAllowedRange (int index);

	// Data

	ClientToObjectMapType  ms_clientToObjectMap;
	Node                   ms_nodeMap[ms_maxNodeIndex+1][ms_maxNodeIndex+1][ms_maxNodeIndex+1];
	TrackedObjectsType     ms_trackedObjects;

} //namespace

using namespace SpaceVisibilityManager_namespace;

// ======================================================================

void SpaceVisibilityManager::addClient(Client & client, ServerObject & observingObject)
{
	PROFILER_AUTO_BLOCK_DEFINE("SpaceVisibilityManager::addClient");
	DEBUG_REPORT_LOG(ConfigServerGame::getDebugSpaceVisibilityManager(),("SpaceVisibilityManager::addClient(Client & client, %s);\n",observingObject.getNetworkId().getValueString().c_str()));

	TrackedObjectsType::iterator i=ms_trackedObjects.find(observingObject.getNetworkId());
	TrackedObject *to=nullptr;
	if (i!=ms_trackedObjects.end())
	{
		to=i->second;
	}
	else
	{
		to=internalAddObject(observingObject, static_cast<int>(observingObject.getFarNetworkUpdateRadius()));
	}
	NOT_NULL(to);

	std::pair<ClientToObjectMapType::iterator, bool> result =ms_clientToObjectMap.insert(std::make_pair(&client,to));
	if (!result.second)
	{
		DEBUG_WARNING(result.first->second != to,("Programmer bug:  called addClient(c,%s) with a client that was already in the map with a different object (%s).  removeClient() should have been called first.  (SpaceVisibilityTracker does not support controlling multiple objects that aren't in the same containment tree.)", observingObject.getNetworkId().getValueString().c_str(), result.first->second->getNetworkId().getValueString().c_str()));
		removeClient(client);
		result = ms_clientToObjectMap.insert(std::make_pair(&client,to));
		DEBUG_FATAL(!result.second,("Programmer bug:  removeClient did not remove the client from ms_clientToObjectMap"));
		UNREF(result);
	}

	to->addClient(client);
	Node &theNode = to->getCurrentNode();
	theNode.addObservingObject(*to, true);
}

// ----------------------------------------------------------------------

void SpaceVisibilityManager::removeClient(Client & client)
{
	PROFILER_AUTO_BLOCK_DEFINE("SpaceVisibilityManager::removeClient");
	DEBUG_REPORT_LOG(ConfigServerGame::getDebugSpaceVisibilityManager(),("SpaceVisibilityManager::removeClient();\n"));

	ClientToObjectMapType::iterator i=ms_clientToObjectMap.find(&client);
	if (i!=ms_clientToObjectMap.end())
	{
		TrackedObject *to=i->second;
		NOT_NULL(to);
		to->removeClient(client);
		ms_clientToObjectMap.erase(i);
	}
	else
		DEBUG_REPORT_LOG(ConfigServerGame::getDebugSpaceVisibilityManager(),("SpaceVisibilityManager::removeClient() called with a client that was not in the map, ignoring.\n"));
}

// ----------------------------------------------------------------------

void SpaceVisibilityManager::addObject(ServerObject & object, int updateRadius)
{
	PROFILER_AUTO_BLOCK_DEFINE("SpaceVisibilityManager::addObject");
	IGNORE_RETURN(internalAddObject(object,updateRadius));
}

// ----------------------------------------------------------------------

TrackedObject * SpaceVisibilityManager_namespace::internalAddObject(ServerObject & object, int updateRadius)
{
	PROFILER_AUTO_BLOCK_DEFINE("SpaceVisibilityManager::addObject");
	DEBUG_REPORT_LOG(ConfigServerGame::getDebugSpaceVisibilityManager(),("SpaceVisibilityManager::addObject(%s, %i);\n",object.getNetworkId().getValueString().c_str(), updateRadius));

	updateRadius = updateRadius==0 ? 0 : (1 + (updateRadius-1) / ms_nodeSize); // divide specified radius by node size, but always round up
	TrackedObjectsType::const_iterator i=ms_trackedObjects.find(object.getNetworkId());
	if (i!=ms_trackedObjects.end())
	{
		DEBUG_FATAL(i->second->getUpdateRadius()!=updateRadius,
					("Programmer bug:  Added object %s to SpaceVisibilityTracker, but it had already been added with a different update radius.",object.getNetworkId().getValueString().c_str()));
		return i->second;
	}
	else
	{
		TrackedObject *newObject=new TrackedObject(object, updateRadius);
		object.addNotification(VisibleObjectNotification::getInstance(), true);

		return newObject;
	}
}

// ----------------------------------------------------------------------

void SpaceVisibilityManager::moveObject(const ServerObject & object)
{
	PROFILER_AUTO_BLOCK_DEFINE("SpaceVisibilityManager::moveObject");

	TrackedObject *vis = ms_trackedObjects[object.getNetworkId()];
	DEBUG_FATAL(!vis,("Programmer Bug:  Called SpaceVisibilityManager::moveObject(%s), but the object had not been added to SpaceVisibilityManager",object.getNetworkId().getValueString().c_str()));
	if (!vis) //lint !e774 //always false (in debug build only)
		return;

	vis->moveTo(NodeId(object.getPosition_w()));
}

// ----------------------------------------------------------------------

void SpaceVisibilityManager::removeObject(ServerObject & object)
{
	PROFILER_AUTO_BLOCK_DEFINE("SpaceVisibilityManager::removeObject");
	DEBUG_REPORT_LOG(ConfigServerGame::getDebugSpaceVisibilityManager(),("SpaceVisibilityManager::removeObject(%s);\n",object.getNetworkId().getValueString().c_str()));

	TrackedObject *vis = ms_trackedObjects[object.getNetworkId()];
	DEBUG_FATAL(!vis,("Programmer Bug:  Called SpaceVisibilityManager::removeObject(%s), but the object had not been added to SpaceVisibilityManager",object.getNetworkId().getValueString().c_str()));
	if (!vis)   //lint !e774 //always false (in debug build only)
		return;
	delete vis;
	vis=nullptr;

	object.removeNotification(VisibleObjectNotification::getInstance(),true);
}

// ----------------------------------------------------------------------

void SpaceVisibilityManager_namespace::addToNodesInRange(TrackedObject &object, const NodeId &location, int range)
{
	addToNodesInVolume(object, location.m_x-range, location.m_x+range,location.m_y-range, location.m_y+range,location.m_z-range, location.m_z+range);
}

// ----------------------------------------------------------------------

int SpaceVisibilityManager_namespace::trimIndexToAllowedRange(int index)
{
	if (index < 0)
		return 0;
	else if (index > ms_maxNodeIndex)
		return ms_maxNodeIndex;
	else
		return index;
}

// ----------------------------------------------------------------------

/**
 * Add a visible object from a rectangular volume of space, specified by x,y, and z ranges, which must be node boundaries
 */
void SpaceVisibilityManager_namespace::addToNodesInVolume(TrackedObject & object, int minX, int maxX, int minY, int maxY, int minZ, int maxZ)
{
	minX = std::max(0, minX);
	maxX = std::min(ms_maxNodeIndex, maxX);
	minY = std::max(0, minY);
	maxY = std::min(ms_maxNodeIndex, maxY);
	minZ = std::max(0, minZ);
	maxZ = std::min(ms_maxNodeIndex, maxZ);

	for (int x=minX; x<=maxX; ++x)
		for (int y=minY; y<=maxY; ++y)
			for (int z=minZ; z<=maxZ; ++z)
			{
				Node &theNode=getNode(NodeId(x,y,z));
				theNode.addVisibleObject(object);
			}
}

// ----------------------------------------------------------------------

void SpaceVisibilityManager_namespace::removeFromNodesInRange(TrackedObject &object, const NodeId &location, int range)
{
	removeFromNodesInVolume(object, location.m_x-range, location.m_x+range,location.m_y-range, location.m_y+range,location.m_z-range, location.m_z+range);
}

// ----------------------------------------------------------------------

/**
 * Remove a visible object from a rectangular volume of space, specified by x,y, and z ranges, which must be node boundaries
 */
void SpaceVisibilityManager_namespace::removeFromNodesInVolume(TrackedObject & object, int minX, int maxX, int minY, int maxY, int minZ, int maxZ)
{
	minX = std::max(0, minX);
	maxX = std::min(ms_maxNodeIndex, maxX);
	minY = std::max(0, minY);
	maxY = std::min(ms_maxNodeIndex, maxY);
	minZ = std::max(0, minZ);
	maxZ = std::min(ms_maxNodeIndex, maxZ);

	for (int x=minX; x<=maxX; ++x)
		for (int y=minY; y<=maxY; ++y)
			for (int z=minZ; z<=maxZ; ++z)
			{
				Node &theNode=getNode(NodeId(x,y,z));
				theNode.removeVisibleObject(object);
			}
}

// ----------------------------------------------------------------------

/**
 * Rounds to the node boundary that contains this coordinate.
 */
int SpaceVisibilityManager_namespace::roundToNodeBoundary(int unroundedLocation)
{
	return (unroundedLocation >= 0) ? (unroundedLocation/ms_nodeSize)*ms_nodeSize : ((unroundedLocation+1-ms_nodeSize)/ms_nodeSize)*ms_nodeSize;
}

// ----------------------------------------------------------------------

int SpaceVisibilityManager_namespace::coordinateToNodeIndex(float coordinate)
{
	int rounded=static_cast<int>(coordinate);

	if (rounded > ms_maxCoordinate)
	{ 
		rounded = ms_maxCoordinate;
	}
	if (rounded < -ms_maxCoordinate) 
	{ 
		rounded = -ms_maxCoordinate; 
	}

	DEBUG_FATAL(rounded + ms_maxCoordinate < 0,("Programmer bug:  the result of coordinate + ms_maxCoordinate was negative."));
	int result = (rounded + ms_maxCoordinate ) >> ms_nodeSizeLn; //lint !e702 shift right of signed - OK because we make sure it's positive
	DEBUG_FATAL(result < 0 || result > ms_maxNodeIndex,("Programmer bug:  the result of coordinateToNodeIndex() was not within the allowed size."));
	return result;
}

// ----------------------------------------------------------------------

/**
 * Return the node at the specified coordinates
 */
Node &SpaceVisibilityManager_namespace::getNode(const NodeId &location)
{ //lint !e1929 returning a reference
#ifdef _DEBUG
	int x=trimIndexToAllowedRange(location.m_x);
	int y=trimIndexToAllowedRange(location.m_y);
	int z=trimIndexToAllowedRange(location.m_z);

	DEBUG_FATAL(location != NodeId(x,y,z),("getNode was called with NodeId (%i, %i, %i), which is not in the allowed range.",location.m_x, location.m_y, location.m_z));
#endif

	return ms_nodeMap[location.m_x][location.m_y][location.m_z];
}

// ----------------------------------------------------------------------

void SpaceVisibilityManager::getObjectsVisibleFromLocation(const Vector & location, std::vector<ServerObject*> &results)
{
	const Node &theNode = getNode(NodeId(location));
	theNode.getAllVisibleObjects(results);
}

// ----------------------------------------------------------------------

void SpaceVisibilityManager::getClientsInRange(Vector const &location, float radius, std::set<Client *> &results)
{
	int const minX = coordinateToNodeIndex(location.x-radius);
	int const maxX = coordinateToNodeIndex(location.x+radius);
	int const minY = coordinateToNodeIndex(location.y-radius);
	int const maxY = coordinateToNodeIndex(location.y+radius);
	int const minZ = coordinateToNodeIndex(location.z-radius);
	int const maxZ = coordinateToNodeIndex(location.z+radius);

	float const radiusSquared = sqr(radius);

	for (int x = minX; x <= maxX; ++x)
	{
		for (int y = minY; y <= maxY; ++y)
		{
			for (int z = minZ; z <= maxZ; ++z)
			{
				ObjectSet const &observingObjects = getNode(NodeId(x, y, z)).getObservingObjects();
				for (ObjectSet::const_iterator i = observingObjects.begin(); i != observingObjects.end(); ++i)
				{
					ServerObject const &so = (*i)->getServerObject();
					if (location.magnitudeBetweenSquared(so.getPosition_w()) < radiusSquared)
					{
						ClientSet const * const clients = (*i)->getClients();
						if (clients && !clients->empty())
							results.insert(clients->begin(), clients->end());
					}
				}
			}
		}
	}
}

// ======================================================================

Node::Node() :
		m_visibleObjects(),
		m_observingObjects()
{
}

// ----------------------------------------------------------------------

void Node::addVisibleObject(TrackedObject &newObject)
{
	std::pair<ObjectSet::iterator, bool> result = m_visibleObjects.insert(&newObject);
	DEBUG_FATAL(!result.second,("Called Node::addVisibleObject(%s), but the object was already in the node.",newObject.getNetworkId().getValueString().c_str()));
	UNREF(result);

	ServerObject &asServerObject = newObject.getServerObject();

	for (ObjectSet::iterator i=m_observingObjects.begin(); i!=m_observingObjects.end(); ++i)
	{
		if (*i != &newObject)
		{
			NOT_NULL(*i);
			(*i)->allClientsObserveObject(asServerObject);
		}
	}
} //lint !e1764 parameter can't really be made const, but lint thinks it can

// ----------------------------------------------------------------------

void Node::getAllVisibleObjects(std::vector<ServerObject*> & results) const
{
	results.reserve(results.size() + m_visibleObjects.size());
	
	for (ObjectSet::const_iterator i=m_visibleObjects.begin(); i!=m_visibleObjects.end(); ++i)
	{
		NOT_NULL(*i);
		results.push_back(&((*i)->getServerObject()));
	}
}

// ----------------------------------------------------------------------

void Node::removeVisibleObject(TrackedObject &object)
{
	size_t result = m_visibleObjects.erase(&object);
	UNREF(result);  // for release mode
	DEBUG_FATAL(result != 1,("Programmer bug:  Node::removeVisibleObject(%s) called on an object that was not in the list",object.getNetworkId().getValueString().c_str()));

	ServerObject &asServerObject = object.getServerObject();

	for (ObjectSet::iterator i=m_observingObjects.begin(); i!=m_observingObjects.end(); ++i)
	{
		if (*i != &object)
		{
			NOT_NULL(*i);
			(*i)->allClientsUnobserveObject(asServerObject);
		}
	}
} //lint !e1764 parameter can't really be made const, but lint thinks it can

// ----------------------------------------------------------------------

void Node::addObservingObject (TrackedObject & newObject, bool observeNow)
{
	DEBUG_FATAL(!newObject.hasClients(),("Programmer bug:  called Node::addObservingObject(%s) with an object that did not have any clients attached.",newObject.getNetworkId().getValueString().c_str()));
	std::pair<ObjectSet::iterator, bool> result = m_observingObjects.insert(&newObject);
	UNREF(result);
	DEBUG_REPORT_LOG(ConfigServerGame::getDebugSpaceVisibilityManager() && !result.second,("Called Node::addObservingObject(%s), but it was already on the list\n",newObject.getNetworkId().getValueString().c_str()));

	if (observeNow)
	{
		for (ObjectSet::iterator j=m_visibleObjects.begin(); j!=m_visibleObjects.end(); ++j)
			newObject.allClientsObserveObject((*j)->getServerObject());
	}
} //lint !e1764 parameter can't really be made const, but lint thinks it can

// ----------------------------------------------------------------------

/**
 * Remove an observing object from a node.
 *
 * @param oldObject the object
 */
void Node::removeObservingObject (TrackedObject & oldObject)
{
	size_t result=m_observingObjects.erase(&oldObject);
	UNREF(result); // for release mode
	DEBUG_FATAL(result!=1,("Programmer bug:  removeObservingObject called on an object that was not in the observing list on the node"));
} //lint !e1764 parameter can't really be made const, but lint thinks it can

// ----------------------------------------------------------------------

/**
 * Find the difference in the visible object list between two nodes
 *
 * @param otherNode The node to compare againse.
 * @param lostObjects (output) The list of objects that are in the other node, but are
 *    not in this node.
 * @param gainedObjects (output) The list of objects that are in this node, but are
 *    not in the other node.
 */
void Node::getVisibleObjectDifference(const Node &otherNode, std::vector<TrackedObject*> & lostObjects, std::vector<TrackedObject*> & gainedObjects) const
{
	IGNORE_RETURN(std::set_difference(otherNode.m_visibleObjects.begin(), otherNode.m_visibleObjects.end(), m_visibleObjects.begin(), m_visibleObjects.end(), std::back_insert_iterator<std::vector<TrackedObject*> >(lostObjects)));
	IGNORE_RETURN(std::set_difference(m_visibleObjects.begin(), m_visibleObjects.end(), otherNode.m_visibleObjects.begin(), otherNode.m_visibleObjects.end(), std::back_insert_iterator<std::vector<TrackedObject*> >(gainedObjects)));
}

// ----------------------------------------------------------------------

ObjectSet const &Node::getObservingObjects() const
{
	return m_observingObjects;
}

// ======================================================================

TrackedObject::TrackedObject(const ServerObject &object, int updateRadius) :
		m_object(object),
		m_updateRadius(updateRadius),
		m_currentLocation(NodeId(object.getPosition_w())),
		m_clients(nullptr)
{
	ms_trackedObjects[object.getNetworkId()]=this;
	addToNodesInRange(*this, m_currentLocation, updateRadius);
}

// ----------------------------------------------------------------------

TrackedObject::~TrackedObject()
{
	if (m_clients)
	{
		Node &theNode = getNode(m_currentLocation);
		theNode.removeObservingObject(*this);
		for (ClientSet::iterator i=m_clients->begin(); i!=m_clients->end(); ++i)
		{
			IGNORE_RETURN(ms_clientToObjectMap.erase(*i));
		}

		delete m_clients;
		m_clients=nullptr;
	}

	removeFromNodesInRange(*this, m_currentLocation, m_updateRadius);
	IGNORE_RETURN(ms_trackedObjects.erase(m_object));
} //lint !e1740 m_clients not freed (actually it is)

// ----------------------------------------------------------------------

const CachedNetworkId & TrackedObject::getNetworkId() const
{
	return m_object;
}

// ----------------------------------------------------------------------

bool TrackedObject::hasClients() const
{
	return (m_clients != nullptr);
}

// ----------------------------------------------------------------------

void TrackedObject::addClient(Client &newClient)
{
	if (!m_clients)
		m_clients=new ClientSet;
	IGNORE_RETURN(m_clients->insert(&newClient));
} //lint !e1764 (newClient can't really be made const)

// ----------------------------------------------------------------------

void TrackedObject::removeClient(Client &client)
{
	if (m_clients)
	{
		IGNORE_RETURN(m_clients->erase(&client));
		if (m_clients->empty())
		{
			delete m_clients;
			m_clients=nullptr;

			getNode(m_currentLocation).removeObservingObject(*this);
		}
	}
}  //lint !e1764 (client can't really be made const)

// ----------------------------------------------------------------------

/**
 * Tell all clients watching this object about a new object they should
 * observe.
 */
void TrackedObject::allClientsObserveObject(ServerObject &newObject) const
{
	if (m_clients)
	{
		for (ClientSet::iterator j=m_clients->begin(); j!=m_clients->end(); ++j)
		{
			NOT_NULL(*j);
			ObserveTracker::onClientAddedVisibleObjectSpace(**j, newObject);
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Tell all clients watching this object that an object should not longer
 * be observed
 */
void TrackedObject::allClientsUnobserveObject(ServerObject &object) const
{
	if (m_clients)
	{
		for (ClientSet::iterator j=m_clients->begin(); j!=m_clients->end(); ++j)
		{
			NOT_NULL(*j);
			ObserveTracker::onClientLostVisibleObjectSpace(**j, object);
		}
	}
}

// ----------------------------------------------------------------------

Node & TrackedObject::getCurrentNode() const
{
	return getNode(m_currentLocation);
}

// ----------------------------------------------------------------------

ServerObject & TrackedObject::getServerObject() const
{
	Object *object = m_object.getObject();
	NOT_NULL(object);
	ServerObject *asServerObject = object->asServerObject();
	NOT_NULL(asServerObject);
	return *asServerObject;
}

// ----------------------------------------------------------------------

const NodeId & TrackedObject::getCurrentLocation() const
{
	return m_currentLocation;
}

// ----------------------------------------------------------------------

void TrackedObject::moveTo(const NodeId &newLocation)
{
	if (newLocation != m_currentLocation)
	{
		DEBUG_REPORT_LOG(ConfigServerGame::getDebugSpaceVisibilityManager(),("SpaceVisibilityManager::TrackedObject::moveTo(); (object id %s)\n",m_object.getValueString().c_str()));

		const NodeId oldLocation(m_currentLocation);

		if (newLocation.magnitudeBetweenSquared(oldLocation) < 4)
		{
			// small move.  Do it incrementally to minimize the unobserves/reobserves
			while (m_currentLocation != newLocation)
			{
				if (m_currentLocation.m_x < newLocation.m_x)
				{
					removeFromNodesInVolume( *this,
											 m_currentLocation.m_x - m_updateRadius, m_currentLocation.m_x - m_updateRadius,
											 m_currentLocation.m_y - m_updateRadius, m_currentLocation.m_y + m_updateRadius,
											 m_currentLocation.m_z - m_updateRadius, m_currentLocation.m_z + m_updateRadius);
					m_currentLocation.m_x = m_currentLocation.m_x + 1;
					addToNodesInVolume( *this,
										m_currentLocation.m_x + m_updateRadius, m_currentLocation.m_x + m_updateRadius,
										m_currentLocation.m_y - m_updateRadius, m_currentLocation.m_y + m_updateRadius,
										m_currentLocation.m_z - m_updateRadius, m_currentLocation.m_z + m_updateRadius);
				}
				if (m_currentLocation.m_x > newLocation.m_x)
				{
					removeFromNodesInVolume( *this,
											 m_currentLocation.m_x + m_updateRadius, m_currentLocation.m_x + m_updateRadius,
											 m_currentLocation.m_y - m_updateRadius, m_currentLocation.m_y + m_updateRadius,
											 m_currentLocation.m_z - m_updateRadius, m_currentLocation.m_z + m_updateRadius);
					m_currentLocation.m_x = m_currentLocation.m_x - 1;
					addToNodesInVolume( *this,
										m_currentLocation.m_x - m_updateRadius, m_currentLocation.m_x - m_updateRadius,
										m_currentLocation.m_y - m_updateRadius, m_currentLocation.m_y + m_updateRadius,
										m_currentLocation.m_z - m_updateRadius, m_currentLocation.m_z + m_updateRadius);
				}

				if (m_currentLocation.m_y < newLocation.m_y)
				{
					removeFromNodesInVolume( *this,
											 m_currentLocation.m_x - m_updateRadius, m_currentLocation.m_x + m_updateRadius,
											 m_currentLocation.m_y - m_updateRadius, m_currentLocation.m_y - m_updateRadius,
											 m_currentLocation.m_z - m_updateRadius, m_currentLocation.m_z + m_updateRadius);
					m_currentLocation.m_y = m_currentLocation.m_y + 1;
					addToNodesInVolume( *this,
										m_currentLocation.m_x - m_updateRadius, m_currentLocation.m_x + m_updateRadius,
										m_currentLocation.m_y + m_updateRadius, m_currentLocation.m_y + m_updateRadius,
										m_currentLocation.m_z - m_updateRadius, m_currentLocation.m_z + m_updateRadius);
				}
				if (m_currentLocation.m_y > newLocation.m_y)
				{
					removeFromNodesInVolume( *this,
											 m_currentLocation.m_x - m_updateRadius, m_currentLocation.m_x + m_updateRadius,
											 m_currentLocation.m_y + m_updateRadius, m_currentLocation.m_y + m_updateRadius,
											 m_currentLocation.m_z - m_updateRadius, m_currentLocation.m_z + m_updateRadius);
					m_currentLocation.m_y = m_currentLocation.m_y - 1;
					addToNodesInVolume( *this,
										m_currentLocation.m_x - m_updateRadius, m_currentLocation.m_x + m_updateRadius,
										m_currentLocation.m_y - m_updateRadius, m_currentLocation.m_y - m_updateRadius,
										m_currentLocation.m_z - m_updateRadius, m_currentLocation.m_z + m_updateRadius);
				}

				if (m_currentLocation.m_z < newLocation.m_z)
				{
					removeFromNodesInVolume( *this,
											 m_currentLocation.m_x - m_updateRadius, m_currentLocation.m_x + m_updateRadius,
											 m_currentLocation.m_y - m_updateRadius, m_currentLocation.m_y + m_updateRadius,
											 m_currentLocation.m_z - m_updateRadius, m_currentLocation.m_z - m_updateRadius);
					m_currentLocation.m_z = m_currentLocation.m_z + 1;
					addToNodesInVolume( *this,
										m_currentLocation.m_x - m_updateRadius, m_currentLocation.m_x + m_updateRadius,
										m_currentLocation.m_y - m_updateRadius, m_currentLocation.m_y + m_updateRadius,
										m_currentLocation.m_z + m_updateRadius, m_currentLocation.m_z + m_updateRadius);
				}
				if (m_currentLocation.m_z > newLocation.m_z)
				{
					removeFromNodesInVolume( *this,
											 m_currentLocation.m_x - m_updateRadius, m_currentLocation.m_x + m_updateRadius,
											 m_currentLocation.m_y - m_updateRadius, m_currentLocation.m_y + m_updateRadius,
											 m_currentLocation.m_z + m_updateRadius, m_currentLocation.m_z + m_updateRadius);
					m_currentLocation.m_z = m_currentLocation.m_z - 1;
					addToNodesInVolume( *this,
										m_currentLocation.m_x - m_updateRadius, m_currentLocation.m_x + m_updateRadius,
										m_currentLocation.m_y - m_updateRadius, m_currentLocation.m_y + m_updateRadius,
										m_currentLocation.m_z - m_updateRadius, m_currentLocation.m_z - m_updateRadius);
				}
			}
		}
		else
		{
			// big move.  Probably little overlap between the objects it was observing and the objects it will be observing
			removeFromNodesInRange(*this, oldLocation, m_updateRadius);
			m_currentLocation = newLocation;
			addToNodesInRange(*this, newLocation, m_updateRadius);
		}

		if (m_clients)
		{
			Node &oldNode = getNode(oldLocation);
			Node &newNode = getNode(newLocation);

			static std::vector<TrackedObject*> lostObjects;
			static std::vector<TrackedObject*> gainedObjects;
			lostObjects.clear();
			gainedObjects.clear();

			oldNode.removeObservingObject(*this);
			newNode.addObservingObject(*this, false);
			newNode.getVisibleObjectDifference(oldNode, lostObjects, gainedObjects);

			{
				for (std::vector<TrackedObject*>::const_iterator i = lostObjects.begin(); i != lostObjects.end(); ++i)
					if (*i != this)
						allClientsUnobserveObject((*i)->getServerObject());
			}
				
			{
				for (std::vector<TrackedObject*>::const_iterator i = gainedObjects.begin(); i != gainedObjects.end(); ++i)
					if (*i != this)
						allClientsObserveObject((*i)->getServerObject());
			}
		}
	}
}

// ----------------------------------------------------------------------

int TrackedObject::getUpdateRadius() const
{
	return m_updateRadius;
}

// ----------------------------------------------------------------------

ClientSet const *TrackedObject::getClients() const
{
	return m_clients;
}

// ======================================================================

NodeId::NodeId(const Vector &location) :
		m_x(coordinateToNodeIndex(location.x)),
		m_y(coordinateToNodeIndex(location.y)),
		m_z(coordinateToNodeIndex(location.z))
{
}

// ----------------------------------------------------------------------

NodeId::NodeId(int x, int y, int z) :
		m_x(x),
		m_y(y),
		m_z(z)
{
	DEBUG_FATAL(m_x!=trimIndexToAllowedRange(m_x),("Programmer bug:  NodeId::NodeId(%i,%i,%i) -- indexes are out of the allowed range",m_x,m_y,m_z));
	DEBUG_FATAL(m_y!=trimIndexToAllowedRange(m_y),("Programmer bug:  NodeId::NodeId(%i,%i,%i) -- indexes are out of the allowed range",m_x,m_y,m_z));
	DEBUG_FATAL(m_z!=trimIndexToAllowedRange(m_z),("Programmer bug:  NodeId::NodeId(%i,%i,%i) -- indexes are out of the allowed range",m_x,m_y,m_z));
}

// ----------------------------------------------------------------------

bool NodeId::operator==(const NodeId &rhs) const
{
	return (m_x == rhs.m_x) && (m_y == rhs.m_y) && (m_z == rhs.m_z);
}

// ----------------------------------------------------------------------

bool NodeId::operator!=(const NodeId &rhs) const
{
	return !(operator==(rhs));
}

// ----------------------------------------------------------------------

int NodeId::magnitudeBetweenSquared(const NodeId &rhs) const
{
	return ((m_x - rhs.m_x) * (m_x - rhs.m_x) +
			(m_y - rhs.m_y) * (m_y - rhs.m_y) +
			(m_z - rhs.m_z) * (m_z - rhs.m_z));
}

// ======================================================================
