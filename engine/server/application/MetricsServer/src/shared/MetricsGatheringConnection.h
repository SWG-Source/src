// MetricsGatheringConnection.h
// Copyright 2000-2002 Sony Online Entertainment

#ifndef _MetricsGatheringConnection_H_
#define _MetricsGatheringConnection_H_

#include "serverUtility/ServerConnection.h"

#include "serverNetworkMessages/MetricsDataMessage.h"

#include <map>
#include <string>
#include <vector>

//-----------------------------------------------------------------------

class MetricsGatheringConnection : public ServerConnection
{
public:
	MetricsGatheringConnection(UdpConnectionMT * u, TcpClient *);
	virtual ~MetricsGatheringConnection();

    virtual void  onConnectionClosed();
    virtual void  onConnectionOpened();
    virtual void  onReceive(const Archive::ByteStream & message);

private:
	MetricsGatheringConnection();
	MetricsGatheringConnection(const MetricsGatheringConnection&);
	MetricsGatheringConnection& operator= (const MetricsGatheringConnection&);

	void           initialize(const std::string &, const std::string &, bool, const int);
	void           remove();
	void           update(const std::vector<MetricsPair> & data);

	bool           m_initialized;
	std::string    m_label;
	int            m_processIndex;
	std::string    m_processLabel;

	std::map<std::string, std::pair<std::string, int> > m_metricsChannels;

	static std::map<std::string, int> ms_metricsSummaryTotalVal;
	       std::map<std::string, int> m_metricsSummaryMyVal;
}; 


#endif
