// ======================================================================
//
// CommoditiesServerConnection.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved.
// Author: Mike Howard
//
// This is message handler for incoming commodities messages.
//
// ======================================================================

#ifndef	_INCLUDED_CommoditiesServerConnection_H
#define	_INCLUDED_CommoditiesServerConnection_H


#include "sharedMessageDispatch/Receiver.h"
#include "serverUtility/ServerConnection.h"

// ======================================================================

class CommoditiesServerConnection : public ServerConnection, public MessageDispatch::Receiver
{
public:
	CommoditiesServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	explicit CommoditiesServerConnection(UdpConnectionMT *, TcpClient * t);
	~CommoditiesServerConnection();

	virtual void  onConnectionClosed  ();
	virtual void  onConnectionOpened  ();
	virtual void  onReceive           (const Archive::ByteStream & message);
	virtual void  receiveMessage      (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

private:
	int               m_gameServerId;
	bool              m_showAllDebugInfo;

	CommoditiesServerConnection();
	CommoditiesServerConnection & operator = (const CommoditiesServerConnection & rhs);
	CommoditiesServerConnection(const CommoditiesServerConnection & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_CommoditiesServerConnection_H
