// ======================================================================
//
// WatcherConnection.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstPlanetServer.h"
#include "WatcherConnection.h"

#include "ConfigPlanetServer.h"
#include "sharedNetworkMessages/PlanetNodeStatusMessage.h"
#include "sharedNetworkMessages/PlanetObjectStatusMessage.h"

//-----------------------------------------------------------------------

WatcherConnection::WatcherConnection(UdpConnectionMT * u, TcpClient * t) :
		ServerConnection(u, t),
		m_objectData(new ObjectDataList),
		m_nodeData(new NodeDataList)
{
}

//-----------------------------------------------------------------------

WatcherConnection::~WatcherConnection()
{
	delete m_objectData;
	delete m_nodeData;
	m_objectData = 0;
	m_nodeData = 0;

}  //lint !e1740 // thinks we didn't delete m_objectData and m_nodeData, but clearly we did

//-----------------------------------------------------------------------

void WatcherConnection::onConnectionClosed()
{
	DEBUG_REPORT_LOG(true,("WatcherConnection closed.\n"));
	ServerConnection::onConnectionClosed();

	static MessageConnectionCallback m("WatcherConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void WatcherConnection::onConnectionOpened()
{
	DEBUG_REPORT_LOG(true,("WatcherConnection opened.\n"));
	ServerConnection::onConnectionOpened();

	static MessageConnectionCallback m("WatcherConnectionOpened");
	emitMessage(m);
	setOverflowLimit(ConfigPlanetServer::getWatcherOverflowLimit());
}

// ----------------------------------------------------------------------

void WatcherConnection::addObjectUpdate(const NetworkId &objectId, int x, int z, uint32 authoritativeServer, int interestRadius, bool deleteObject, int objectTypeTag, int const level, bool const hibernating, uint32 const templateCrc, int const aiActivity, int const creationType)
{
	NOT_NULL(m_objectData);	
	m_objectData->push_back(PlanetObjectStatusMessageData(objectId, x,z, authoritativeServer, interestRadius, static_cast<int>(deleteObject), objectTypeTag, level, hibernating, templateCrc, aiActivity, creationType));

	if (static_cast<int>(m_objectData->size()) > ConfigPlanetServer::getMaxWatcherUpdatesPerMessage())
	{
		flushQueuedObjectData();
	}
}

//-----------------------------------------------------------------------

void WatcherConnection::flushQueuedObjectData()
{
	NOT_NULL(m_objectData);
	if (m_objectData->size() != 0)
	{
		PlanetObjectStatusMessage msg(*m_objectData);
		send(msg,true);
		m_objectData->clear();
	}
}

// ----------------------------------------------------------------------

void WatcherConnection::addNodeUpdate(int x, int z, bool loaded, const std::vector<uint32> &servers, const std::vector<int> &subscriptionCounts)
{
	NOT_NULL(m_nodeData);	
	m_nodeData->push_back(PlanetNodeStatusMessageData(x,z,loaded,servers,subscriptionCounts));

	if (static_cast<int>(m_objectData->size()) > ConfigPlanetServer::getMaxWatcherUpdatesPerMessage())
	{
		flushQueuedNodeData();
	}
}

// ----------------------------------------------------------------------

void WatcherConnection::flushQueuedNodeData()
{
	NOT_NULL(m_nodeData);
	if (m_nodeData->size() != 0)
	{
		PlanetNodeStatusMessage msg(*m_nodeData);
		send(msg,true);
		m_nodeData->clear();
	}
}

// ----------------------------------------------------------------------
