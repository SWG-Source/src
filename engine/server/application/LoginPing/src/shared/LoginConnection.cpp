// LoginConnection.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "FirstLoginPing.h"
#include "LoginPing.h"
#include "LoginConnection.h"
#include "serverNetworkMessages/TaskEnumCluster.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------

LoginConnection::LoginConnection(const std::string & a, const unsigned short p) :
Connection(a, p, NetworkSetupData())
{
	REPORT_LOG(true, ("Waiting for loginserver connection %s:%d\n", a.c_str(), p));
}

//-----------------------------------------------------------------------

LoginConnection::~LoginConnection()
{
	LoginPing::onLoginConnectionDestroyed(this);
}

//-----------------------------------------------------------------------

void LoginConnection::onConnectionClosed()
{
}

//-----------------------------------------------------------------------

void LoginConnection::onConnectionOpened()
{
	REPORT_LOG(true, ("Connection with loginserver opened\n"));
}

//-----------------------------------------------------------------------

void LoginConnection::onConnectionOverflowing(const unsigned int )
{
}

//-----------------------------------------------------------------------

void LoginConnection::onReceive(const Archive::ByteStream & message )
{
	Archive::ReadIterator r(message);
	GameNetworkMessage g(r);

	if (g.isType("LoginPingMessage")) 
	{
		LoginPing::receiveReplyMessage();
	}
}

//-----------------------------------------------------------------------

