// ======================================================================
//
// Scene.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Scene_H
#define INCLUDED_Scene_H

// ======================================================================

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <set>

#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Timer.h"
#include "sharedMessageDispatch/Receiver.h"
#include "Singleton/Singleton2.h"
#include "sharedMath/Vector.h"

class PlanetProxyObject;
class PopulationList;
class Node;

// ======================================================================

/**
 * Organizes all data on the Planet server relating to a particular scene.
 */
class Scene : public Singleton2<Scene>, public MessageDispatch::Receiver
{
  public:
	// Some functions return lists of nodes
	typedef std::vector<Node*> NodeListType;

  public:
	Scene();
	~Scene();

  public:
	virtual void         receiveMessage                  (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	Node *               findNodeByPosition              (int x, int z);
	Node *               findNodeByRoundedPosition       (int x, int z);
	void                 addObject                       (PlanetProxyObject *newObject);
	void                 findIntersection                (NodeListType &results,int x, int z, int radius);
	void                 assignPreloadsToNodes           ();
	void                 loadAllNodesForServer           (uint32 server, PreloadServerId preloadServerId);
	void                 checkServerAssignments          ();
	void                 removeObjectFromMap             (const NetworkId &objectId, bool addToDeletedList);
	void                 setSceneId                      (const std::string &sceneId);
	void                 subscribeServer                 (uint32 server, int x, int z, int interestRadius, int count=1);
	void                 update                          (float time);
	bool                 requestSameServer               (const NetworkId &id1, const NetworkId &id2);
	bool                 requestAuthTransfer             (const NetworkId &id1, uint32 newServer);
	void                 setMapSize                      (int mapSize);
	void                 createAllProxies                ();

  public:
	const Node * findNodeByPositionConst         (int x, int z) const;
	const Node * findNodeByRoundedPositionConst  (int x, int z) const;
	bool                 isObjectLoaded                  (const NetworkId &networkId) const;
	PlanetProxyObject *  findObjectByID                  (const NetworkId &objectID) const;
	const std::string &  getSceneId                      () const;
	uint32               getGameServerForObject          (const NetworkId &object) const;
	void                 outputStatus                    (WatcherConnection &conn) const;
	int                  getServerPopulationByLocation   (int x, int z) const;
	int                  getMaxCoordinate                () const;
		
  private:
	void                 handleRemoveObject              (const NetworkId &objectId);
	void                 handleCrash                     (uint32 gameServerId);
	void                 handleForceLoadArea             (uint32 gameServerId, int x1, int z1, int x2, int z2);

  private:
	struct Coordinates
	{
		public:
		int m_x;
		int m_z;
		Coordinates(int x, int z);
		bool operator==(const Coordinates &rhs) const;
		
		class Hasher
			{
			  public:
				size_t operator()(const Coordinates &c) const;
			};
		
		private:
		Coordinates();
	};

	typedef std::unordered_map<NetworkId, PlanetProxyObject*, NetworkId::Hash>  ObjectMapType;
	typedef std::unordered_set<NetworkId, NetworkId::Hash>                      DeletedSetType;
	typedef std::unordered_map<Coordinates, Node*, Coordinates::Hasher>         NodeMapType;
	
  private:
	std::string       m_sceneId;
	ObjectMapType     m_objects;
	DeletedSetType    m_deletedObjects;
	NodeMapType       m_nodeMap;
	PopulationList *  m_populationList;
	Timer             m_populationCountTimer;
	int               m_maxCoordinate;
};

// ======================================================================

inline Scene::Coordinates::Coordinates(int x, int z) :
	m_x(x),
	m_z(z)
{
}

// ----------------------------------------------------------------------

inline bool Scene::Coordinates::operator==(const Coordinates &rhs) const
{
	return ((m_x==rhs.m_x) && (m_z==rhs.m_z));
}

// ----------------------------------------------------------------------

inline size_t Scene::Coordinates::Hasher::operator()(const Coordinates &c) const
{
	// will need to change this if we make planets bigger than 32k * 32k
	return (c.m_x<<16 | c.m_z); //lint !e701 shift left of signed
}

// ----------------------------------------------------------------------

inline const std::string &Scene::getSceneId() const
{
	return m_sceneId;
}

// ======================================================================

#endif
