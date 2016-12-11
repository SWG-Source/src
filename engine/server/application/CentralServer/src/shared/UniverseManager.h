// ======================================================================
//
// UniverseManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UniverseManager_H
#define INCLUDED_UniverseManager_H

#include "Singleton/Singleton.h"
#include "sharedMessageDispatch/Receiver.h"

// ======================================================================

/**
 * A singleton that manages the Universe Objects on the cluster.
 */
class UniverseManager : public Singleton <UniverseManager>, public MessageDispatch::Receiver
{
public:
	UniverseManager();
	~UniverseManager();

	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void onGameServerDisconnect(const GameServerConnection &gameServer);
	void onGameServerReady(const GameServerConnection &gameServer);

	uint32 getUniverseProcess() const;

private:
	UniverseManager(UniverseManager const &);
	UniverseManager &operator=(UniverseManager const &);

	void onDatabaseProcessConnect(const GameServerConnection &databaseProcess);
	void sendUniverseToServer(uint32 gameServerId);

private:
	typedef std::set<uint32> ServersLoadingUniverseType;

	uint32 m_universeProcess;  ///< Process that owns the universe objects
	bool m_databaseAuthoritative; ///< True if database is authoritative for the universe objects
	ServersLoadingUniverseType *m_serversLoadingUniverse;
};


inline uint32 UniverseManager::getUniverseProcess() const
{
	return m_universeProcess;
}

// ======================================================================

#endif
