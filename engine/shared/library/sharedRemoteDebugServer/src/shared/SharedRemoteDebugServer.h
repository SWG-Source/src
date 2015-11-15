// ======================================================================
//
// SharedRemoteDebugServer.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SharedRemoteDebugServer_H
#define INCLUDED_SharedRemoteDebugServer_H

class SharedRemoteDebugServerConnectionHandler;
class SharedRemoteDebugServerConnection;
class Service;

class SharedRemoteDebugServer
{
friend class SharedRemoteDebugServerConnectionHandler;

public:
	static void install();
	static void remove(void);

private:
	///The client-specific Service used to transmit data
	///true if system has been installed, false otherwise
	static bool                ms_installed;

	static unsigned int        ms_remoteDebugToolChannelID;

	static Service *                      ms_serviceHandle;
	static SharedRemoteDebugServerConnection *  ms_connection;

	static void open(const char *server, uint16 port);
	static void close(void);
	static void send(void *buffer, uint32 bufferLen);
	static void receive(void *buffer, uint32 bufferLen);
};

#endif // INCLUDED_SharedRemoteDebugServer_H

