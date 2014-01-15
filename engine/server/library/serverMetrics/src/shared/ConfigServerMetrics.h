// ConfigServerMetrics.h
// copyright 2000 Verant Interactive


#ifndef	_ConfigServerMetrics_H
#define	_ConfigServerMetrics_H

//-----------------------------------------------------------------------

class ConfigServerMetrics
{
public:
	struct Data
	{
		const char * metricsServerAddress;
		uint16       metricsServerPort;
		uint16       secondsBetweenUpdates;
		const char * primaryName;
		const char * secondaryName;
		int          frameTimeAveragingSize;

	};
	static void			         install                     ();
	static void			         remove                      ();

	static const char* getMetricsServerAddress();
	static uint16      getMetricsServerPort();
	static uint16      getSecondsBetweenUpdates();
	static const char* getPrimaryName();
	static const char* getSecondaryName();
	static int         getFrameTimeAveragingSize();
	
private:
	static Data *	data;
};

//-----------------------------------------------------------------------

inline const char* ConfigServerMetrics::getMetricsServerAddress()
{
	return data->metricsServerAddress;
}
//-----------------------------------------------------------------------

inline uint16 ConfigServerMetrics::getMetricsServerPort()
{
	return data->metricsServerPort;
}
//-----------------------------------------------------------------------

inline uint16 ConfigServerMetrics::getSecondsBetweenUpdates()
{
	return data->secondsBetweenUpdates;
}

//-----------------------------------------------------------------------
inline const char* ConfigServerMetrics::getPrimaryName()
{
	return data->primaryName;
}
//-----------------------------------------------------------------------

inline const char* ConfigServerMetrics::getSecondaryName()
{
	return data->secondaryName;
}
//-----------------------------------------------------------------------

inline int ConfigServerMetrics::getFrameTimeAveragingSize()
{
	return data->frameTimeAveragingSize;
}

//-----------------------------------------------------------------------

#endif	// _ConfigServerMetrics_H

