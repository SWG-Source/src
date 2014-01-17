// ======================================================================
//
// LoggerConnection.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LoggerConnection_H
#define INCLUDED_LoggerConnection_H

// ======================================================================

#include "serverUtility/ServerConnection.h"

// ======================================================================

class LoggerConnection: public ServerConnection
{
public:
	LoggerConnection(UdpConnectionMT *, TcpClient *);
	virtual ~LoggerConnection();

	virtual void onConnectionClosed();
	virtual void onConnectionOpened();

private:
	LoggerConnection();
	LoggerConnection(LoggerConnection const &);
	LoggerConnection &operator=(LoggerConnection const &);
};

// ======================================================================

#endif

