// ======================================================================
//
// DatabaseProcess.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DatabaseProcess_H
#define INCLUDED_DatabaseProcess_H

// ======================================================================

#include <unordered_map>
#include <set>
#include <string>
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedMessageDispatch/Receiver.h"

class CentralServerConnection;
class ChatServerConnection;
class CommoditiesServerConnection;
class DatabaseMetricsData;
class GameNetworkMessage;
class GameServerConnection;
class Service;
class StringId;
class TaskManagerConnection;

namespace DB
{
	class Server;
}

// ======================================================================

class DatabaseProcess : public MessageDispatch::Receiver
{
  public:
	static DatabaseProcess &getInstance();
	
// running:
	virtual void run (void) = 0;
	void setDone(char const *reasonfmt, ...);

	virtual void frameTick(void) {}

// setters:
	void setCentralServerConnection(CentralServerConnection *_connection);
	void setProcessId(uint32 _processId);

// getters:
	GameServerConnection * getConnectionByProcess(const uint32 processId);
	void getGameServerProcessIds(std::vector<uint32> &processIds) const;
	uint32 getProcessId(void);
	DB::Server *getDBServer(void);
	const std::string &getClusterName() const;
	const std::string getSchemaQualifier() const;
	const std::string getGoldSchemaQualifier() const;
	const std::string getSchema() const;
	float getQueryExecRate() const;
	float getQueryFetchRate() const;

// communication:
	void connectToGameServer(const char *addres, uint16 port, uint32 processId);

	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void gameServerGoByeBye(uint32 processId);
	void sendToChatServer(GameNetworkMessage const &message);
	void sendToCentralServer(GameNetworkMessage const &message, bool reliable);
	void sendToCommoditiesServer(GameNetworkMessage const &message, bool reliable);
	void sendToAllGameServers(GameNetworkMessage const &message, bool reliable);
	void sendToGameServer(uint32 serverId, GameNetworkMessage const &message);
	void sendToAnyGameServer(GameNetworkMessage const &message);
	
  private:

	bool done;

	uint32 processId;
	std::string clusterName;
	CentralServerConnection *centralServerConnection;
	DB::Server *dbServer;

	Service *                     gameService;
	TaskManagerConnection *       taskService;
	ChatServerConnection *        chatServerConnection;
	Service *                     commoditiesService;
	CommoditiesServerConnection * commoditiesConnection;
	DatabaseMetricsData *         m_metricsData;
	
	std::unordered_map<uint32, GameServerConnection *>     gameServerConnections;
	std::set<std::pair<std::string, unsigned short> > pendingGameServerConnections;

	int m_queryExecCount;
	int m_queryFetchCount;

//creation	
  protected:
	static void installDerived(DatabaseProcess *derivedInstance);
	DatabaseProcess();
	virtual ~DatabaseProcess();
	
  private:
	void takeQueryCountSnapshot();
	
	static void remove();
	static DatabaseProcess *ms_theInstance;
};

// ======================================================================

inline DB::Server *DatabaseProcess::getDBServer(void)
{
	return dbServer;
}

// ----------------------------------------------------------------------

inline const std::string &DatabaseProcess::getClusterName() const
{
	return clusterName;
}

// ----------------------------------------------------------------------

inline DatabaseProcess &DatabaseProcess::getInstance()
{
	DEBUG_FATAL(!ms_theInstance,("DatabaseProcess was not installed.\n"));
	return *ms_theInstance;
}

// ======================================================================
#endif
