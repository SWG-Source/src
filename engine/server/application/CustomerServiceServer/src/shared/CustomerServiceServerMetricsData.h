//CustomerServiceServerMetricsData.h
//Copyright 2002 Sony Online Entertainment


#ifndef	_CustomerServiceServerMetricsData_H
#define	_CustomerServiceServerMetricsData_H

//-----------------------------------------------------------------------

#include "serverMetrics/MetricsData.h"

//-----------------------------------------------------------------------

class CustomerServiceServerMetricsData : public MetricsData
{
public:
	CustomerServiceServerMetricsData();
	~CustomerServiceServerMetricsData();

	virtual void updateData();

private:

	// Disabled.
	CustomerServiceServerMetricsData(const CustomerServiceServerMetricsData&);
	CustomerServiceServerMetricsData &operator =(const CustomerServiceServerMetricsData&);
};


//-----------------------------------------------------------------------
#endif
