// ======================================================================
//
// CommoditiesServerConnection.cpp
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved.
// Author: Mike Howard
// Server Infrastructure by: Justin Randall
//
// This is message handler for incoming commodities messages.
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "Archive/ByteStream.h"
#include "serverDatabase/CommoditiesServerConnection.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverNetworkMessages/GameGameServerMessages.h"
#include "serverUtility/ConfigServerUtility.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------

namespace CommoditiesServerConnectionNamespace
{
}

using namespace CommoditiesServerConnectionNamespace;

//-----------------------------------------------------------------------

CommoditiesServerConnection::CommoditiesServerConnection(const std::string & a, const unsigned short p) :
	ServerConnection(a, p, NetworkSetupData()),
	MessageDispatch::Receiver()
{
}

//-----------------------------------------------------------------------

CommoditiesServerConnection::CommoditiesServerConnection(UdpConnectionMT * u, TcpClient * t) :
	ServerConnection(u, t),
	MessageDispatch::Receiver()
{
}

//-----------------------------------------------------------------------

CommoditiesServerConnection::~CommoditiesServerConnection()
{
	DEBUG_REPORT_LOG(true, ("Deleting commodities server connection\n"));
}

//-----------------------------------------------------------------------

void CommoditiesServerConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("CommoditiesConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CommoditiesServerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();
	static MessageConnectionCallback m("CommoditiesConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CommoditiesServerConnection::onReceive(const Archive::ByteStream & message)
{
	DEBUG_REPORT_LOG(true, ("CommoditiesServerConnection::onReceive.\n"));
	ServerConnection::onReceive(message);
	
	Archive::ReadIterator ri = message.begin();
	const GameNetworkMessage msg(ri);
	ri = message.begin();
	DEBUG_REPORT_LOG(true, ("CommoditiesServerConnection::onReceive  %s\n", msg.getCmdName().c_str()));
}

//-----------------------------------------------------------------------

void CommoditiesServerConnection::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	DEBUG_REPORT_LOG(true, ("CommoditiesServerConnection::receiveMessage\n"));
	UNREF(source);
	UNREF(message);
}

// ======================================================================
