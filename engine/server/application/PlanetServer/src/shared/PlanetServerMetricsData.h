//PlanetServerMetricsData.h
//Copyright 2002 Sony Online Entertainment


#ifndef	_PlanetServerMetricsData_H
#define	_PlanetServerMetricsData_H

//-----------------------------------------------------------------------

#include "serverMetrics/MetricsData.h"

//-----------------------------------------------------------------------

class PlanetServerMetricsData : public MetricsData
{
public:
	PlanetServerMetricsData();
	~PlanetServerMetricsData();

	virtual void updateData();

private:
	unsigned long  m_numGameServers;
	unsigned long  m_watcherPort;

	// used for reporting load time of each game server
	std::map<uint32, unsigned long> m_gameServerLoadTime;

private:

	// Disabled.
	PlanetServerMetricsData(const PlanetServerMetricsData&);
	PlanetServerMetricsData &operator =(const PlanetServerMetricsData&);
};


//-----------------------------------------------------------------------
#endif
