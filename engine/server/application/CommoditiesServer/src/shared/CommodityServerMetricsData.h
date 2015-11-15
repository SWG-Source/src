//CommodityServerMetricsData.h
//Copyright 2002 Sony Online Entertainment


#ifndef	_CommodityServerMetricsData_H
#define	_CommodityServerMetricsData_H

//-----------------------------------------------------------------------

#include "serverMetrics/MetricsData.h"

//-----------------------------------------------------------------------

class CommodityServerMetricsData : public MetricsData
{
public:
	CommodityServerMetricsData();
	~CommodityServerMetricsData();

	virtual void updateData();

private:
	unsigned long m_numItems;
	unsigned long m_numLocations;
	unsigned long m_loadTime;

	std::map<std::string, int> m_mapAuctionsCountByGameObjectTypeIndex;

private:

	// Disabled.
	CommodityServerMetricsData(const CommodityServerMetricsData&);
	CommodityServerMetricsData &operator =(const CommodityServerMetricsData&);
};


//-----------------------------------------------------------------------
#endif
