// MetricsManager.h
// Copyright 2002 Sony Online Entertainment


#ifndef	_MetricsManager_H
#define	_MetricsManager_H

#include "Singleton/Singleton.h"

class GameNetworkMessage;
class MetricsConnection;
class MetricsData;

class MetricsManager
{
	friend class MetricsConnection;
public:
	static void install(MetricsData*, bool, const std::string & primaryName, const std::string & secondaryName, const int index);
	static void remove();
	static void update(float time);

private:

	MetricsManager();
	~MetricsManager();

private:
	
	enum ConnectionState
	{
		CS_Unconnected,
		CS_Connecting,
		CS_Connected,
		CS_Disabled
	};
	static void                     connect(MetricsConnection &);
	static void                     disconnect();
	
	static ConnectionState          m_connectionState;
	static bool                     m_isDynamic;
	static bool                     m_isInstalled;
	static MetricsConnection*       m_metricsServerConnection;
	static MetricsData*             m_metricsData;
	static std::string              m_primaryName;
	static std::string              m_secondaryName;
	static int                      m_index;
	static float                    m_timer;

};

//---------------------------------------------------------------


//---------------------------------------------------------------

inline void MetricsManager::disconnect()
{
	DEBUG_REPORT_LOG(true, ("Metrics server disconnected\n"));
	m_connectionState = CS_Unconnected;
	m_metricsServerConnection = 0;
}
//---------------------------------------------------------------

#endif
