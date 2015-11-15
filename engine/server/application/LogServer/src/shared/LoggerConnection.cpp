// ======================================================================
//
// LoggerConnection.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstLogServer.h"
#include "LoggerConnection.h"

// ======================================================================

LoggerConnection::LoggerConnection(UdpConnectionMT * u, TcpClient * t) :
	ServerConnection(u, t)
{
	REPORT_LOG(true, ("LoggerConnection created.\n"));
}

// ----------------------------------------------------------------------

LoggerConnection::~LoggerConnection()
{
	REPORT_LOG(true, ("LoggerConnection destroyed.\n"));
}

// ----------------------------------------------------------------------

void LoggerConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();
}

// ----------------------------------------------------------------------

void LoggerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();
}

// ======================================================================

