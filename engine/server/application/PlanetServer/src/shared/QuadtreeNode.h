// ======================================================================
//
// Node.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Node_H
#define INCLUDED_Node_H

// ======================================================================

#include <list>
#include <map>
#include <vector>
#include <set>

class PlanetProxyObject;
class MemoryBlockManager;
class WatcherConnection;

// ======================================================================

class Node
{
  public:
	typedef std::map<uint32, int> SubscriptionListType;

  public:
	Node(int x, int z);
	
  public:
	void         addObject         (PlanetProxyObject *newObject);
	void         checkServerAssignment();
	void         handleCrash       (uint32 crashedServer, std::vector<PlanetProxyObject*> &subscriptionFixups);
	void         load              ();
	void         migrateToServer   (uint32 newServerId);
	void         load              (uint32 serverId);
	void         removeObject      (const PlanetProxyObject *object);
	void         subscribeServer   (uint32 serverId, int count);
	void         unsubscribeServer (uint32 serverId, int count, bool clearIfZero);
	void         setPreloadServerId(PreloadServerId preloadServerId);

  public:
	static int   getNodeSize       ();
	static int   roundToNode       (int coordinate);
	bool         isBorderNode      () const;
	bool         isLoaded          () const;
	bool         isServerSubscribed(uint32 serverId) const;
	std::string  getDebugNodeString() const;
	uint32       getPreferredServer() const;
	PreloadServerId getPreloadServerId() const;
	void         getServers        (std::vector<uint32> &serverList) const;	
	void         outputStatus      (WatcherConnection &conn) const;
	void         outputStatusToAll () const;
	
  public:
	static void  install           ();
	static void  remove            ();
	static void* operator new      (size_t size);
	static void  operator delete   (void* pointer);

  private:
	void         pickPreferredServer();
	void         unproxyAllObjects (uint32 serverId);

  private:
	typedef std::vector<PlanetProxyObject*> ObjectListType;

  private:
	ObjectListType        m_objectList;
	SubscriptionListType  m_subscriptionList;
	uint32                m_preferredServer;
	PreloadServerId       m_preloadServerId;
	bool                  m_loaded;
	mutable bool          m_isBorder;
	
	/**
	 * Coordinates of the node are represented by the minimum X and minimum Z coordinates.
	 */
	int m_x,  m_z;

 	static MemoryBlockManager* memoryBlockManager;

  private:
	Node(); //disable
};

// ======================================================================

inline bool Node::isLoaded() const
{
	return m_loaded;
}

// ----------------------------------------------------------------------

inline int Node::roundToNode(int coordinate)
{
	return (coordinate >= 0) ? (coordinate/getNodeSize())*getNodeSize() : ((coordinate-getNodeSize()+1)/getNodeSize())*getNodeSize(); //lint !e834 // Lint doesn't like a-b+c
}

// ----------------------------------------------------------------------

inline int Node::getNodeSize()
{
	return 100;
}

// ----------------------------------------------------------------------

inline uint32 Node::getPreferredServer() const
{
	return m_preferredServer;
}

// ======================================================================

#endif
