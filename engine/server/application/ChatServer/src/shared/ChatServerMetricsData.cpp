//ChatServerMetricsData.cpp
//Copyright 2002 Sony Online Entertainment

#include "FirstChatServer.h"
#include "ChatServerMetricsData.h"

//-----------------------------------------------------------------------

ChatServerMetricsData::ChatServerMetricsData() :
MetricsData(),
m_clientCount(0),
m_connectionServerCount(0),
m_gameServerCount(0)
{
	MetricsPair p;
	ADD_METRICS_DATA(clientCount, 0, false);
	ADD_METRICS_DATA(connectionServerCount, 0, false);
	ADD_METRICS_DATA(gameServerCount, 0, false);
}

//-----------------------------------------------------------------------

ChatServerMetricsData::~ChatServerMetricsData()
{
}

//-----------------------------------------------------------------------

void ChatServerMetricsData::setClientCount(const int clientCount)
{
	m_data[m_clientCount].m_value = clientCount;
}

//-----------------------------------------------------------------------

void ChatServerMetricsData::setConnectionServerConnectionCount(const int connectionServerConnectionCount)
{
	m_data[m_connectionServerCount].m_value = connectionServerConnectionCount;
}

//-----------------------------------------------------------------------

void ChatServerMetricsData::setGameServerConnectionCount(const int gameServerConnectionCount)
{
	m_data[m_gameServerCount].m_value = gameServerConnectionCount;
}

//-----------------------------------------------------------------------

void ChatServerMetricsData::updateData()
{
	MetricsData::updateData();
}

//-----------------------------------------------------------------------
