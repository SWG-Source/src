// MetricsGatheringConnection.cpp
// Copyright 2000-2002 Sony Online Entertainment


#include "FirstMetricsServer.h"
#include "MetricsGatheringConnection.h"

#include "ConfigMetricsServer.h"
#include "MetricsServer.h"
#include "MonAPI2/MonitorAPI.h"
#include "serverNetworkMessages/MetricsInitiationMessage.h"

#include <algorithm>

//-----------------------------------------------------------------------

std::map<std::string, int> MetricsGatheringConnection::ms_metricsSummaryTotalVal;

namespace MetricsGatheringConnectionNamespace
{
	int s_masterChannelIndex = 1;
	std::map<std::string, int> s_masterGraphMap;
};

using namespace MetricsGatheringConnectionNamespace;

//-----------------------------------------------------------------------

MetricsGatheringConnection::MetricsGatheringConnection(UdpConnectionMT * u, TcpClient * t) :
		ServerConnection(u, t),
		m_initialized(false),
		m_label(),
		m_processIndex(0),
		m_processLabel(),
		m_metricsChannels()
{
	m_label.clear();
	m_processLabel.clear();
}

//-----------------------------------------------------------------------

MetricsGatheringConnection::~MetricsGatheringConnection()
{

}

//-----------------------------------------------------------------------



void MetricsGatheringConnection::onConnectionClosed()
{
	m_initialized = false;
	DEBUG_REPORT_LOG(true, ("Metrics Connection with Server %s closed\n", m_processLabel.c_str()));
	CMonitorAPI * mon = MetricsServer::getMonitor();
	NOT_NULL(mon);

	if (m_processLabel == "CentralServer")
	{
		mon->set(MetricsServer::getWorldCountChannel(), STATUS_DOWN);		
	}

	remove();
}

//-----------------------------------------------------------------------

void MetricsGatheringConnection::onConnectionOpened()
{
	DEBUG_REPORT_LOG(true, ("Metrics Connection with Server opened\n"));
}

//-----------------------------------------------------------------------

void MetricsGatheringConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage m(ri);
	ri = message.begin(); 

	if(m.isType("ConnectionServerMetricsMessage"))
	{
		DEBUG_FATAL(true, ("This message has been depricated"));
	}
	else if (m.isType("MetricsInitiationMessage"))
	{
		WARNING_STRICT_FATAL(m_initialized, ("Received initialize metrics on initialized connection"));
		//Get process name and secondary field (i.e. planet)
		MetricsInitiationMessage mx(ri);
		bool dynamic = mx.getIsDynamic();
		std::string process = mx.getPrimaryName();
		std::string planet = mx.getSecondaryName();
		int index = mx.getIndex();
		initialize(process, planet, dynamic, index);
	}
	else if (m.isType("MetricsDataMessage"))
	{
		if (!m_initialized)
		{
			//Received data on uninitialized connection.
			return;
		}
		MetricsDataMessage metricsData(ri);
		update(metricsData.getData());
	}
	else
	{
		ServerConnection::onReceive(message);
	}
}

//-----------------------------------------------------------------------

void MetricsGatheringConnection::initialize(const std::string & process, const std::string & planet, bool numberedProcess, int index)
{
	const std::string dot = ".";
	m_label = process;
	m_label = m_label + dot + planet;

	m_processLabel = m_label;
	if (numberedProcess)
	{		
		m_processIndex = index;
		char tmpBuf[100];
		m_label = m_label + dot + _itoa(m_processIndex, tmpBuf, 10);
		
	}
	m_label = m_label + dot;
	m_metricsChannels.clear();
	m_initialized = true;
}

//-----------------------------------------------------------------------

