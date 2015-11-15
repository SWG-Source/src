// MetricsConnection.cpp
// copyright 2000 Verant Interactive



//-----------------------------------------------------------------------

#include "serverMetrics/FirstServerMetrics.h"
#include "serverMetrics/MetricsConnection.h"

#include "serverMetrics/MetricsManager.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

MetricsConnection::MetricsConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

MetricsConnection::MetricsConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t)
{
}

//-----------------------------------------------------------------------

MetricsConnection::~MetricsConnection()
{
}

//-----------------------------------------------------------------------

void MetricsConnection::onConnectionClosed()
{
	DEBUG_REPORT_LOG(true, ("Connection with Metrics Server closed\n"));
	MetricsManager::disconnect();
}

//-----------------------------------------------------------------------

void MetricsConnection::onConnectionOpened()
{
	DEBUG_REPORT_LOG(true, ("Connection with Metrics Server opened\n"));
	MetricsManager::connect(*this);
}

//-----------------------------------------------------------------------

void MetricsConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator r(message);
	GameNetworkMessage m(r);
	UNREF(m);
//	DEBUG_WARNING(true, ("This connection should receive no data (metrics) \n"));
}

//-----------------------------------------------------------------------
