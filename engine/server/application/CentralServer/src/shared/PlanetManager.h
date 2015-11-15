// ======================================================================
//
// PlanetManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlanetManager_H
#define INCLUDED_PlanetManager_H

// ======================================================================

//TODO:  forward declare map & string?  May not be worth it because this header file
// is not widely included
#include <map>
#include <string>

#include "Singleton/Singleton.h"
#include "sharedMessageDispatch/Receiver.h"

// ----------------------------------------------------------------------

class PlanetServerConnection;
class GameServerConnection;

// ======================================================================

/**
 * The PlanetManager is a singleton that manages information about
 * all the planets in the game universe.
 *
 * It keeps a map of scene ID's to Planet Server connections.  It also
 * keeps track of who is authoritative for the PlanetObjects for each
 * planet.
 */

class PlanetManager : public MessageDispatch::Receiver
{
public:
	PlanetManager();

public:
	static void addServer(const std::string &sceneId, PlanetServerConnection *connection);
	static void addGameServerForScene(const std::string &sceneId, GameServerConnection *gameServer);
	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	static void onGameServerDisconnect(const GameServerConnection *gameServer);
	static PlanetServerConnection *getPlanetServerForScene(const std::string &sceneId);

private:
	struct PlanetRec
	{
		PlanetServerConnection *m_connection;
		NetworkId m_planetObjectId;

		PlanetRec();
	};

	static PlanetManager & instance();

private:
	typedef std::map<std::string, PlanetRec> ServerListType;
	typedef std::vector<std::pair<std::string, GameServerConnection *> > PendingGameServersType;
	PendingGameServersType m_pendingGameServers;
	ServerListType m_servers;
};

// ======================================================================

#endif
