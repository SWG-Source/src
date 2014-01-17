// TaskManager.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskManager_H
#define	_INCLUDED_TaskManager_H

#include <map>
#include <string>
#include <set>
#include <vector>

//-----------------------------------------------------------------------

class Connection;
class ConsoleConnection;
class GameNetworkMessage;
class LoginConnection;
class Service;
class ManagerConnection;
class TaskConnection;
class TaskKillProcess;
class TaskManagerSysInfo;

//-----------------------------------------------------------------------

struct ProcessKilled
{
	std::string commandLine;
	std::string hostName;
};

//-----------------------------------------------------------------------

struct ProcessAborted
{
	std::string commandLine;
	std::string hostName;
};

//-----------------------------------------------------------------------

struct ProcessStarted
{
	unsigned long pid;
	std::string commandLine;
	std::string hostName;
};

//-----------------------------------------------------------------------

struct RequestProcessKill
{
	std::string commandLine;
	std::string hostName;
	unsigned long pid;
};

//-----------------------------------------------------------------------

struct RequestProcessStart
{
	std::string commandLine;
	std::string hostName;
};

//-----------------------------------------------------------------------

struct ProcessEntry
{
	std::string processName;
	std::string targetHost;
	std::string executable;
	std::string options;
};

//-----------------------------------------------------------------------

class TaskManager
{
public:
	typedef unsigned int       SpawnDelaySeconds;

	TaskManager();
	~TaskManager();
	static std::string         executeCommand        (const std::string & command);
	static const float         getLoadAverage        ();
	static const std::string & getNodeLabel();
	static time_t              getStartTime();
	static void                killProcess           (const TaskKillProcess &);
	static void                retryConnection       (ManagerConnection const *connection);
	static void                sendToCentralServer   (const GameNetworkMessage & msg);
	static void                setCentralConnection  (TaskConnection * connection);
	static void                onDatabaseIdle        (bool isIdle);
	static void                onPreloadFinished     ();
	static void                runSpawnRequestQueue  ();
	static unsigned long       startServer           (const std::string & processName, const std::string & options, const std::string& hostName, SpawnDelaySeconds spawnDelay);
	static void                stopCluster           ();
	static void                resendUnacknowledgedSpawnRequests  (Connection * connection, const std::string & nodeLabel);
	static void                removePendingSpawnProcessAck       (int transactionId);

	static void           install ();
	static void           remove  ();
	static void           run     ();
	static void           update  ();

	static const std::set<std::pair< std::string, unsigned long> > & getLocalServers();
	static const std::map<std::string, std::pair<std::string, unsigned long> > & getRemoteServers();

	static void  addToGameConnections(int x);
	static int   getNumGameConnections();
	
private:
	TaskManager & operator = (const TaskManager & rhs);
	TaskManager(const TaskManager & source);
	static TaskManager & instance();

	void                 processRcFile();
	void                 processEnvironmentVariables();
	void                 setupNodeList();
	static unsigned long startServerLocal(const ProcessEntry & processEntry, const std::string & options);
	static void          startCluster();
	
private:

	struct NodeEntry
	{

		NodeEntry(const std::string &, const std::string & , int);
		bool operator==(const NodeEntry &) const;
		bool operator==(const std::string & address) const;
		
		std::string   m_address;
		std::string   m_nodeLabel;
		int           m_nodeNumber;
		
	};
	
	std::map<std::string, ProcessEntry>                           m_processEntries;
	std::set<std::pair<std::string, unsigned long> >              m_localServers;
	std::map<std::string, std::pair<std::string, unsigned long> > m_remoteServers;
	std::string                                                   m_nodeLabel;
	time_t const                                                  m_startTime;
	std::vector<NodeEntry>                                        m_nodeList;
	int                                                           m_nodeNumber;
	std::vector<NodeEntry>                                        m_nodeToConnectToList;

	Service *               m_managerService;
	Service *               m_taskService;
	Service *               m_consoleService;
	TaskManagerSysInfo *    m_sysInfoSource;
	TaskConnection *        m_centralConnection;
	
};

//-----------------------------------------------------------------------

inline const std::set<std::pair<std::string, unsigned long> > & TaskManager::getLocalServers()
{
	return instance().m_localServers;
}

//-----------------------------------------------------------------------

inline const std::string& TaskManager::getNodeLabel()
{
	return instance().m_nodeLabel;
}

//-----------------------------------------------------------------------

inline time_t TaskManager::getStartTime()
{
	return instance().m_startTime;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskManager_H