void MetricsGatheringConnection::update(const std::vector<MetricsPair> & data)
{
	CMonitorAPI * mon = MetricsServer::getMonitor();
	NOT_NULL(mon);
	
	std::vector<MetricsPair>::const_iterator dataIter = data.begin();
	for(; dataIter != data.end(); ++dataIter)
	{
		std::map<std::string, std::pair<std::string, int> >::iterator i = m_metricsChannels.find((*dataIter).m_label);
		if (i == m_metricsChannels.end())
		{
			//If this is the first update for a metrics label, we need to assign it a channel number.
			//Then add it to the montior
			int newChannel = 0;
			std::string graphName = m_label + ((*dataIter).m_label);
			if ( dataIter->m_summary )
				graphName = ((*dataIter).m_label);
			std::map<std::string, int>::iterator graphIter = s_masterGraphMap.find(graphName);
			if (graphIter != s_masterGraphMap.end())
			{
				newChannel = graphIter->second; 
			}
			else
			{
				newChannel = ++s_masterChannelIndex;
				s_masterGraphMap[graphName] = newChannel;
				int ping = MON_HISTORY;
				if(! dataIter->m_persistData)
					ping = MON_ONLY_SHOW;
				mon->add(const_cast<char*>(graphName.c_str()), newChannel, ping);
			}
			m_metricsChannels[(*dataIter).m_label] = std::make_pair(graphName, newChannel);
			mon->set(newChannel, (*dataIter).m_value);

			//note: we know that the "isSecret", and "isLocked" nodes will come after the "population" node in the list 
			//		we are iterating over, so they will override the population setting
			if (strstr((*dataIter).m_label.c_str(), "population") != nullptr)
			{
				// there are nodes under the population node, so don't interpret them as the population
				if (strstr((*dataIter).m_label.c_str(), "population.") == nullptr)
					mon->set(MetricsServer::getWorldCountChannel(), std::max((*dataIter).m_value, 0));
			}
			else if (strstr((*dataIter).m_label.c_str(), "isSecret") != nullptr
				|| strstr((*dataIter).m_label.c_str(), "isLocked") != nullptr)
			{
				if ((*dataIter).m_value == 1)
					mon->set(MetricsServer::getWorldCountChannel(), STATUS_LOCKED);
			}
			else if (strstr((*dataIter).m_label.c_str(), "isLoading") != nullptr)
			{
				// include in the root node description how
				// long the cluster has been loading
				if ((*dataIter).m_value > 0)
				{
					char buffer[128];
					snprintf(buffer, sizeof(buffer)-1, "(isLoading: %d seconds) ", (*dataIter).m_value);
					buffer[sizeof(buffer)-1] = '\0';

					std::string description = buffer;
					description += MetricsServer::getWorldCountChannelDescription();
					mon->setDescription(MetricsServer::getWorldCountChannel(), description.c_str());
				}
				else
				{
					mon->setDescription(MetricsServer::getWorldCountChannel(), MetricsServer::getWorldCountChannelDescription().c_str());
				}
			}

			mon->setDescription(newChannel, (*dataIter).m_description.c_str());			
		}
		else
		{
			if ( dataIter->m_summary )
			{
				int val = (*dataIter).m_value;
				if ( val < 0 )
					val = 0;
				int i_delta = val - m_metricsSummaryMyVal[ (*dataIter).m_label ];
				ms_metricsSummaryTotalVal[ (*dataIter).m_label ] += i_delta;
				m_metricsSummaryMyVal[ (*dataIter).m_label ] = val;

				val = ms_metricsSummaryTotalVal[ (*dataIter).m_label ];   // Don't go below 0 (shouldn't happen)
				if ( val < 0 )
					val = 0;
				mon->set(i->second.second, val );
			}
			else
			{
				mon->set(i->second.second, (*dataIter).m_value);
			}
			
			//note: we know that the "isSecret", and "isLocked" nodes will come after the "population" node in the list 
			//		we are iterating over, so they will override the population setting
			if (strstr((*dataIter).m_label.c_str(), "population") != nullptr)
			{
				// there are nodes under the population node, so don't interpret them as the population
				if (strstr((*dataIter).m_label.c_str(), "population.") == nullptr)
					mon->set(MetricsServer::getWorldCountChannel(), std::max((*dataIter).m_value, 0));
			}
			else if (strstr((*dataIter).m_label.c_str(), "isSecret") != nullptr
				|| strstr((*dataIter).m_label.c_str(), "isLocked") != nullptr)
			{
				if ((*dataIter).m_value == 1)
					mon->set(MetricsServer::getWorldCountChannel(), STATUS_LOCKED);
			}
			else if (strstr((*dataIter).m_label.c_str(), "isLoading") != nullptr)
			{
				// include in the root node description how
				// long the cluster has been loading
				if ((*dataIter).m_value > 0)
				{
					char buffer[128];
					snprintf(buffer, sizeof(buffer)-1, "(isLoading: %d seconds) ", (*dataIter).m_value);
					buffer[sizeof(buffer)-1] = '\0';

					std::string description = buffer;
					description += MetricsServer::getWorldCountChannelDescription();
					mon->setDescription(MetricsServer::getWorldCountChannel(), description.c_str());
				}
				else
				{
					mon->setDescription(MetricsServer::getWorldCountChannel(), MetricsServer::getWorldCountChannelDescription().c_str());
				}
			}

			mon->setDescription(i->second.second, (*dataIter).m_description.c_str());									
		}
	}
}


//-----------------------------------------------------------------------

void MetricsGatheringConnection::remove()
{
	CMonitorAPI * mon = MetricsServer::getMonitor();
	NOT_NULL(mon);
	std::map<std::string, std::pair<std::string, int> >::iterator i = m_metricsChannels.begin();
	for (; i != m_metricsChannels.end(); ++i)
	{
		if ( ms_metricsSummaryTotalVal.count( i->first ) )   // summary of multiple servers data
		{
			// This is a summary entry, we can't remove it, just factor our info out of the total
			ms_metricsSummaryTotalVal[ i->first ] -= m_metricsSummaryMyVal[ i->first ];
			m_metricsSummaryMyVal[ i->first ] = 0;
			mon->set(i->second.second, ms_metricsSummaryTotalVal[ i->first ] );
		}
		else if (m_processIndex)
		{
			mon->set(i->second.second, STATUS_LOADING);
		}
		else
		{
			mon->remove(i->second.second);
			IGNORE_RETURN(s_masterGraphMap.erase(i->second.first));
		}
	}
	
	if(! m_processIndex)
		m_metricsChannels.clear();	
}

//-----------------------------------------------------------------------
