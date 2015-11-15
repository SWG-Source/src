// ManagerConnection.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ManagerConnection_H
#define	_INCLUDED_ManagerConnection_H

//-----------------------------------------------------------------------

#include "sharedMessageDispatch/Transceiver.h"
#include "TaskHandler.h"
#include "sharedNetwork/Connection.h"

class GameNetworkMessage;
class TaskConnection;

//-----------------------------------------------------------------------

class ManagerConnection : public Connection
{
public:
	ManagerConnection(const std::string & remoteAddres, const unsigned short remotePort);
	ManagerConnection(UdpConnectionMT*, TcpClient *);
	~ManagerConnection();

	static int         getConnectionCount      ();
	const std::string* getNodeLabel() const;
	virtual void       onConnectionClosed      ();
	virtual void       onConnectionOpened      ();
	virtual void       onReceive               (const Archive::ByteStream & message);
	void               send(const GameNetworkMessage & message);
private:
	ManagerConnection & operator = (const ManagerConnection & rhs);
	ManagerConnection(const ManagerConnection & source);
	ManagerConnection();
	
private:
	std::string                 *m_nodeLabel; 
	float                       m_remoteUtilAvg;
};

//-----------------------------------------------------------------------

inline const std::string* ManagerConnection::getNodeLabel() const
{
	return m_nodeLabel;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ManagerConnection_H
