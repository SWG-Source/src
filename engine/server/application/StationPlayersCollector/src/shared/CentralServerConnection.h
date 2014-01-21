// ======================================================================
//
// CentralServerConnection.h
// copyright (c) 2005 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef	_CentralServerConnection_H
#define	_CentralServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class CentralServerConnection : public ServerConnection
{
public:
	CentralServerConnection(UdpConnectionMT *, TcpClient * t);
	CentralServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	~CentralServerConnection();
	virtual void         onConnectionClosed      ();
	virtual void         onConnectionOpened      ();
	virtual void         onReceive               (const Archive::ByteStream & message);

	// Internal Connection tracking by processid
	static CentralServerConnection *  getConnectionById       (const uint32 serverId);
	static void                    getServerIds            (std::vector<uint32> &serverIds);

private:

	CentralServerConnection (const CentralServerConnection&);
	CentralServerConnection& operator= (const CentralServerConnection&);
};

//-----------------------------------------------------------------------

#endif	// _CentralServerConnection_H
