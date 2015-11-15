// ======================================================================
//
// NetLogConnection.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedLog/FirstSharedLog.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/NetLogObserver.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "NetLogConnection.h"

// ======================================================================

NetLogConnection::NetLogConnection(NetLogObserver *owner) :
	Connection(owner->getRemoteAddress(), owner->getRemotePort(), NetworkSetupData()),
	m_owner(owner)
{
}

// ----------------------------------------------------------------------

NetLogConnection::~NetLogConnection()
{
	if (m_owner)
		m_owner->onConnectionDestroyed(this);
	m_owner = 0;
}

// ----------------------------------------------------------------------

void NetLogConnection::onConnectionClosed()
{
	if (m_owner)
		m_owner->onConnectionClosed();
}

// ----------------------------------------------------------------------

void NetLogConnection::onConnectionOpened()
{
	if (m_owner)
		m_owner->onConnectionOpened();
}

// ----------------------------------------------------------------------

void NetLogConnection::clearOwner()
{
	m_owner = 0;
}

// ----------------------------------------------------------------------

void NetLogConnection::onReceive(Archive::ByteStream const &)
{
}

// ----------------------------------------------------------------------

bool NetLogConnection::isNetLogConnection() const
{
	return true;
}

// ======================================================================

