// ======================================================================
//
// ServerPathfindingMessaging.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_ServerPathfindingMessaging_H
#define	INCLUDED_ServerPathfindingMessaging_H

#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"

class CityPathGraph;
class CityPathNode;
class Client;
class NetworkId;
struct ClientDestroy;
class AiLocation;

// ======================================================================

class ServerPathfindingMessaging : public MessageDispatch::Receiver
{
public:

	static void install ( void );
	static void remove  ( void );

	static ServerPathfindingMessaging & getInstance ( void );

	// ----------

	ServerPathfindingMessaging();
	virtual ~ServerPathfindingMessaging();

	// ----------

	void sendGraphInfo    ( CityPathGraph const * graph );
	void sendEraseGraph   ( CityPathGraph const * graph );
	void sendNodeInfo     ( CityPathNode const * node );
	void sendNeighborInfo ( CityPathNode const * node );
	void sendEraseNode    ( CityPathNode const * node );
	void sendWaypointInfo ( AiLocation const & loc );
	void sendEraseWaypoint( AiLocation const & loc );

protected:

	void sendGraphInfo    ( CityPathGraph const * graph, Client * client );
	void sendEraseGraph   ( CityPathGraph const * graph, Client * client );
	void sendNodeInfo     ( CityPathNode const * node, Client * client );
	void sendNeighborInfo ( CityPathNode const * node, Client * client );
	void sendEraseNode    ( CityPathNode const * node, Client * client );
	void sendWaypointInfo ( AiLocation const & loc, Client * client );
	void sendEraseWaypoint( AiLocation const & loc, Client * client );

	// ----------

	void receiveMessage   ( MessageDispatch::Emitter const & source, MessageDispatch::MessageBase const & message );

	void watchObjectPath  ( Client * client, NetworkId const & object );
	void ignoreObjectPath ( Client * client, NetworkId const & object );

	void watchPathMap     ( Client * client );
	void ignorePathMap    ( Client * client );

	void onClientDestroy  ( ClientDestroy & d );

	// ----------

	typedef std::set<Client*> ClientList;

	ClientList * m_clientList;

	MessageDispatch::Callback * m_callback;
};

// ======================================================================

#endif

