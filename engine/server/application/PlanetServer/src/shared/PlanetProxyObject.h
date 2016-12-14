// ======================================================================
//
// PlanetProxyObject.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlanetProxyObject_H
#define INCLUDED_PlanetProxyObject_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"

class Node;
class MemoryBlockManager;
class WatcherConnection;

// ======================================================================

/**
 * Lightweight proxy of an object, containing just the data the planet server
 * cares about.
 *
 * Coordinates are made into integers because none of the Planet Server's
 * algorithms care about sub-meter distances.
 */
class PlanetProxyObject
{
  public:
	explicit PlanetProxyObject         (const NetworkId &objectId);
	~PlanetProxyObject                 ();

  public:
	void   addServerStatistics         () const;
	void   changeAuthority             (uint32 newGameServer, bool handlingCrash, bool forcedByGame);
	void   changeAuthorityAndSubscriptions(uint32 newGameServer);
	void   onReceivedMessageFromServer (uint32 server);
	void   outputStatus                (WatcherConnection &conn, bool deleteObject) const;
	void   outputStatusToAll           (bool deleteObject) const;
	void   removeServerStatistics      () const;
	void   restoreAuthority            (uint32 gameServer);
	void   sendAddProxy                (uint32 proxyServer);
	void   sendRemoveProxy             (uint32 proxyServer);
	void   setNode                     (Node *node);
	void   subscribeSurroundingNodes   ();
	void   unload                      (bool tellGameServer);
	void   unsubscribeSurroundingNodes (bool clearZeroSubscriptions);
	void   update                      (int x, int y, int z, NetworkId containedBy, uint32 authoritativeServer, int interestRadius, int objectTypeTag, int level, bool hibernating, uint32 templateCrc, int aiActivity, int creationType);

  public:
	int    getX                        () const;
	int    getY                        () const;
	int    getZ                        () const;
	uint32 getAuthoritativeServer      () const;
	int    getInterestRadius           () const;
	bool   isContained                 () const;
	bool   isAuthorityClean            () const;
	const NetworkId & getObjectId      () const;
	const NetworkId & getContainedBy   () const;
	Node * getNode             () const;
	bool   wouldAuthorityBeOk          (uint32 possibleServer) const;
	bool   isCreature                  () const;
	int    getLevel                    () const;
	bool   getHibernating              () const;
	uint32 getTemplateCrc              () const;
	int    getAiActivity               () const;
	int    getCreationType             () const;

  public:
	static void * operator new         (size_t size);
	static void   operator delete      (void* pointer);
	static void   install              ();
	static void   remove               ();
	
  private:
	void   getServers                  (std::vector<uint32> &serverList) const;
	void   changeProxies               (const std::vector<uint32> &oldServers, const std::vector<uint32> &newServers);
	void   sendAuthorityChange         (uint32 currentAuthServer, uint32 newAuthServer, bool handlingCrash);
	bool   isAuthorityOk               () const;
	void   addContainedObject          (const NetworkId & theObject);
	void   removeContainedObject       (const NetworkId & theObject);
	void   updateContentsTracking      (const NetworkId & newContainedBy);

  private:
	typedef std::vector<std::pair<uint32, GameNetworkMessage*> > MessageListType;

  private:
	const NetworkId m_objectId;
	NetworkId       m_containedBy;
	int             m_x;
	int             m_y;
	int             m_z;
	uint32          m_authoritativeServer;
	uint32          m_lastReportedServer;
	int             m_interestRadius;
	int             m_objectTypeTag;
	Node *  m_quadtreeNode;
	unsigned long   m_authTransferTimeMs;
	std::vector<NetworkId> * m_contents;
	int             m_level;
	bool            m_hibernating;
	uint32          m_templateCrc;
	int             m_aiActivity;
	int             m_creationType;

	static MemoryBlockManager* memoryBlockManager;

  private:
	PlanetProxyObject(); //disable
	PlanetProxyObject &operator=(const PlanetProxyObject&); //disable
	PlanetProxyObject(const PlanetProxyObject&); //disable
};

// ======================================================================

inline const NetworkId &PlanetProxyObject::getObjectId() const
{
	return m_objectId;
}

// ----------------------------------------------------------------------

inline int PlanetProxyObject::getX() const
{
	return m_x;
}

// ----------------------------------------------------------------------

inline int PlanetProxyObject::getY() const
{
	return m_y;
}

// ----------------------------------------------------------------------

inline int PlanetProxyObject::getZ() const
{
	return m_z;
}

// ----------------------------------------------------------------------

inline int PlanetProxyObject::getLevel() const
{
	return m_level;
}

// ----------------------------------------------------------------------

inline bool PlanetProxyObject::getHibernating() const
{
	return m_hibernating;
}

// ----------------------------------------------------------------------

inline uint32 PlanetProxyObject::getTemplateCrc() const
{
	return m_templateCrc;
}

// ----------------------------------------------------------------------

inline int PlanetProxyObject::getAiActivity() const
{
	return m_aiActivity;
}

// ----------------------------------------------------------------------

inline int PlanetProxyObject::getCreationType() const
{
	return m_creationType;
}

// ----------------------------------------------------------------------

inline uint32 PlanetProxyObject::getAuthoritativeServer() const
{
	return m_authoritativeServer;
}

// ----------------------------------------------------------------------

inline int PlanetProxyObject::getInterestRadius() const
{
	return m_interestRadius;
}

// ----------------------------------------------------------------------

inline void PlanetProxyObject::setNode(Node *node)
{
	m_quadtreeNode=node;
}

// ----------------------------------------------------------------------

/**
 * Returns true if the object's authority is "clean", i.e. the Game Servers
 * think it is authoritative on the same server the Planet Server does.
 * This returns false when we've changed the object's authority until the
 * newly authoritative server sends us an update, at which point we know
 * that it thinks it is authoritative.
 */
inline bool PlanetProxyObject::isAuthorityClean() const
{
	return m_lastReportedServer == m_authoritativeServer;
}

// ----------------------------------------------------------------------

inline Node *PlanetProxyObject::getNode() const
{
	return m_quadtreeNode;
} //lint !e1763

// ----------------------------------------------------------------------

inline const NetworkId &PlanetProxyObject::getContainedBy() const
{
	return m_containedBy;
}

// ----------------------------------------------------------------------

inline bool PlanetProxyObject::isContained() const
{
	return (m_containedBy != NetworkId::cms_invalid);
}

// ======================================================================

#endif
