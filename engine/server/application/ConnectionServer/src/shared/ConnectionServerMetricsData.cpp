//ConnectionServerMetricsData.cpp
//Copyright 2002 Sony Online Entertainment

#include "FirstConnectionServer.h"
#include "ConnectionServerMetricsData.h"

#include "ConnectionServer.h"
#include "serverNetworkMessages/MetricsDataMessage.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "ClientConnection.h"
#include "sharedLog/Log.h"


//-----------------------------------------------------------------------

ConnectionServerMetricsData::ConnectionServerMetricsData() :
		MetricsData()
{
	MetricsPair p;

	ADD_METRICS_DATA(numUsers, 0, false);
	ADD_METRICS_DATA(numGameServers, 0, false);
	ADD_METRICS_DATA(pingTrafficNumBytes, 0, false);
}

//-----------------------------------------------------------------------

ConnectionServerMetricsData::~ConnectionServerMetricsData()
{
}

//-----------------------------------------------------------------------

void ConnectionServerMetricsData::updateData() 
{
	MetricsData::updateData();
	m_data[m_numUsers].m_value = ConnectionServer::getNumberOfClients();
	m_data[m_numGameServers].m_value = ConnectionServer::getNumberOfGameServers();
	m_data[m_pingTrafficNumBytes].m_value = ConnectionServer::getPingTrafficNumBytes();

/****************** disabled due to stats failing to update on live *********************
	std::map< std::string, uint32 >& cpmap = ClientConnection::getPacketBytesPerMinStats();
	std::map< std::string, uint32 >::iterator cpiter;

	// Shared packet metrics
	for ( cpiter = cpmap.begin(); cpiter != cpmap.end(); ++cpiter )
	{
		std::map< std::string, unsigned long>::iterator miter = m_packetDataMap.find( cpiter->first );
		if ( miter == m_packetDataMap.end() )  // If we haven't added this packet metric yet....
		{
			MetricsData* p_met = MetricsData::getInstance();
			std::string s_key("ConnectionClientPackets_BytesPerMin.");
			s_key+=cpiter->first;
			m_packetDataMap[ cpiter->first ] = p_met->addMetric(s_key.c_str(), 0, 0, true, true);   // add the ID to the local map
			miter = m_packetDataMap.find( cpiter->first );
		}
		m_data[ miter->second ].m_value = cpiter->second;
	}
***********************************************************************************************/
}

//-----------------------------------------------------------------------
