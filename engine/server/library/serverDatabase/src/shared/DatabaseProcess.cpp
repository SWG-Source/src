// ======================================================================
//
// DatabaseProcess.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"

#include "ConsoleManager.h"
#include "serverDatabase/CentralServerConnection.h"
#include "serverDatabase/ChatServerConnection.h"
#include "serverDatabase/CMLoader.h"
#include "serverDatabase/CommoditiesServerConnection.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseMetricsData.h"
#include "serverDatabase/DataLookup.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/LazyDeleter.h"
#include "serverDatabase/Loader.h"
#include "serverDatabase/Persister.h"
#include "serverDatabase/TaskManagerConnection.h"
#include "serverMetrics/MetricsManager.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/CentralPingMessage.h"
#include "serverNetworkMessages/ChatServerOnline.h"
#include "serverNetworkMessages/ExcommunicateGameServerMessage.h"
#include "serverNetworkMessages/GameServerConnectAck.h"
#include "serverNetworkMessages/GameTaskManagerMessages.h"
#include "serverNetworkMessages/ProfilerOperationMessage.h"
#include "serverNetworkMessages/TaskConnectionIdMessage.h"
#include "serverUtility/ServerClock.h"
#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetworkMessages/FrameEndMessage.h"
#include "UnicodeUtils.h"

// ----------------------------------------------------------------------

DatabaseProcess *DatabaseProcess::ms_theInstance = nullptr;

// ----------------------------------------------------------------------

namespace MemoryManagerNamespace
{
	extern int ms_allocateCalls;
}

//-----------------------------------------------------------------------

void DatabaseProcess::installDerived(DatabaseProcess *derivedInstance)
{
	DEBUG_FATAL(ms_theInstance,("Called DatabaseProcess::installDerived twice.\n"));
	ms_theInstance = derivedInstance;
}

// ----------------------------------------------------------------------

void DatabaseProcess::remove()
{
	DEBUG_FATAL(!ms_theInstance,("DatabaseProcess was not installed.\n"));
	delete ms_theInstance;
	ms_theInstance = 0;
}

// ----------------------------------------------------------------------

