// ======================================================================
//
// NetLogObserver.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedLog/FirstSharedLog.h"
#include "sharedLog/NetLogObserver.h"

#include "sharedFoundation/Clock.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/LogManager.h"
#include "sharedLog/ConfigSharedLog.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/LogMessage.h"
#include "Archive/ByteStream.h"
#include "NetLogConnection.h"
#include <string>
#include <vector>

// ======================================================================

void NetLogObserver::install()
{
	LogManager::registerObserverType("net", create);
}

// ----------------------------------------------------------------------

LogObserver *NetLogObserver::create(std::string const &spec)
{
	// parse spec into address:port
	size_t pos = spec.find(":");
	if (pos != std::string::npos)
	{
		std::string address = spec.substr(0, pos);
		unsigned short port = static_cast<unsigned short>(atoi(spec.c_str()+pos+1));
		// create a NetLogObserver to forward to the specified address/port
		return new NetLogObserver(address, port);
	}
	return 0;
}

// ----------------------------------------------------------------------

NetLogObserver::NetLogObserver(const std::string &remoteAddress, unsigned short remotePort) :
	LogObserver(),
	m_mutex(),
	m_remoteAddress(remoteAddress),
	m_remotePort(remotePort),
	m_connection(0),
	m_connectionOpen(false),
	m_connectTime(0),
	m_logMessageQueue1(new std::vector<LogMessage>),
	m_logMessageQueue2(new std::vector<LogMessage>)
{
}

// ----------------------------------------------------------------------

NetLogObserver::~NetLogObserver()
{
	delete m_logMessageQueue1;
	delete m_logMessageQueue2;

	m_logMessageQueue1 = 0;
	m_logMessageQueue2 = 0;

	// the observer owns the connection
	if (m_connection)
	{
		m_connection->clearOwner();
		m_connection->setDisconnectReason("NetLogObserver destroyed");
		m_connection->disconnect();
		m_connection = 0;
	}
}

// ----------------------------------------------------------------------

void NetLogObserver::onConnectionOpened()
{
	m_connectionOpen = true;
}

// ----------------------------------------------------------------------

void NetLogObserver::onConnectionClosed()
{
	m_connectionOpen = false;
	if (m_connection)
	{
		m_connection->clearOwner();
		m_connection = 0;
	}
}

// ----------------------------------------------------------------------

void NetLogObserver::onConnectionDestroyed(NetLogConnection *connection)
{
	if (m_connection == connection)
		m_connection = 0;
}

// ----------------------------------------------------------------------

void NetLogObserver::sendLogMessage(LogMessage const &msg) const
{
	if (m_connection)
	{
		Archive::ByteStream a;
		a.clear();
		msg.pack(a);
		m_connection->send(a, true);
	}
}

// ----------------------------------------------------------------------

void NetLogObserver::log(LogMessage const &msg)
{
	static unsigned int logNetQueueSize = static_cast<unsigned int>(ConfigSharedLog::getLogNetQueueSize());
	if (logNetQueueSize)
	{
		m_mutex.enter();
		if (m_logMessageQueue1->size() < logNetQueueSize)
			m_logMessageQueue1->push_back(msg);
		m_mutex.leave();
	}
}

//------------------------------------------------------------------------------------------

void NetLogObserver::update()
{
	static unsigned long reconnectTime = static_cast<unsigned long>(ConfigSharedLog::getLogNetReconnectTimeMs());

	if (m_connection && m_connectionOpen)
	{
		m_mutex.enter();
		std::vector<LogMessage> *q = m_logMessageQueue1;
		m_logMessageQueue1 = m_logMessageQueue2;
		m_logMessageQueue2 = q;
		m_mutex.leave();
		// send any queued up log messages

		for (std::vector<LogMessage>::const_iterator i = m_logMessageQueue2->begin(); i != m_logMessageQueue2->end(); ++i)
			sendLogMessage(*i);

		m_logMessageQueue2->clear();
	}
	else if (Clock::timeMs()-m_connectTime > reconnectTime)
		connect();
}

// ----------------------------------------------------------------------

void NetLogObserver::connect()
{
	if (m_connection)
	{
		m_connection->clearOwner();
		m_connection->setDisconnectReason("About to connect again");
		m_connection->disconnect();
		m_connection = 0;
	}
	m_connectTime = Clock::timeMs();
	m_connection = new NetLogConnection(this);
}

// ======================================================================
