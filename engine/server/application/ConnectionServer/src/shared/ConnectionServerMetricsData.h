//ConnectionServerMetricsData.h
//Copyright 2002 Sony Online Entertainment


#ifndef	_ConnectionServerMetricsData_H
#define	_ConnectionServerMetricsData_H

//-----------------------------------------------------------------------

#include "serverMetrics/MetricsData.h"
#include <map>

//-----------------------------------------------------------------------

class ConnectionServerMetricsData : public MetricsData
{
public:
	ConnectionServerMetricsData();
	~ConnectionServerMetricsData();

	virtual void updateData();

private:
	unsigned long  m_numUsers;
	unsigned long  m_numGameServers;
	unsigned long  m_pingTrafficNumBytes;

	std::map< std::string, unsigned long > m_packetDataMap;

private:

	// Disabled.
	ConnectionServerMetricsData(const ConnectionServerMetricsData&);
	ConnectionServerMetricsData &operator =(const ConnectionServerMetricsData&);
};


//-----------------------------------------------------------------------
#endif