DatabaseProcess::DatabaseProcess() :
done(false),
processId(0),	
centralServerConnection(0),
dbServer(0),
chatServerConnection(0),
commoditiesConnection(0),
m_metricsData(0),
m_queryExecCount(0),
m_queryFetchCount(0)
{
	ExitChain::add(DatabaseProcess::remove,"DatabaseProcess::remove");

	centralServerConnection = new CentralServerConnection(ConfigServerDatabase::getCentralServerAddress(), ConfigServerDatabase::getCentralServerPort());
	NetworkSetupData setup;
	setup.maxConnections = 100;
	setup.port = 0;
	setup.bindInterface = ConfigServerDatabase::getGameServiceBindInterface();
	gameService = new Service(ConnectionAllocator<GameServerConnection>(), setup);

	NetworkSetupData cmSetup;
	cmSetup.maxConnections = 100;
	cmSetup.port = ConfigServerDatabase::getCommoditiesServerPort();
	cmSetup.bindInterface = ConfigServerDatabase::getCommoditiesServerAddress();
	commoditiesService = new Service(ConnectionAllocator<CommoditiesServerConnection>(), cmSetup);
	
	taskService = new TaskManagerConnection("127.0.0.1", ConfigServerDatabase::getTaskManagerPort());

	dbServer = DB::Server::create(ConfigServerDatabase::getDSN(),
								  ConfigServerDatabase::getDatabaseUID(),
								  ConfigServerDatabase::getDatabasePWD(),
								  DB::Server::getProtocolByName(ConfigServerDatabase::getDatabaseProtocol()),
								  ConfigServerDatabase::getUseMemoryManagerForOCI());
	
	if (ConfigServerDatabase::getEnableQueryProfile())
		DB::Server::enableProfiling();

	if (ConfigServerDatabase::getVerboseQueryMode())
		DB::Server::enableVerboseMode();

	DB::TaskQueue::enableWorkerThreadsLogging(ConfigServerDatabase::getLogWorkerThreads());

	if (ConfigServerDatabase::getPrefetchNumRows()!=0)
		DB::Server::enablePrefetch(ConfigServerDatabase::getPrefetchNumRows(), ConfigServerDatabase::getPrefetchMemory());
	
	DB::Server::setReconnectTime(ConfigServerDatabase::getDatabaseReconnectTime());
	DB::Server::setMaxErrorCountBeforeDisconnect(ConfigServerDatabase::getMaxErrorCountBeforeDisconnect());
	DB::Server::setMaxErrorCountBeforeBailout(ConfigServerDatabase::getMaxErrorCountBeforeBailout());
	DB::Server::setErrorSleepTime(ConfigServerDatabase::getErrorSleepTime());
	DB::Server::setDisconnectSleepTime(ConfigServerDatabase::getDisconnectSleepTime());
	DB::Server::setFatalOnDataError(ConfigServerDatabase::getFatalOnDataError());
	
    // the DatabaseProcess singleton will want to know when major, connection
	// oriented events occur. These events are defined in Game, Task and Central
	// connection classes.
	connectToMessage("CentralConnectionOpened");
	connectToMessage("GameConnectionOpened");
	connectToMessage("TaskConnectionOpened");
	connectToMessage("CommoditiesConnectionOpened");

	connectToMessage("CentralConnectionClosed");
	connectToMessage("GameConnectionClosed");
	connectToMessage("TaskConnectionClosed");
	connectToMessage("CommoditiesConnectionClosed");

	connectToMessage("ChatServerOnline");
	connectToMessage("ChatServerConnectionClosed");

	// other messages the DatabaseProcess singleton will want...
	connectToMessage("CentralGameServerSetProcessId");
	connectToMessage("LoadObjectMessage");
	connectToMessage("LoadUniverseMessage");
	connectToMessage("GameSetProcessId");
	connectToMessage("ProfilerOperationMessage");
	connectToMessage("FrameEndMessage");
	connectToMessage("CentralPingMessage");
	connectToMessage("ExcommunicateGameServerMessage");

	m_metricsData = new DatabaseMetricsData;
	MetricsManager::install(m_metricsData, false, "Database", "", 0);
	ConsoleManager::install();

	Profiler::setTemporaryExpandAll(ConfigServerDatabase::getProfilerExpandAll());
	Profiler::setDisplayPercentageMinimum(ConfigServerDatabase::getProfilerDisplayPercentageMinimum());
}

//-----------------------------------------------------------------------

DatabaseProcess::~DatabaseProcess()
{
	ConsoleManager::remove();
	MetricsManager::remove();
	
	delete m_metricsData;
	m_metricsData = 0;
	
	dbServer->disconnect();
	if (ConfigServerDatabase::getEnableQueryProfile())
	{
		DB::Server::debugOutputProfile();
		DB::Server::endProfiling();
	}

	delete dbServer;

	commoditiesConnection = 0;
	chatServerConnection = 0;
	centralServerConnection = 0;
	commoditiesService = 0;
	gameService = 0;
	taskService = 0;
	dbServer = 0;
}

//-----------------------------------------------------------------------

