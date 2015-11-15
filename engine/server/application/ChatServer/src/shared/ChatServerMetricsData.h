//ChatServerMetricsData.h
//Copyright 2002 Sony Online Entertainment


#ifndef	_ChatServerMetricsData_H
#define	_ChatServerMetricsData_H

//-----------------------------------------------------------------------

#include "serverMetrics/MetricsData.h"

//-----------------------------------------------------------------------

class ChatServerMetricsData : public MetricsData
{
public:
	ChatServerMetricsData();
	~ChatServerMetricsData();

	virtual void updateData();
	void         setClientCount(const int clientCount);
	void         setConnectionServerConnectionCount(const int connectionServerConnectionCount);
	void         setGameServerConnectionCount(const int gameServerConnectionCount);
	
private:

	// Disabled.
	ChatServerMetricsData(const ChatServerMetricsData&);
	ChatServerMetricsData &operator =(const ChatServerMetricsData&);

private:
	unsigned long  m_clientCount;
	unsigned long  m_connectionServerCount;
	unsigned long  m_gameServerCount;
};


//-----------------------------------------------------------------------
#endif
