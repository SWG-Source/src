// ======================================================================
//
// SharedRemoteDebugServerConnection.h
// copyright 2001, Sony Online Entertainment
//
// ======================================================================

#ifndef _SharedRemoteDebugServerConnection_H
#define _SharedRemoteDebugServerConnection_H

//-----------------------------------------------------------------------

#include "sharedNetwork/Connection.h"

class SharedRemoteDebugServerCommandChannel;

//-----------------------------------------------------------------------

class SharedRemoteDebugServerConnection : public Connection
{
public:
	SharedRemoteDebugServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	SharedRemoteDebugServerConnection(UdpConnectionMT *, TcpClient *);
	virtual ~SharedRemoteDebugServerConnection();
	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();
	void                          onConnectionOverflowing (const unsigned int bytesPending);
	void	                      onReceive               (const Archive::ByteStream & message);

private:
	SharedRemoteDebugServerConnection(const SharedRemoteDebugServerConnection &);
	SharedRemoteDebugServerConnection & operator = (const SharedRemoteDebugServerConnection &);
	SharedRemoteDebugServerCommandChannel *m_remotedebugCommandChannel;
};

//-----------------------------------------------------------------------

#endif // _SharedRemoteDebugServerConnection_H