void DatabaseProcess::run(void)
{
	static bool shouldSleep = ConfigServerDatabase::getShouldSleep();
	bool idle=false;
	int loopcount=0;
	float nextMemoryReportTime=0;
	float nextQueryCountTime=0;

	LOG("ServerStartup",("DatabaseServer starting")); 
	
	while (!done)
	{
		PROFILER_AUTO_BLOCK_DEFINE("main loop");

		if (!Os::update())
			setDone("OS condition (Parent pid change)");

		real updateTime = Clock::frameTime();
		if (updateTime > ConfigServerDatabase::getReportLongFrameTime())
		{
			LOG("profile",("Long loop %f seconds:\n%s",updateTime,Profiler::getLastFrameData()));
			DEBUG_REPORT_LOG(true,("Long loop %f seconds:\n%s",updateTime,Profiler::getLastFrameData()));
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("Persister::update");
			Persister::getInstance().update(updateTime);
		}
		{
			PROFILER_AUTO_BLOCK_DEFINE("Loader::update");
			Loader::getInstance().update(updateTime);
		}
		{
			PROFILER_AUTO_BLOCK_DEFINE("CMLoader::update");
			CMLoader::getInstance().update();
		}
		{
			PROFILER_AUTO_BLOCK_DEFINE("DataLookup::update");
			DataLookup::getInstance().update(updateTime);
		}
		{
			PROFILER_AUTO_BLOCK_DEFINE("LazyDeleter::update");
			LazyDeleter::getInstance().update(updateTime);
		}
		{
			PROFILER_AUTO_BLOCK_DEFINE("NetworkHandler::update");
			NetworkHandler::update();
		}
		nextMemoryReportTime-=updateTime;
		if (nextMemoryReportTime < 0)
		{
#ifndef _WIN32
			LOG("DatabaseMemory",("Bytes used:  %lu (VmSize %dK)  Open allocations:  %d  Total Allocations %d",MemoryManager::getCurrentNumberOfBytesAllocated(static_cast<int>(Os::getProcessId())),MemoryManager::getProcessVmSizeKBytes(static_cast<int>(Os::getProcessId())),MemoryManager::getCurrentNumberOfAllocations(),MemoryManagerNamespace::ms_allocateCalls));
#else
			LOG("DatabaseMemory",("Bytes used:  %lu  Open allocations:  %d  Total Allocations %d",MemoryManager::getCurrentNumberOfBytesAllocated(static_cast<int>(Os::getProcessId())),MemoryManager::getCurrentNumberOfAllocations(),MemoryManagerNamespace::ms_allocateCalls));
#endif
			nextMemoryReportTime=10;
		}

		nextQueryCountTime-=updateTime;
		if (nextQueryCountTime < 0)
		{
			takeQueryCountSnapshot();
			nextQueryCountTime = 60.0f;
		}

		MetricsManager::update(updateTime*1000);
				
		// TODO: sleep longer if idle
		if (shouldSleep)
		{
			Os::sleep(1);
		}

		NetworkHandler::dispatch();

		Persister::getInstance().onFrameBarrierReached();			
		if (Persister::getInstance().isIdle() && Loader::getInstance().isIdle() && DataLookup::getInstance().isIdle())
		{
			DEBUG_REPORT_LOG(ConfigServerDatabase::getReportSaveTimes() && !idle,("Database process is idle.\n"));
			idle=true;
			if (taskService)
			{
				ServerIdleMessage msg(true);
				taskService->send(msg,true);
			}
		}
		else
		{
			idle=false;
			if (taskService)
			{
				ServerIdleMessage msg(false);
				taskService->send(msg,true);
			}
		}

		if (ConfigServerDatabase::getEnableQueryProfile() && ++loopcount>40)
		{
			loopcount=0;
			DB::Server::debugOutputProfile();
		}

		frameTick();  // virtual called 1x per frame

		NetworkHandler::clearBytesThisFrame();

		ServerClock::getInstance().incrementServerFrame();
	}
	
	// Give all of the task queues a chance to finish before we delete anything
	Persister::getInstance().shutdown();
	Loader::getInstance().shutdown();
	DataLookup::getInstance().shutdown();
}

// ----------------------------------------------------------------------

