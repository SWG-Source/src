//PlanetServerMetricsData.cpp
//Copyright 2002 Sony Online Entertainment

#include "FirstPlanetServer.h"
#include "PlanetServerMetricsData.h"

#include "PlanetServer.h"
#include "ConfigPlanetServer.h"
#ifndef WIN32
#include "MonAPI2/MonitorData.h"
#endif
#include "serverNetworkMessages/MetricsDataMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <algorithm>

//-----------------------------------------------------------------------

PlanetServerMetricsData::PlanetServerMetricsData() :
MetricsData(),
m_numGameServers(0),
m_watcherPort(0),
m_gameServerLoadTime()
{
	MetricsPair p;

	ADD_METRICS_DATA(numGameServers, 0, false); //lint !e713 loss of precision, couldn't figure out a cast that made Lint happy
	ADD_METRICS_DATA(watcherPort, ConfigPlanetServer::getWatcherServicePort(), false);

	std::string label = NetworkHandler::getHostName();

	for (std::string::iterator i = label.begin(); i != label.end(); ++i)
	{
		if (*i == '.')
		{
			*i = '+';
		}
	}

	label += ":";
	char tmp[256];
	label += _itoa(Os::getProcessId(), tmp, 10);
	
	m_data[m_watcherPort].m_description = label;
}

//-----------------------------------------------------------------------

PlanetServerMetricsData::~PlanetServerMetricsData()
{
}

//-----------------------------------------------------------------------

void PlanetServerMetricsData::updateData() 
{
	MetricsData::updateData();
	m_data[m_numGameServers].m_value = PlanetServer::getInstance().getNumberOfGameServers(); //lint !e732 !e713

#ifndef WIN32
	// if all game servers for planet has not finished loading,
	// set value to "loading" to cause the planet node to appear
	// yellow to help figure out which planet is still loading
	if (!PreloadManager::getInstance().isPreloadComplete())
		m_data[m_watcherPort].m_value = STATUS_LOADING;
	else
#endif
		m_data[m_watcherPort].m_value = ConfigPlanetServer::getWatcherServicePort();

	// report load time of each game server
	PreloadManager::ServerMapType const * serverMap = PreloadManager::getInstance().getServerMap();
	if (serverMap && !serverMap->empty())
	{
		std::map<uint32, unsigned long>::const_iterator iter2;
		MetricsPair p;
		char buffer[128];
		for (PreloadManager::ServerMapType::const_iterator iter = serverMap->begin(); iter != serverMap->end(); ++iter)
		{
			iter2 = m_gameServerLoadTime.find(iter->first);
			if (iter2 == m_gameServerLoadTime.end())
			{
				// create a node to report load time for this game server
				snprintf(buffer, sizeof(buffer)-1, "loadTime_%s_%lu", ConfigPlanetServer::getSceneID(), iter->first);
				buffer[sizeof(buffer)-1] = '\0';

				p.m_label = buffer;
				p.m_value = std::max(static_cast<int>(0), static_cast<int>((iter->second.m_loadTime != -1) ? iter->second.m_loadTime : (time(0) - iter->second.m_timeLoadStarted)));
				p.m_description = "seconds";
				p.m_persistData = true;
				p.m_summary = false;
				m_gameServerLoadTime[iter->first] = m_data.size();
				m_data.push_back(p);
			}
			else
			{
				m_data[iter2->second].m_value = std::max(static_cast<int>(0), static_cast<int>((iter->second.m_loadTime != -1) ? iter->second.m_loadTime : (time(0) - iter->second.m_timeLoadStarted)));
			}
		}
	}
}

//-----------------------------------------------------------------------
