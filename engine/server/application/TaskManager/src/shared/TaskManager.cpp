// TaskManager.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------
#include "FirstTaskManager.h"

#include "CentralConnection.h"
#include "ConfigTaskManager.h"
#include "Console.h"
#include "ConsoleConnection.h"
#include "GameConnection.h"
#include "Locator.h"
#include "ManagerConnection.h"
#include "ProcessSpawner.h"
#include "serverNetworkMessages/SetConnectionServerPublic.h"
#include "serverNetworkMessages/TaskKillProcess.h"
#include "serverNetworkMessages/TaskProcessDiedMessage.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "fileInterface/StdioFile.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedLog/Log.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/Address.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "EnvironmentVariable.h"
#include "TaskConnection.h"
#include "TaskManager.h"
#include "TaskManagerSysInfo.h"

namespace TaskManagerNameSpace
{
	std::vector<ProcessEntry>     ms_deferredSpawns;

	int ms_numGameConnections = 0;
	bool ms_doUpdate=false;
	TaskManager * gs_instance = 0;

	int ms_idleFrames = 0;
	bool ms_preloadFinished = false;
	bool ms_done = false;

	std::map<std::string, TaskManager::SpawnDelaySeconds> s_processRestartDelayInformation;

	void initializeRestartDelayInformation()
	{
		s_processRestartDelayInformation["CentralServer"]     = ConfigTaskManager::getRestartDelayCentralServer();
		s_processRestartDelayInformation["LogServer"]         = ConfigTaskManager::getRestartDelayLogServer();
		s_processRestartDelayInformation["MetricsServer"]     = ConfigTaskManager::getRestartDelayMetricsServer();
		s_processRestartDelayInformation["CommoditiesServer"] = ConfigTaskManager::getRestartDelayCommoditiesServer();
		s_processRestartDelayInformation["CommodityServer"]   = ConfigTaskManager::getRestartDelayCommoditiesServer();
		s_processRestartDelayInformation["TransferServer"]    = ConfigTaskManager::getRestartDelayTransferServer();
	}

	TaskManager::SpawnDelaySeconds getRestartDelayInformation(const std::string & name)
	{
		TaskManager::SpawnDelaySeconds result = 0;

		for (std::map<std::string, TaskManager::SpawnDelaySeconds>::const_iterator i = s_processRestartDelayInformation.begin(); i != s_processRestartDelayInformation.end(); ++i)
		{
			if (name.find(i->first) != std::string::npos)
			{
				result = i->second;
				break;
			}
		}

		return result;
	}

	std::map<std::string, float> s_processLoadInformation;

	void initializeLoadInformation()
	{
		s_processLoadInformation["ConnectionServer"] = ConfigTaskManager::getLoadConnectionServer();
		s_processLoadInformation["PlanetServer"]     = ConfigTaskManager::getLoadPlanetServer();
		s_processLoadInformation["SwgGameServer"]    = ConfigTaskManager::getLoadGameServer();
	}

	float getLoadForProcess(const std::string & name)
	{
		float result = 0.0f;

		for (std::map<std::string, float>::const_iterator i = s_processLoadInformation.begin(); i != s_processLoadInformation.end(); ++i)
		{
			if (name.find(i->first) != std::string::npos)
			{
				result = i->second;
				break;
			}
		}

		return result;
	}

	struct RestartRequest
	{
		std::string commandLine;
		unsigned long timeQueued;
		TaskManager::SpawnDelaySeconds spawnDelay;
	};
	std::vector<RestartRequest> s_restartRequests;

	struct QueuedSpawnRequest
	{
		std::string processName;
		std::string options;
		std::string nodeLabel;
		unsigned long timeQueued;
		TaskManager::SpawnDelaySeconds spawnDelay;
	};

	std::vector<QueuedSpawnRequest> s_queuedSpawnRequests;
	std::vector<QueuedSpawnRequest> s_delayedSpawnRequests;
	struct OutstandingSpawnRequestAck
	{
		OutstandingSpawnRequestAck(const std::string &n, const Archive::ByteStream & a, int t) :
		nodeLabel(n), request(a), transactionId(t)
		{
		}
		std::string nodeLabel;
		Archive::ByteStream request;
		int transactionId;
	};
	
	std::vector<OutstandingSpawnRequestAck>  s_outstandingSpawnRequestAcks;
}

using namespace TaskManagerNameSpace;
//-----------------------------------------------------------------------

TaskManager::NodeEntry::NodeEntry(const std::string & addr, const std::string & label, int index) :
		m_address(addr),
		m_nodeLabel(label),
		m_nodeNumber(index)
{
}

//-----------------------------------------------------------------------

bool TaskManager::NodeEntry::operator==(const NodeEntry & rhs) const
{
	return (m_nodeNumber == rhs.m_nodeNumber);
}

