// MetricsManager.h
// copyright 2002 Verant Interactive

#include "serverMetrics/FirstServerMetrics.h"
#include "MetricsManager.h"

#include "ConfigServerMetrics.h"
#include "serverMetrics/MetricsConnection.h"
#include "serverMetrics/MetricsData.h"
#include "serverNetworkMessages/MetricsInitiationMessage.h"

//-------------------------------------------------------

MetricsManager::ConnectionState MetricsManager::m_connectionState = MetricsManager::CS_Unconnected;
bool                     MetricsManager::m_isDynamic = false;
bool                     MetricsManager::m_isInstalled = false;
MetricsConnection*       MetricsManager::m_metricsServerConnection = 0;
MetricsData*             MetricsManager::m_metricsData = 0;
std::string              MetricsManager::m_primaryName;
std::string              MetricsManager::m_secondaryName;
float                    MetricsManager::m_timer = 0.0f;
int                      MetricsManager::m_index = 0;

//-------------------------------------------------------
MetricsManager::MetricsManager() 
{
}

//-------------------------------------------------------

MetricsManager::~MetricsManager()
{
}

//-------------------------------------------------------

void MetricsManager::connect(MetricsConnection& c)
{
	DEBUG_FATAL(!m_isInstalled, ("MetricsManager not installed\n"));
	DEBUG_REPORT_LOG(true, ("Metrics server connected\n"));
	m_metricsServerConnection = &c;
	m_connectionState = CS_Connected;
	
	MetricsInitiationMessage m(m_isDynamic, m_primaryName, m_secondaryName, m_index);
	c.send(m, true);
}

//-------------------------------------------------------
void MetricsManager::install(MetricsData* d, bool isDynamic, const std::string & primaryName, const std::string & secondaryName, const int index)
{
	DEBUG_FATAL(m_isInstalled, ("MetricsManager already installed\n"));
	NOT_NULL(d);
	ConfigServerMetrics::install();
	m_isInstalled = true;
	m_metricsData = d;
	m_index = index;

	if (ConfigServerMetrics::getMetricsServerPort()!=0)
	{
		DEBUG_REPORT_LOG(true, ("Attempting to connect to metrics server\n"));
		m_metricsServerConnection = new MetricsConnection(ConfigServerMetrics::getMetricsServerAddress(), ConfigServerMetrics::getMetricsServerPort());
		m_connectionState = CS_Connecting;
	}
	else
	{
		DEBUG_REPORT_LOG(true,("Metrics server disabled in config file (port is set to 0)\n"));
		m_connectionState = CS_Disabled;
	}

	m_isDynamic = isDynamic;
	
	if (primaryName == "")
		m_primaryName = ConfigServerMetrics::getPrimaryName();
	else
		m_primaryName = primaryName;

	if (secondaryName == "")
		m_secondaryName = ConfigServerMetrics::getSecondaryName();
	else
		m_secondaryName = secondaryName;
}

//-------------------------------------------------------

void MetricsManager::remove()
{
	DEBUG_FATAL(!m_isInstalled, ("MetricsManager not installed\n"));
	m_metricsServerConnection = 0;
	ConfigServerMetrics::remove();
}

//-------------------------------------------------------

void MetricsManager::update(float time)
{
	DEBUG_FATAL(!m_isInstalled, ("MetricsManager not installed\n"));
	//each update time send out metrics data to the metrics server.
	//if we are unconnected, try to connect to metrics server
	//if we are connecting do nothing
	//if we are connected, send data

	switch(m_connectionState)
	{
		case CS_Unconnected:
			DEBUG_REPORT_LOG(true, ("Attempting to connect to metrics server\n"));
			m_metricsServerConnection = new MetricsConnection(ConfigServerMetrics::getMetricsServerAddress(), ConfigServerMetrics::getMetricsServerPort());
			m_connectionState = CS_Connecting;
			break;

		case CS_Connecting:
			break;

		case CS_Connected:
			
			m_timer -= time;
			if (m_timer <= 0)
			{
				//			DEBUG_REPORT_LOG(true, ("Sending data to Metrics server"));
				NOT_NULL(m_metricsData);
				const GameNetworkMessage& m = m_metricsData->getDataToSend();
				if (m_metricsServerConnection)
					m_metricsServerConnection->send(m, true);
				m_timer = static_cast<float>(ConfigServerMetrics::getSecondsBetweenUpdates()) * 1000;
			}
			break;

		case CS_Disabled:
			break;
			
		default:
			break;
	}
}

//-------------------------------------------------------
