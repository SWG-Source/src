// ======================================================================
//
// CentralServerConnection.cpp
// copyright (c) 2005 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#include "FirstStationPlayersCollector.h"
#include "StationPlayersCollector.h"
#include "CentralServerConnection.h"

#include "Archive/ByteStream.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "UnicodeUtils.h"

#include "sharedFoundation/CrcConstexpr.hpp"

// ======================================================================

namespace CentralServerConnectionNamespace
{
	static std::map<uint32, CentralServerConnection *>     s_centralServerConnections;
}

using namespace CentralServerConnectionNamespace;

// ======================================================================


CentralServerConnection::CentralServerConnection(UdpConnectionMT * u, TcpClient * t) :
	ServerConnection(u, t)
{
}

//-----------------------------------------------------------------------

CentralServerConnection::CentralServerConnection(const std::string & a, const unsigned short p) :
    ServerConnection(a, p, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

CentralServerConnection::~CentralServerConnection()
{
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionClosed()
{

    DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : CentralServerConnection::onConnectionClosed()\n"));

    ServerConnection::onConnectionClosed();

	s_centralServerConnections.erase(getProcessId());

    //static MessageConnectionCallback m("centralConnectionClosed");
    //emitMessage(m);
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionOpened()
{

    DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : CentralServerConnection::onConnectionOpened()\n"));

    ServerConnection::onConnectionOpened();

	s_centralServerConnections[getProcessId()]=this;

}

// ----------------------------------------------------------------------

void CentralServerConnection::onReceive(const Archive::ByteStream & message)
{
    DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : CentralServerConnection::onReceive() ... "));
    Archive::ReadIterator ri = message.begin();
    GameNetworkMessage msg(ri);
    ri = message.begin();
    
    if (msg.getType() == constcrc("SPCharacterProfileMessage"))
    {
		DEBUG_REPORT_LOG(true,(" Got SPCharacterProfileMessage:"));
		StationPlayersCollector::handleSPCharacterProfileData(msg);
    }
    // else if ( msg.isType("SomeOtherMessage") ) {} leaving this here just so i can ask, what the fuck sony? 
    else
    {
		DEBUG_REPORT_LOG(true, ("Got (%s) handing off to ServerConnection::onReceive\n", msg.getCmdName().c_str()));
		ServerConnection::onReceive(message);
    }
}

//-----------------------------------------------------------------------

CentralServerConnection * CentralServerConnection::getConnectionById(const uint32 serverId)
{
	CentralServerConnection * result = 0;
	std::map<uint32, CentralServerConnection *>::const_iterator j = s_centralServerConnections.find(serverId);
	if(j != s_centralServerConnections.end())
	{
		result = (*j).second;
	}
	return result;
}

// ----------------------------------------------------------------------

//These ids might not be unique across multiple clusters
void CentralServerConnection::getServerIds(std::vector<uint32> &serverIds)
{
	for (std::map<uint32, CentralServerConnection *>::const_iterator i = s_centralServerConnections.begin(); i != s_centralServerConnections.end(); ++i)
		serverIds.push_back((*i).first);
}

// ----------------------------------------------------------------------

