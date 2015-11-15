// ConnectionServerConnection.h
// copyright 2001 Verant Interactive

#ifndef	_INCLUDED_ConnectionServerConnection_H
#define	_INCLUDED_ConnectionServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class ConnectionServerConnection : public ServerConnection
{
public:
	ConnectionServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	~ConnectionServerConnection();
	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();
	void	                      onReceive               (const Archive::ByteStream & message);
	void setSyncStamps(uint16 syncStampShort, uint32 syncStampLong);
	uint16 getSyncStampShort() const;
	uint32 getSyncStampLong() const;

private:
	ConnectionServerConnection & operator = (const ConnectionServerConnection & rhs);
	ConnectionServerConnection(const ConnectionServerConnection & source);

	uint16 m_syncStampShort;
	uint32 m_syncStampLong;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConnectionServerConnection_H