void DatabaseProcess::setDone(char const *reasonfmt, ...)
{
	if (!done)
	{
		char reason[1024];
		va_list ap;
		va_start(ap, reasonfmt);
		_vsnprintf(reason, sizeof(reason), reasonfmt, ap);
		reason[sizeof(reason)-1] = '\0';

		LOG(
			"ServerShutdown",
			(
				"DatabaseServer %d (pid %d) shutdown, reason: %s",
				static_cast<int>(getProcessId()),
				static_cast<int>(Os::getProcessId()),
				reason));

		REPORT_LOG(
			true,
			(
				"DatabaseServer %d (pid %d) shutdown, reason: %s\n",
				static_cast<int>(getProcessId()),
				static_cast<int>(Os::getProcessId()),
				reason));
		
		va_end(ap);
		done = true;
	}
}

// ----------------------------------------------------------------------

void DatabaseProcess::setCentralServerConnection(CentralServerConnection *_connection)
{
	centralServerConnection=_connection;
}

// ----------------------------------------------------------------------

void DatabaseProcess::setProcessId(uint32 _processId)
{
	processId=_processId;
}

// ----------------------------------------------------------------------

/** 
 * Establish a connection to a game server (if not already connected).
 */
void DatabaseProcess::connectToGameServer(const char *address, uint16 port, uint32 processId)
{
	DEBUG_REPORT_LOG(true,("connectToGameServer is still getting invoked.\n"));
	
	std::pair<std::string, unsigned short> a(std::make_pair(std::string(address), port));
	std::set<std::pair<std::string, unsigned short> >::const_iterator i = pendingGameServerConnections.find(a);
	if(i == pendingGameServerConnections.end())
	{		
		// check to see if there is already a connection active
		std::unordered_map<uint32, GameServerConnection *>::const_iterator j = gameServerConnections.find(processId);
		if(j == gameServerConnections.end())
		{
			// make the connection
			new GameServerConnection(std::string(address), port);
		}
	}
}

//-----------------------------------------------------------------------

GameServerConnection * DatabaseProcess::getConnectionByProcess(const uint32 processId)
{
	GameServerConnection * result = 0;
	std::unordered_map<uint32, GameServerConnection *>::const_iterator j = gameServerConnections.find(processId);
	if(j != gameServerConnections.end())
	{
		result = (*j).second;
	}
	return result;
}

// ----------------------------------------------------------------------

void DatabaseProcess::getGameServerProcessIds(std::vector<uint32> &processIds) const
{
	for (std::unordered_map<uint32, GameServerConnection *>::const_iterator i = gameServerConnections.begin(); i != gameServerConnections.end(); ++i)
		processIds.push_back((*i).first);
}

// ----------------------------------------------------------------------

uint32 DatabaseProcess::getProcessId(void)
{
	return processId;
}

//-----------------------------------------------------------------------

void DatabaseProcess::gameServerGoByeBye(uint32 processId)
{
	DataLookup::getInstance().releaseNamesForProcess(processId);
	gameServerConnections.erase(processId);
	Loader::getInstance().discardPendingLoadsForServer(processId);
}

// ----------------------------------------------------------------------

