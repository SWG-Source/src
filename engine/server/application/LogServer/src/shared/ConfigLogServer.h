// ======================================================================
//
// ConfigLogServer.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConfigLogServer_H
#define INCLUDED_ConfigLogServer_H

// ======================================================================

class ConfigLogServer
{
public:
	struct Data
	{
		int logServicePort;
		const char *  logServiceBindInterface;
		const char *  loggingServerApiAddress;
		const char *  loggingServerApiLoginName;
		const char *  loggingServerApiPassword;
		const char *  loggingServerApiDefaultDirectory;
		const char *  clusterName;
		int logServiceMaxConnections;
		int loggingServerApiQueueSize;
	};

	static void install();
	static void remove();

	static int           getLogServiceMaxConnections();
	static uint16 getLogServicePort();
	static const char *  getLogServiceBindInterface();
	static const char *  getLoggingServerApiAddress();
	static const char *  getLoggingServerApiLoginName();
	static const char *  getLoggingServerApiPassword();
	static const char *  getLoggingServerApiDefaultDirectory();
	static const char *  getClusterName();
	static int           getLoggingServerApiQueueSize();

private:
	static Data *data;
};

// ======================================================================

inline uint16 ConfigLogServer::getLogServicePort()
{
	return static_cast<uint16>(data->logServicePort);
}

//-----------------------------------------------------------------------

inline const char * ConfigLogServer::getLogServiceBindInterface()
{
	return data->logServiceBindInterface;
}

//-----------------------------------------------------------------------

inline const char * ConfigLogServer::getLoggingServerApiAddress()
{
	return data->loggingServerApiAddress;
}

//-----------------------------------------------------------------------

inline const char * ConfigLogServer::getLoggingServerApiLoginName()
{
	return data->loggingServerApiLoginName;
}

//-----------------------------------------------------------------------

inline const char * ConfigLogServer::getLoggingServerApiPassword()
{
	return data->loggingServerApiPassword;
}

//-----------------------------------------------------------------------

inline const char * ConfigLogServer::getLoggingServerApiDefaultDirectory()
{
	return data->loggingServerApiDefaultDirectory;
}

//-----------------------------------------------------------------------

inline const char * ConfigLogServer::getClusterName()
{
	return data->clusterName;
}

//-----------------------------------------------------------------------

inline int ConfigLogServer::getLogServiceMaxConnections()
{
	return data->logServiceMaxConnections;
}

//-----------------------------------------------------------------------

inline int ConfigLogServer::getLoggingServerApiQueueSize()
{
	return data->loggingServerApiQueueSize;
}

// ======================================================================

#endif

