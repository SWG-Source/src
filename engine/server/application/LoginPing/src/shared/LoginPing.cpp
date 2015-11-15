// ======================================================================
//
// LoginPing.cpp
//
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
//
// ======================================================================

#include "FirstLoginPing.h"
#include "LoginPing.h"
#include "ConfigLoginPing.h"
#include "LoginConnection.h"
#include "sharedFoundation/Clock.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

LoginPing::LoginPing() :
	m_loginConnection(new LoginConnection(ConfigLoginPing::getLoginServerAddress(), ConfigLoginPing::getLoginServerPingServicePort())),
	m_waitingForResponse(true)
{
}

// ----------------------------------------------------------------------

LoginPing::~LoginPing()
{
	if (m_loginConnection)
		m_loginConnection->disconnect();
}

// ----------------------------------------------------------------------

LoginPing & LoginPing::instance()
{
	static LoginPing l;
	return l;
}

// ----------------------------------------------------------------------

bool LoginPing::ping()
{
	if (ConfigLoginPing::getPassthroughMode())
	{
		return true;
	}
	if (!instance().m_loginConnection)
	{
		return false;
	}
	GameNetworkMessage m("LoginPingMessage");
	Archive::ByteStream b;
	m.pack(b);
	instance().m_loginConnection->send(b, true);
	int numTries = 0;
	while(instance().m_waitingForResponse)
	{
		LoginPing::update();
		numTries++;
		if (numTries > 100) 
		{
			break;
		}
	}
	return !instance().m_waitingForResponse;
}

// ----------------------------------------------------------------------

void LoginPing::update()
{
	NetworkHandler::update();
	NetworkHandler::dispatch();
	Clock::setFrameRateLimit(4.0f);

	Clock::update();
	Clock::limitFrameRate();
}

// ----------------------------------------------------------------------

void LoginPing::receiveReplyMessage()
{
	instance().m_waitingForResponse = false;
}

// ----------------------------------------------------------------------

void LoginPing::onLoginConnectionDestroyed(LoginConnection const *loginConnection)
{
	if (loginConnection == instance().m_loginConnection)
		instance().m_loginConnection = 0;
}

// ======================================================================

