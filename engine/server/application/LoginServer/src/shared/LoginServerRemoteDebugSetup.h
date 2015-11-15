// ======================================================================
//
// LoginServerRemoteDebugSetup.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

class Connection;
class LoginServerRemoteDebugConnection;
class Service;

class LoginServerRemoteDebugSetup
{
friend class LoginServerRemoteDebugConnection;

public:
	static void install(void);
	static void remove(void);

private:
	///The client-specific network service data instance
	///true if system has been installed, false otherwise
	static bool                ms_installed;

	static unsigned int        ms_remoteDebugToolChannelID;
	static Connection *        ms_connection;
	static Service *           ms_remoteDebugService;
	static void open(const char *server, uint16 port);
	static void close(void);
	static void send(void *buffer, uint32 bufferLen);
	static void receive(void *buffer, uint32 bufferLen);
};