void DatabaseProcess::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);

	//---
	// Connection info messages
	//
	if(message.isType("CentralConnectionOpened"))
	{
		DEBUG_REPORT_LOG(true, ("Connection with Central server opened\n"));
		DEBUG_FATAL(centralServerConnection != &source,("Got CentralConnectionOpened from something other than our CentralServer connection.\n"));
		
		//TODO:  Make a DatabaseProcessConnect version of this message, perhaps ?
		CentralGameServerConnect c("database", "127.0.0.1", 0, gameService->getBindAddress(), gameService->getBindPort()); 
		centralServerConnection->send(c, true);

	}
	else if(message.isType("GameConnectionOpened"))
	{
		DEBUG_REPORT_LOG(true, ("Connection with Game server opened\n"));
	}
	else if(message.isType("TaskConnectionOpened"))
	{
		DEBUG_REPORT_LOG(true, ("Connection with Task manager opened\n"));

		// get cluster name
		std::string clusterName;
		ConfigFile::Section const * const sec = ConfigFile::getSection("TaskManager");
		if (sec)
		{
			ConfigFile::Key const * const ky = sec->findKey("clusterName");
			if (ky)
			{
				clusterName = ky->getAsString(ky->getCount()-1, "");
			}
		}

		TaskConnectionIdMessage tid(TaskConnectionIdMessage::Database, "", clusterName);
		taskService->send(tid, true);
	}
	else if(message.isType("CommoditiesConnectionOpened"))
	{
		DEBUG_REPORT_LOG(true, ("Connection with Commodities Server opened\n"));
		CommoditiesServerConnection * con = const_cast<CommoditiesServerConnection *>(static_cast<const CommoditiesServerConnection *>(&source));
		if (commoditiesConnection)
			LOG("CommoditiesConnectionOpened",("Connection Opened while another connection already exists."));
		commoditiesConnection = con;
	}
	else if(message.isType("CentralConnectionClosed"))
	{
		DEBUG_REPORT_LOG(true, ("Central server closed its connection, we must die a tragicly heroic death.\n"));
		setDone("CentralConnectionClosed : %s", centralServerConnection->getDisconnectReason().c_str());
		centralServerConnection = 0;
		WARNING(true,("Database lost connection to central."));
	}
	else if(message.isType("GameConnectionClosed"))
	{
		DEBUG_REPORT_LOG(true, ("a Game server closed its connection\n"));
	}
	else if(message.isType("TaskConnectionClosed"))
	{
		DEBUG_REPORT_LOG(true, ("Task manager closed its connection\n"));
		taskService=0;
	}
	else if(message.isType("CommoditiesConnectionClosed"))
	{
		DEBUG_REPORT_LOG(true, ("Commodities Server closed its connection\n"));
		commoditiesService = 0;
		commoditiesConnection = 0;
	}
	// end connection info messages
	//---

	else if(message.isType("CentralGameServerSetProcessId"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		CentralGameServerSetProcessId id(ri);
		setProcessId(id.getProcessId());
		clusterName = id.getClusterName();

		Loader::getInstance().checkVersionNumber(ConfigServerDatabase::getExpectedDBVersion(), ConfigServerDatabase::isCorrectDBVersionRequired());
		Loader::getInstance().loadClock();
	}
	else if(message.isType("GameSetProcessId") || message.isType("GameGameServerConnect"))
	{
		GameServerConnection * g = const_cast<GameServerConnection *>(static_cast<const GameServerConnection *>(&source));
		gameServerConnections[g->getProcessId()] = g;

		GameServerConnectAck reply;
		g->send(reply,true);
	}
	else if(message.isType("ProfilerOperationMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ProfilerOperationMessage m(ri);
		uint32 processId = m.getProcessId();
		if (processId == 0 || processId == getProcessId())
		Profiler::handleOperation(m.getOperation().c_str());
	}
	else if(message.isType("ChatServerOnline"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ChatServerOnline cso(ri);

		// establish a connection with the chat server if one doesn't already exist
		if (!chatServerConnection)
		{
			REPORT_LOG(true, ("New chat server connection active\n"));
			chatServerConnection = new ChatServerConnection(cso.getAddress(), cso.getPort());
		}
	}
	else if(message.isType("ChatServerConnectionClosed"))
	{
		REPORT_LOG(true, ("DatabaseProcess: Chat Server connection closed\n"));
		chatServerConnection = 0;
	}
	else if(message.isType("FrameEndMessage"))
	{
		// TODO: handle this game server's frame ending
	}
	else if (message.isType("CentralPingMessage"))
	{
		CentralPingMessage reply;
		sendToCentralServer(reply,true);
	}
	else if (message.isType("ExcommunicateGameServerMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ExcommunicateGameServerMessage msg(ri);

		LOG("GameGameConnect",("Database Process was told to drop connection to %lu by Central",msg.getServerId()));
		
		FATAL (msg.getServerId() == getProcessId(),("Crashing because Central told us to (probably indicates we weren't responding to pings)"));
		GameServerConnection *conn =getConnectionByProcess(msg.getServerId());
		if (conn)
		{
			conn->setDisconnectReason("Told to excommunicate by central");
			conn->disconnect();
		}
	}		
}

// ----------------------------------------------------------------------

const std::string DatabaseProcess::getSchemaQualifier() const
{
	if (ConfigServerDatabase::getSchemaOwner()[0]!='\0')
		return std::string(ConfigServerDatabase::getSchemaOwner())+'.';
	else
		return std::string();
}

// ----------------------------------------------------------------------

const std::string DatabaseProcess::getGoldSchemaQualifier() const
{
	static std::string result = std::string(ConfigServerDatabase::getGoldSchemaOwner()) + '.';
	return result;
}

// ----------------------------------------------------------------------

const std::string DatabaseProcess::getSchema() const
{
	if (ConfigServerDatabase::getSchemaOwner()[0]!='\0')
		return std::string(Unicode::toUpper(ConfigServerDatabase::getSchemaOwner()));
	else
		return std::string(Unicode::toUpper(ConfigServerDatabase::getDatabaseUID()));
}

// ----------------------------------------------------------------------

void DatabaseProcess::sendToChatServer(GameNetworkMessage const &message)
{
	if (chatServerConnection)
		chatServerConnection->send(message, true);
}

// ----------------------------------------------------------------------

void DatabaseProcess::sendToCentralServer(GameNetworkMessage const &message, bool reliable)
{
	if (centralServerConnection)
		centralServerConnection->send(message,reliable);
}

// ----------------------------------------------------------------------

void DatabaseProcess::sendToCommoditiesServer(GameNetworkMessage const &message, bool reliable)
{
	if (commoditiesConnection)
	{
		commoditiesConnection->send(message,reliable);
	}
	else
		DEBUG_REPORT_LOG(true, ("commoditiesConnection is nullptr\n"));
}

// ----------------------------------------------------------------------

void DatabaseProcess::sendToAllGameServers(GameNetworkMessage const &message, bool reliable)
{
	for (std::unordered_map<uint32, GameServerConnection *>::const_iterator i=gameServerConnections.begin(); i!=gameServerConnections.end(); ++i)
		i->second->send(message,reliable);
}

// ----------------------------------------------------------------------

void DatabaseProcess::sendToGameServer(uint32 serverId, GameNetworkMessage const &message)
{
	std::unordered_map<uint32, GameServerConnection *>::const_iterator i=gameServerConnections.find(serverId);
	if (i!=gameServerConnections.end())
		i->second->send(message,true);
	else
		DEBUG_WARNING(true,("Tried to send to game server %lu without connection.\n",serverId));
}

// ----------------------------------------------------------------------

void DatabaseProcess::sendToAnyGameServer(GameNetworkMessage const &message)
{
	if (!gameServerConnections.empty())
		gameServerConnections.begin()->second->send(message,true);
}
// ----------------------------------------------------------------------

float DatabaseProcess::getQueryExecRate() const
{
	return ((static_cast<float>(m_queryExecCount) / 60.0f) * static_cast<float>(ConfigServerDatabase::getQueryReportingRate()));
}

// ----------------------------------------------------------------------

float DatabaseProcess::getQueryFetchRate() const
{
	return ((static_cast<float>(m_queryFetchCount) / 60.0f) * static_cast<float>(ConfigServerDatabase::getQueryReportingRate()));
}

// ----------------------------------------------------------------------

/**
 * Record the current rate of queries and fetches for the metrics system
 * (averages over 1 minute, because the instantaneous rate would fluctuate too much)
 */
void DatabaseProcess::takeQueryCountSnapshot()
{
	dbServer->getAndResetQueryCount(m_queryExecCount, m_queryFetchCount);
}

// ======================================================================