//-----------------------------------------------------------------------

bool TaskManager::NodeEntry::operator==(const std::string & address) const
{
	return (m_address == address);
}

//-----------------------------------------------------------------------

TaskManager::TaskManager() :
m_processEntries(),
m_localServers(),
m_remoteServers(),
m_nodeLabel(),
m_startTime(::time(nullptr)),
m_nodeList(),
m_nodeNumber(-1),
m_nodeToConnectToList(),
m_managerService(0),
m_taskService(0),
m_sysInfoSource(new TaskManagerSysInfo),
m_centralConnection(0)
{
	processRcFile();
	processEnvironmentVariables();
	setupNodeList();
}

//-----------------------------------------------------------------------

TaskManager::~TaskManager()
{
	delete m_sysInfoSource;
}

//-----------------------------------------------------------------------

void TaskManager::processEnvironmentVariables()
{
	int const numberOfEnvironmentVariables = ConfigTaskManager::getNumberOfEnvironmentVariables();

	for (int i = 0; i < numberOfEnvironmentVariables; ++i)
	{
		char const * const p = ConfigTaskManager::getEnvironmentVariable(i);
		if(p)
		{
			const char* plus = strchr(p, '+');
			const char* value = strchr(p, '=');
			WARNING_STRICT_FATAL(!value, ("Could not find an equal sign (=) in environment setting %s", p));
			WARNING_STRICT_FATAL(plus && value != plus + 1, ("Mismatched += sign in %s.", p));
#ifdef _DEBUG
			const char* dollar = strchr(p, '$');
			DEBUG_FATAL(dollar, ("We don't support using variables (with $) within environment variables yet %s", p));
#endif
			std::string key(p, (plus ? plus : value) - p);
			++value;
			if (plus)
			{
				if (!EnvironmentVariable::addToEnvironmentVariable(key.c_str(), value))
				{
					WARNING(true, ("Failed to add environment variable %s", p));
				}
			}
			else
			{
				if (!EnvironmentVariable::setEnvironmentVariable(key.c_str(), value))
				{
					WARNING(true, ("Failed to set environment variable %s", p));
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

void TaskManager::install()
{
	gs_instance = new TaskManager();
}

//-----------------------------------------------------------------------

void TaskManager::remove()
{
	delete gs_instance;
}

//-----------------------------------------------------------------------

void TaskManager::processRcFile()
{
	AbstractFile * file = new StdioFile(ConfigTaskManager::getRcFileName(), "r");
	if(file->isOpen())
	{
		// read spawn directives
		int fileLength = file->length();
		if(fileLength > 0)
		{
			char * buffer = new char[fileLength + 1]; //lint !e737 Loss of sign in promotion from int to unsigned long
			IGNORE_RETURN(memset(buffer, 0, fileLength + 1)); //lint !e732 loss of sign in promotion from int to unsigned long
			int readResult = file->read(buffer, fileLength);
			if(readResult > 0)
			{
				// parse the file
				size_t start = 0;
				std::string rcData = buffer;
				while(start < rcData.size())
				{
					size_t recordEnd = rcData.find_first_of('\n', start);
					if(recordEnd < std::string::npos)
					{
						std::string record = rcData.substr(start, recordEnd - start);
						if(record.length() > 0)
						{
							size_t firstChar = record.find_first_not_of(" \t");
							if(firstChar < std::string::npos)
							{
								if(record[firstChar] == '#')
								{
									start = recordEnd + 1;
									continue;
								}
							}
							ProcessEntry pe;

							// get server name
							size_t serverNameEndPos = record.find_first_of(' ', firstChar);
							if(serverNameEndPos < std::string::npos)
							{
								pe.processName = record.substr(firstChar, serverNameEndPos - firstChar);

								// get target host directive
								firstChar = record.find_first_not_of(' ', serverNameEndPos);
								if(firstChar < std::string::npos)
								{
									size_t hostEndPos = record.find_first_of(' ', firstChar);
									if(hostEndPos < std::string::npos)
									{
										pe.targetHost = record.substr(firstChar, hostEndPos - firstChar);
										if(pe.targetHost != "any" && pe.targetHost != "local")
										{
											Address a(pe.targetHost, 0);
											pe.targetHost = a.getHostAddress();
										}

										// get executable name
										firstChar = record.find_first_not_of(' ', hostEndPos);
										if(firstChar < std::string::npos)
										{
											size_t executableEndPos = record.find_first_of(' ', firstChar);
											if(executableEndPos < std::string::npos)
											{
												pe.executable = record.substr(firstChar, executableEndPos - firstChar);

												// get options
												firstChar = record.find_first_not_of(' ', executableEndPos);
												if(firstChar < std::string::npos)
												{
													pe.options = record.substr(firstChar);

													IGNORE_RETURN(m_processEntries.insert(std::make_pair(pe.processName, pe)));
												}
												else
												{
													REPORT_LOG(true, ("Could not parse taskmanager.rc entry [%s]\n", record.c_str()));
												}
											}
											else
											{
												REPORT_LOG(true, ("Could not parse taskmanager.rc entry [%s]\n", record.c_str()));
											}
										}
										else
										{
											REPORT_LOG(true, ("Could not parse taskmanager.rc entry [%s]\n", record.c_str()));
										}
									}
									else
									{
										REPORT_LOG(true, ("Could not parse taskmanager.rc entry [%s]\n", record.c_str()));
									}
								}
								else
								{
									REPORT_LOG(true, ("Could not parse taskmanager.rc entry [%s]\n", record.c_str()));
								}
							}
							else
							{
								REPORT_LOG(true, ("Could not parse taskmanager.rc ent-ry [%s]\n", record.c_str()));
							}
//									m_localServers.insert(std::pair<std::string, unsigned long>(record,0));
//								IGNORE_RETURN(loadOnStart.insert(record));
						}
					}
					else
					{
						break;
					}
					start = recordEnd + 1;
				}
			}
			delete [] buffer;
		}
		file->close();
		delete file;
	}
}

//-----------------------------------------------------------------------

void TaskManager::setupNodeList()
{
	char buffer[64];
	const char* result = nullptr;
	int nodeIndex = 0;
	bool found = true;

	const std::string & localAddress = NetworkHandler::getHostName();
	const std::string localName = NetworkHandler::getHumanReadableHostName();
	
	m_nodeNumber = -1;

	do
	{
		found = false;
		sprintf(buffer, "node%d", nodeIndex);
		result = ConfigFile::getKeyString("TaskManager", buffer, nullptr);
		if (result)
		{
			NodeEntry n(result, buffer, nodeIndex);
			m_nodeList.push_back(n);;
			found = true;
			if (result == localAddress || result == localName)
			{
				m_nodeNumber = nodeIndex;
				m_nodeLabel = buffer;
			}
			++nodeIndex;
		}

	} while(found);

	if (m_nodeList.empty())
	{
		m_nodeLabel = "node0";
		m_nodeNumber = 0;
	}

	if (m_nodeNumber != -1)
	{
		DEBUG_REPORT_LOG(true, ("This taskmanager is node %s\n", m_nodeLabel.c_str()));
	}
	else
	{
		WARNING(true, ("Could not find node for this host: %s.", localAddress.c_str()));
	}

	std::vector<NodeEntry>::iterator i = m_nodeList.begin();
	for (; i != m_nodeList.end(); ++i)
	{
		if (m_nodeNumber > i->m_nodeNumber)
		{
			DEBUG_REPORT_LOG(true, ("Adding node %d to attempt list\n", i->m_nodeNumber));
			m_nodeToConnectToList.push_back(*i);
		}
	}
}

//-----------------------------------------------------------------------

std::string TaskManager::executeCommand(const std::string & command)
{
	std::string result = "unknown command";

	if(command == "start")
	{
		startCluster();
		result = "start command issued and handled by the TaskManager";
	}
	else if(command == "stop")
	{
		stopCluster();
		result = "stop command issued and handled by the TaskManager";
	}
	else if(command == "public")
	{
		SetConnectionServerPublic p(true);
		sendToCentralServer(p);
		result = "public command issued and handled by the TaskManager";
	}
	else if(command == "private")
	{
		SetConnectionServerPublic p(false);
		sendToCentralServer(p);
		result = "private command issued and handled by the TaskManager";
	}
	else if(command == "taskConnectionCount")
	{
		char countBuf[32];
		IGNORE_RETURN(snprintf(countBuf, sizeof(countBuf)-1, "%d", ManagerConnection::getConnectionCount()));
		countBuf[sizeof(countBuf)-1] = '\0';
		result = countBuf;
	}
	else if(command == "exit")
	{
		stopCluster();
		ms_done = true;
		result = "exiting";
	}
	else if(command == "runState")
	{
		result = "running";
	}
	LOG("TaskManager",("Execute command: %s.", result.c_str()));
	return result;
}

//-----------------------------------------------------------------------

const float TaskManager::getLoadAverage()
{
	return Locator::getMyLoad();
}

//-----------------------------------------------------------------------

TaskManager & TaskManager::instance()
{
	NOT_NULL(gs_instance);
	return *gs_instance;
}

//-----------------------------------------------------------------------

void TaskManager::killProcess(const TaskKillProcess & killProcessMessage)
{
	if(NetworkHandler::isAddressLocal(killProcessMessage.getHostName()) || killProcessMessage.getHostName() == instance().m_nodeLabel)
	{
		// local destroy find process by pid

		std::set<std::pair<std::string, unsigned long> >::iterator i = instance().m_localServers.begin();
		while (i != instance().m_localServers.end())
		{
			if(i->second == killProcessMessage.getPid())
			{
				// confirmed kill
				if(killProcessMessage.getForceCore())
					ProcessSpawner::forceCore(killProcessMessage.getPid());
				else
					ProcessSpawner::kill(killProcessMessage.getPid());

				// emit kill notification message
				static MessageDispatch::Transceiver<const ProcessKilled &> t;
				static ProcessKilled k;
				k.commandLine = (*i).first;
				k.hostName = killProcessMessage.getHostName();
				t.emitMessage(k);
				instance().m_localServers.erase(i++);
				Locator::decrementMyLoad(getLoadForProcess(k.commandLine));
			}
			else
				++i;
		}
	}
}

//-----------------------------------------------------------------------

void TaskManager::run()
{
	NetworkHandler::install();
	NetworkSetupData setup;
	setup.port = ConfigTaskManager::getGameServicePort();
	setup.maxConnections = 1000;
	setup.bindInterface = ConfigTaskManager::getGameServiceBindInterface();
	instance().m_taskService = new Service(ConnectionAllocator<TaskConnection>(), setup);
	setup.port = ConfigTaskManager::getTaskManagerServicePort();
	setup.bindInterface = ConfigTaskManager::getTaskManagerServiceBindInterface();
	instance().m_managerService = new Service(ConnectionAllocator<ManagerConnection>(), setup);
	setup.maxConnections = 32;
	setup.port = ConfigTaskManager::getConsoleConnectionPort();
	setup.bindInterface = ConfigTaskManager::getConsoleServiceBindInterface();
	instance().m_consoleService = new Service(ConnectionAllocator<ConsoleConnection>(), setup);

	SetupSharedLog::install("TaskManager");
	Locator::install();
	initializeRestartDelayInformation();
	initializeLoadInformation();

	if(ConfigTaskManager::getAutoStart())
		startCluster();

	while (!ms_done)
	{
		TaskManager::update();
	}

	NetworkHandler::update();
	NetworkHandler::dispatch();
	
	SetupSharedLog::remove();
	NetworkHandler::remove();
}

//-----------------------------------------------------------------------

unsigned long TaskManager::startServerLocal(const ProcessEntry & pe, const std::string & options)
{
	unsigned long pid = 0;
	// build the command
	std::string cmd = pe.executable + " " + pe.options + " " + options;
	DEBUG_REPORT_LOG(true, ("Now Spawning process: %s\n", cmd.c_str()));
	LOG("TaskManager", ("Now spawning process: (%s). I am %s (%s). My Load is %f.", cmd.c_str(), NetworkHandler::getHumanReadableHostName().c_str(), NetworkHandler::getHostName().c_str(), Locator::getMyLoad()));
	if( (pid = ProcessSpawner::execute(cmd)) > 0)
	{
		Locator::incrementMyLoad(getLoadForProcess(pe.processName));
		IGNORE_RETURN(instance().m_localServers.insert(std::make_pair(cmd, pid)));
		static MessageDispatch::Transceiver<const ProcessStarted &> p;
		static ProcessStarted s;
		s.pid = pid;
		s.hostName = NetworkHandler::getHostName();
		s.commandLine = cmd;
		p.emitMessage(s);
	}
	return pid;
}

//-----------------------------------------------------------------------
/** Start a new process

	Processes are read from the taskmanager.rc file. A remote process
	or the local task manager requests a process startup. The process name
	must match one in the taskmanager.rc file. The rc contains default
	options for the local system. Options passed in the request are
	appended to the command line and duplicates will override the
	options specified in the rc file.
*/
unsigned long TaskManager::startServer(const std::string & processName, const std::string & options, const std::string& nodeLabel, const SpawnDelaySeconds spawnDelay)
{
	unsigned long pid = 0;
	static const std::string localAddress(NetworkHandler::getHostName());

	DEBUG_REPORT_LOG(true, ("Now trying to start server %s %s on %s, spawn delay %u\n", processName.c_str(), options.c_str(), nodeLabel.c_str(), spawnDelay));
	DEBUG_REPORT_LOG(true, ("Local address is %s and i am node %s\n", localAddress.c_str(), getNodeLabel().c_str()));

	// get process
	std::map<std::string, ProcessEntry>::const_iterator f = instance().m_processEntries.find(processName);
	if(f != instance().m_processEntries.end())
	{
		// if there's a spawn delay, queue the spawn request
		if (spawnDelay > 0)
		{
			QueuedSpawnRequest r;
			r.processName = processName;
			r.options = options;
			r.nodeLabel = nodeLabel;
			r.timeQueued = Clock::timeSeconds();
			r.spawnDelay = spawnDelay;
			s_delayedSpawnRequests.push_back(r);

			return 0;
		}

		const ProcessEntry pe = (*f).second;

		// does the process run on this box?
		if(pe.targetHost == "local" || pe.targetHost == getNodeLabel() || nodeLabel == getNodeLabel() || nodeLabel == "local")
		{
			if(processName == "SwgGameServer" && Locator::getMyLoad() + getLoadForProcess(processName) >= Locator::getMyMaximumLoad() && ManagerConnection::getConnectionCount() > 1)
			{
				ManagerConnection * conn = Locator::getServer("node0");
				if(conn)
				{
					WARNING(true, ("Tried to spawn a SwgGameServer on this host, but would exceed spawn limit, sending request back to master node with an update containing my load level"));
					TaskSpawnProcess spawn("any", processName, options);
					conn->send(spawn);
				}
				else
				{
					FATAL(true, ("Tried to exceed load limits spawning a game server and could not find the master node to send a spawn request to!"));
				}
			}
			else
			{
				pid = startServerLocal(pe, options);
			}
		}
		else if(pe.targetHost == "any")
		{
			if (nodeLabel == "any")
			{
				// if this is node0, then we can authoritatively spawn this process
				if(getNodeLabel() == "node0")
				{
					// select a task manager on which to spawn the server
					// find the best target
					float cost = getLoadForProcess(pe.processName);
					ManagerConnection * conn = Locator::getBestServer(pe.processName, options, cost);
					if(!conn)
					{

						if(ManagerConnection::getConnectionCount() < 1)
							pid = startServerLocal(pe, options);
						else
						{
							WARNING(true, ("No hosts are available to spawn a process(%s) costing %f without exceeding the host's load limit", processName.c_str(), cost));
							LOG("TaskManager", ("No hosts are available to spawn a process(%s) costing %f without exceeding the host's load limit. Deferring spawn", processName.c_str(), cost));
							// queue the spawn request for another frame
							QueuedSpawnRequest r;
							r.processName = processName;
							r.options = options;
							r.nodeLabel = nodeLabel;
							r.timeQueued = Clock::timeSeconds();
							r.spawnDelay = spawnDelay;
							s_queuedSpawnRequests.push_back(r);
						}
					}
					else
					{
						std::string label = "uninitialized label";
						if(conn->getNodeLabel())
							label = *conn->getNodeLabel();
						DEBUG_FATAL(!conn->getNodeLabel(), ("Told to send to an uninitialized label\n"));
						TaskSpawnProcess spawn( (conn->getNodeLabel() ? *conn->getNodeLabel() : "local"), pe.processName, options);
						conn->send(spawn);
						Locator::incrementServerLoad(conn->getNodeLabel() ? *conn->getNodeLabel() : "local", cost);
						LOG("TaskManager", ("Now spawning process: Sent spawn request to %s (%s) to spawn (%s %s). I think the load on %s is %f/%f", label.c_str(), conn->getRemoteAddress().c_str(), processName.c_str(), options.c_str(), label.c_str(), Locator::getServerLoad(label), Locator::getServerMaximumLoad(label)));
					}
				}
				else
				{
					// this is not the master node, prevent race conditions
					// if two task managers try to select the same node
					// forward request to node0
					ManagerConnection * conn = Locator::getServer("node0");
					if(conn)
					{
						LOG("TaskManager", ("Forwarding spawn request (%s) to master node. My load is %f", processName.c_str(), Locator::getMyLoad()));
						TaskSpawnProcess spawn("any", pe.processName, options);
						conn->send(spawn);
					}
				}
			}
			else //hostName is specified, either a config file or the master node is requesting the spawn
			{
				// start on the specified task manager
				ManagerConnection * conn = Locator::getServer(nodeLabel);
				if (!conn)
				{
					REPORT_LOG(true,
						("Could not spawn %s on %s because that process is not connected to this task manager! Spawn deferred...\n",
						pe.processName.c_str(), nodeLabel.c_str()));

					// ProcessEntry came from the taskmanager.rc file, so
					// we need to update it with the correct targetHost
					// and options information in order to be able to
					// spawn the process when the specified task
					// manager/targetHost is up
					ProcessEntry peCopy = pe;

					peCopy.targetHost = nodeLabel;
					peCopy.options = options;

					ms_deferredSpawns.push_back(peCopy);
				}
				else
				{
					TaskSpawnProcess spawn(nodeLabel, pe.processName, options);
					conn->send(spawn);
					Locator::incrementServerLoad(nodeLabel, getLoadForProcess(pe.processName));
					LOG("TaskManager", ("Now spawning process: Sent spawn request to %s (%s) to spawn (%s %s). I think the load on %s is %f/%f", nodeLabel.c_str(), conn->getRemoteAddress().c_str(), pe.processName.c_str(), options.c_str(), nodeLabel.c_str(), Locator::getServerLoad(nodeLabel), Locator::getServerMaximumLoad(nodeLabel)));
					
					// put this on a pending queue waiting for an ack
					Archive::ByteStream bs;
					spawn.pack(bs);
					OutstandingSpawnRequestAck ack(nodeLabel, bs, spawn.getTransactionId());
					
					s_outstandingSpawnRequestAcks.push_back(ack);
				}
			}
		}
		else
		{
            // start on the specified task manager
			ManagerConnection * conn = Locator::getServer(pe.targetHost);
			if (!conn)
			{

				REPORT_LOG(true, ("Could not spawn %s on %s because %s is not connected to this task manager! Spawn deferred...\n", pe.processName.c_str(), pe.targetHost.c_str(), pe.targetHost.c_str()));
				
				//defer spawn??
				// ProcessEntry came from the taskmanager.rc file, so
				// we need to update it with the correct options
				// information in order to be able to spawn the process
				// when the specified task manager/targetHost is up
				ProcessEntry peCopy = pe;

				peCopy.options = options;

				ms_deferredSpawns.push_back(peCopy);
			}
			else
			{
				TaskSpawnProcess spawn(pe.targetHost, pe.processName, options);
				conn->send(spawn);
				Locator::incrementServerLoad(*conn->getNodeLabel(), getLoadForProcess(pe.processName));
				LOG("TaskManager", ("Now spawning process: Sent spawn request to %s (%s) to spawn (%s %s). I think the load on %s is %f/%f", conn->getNodeLabel()->c_str(), conn->getRemoteAddress().c_str(), pe.processName.c_str(), options.c_str(), conn->getNodeLabel()->c_str(), Locator::getServerLoad(*conn->getNodeLabel()), Locator::getServerMaximumLoad(*conn->getNodeLabel())));
				// put this on a pending queue waiting for an ack
				Archive::ByteStream bs;
				spawn.pack(bs);
				OutstandingSpawnRequestAck ack(nodeLabel, bs, spawn.getTransactionId());
				s_outstandingSpawnRequestAcks.push_back(ack);
			}
		}
	}

	return pid;
}

//-----------------------------------------------------------------------

void TaskManager::retryConnection(ManagerConnection const *connection)
{
	std::vector<NodeEntry>::iterator i = std::find(instance().m_nodeList.begin(), instance().m_nodeList.end(), connection->getRemoteAddress());
	if (i != instance().m_nodeList.end())
	{
		instance().m_nodeToConnectToList.push_back(*i);
	}
	else
	{
		WARNING(true, ("could not find node for %s after it closed connection", connection->getRemoteAddress().c_str()));
	}
}

//-----------------------------------------------------------------------

void TaskManager::runSpawnRequestQueue()
{
	if (!s_queuedSpawnRequests.empty())
	{
		// create a temporary vector to iterate
		std::vector<QueuedSpawnRequest> const temp = s_queuedSpawnRequests;

		// clear the request queue vector
		// if a process can't spawn, it will be re-added to
		// the vector the the next run of the request queue
		s_queuedSpawnRequests.clear();

		for (std::vector<QueuedSpawnRequest>::const_iterator i = temp.begin(); i != temp.end(); ++i)
			IGNORE_RETURN(startServer(i->processName, i->options, i->nodeLabel, 0));
	}
}

//-----------------------------------------------------------------------

void TaskManager::update()
{
	NetworkHandler::update();
	NetworkHandler::dispatch();

	Console::update();

	while(Console::hasPendingCommand())
	{
		IGNORE_RETURN(executeCommand(Console::getNextCommand()));
	}

	Clock::setFrameRateLimit(4.0f);

	static uint32 lastTime = 0;
	uint32 currentTime = Clock::timeMs();
	if (!instance().m_nodeToConnectToList.empty() && currentTime > lastTime + 1000)
	{
		std::vector<NodeEntry>::iterator i = instance().m_nodeToConnectToList.begin();
		for(; i != instance().m_nodeToConnectToList.end(); ++i)
		{
			new ManagerConnection(i->m_address, ConfigTaskManager::getTaskManagerServicePort());
		}
		instance().m_nodeToConnectToList.clear();
		lastTime = currentTime;
	}

	// get process status
	std::set<std::pair<std::string, unsigned long> >::iterator i;
	for(i = instance().m_localServers.begin(); i != instance().m_localServers.end();)
	{
		if(! ProcessSpawner::isProcessActive((*i).second))
		{
			LOG("TaskProcessDied", ("PROCESS DIED: %s", i->first.c_str()));
			// advise the master node that a process has died
			TaskProcessDiedMessage died(i->second, i->first);

			// is this the master node?
			if(getNodeLabel() == "node0")
			{
				// tell CentralServer (if it is still around), that the process died
				if(instance().m_centralConnection)
				{
					LOG("TaskProcessDied", ("advising central that the process is dead"));
					instance().m_centralConnection->send(died);
				}
			}
			else
			{
				ManagerConnection * master = Locator::getServer("node0");
				if(master)
				{
					LOG("TaskProcessDied", ("advising master node that the process is dead"));
					master->send(died);
				}
			}
			// destroy process
			Locator::decrementMyLoad(getLoadForProcess(i->first));
			ProcessAborted a;
			a.commandLine = (*i).first;
			a.hostName = NetworkHandler::getHostName();
			MessageDispatch::emitMessage<const ProcessAborted &>(a);

			if (!fopen(".norestart","r"))
			{
				// respawn CentralServer or LogServer
				if (   (a.commandLine.find("CentralServer") != std::string::npos && ConfigTaskManager::getRestartCentral())
				    || a.commandLine.find("LogServer") != std::string::npos
				    || a.commandLine.find("MetricsServer") != std::string::npos
				    || a.commandLine.find("CommoditiesServer") != std::string::npos
				    || a.commandLine.find("CommodityServer") != std::string::npos 
				    || a.commandLine.find("TransferServer") != std::string::npos)
				{
					RestartRequest r;
					r.commandLine = (*i).first;
					r.timeQueued = Clock::timeSeconds();
					r.spawnDelay = getRestartDelayInformation((*i).first);

					s_restartRequests.push_back(r);
				}
				instance().m_localServers.erase(i++);
				if(i == instance().m_localServers.end())
					break;
			}
		}
		else
			++i;
	}

	if (!s_restartRequests.empty())
	{
		for (std::vector<RestartRequest>::iterator i = s_restartRequests.begin(); i != s_restartRequests.end(); ++i)
		{
			if ((i->spawnDelay == 0) || ((i->timeQueued + i->spawnDelay) < Clock::timeSeconds()))
			{
				unsigned long pid = ProcessSpawner::execute((i->commandLine));
				Locator::incrementMyLoad(getLoadForProcess(i->commandLine));
				IGNORE_RETURN(instance().m_localServers.insert(std::make_pair((i->commandLine), pid)));

				IGNORE_RETURN(s_restartRequests.erase(i));

				// just do 1 per frame to spread out the load
				break;
			}
		}
	}

	if (!ms_deferredSpawns.empty())
	{
		// flush deferred spawns
		std::vector<ProcessEntry>::const_iterator procIter;
		std::vector<ProcessEntry> failures;
		const std::vector<ProcessEntry> & def = ms_deferredSpawns;

		for(procIter = def.begin(); procIter != def.end(); ++procIter)
		{
			ManagerConnection* conn = Locator::getServer(procIter->targetHost);
			if (conn)
			{
				TaskSpawnProcess spawn((*procIter).targetHost, (*procIter).processName, (*procIter).options);
				conn->send(spawn);
				REPORT_LOG(true, ("Sent deferred spawn request for %s to %s\n", (*procIter).processName.c_str(), (*procIter).targetHost.c_str()));
			}
			else
			{
				failures.push_back((*procIter));
			}
		}

		ms_deferredSpawns = failures;
	}

	if (!s_delayedSpawnRequests.empty())
	{
		for (std::vector<QueuedSpawnRequest>::iterator i = s_delayedSpawnRequests.begin(); i != s_delayedSpawnRequests.end(); ++i)
		{
			if ((i->timeQueued + i->spawnDelay) < Clock::timeSeconds())
			{
				IGNORE_RETURN(startServer(i->processName, i->options, i->nodeLabel, 0));
				IGNORE_RETURN(s_delayedSpawnRequests.erase(i));

				// just do 1 per frame to spread out the load
				break;
			}
		}
	}

	// slave TaskManager will periodically send the system time to the
	// the TaskManager to detect when/if system time gets out of sync

	// master TaskManager will periodically send to CentralServer the list
	// of slave TaskManager that has disconnected but has not reconnected,
	// so that an alert can be made in SOEMon so ops can see it and restart
	// the disconnected TaskManager
	static time_t timeSystemTimeCheck = ::time(nullptr) + ConfigTaskManager::getSystemTimeCheckIntervalSeconds();
	time_t const timeNow = ::time(nullptr);
	if (timeSystemTimeCheck <= timeNow)
	{
		if (getNodeLabel() != "node0")
		{
			ManagerConnection * master = Locator::getServer("node0");
			if (master)
			{
				GenericValueTypeMessage<std::pair<std::string, long > > msg("SystemTimeCheck", std::make_pair(getNodeLabel(), static_cast<long>(timeNow)));
				master->send(msg);
			}
		}
		else
		{
			std::string disconnectedTaskManagerList;
			std::map<std::string, std::string> const & disconnectedTaskManager = Locator::getClosedConnections();
			if (!disconnectedTaskManager.empty())
			{
				char buffer[128];
				for (std::map<std::string, std::string>::const_iterator iter = disconnectedTaskManager.begin(); iter != disconnectedTaskManager.end(); ++iter)
				{
					snprintf(buffer, sizeof(buffer)-1, "%s (%s)", iter->first.c_str(), iter->second.c_str());
					buffer[sizeof(buffer)-1] = '\0';

					if (!disconnectedTaskManagerList.empty())
						disconnectedTaskManagerList += ", ";

					disconnectedTaskManagerList += buffer;
				}
			}

			GenericValueTypeMessage<std::string> disconnectedTaskManagerMessage("DisconnectedTaskManagerMessage", disconnectedTaskManagerList);
			TaskManager::sendToCentralServer(disconnectedTaskManagerMessage);
		}

		timeSystemTimeCheck = timeNow + ConfigTaskManager::getSystemTimeCheckIntervalSeconds();
	}

	GameConnection::update();
	Clock::update();
	Clock::limitFrameRate();
}

//-----------------------------------------------------------------------

void TaskManager::sendToCentralServer(const GameNetworkMessage & message)
{
	if(instance().m_centralConnection)
	{
		instance().m_centralConnection->send(message);
	}
}

//-----------------------------------------------------------------------

void TaskManager::setCentralConnection(TaskConnection * newConnection)
{
	instance().m_centralConnection = newConnection;
}

//-----------------------------------------------------------------------

void TaskManager::addToGameConnections(int x)
{
	ms_numGameConnections += x;
	ms_doUpdate = true;
}

//-----------------------------------------------------------------------
int TaskManager::getNumGameConnections()
{
	return ms_numGameConnections;
}

//-----------------------------------------------------------------------

void TaskManager::onDatabaseIdle(bool isIdle)
{
	if (!ms_preloadFinished)
		return;

	if (isIdle)
	{
		++ms_idleFrames;
		if (ConfigTaskManager::getPublishMode() && ms_idleFrames>5) // wait for a few idle frames in a row, just to be safe
		{
			DEBUG_REPORT_LOG(true,("Preloading is done and database is idle.  Shutting down cluster\n"));
			stopCluster();
			ms_done = true;
		}
	}
	else
	{
		ms_idleFrames=0;
	}
}

// ----------------------------------------------------------------------

void TaskManager::startCluster()
{
	IGNORE_RETURN(TaskManager::startServer("TransferServer", "", "local", 0));
	IGNORE_RETURN(TaskManager::startServer("MetricsServer", "", "local", 0));
	IGNORE_RETURN(TaskManager::startServer("LogServer", "", "local", 0));
	std::string options = "-s CentralServer loginServerAddress=";
	options += ConfigTaskManager::getLoginServerAddress();
	options += " clusterName=" + std::string(ConfigTaskManager::getClusterName());
	IGNORE_RETURN(TaskManager::startServer("CentralServer", options, "local", 0));
}

// ----------------------------------------------------------------------

void TaskManager::stopCluster()
{
	std::set<std::pair<std::string, unsigned long> >::iterator i;
	for(i = instance().m_localServers.begin(); i != instance().m_localServers.end(); ++i)
	{
		ProcessSpawner::kill((*i).second);
	}
	instance().m_localServers.clear();
}

// ----------------------------------------------------------------------

void TaskManager::onPreloadFinished()
{
	ms_preloadFinished = true;
}

// ----------------------------------------------------------------------

void TaskManager::resendUnacknowledgedSpawnRequests(Connection * connection, const std::string & nodeLabel)
{
	std::vector<OutstandingSpawnRequestAck>::iterator i;
	for(i = s_outstandingSpawnRequestAcks.begin(); i != s_outstandingSpawnRequestAcks.end(); ++i)
	{
		if(i->nodeLabel == nodeLabel)
		{
			connection->send(i->request, true);
		}
	}
}

// ----------------------------------------------------------------------

void TaskManager::removePendingSpawnProcessAck(int transactionId)
{
	std::vector<OutstandingSpawnRequestAck>::iterator i;
	for(i = s_outstandingSpawnRequestAcks.begin(); i != s_outstandingSpawnRequestAcks.end();)
	{
		if(i->transactionId == transactionId)
		{
			i = s_outstandingSpawnRequestAcks.erase(i);
		}
		else
		{
			++i;
		}
	}
}
// ======================================================================
