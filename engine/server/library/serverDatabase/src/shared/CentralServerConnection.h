// ======================================================================
//
// CentralServerConnection.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CentralServerConnection_H
#define INCLUDED_CentralServerConnection_H

// ======================================================================

#include "sharedMessageDispatch/Receiver.h"
#include "serverUtility/ServerConnection.h"

// ======================================================================

class CentralServerConnection : public ServerConnection, public MessageDispatch::Receiver
{
public:
	CentralServerConnection(const std::string & remoteAddress, const unsigned short port);
	~CentralServerConnection();

	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();
	void                          onReceive               (const Archive::ByteStream & message);
	void                          receiveMessage          (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
private:
	CentralServerConnection(const CentralServerConnection&);
	CentralServerConnection& operator=(CentralServerConnection&);
};

// ======================================================================

#endif
