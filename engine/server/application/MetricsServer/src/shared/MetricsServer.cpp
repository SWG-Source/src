

#include "FirstMetricsServer.h"
#include "MetricsServer.h"

#include "ConfigMetricsServer.h"
#include "MetricsGatheringConnection.h"
#include "TaskConnection.h"

#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Os.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"

#include "MonAPI2/MonitorAPI.h"

#ifdef LINUX
#include <time.h>
#endif
#ifdef WINDOWS
#include <windows.h>
#endif
//----------------------------------------------------------------
std::string MetricsServer::m_commandLine;
int MetricsServer::m_worldCountChannel = 0;
std::string MetricsServer::m_worldCountChannelDescription;
bool MetricsServer::m_done = false;
Service*   MetricsServer::m_metricsService;
CMonitorAPI * MetricsServer::m_soeMonitor;
Service * MetricsServer::ms_service = NULL;
TaskConnection * MetricsServer::ms_taskConnection = NULL;

//----------------------------------------------------------------

namespace MetricsServerNamespace
{
	const char* getDateString()
	{
#ifdef LINUX
		time_t timeV;
		time(&timeV);
		return ctime(&timeV);
#else
		return "no time available";
#endif
	}
};

using namespace MetricsServerNamespace;

//----------------------------------------------------------------

MetricsServer::MetricsServer()
{
}

//----------------------------------------------------------------


MetricsServer::~MetricsServer()
{
}

//----------------------------------------------------------------

CMonitorAPI * MetricsServer::getMonitor()
{
	return m_soeMonitor;
}

//----------------------------------------------------------------

int MetricsServer::getWorldCountChannel()
{
	return m_worldCountChannel;
}

//----------------------------------------------------------------

const std::string & MetricsServer::getWorldCountChannelDescription()
{
	return m_worldCountChannelDescription;
}

//----------------------------------------------------------------

void MetricsServer::install()
{
		// Must be called before initMonitor() on Win32 
	NetworkSetupData setup;
	setup.port = ConfigMetricsServer::getMetricsServicePort();
	setup.maxConnections = 200;
	setup.bindInterface = ConfigMetricsServer::getMetricsServiceBindInterface();
	Service * ms_service;
	ms_service = new Service(ConnectionAllocator<MetricsGatheringConnection>(), setup);

	// connect to the task manager
	ms_taskConnection = new TaskConnection("127.0.0.1", ConfigMetricsServer::getTaskManagerPort());
	
	// load authentication data and bind the monitor to the port
	m_soeMonitor = new CMonitorAPI("metricsAuthentication.cfg", ConfigMetricsServer::getMetricsListenerPort());

	const char *masterChannel = "Population";
	m_worldCountChannel = 0;
	m_soeMonitor->add(masterChannel, m_worldCountChannel);
	char tmpBuf[512];
	std::string host = NetworkHandler::getHumanReadableHostName().c_str();
	size_t dotPos = host.find(".");
	if(dotPos != host.npos)
	{
		host = host.substr(0, dotPos);
	}
	
	snprintf(tmpBuf, sizeof(tmpBuf), "version %s on %s [%s]", ApplicationVersion::getInternalVersion(), host.c_str(), getDateString());
	m_worldCountChannelDescription = tmpBuf;
	m_soeMonitor->setDescription(MetricsServer::getWorldCountChannel(), tmpBuf);
	
	//BEGIN TEST CODE
	if (ConfigMetricsServer::getRunTestStats())
	{
		m_soeMonitor->add("TestStats.PlanetServer.Zandar.Count", 10000);
		m_soeMonitor->add("TestStats.GameServer.Zandar.1.Count", 10001);
		m_soeMonitor->add("TestStats.GameServer.Zandar.2.Count", 10002);
	}
	//END TEST CODE
}

//----------------------------------------------------------------

void MetricsServer::remove()
{

	delete m_soeMonitor;
	m_soeMonitor = 0;
	if(ms_taskConnection)
		ms_taskConnection->disconnect();
	delete ms_service;
}

//----------------------------------------------------------------

void MetricsServer::run()
{
	Clock::setFrameRateLimit(1.0f);
	int nCount=  0;
	int nConnected = 0;
	int nlogin = 0;


	unsigned long initialTime = Clock::timeMs();
	bool okToUpdate = false;
	
	while(m_soeMonitor)
	{
		NetworkHandler::update();
		NetworkHandler::dispatch();
		if (!Os::update())
			break;

		// updateMonitor() will return false on the 
		// next update cyclte following a call to shutdownMonitor()
		if (okToUpdate)
		{
			m_soeMonitor->Update();
		}
		else
		{
			//Don't start updating the MonApi until a minute after startup
			if (Clock::timeMs() - initialTime > 60000)
			{
				okToUpdate = true;
			}
		}

		// Update the counts in the object
		// Test data
		
		// Test data
		if (ConfigMetricsServer::getRunTestStats())
		{
			
			m_soeMonitor->set(10000,	nCount++);
			m_soeMonitor->set(10001,	nConnected++);
			m_soeMonitor->set(10002,	nlogin++);
		}
		
		static bool pDump = false;
		if (pDump)
			m_soeMonitor->dump();
		Clock::limitFrameRate();
#ifdef WIN32
        Sleep(2);
#else
        usleep(2000);
#endif
	}
}

//----------------------------------------------------------------

const std::string & MetricsServer::getCommandLine()
{
	return m_commandLine;
}

//----------------------------------------------------------------

void MetricsServer::setCommandLine(const std::string & s)
{
	m_commandLine = s;
}

//----------------------------------------------------------------

