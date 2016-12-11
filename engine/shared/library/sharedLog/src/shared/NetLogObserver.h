// ======================================================================
//
// NetLogObserver.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NetLogObserver_H
#define INCLUDED_NetLogObserver_H

// ======================================================================

#include "sharedLog/LogObserver.h"
#include "sharedSynchronization/Mutex.h"
#include <string>

// ======================================================================

class NetLogConnection;

// ======================================================================

// A NetLogObserver sends log messages to a another process
class NetLogObserver: public LogObserver
{
public:

	static void install();
	static LogObserver *create(std::string const &spec);

	NetLogObserver(std::string const &remoteAddress, unsigned short remotePort);
	virtual ~NetLogObserver();

	virtual void log(LogMessage const &msg);
	virtual void onConnectionOpened();
	virtual void onConnectionClosed();
	virtual void onConnectionDestroyed(NetLogConnection *connection);

	virtual void update();
	
	std::string const &getRemoteAddress() const;
	unsigned short getRemotePort() const;

private:
	NetLogObserver();
	NetLogObserver(NetLogObserver const &);
	NetLogObserver &operator=(NetLogObserver const &);

	void connect();
	void sendLogMessage(LogMessage const &msg) const;

	Mutex                      m_mutex;
	std::string                m_remoteAddress;
	unsigned short             m_remotePort;
	NetLogConnection *         m_connection;
	bool                       m_connectionOpen;
	unsigned long              m_connectTime;
	std::vector<LogMessage>* m_logMessageQueue1;
	std::vector<LogMessage>* m_logMessageQueue2;
};

// ======================================================================

inline std::string const &NetLogObserver::getRemoteAddress() const
{
	return m_remoteAddress;
}

// ----------------------------------------------------------------------

inline unsigned short NetLogObserver::getRemotePort() const
{
	return m_remotePort;
}

// ======================================================================

#endif

