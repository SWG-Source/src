// MetricsServer.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_MetricsServer_H
#define	_INCLUDED_MetricsServer_H

//-----------------------------------------------------------------------

class CMonitorAPI;
class Service;
class TaskConnection;

//-----------------------------------------------------------------------

class MetricsServer
{
public:
	MetricsServer();
	~MetricsServer();

	static void       install();
	static void       remove();
	static void       run                       ();

	static CMonitorAPI *        getMonitor();
	static const std::string &  getCommandLine();
	static void                 setCommandLine(const std::string & );
	static int                  getWorldCountChannel();
	static const std::string &  getWorldCountChannelDescription();
	
private:
	MetricsServer & operator = (const MetricsServer & rhs);
	MetricsServer(const MetricsServer & source);

private:
	static int                m_worldCountChannel;
	static std::string        m_worldCountChannelDescription;
	static std::string        m_commandLine;
	static bool               m_done;
	static Service *          m_metricsService;
	static CMonitorAPI *      m_soeMonitor;
	static Service *          ms_service;
	static TaskConnection *   ms_taskConnection;
};
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------

#endif	// _INCLUDED_MetricsServer_H
