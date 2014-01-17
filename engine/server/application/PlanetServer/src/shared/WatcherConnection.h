// ======================================================================
//
// WatcherConnection.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_WatcherConnection_H
#define INCLUDED_WatcherConnection_H

// ======================================================================

#include "serverUtility/ServerConnection.h"

class PlanetObjectStatusMessageData;
class PlanetNodeStatusMessageData;

// ======================================================================

class WatcherConnection : public ServerConnection
{
  public:
	WatcherConnection(UdpConnectionMT *, TcpClient *);
	~WatcherConnection();
	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();

	void addObjectUpdate (const NetworkId &objectId, int x, int z, uint32 authoritativeServer, int interestRadius, bool deleteObject, int objectTypeTag, int level, bool hibernating, uint32 templateCrc, int aiActivity, int creationType);
	void addNodeUpdate   (int x, int z, bool loaded, const stdvector<uint32>::fwd &servers, const stdvector<int>::fwd &subscriptionCounts);
	void flushQueuedData ();

  private:
	void flushQueuedObjectData();
	void flushQueuedNodeData();
		
  private:
	typedef stdvector<PlanetObjectStatusMessageData>::fwd ObjectDataList;
	ObjectDataList *m_objectData;

	typedef stdvector<PlanetNodeStatusMessageData>::fwd NodeDataList;
	NodeDataList *m_nodeData;

  private:
	WatcherConnection (const WatcherConnection&);
	WatcherConnection& operator= (const WatcherConnection&);
	WatcherConnection();
};

// ----------------------------------------------------------------------

inline void WatcherConnection::flushQueuedData()
{
	flushQueuedObjectData();
	flushQueuedNodeData();
}

// ======================================================================

#endif
