// TaskConnection.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_TaskConnection_H
#define	_TaskConnection_H

//-----------------------------------------------------------------------

class TaskCommandChannel;

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class TaskConnection : public ServerConnection
{
public:
	TaskConnection(const std::string & remoteAddress, const unsigned short remotePort);
	TaskConnection(UdpConnectionMT * u, TcpClient *);
	~TaskConnection();

    void  onConnectionClosed();
    void  onConnectionOpened();
    void  onReceive(const Archive::ByteStream & message);

						
private:
	TaskConnection(const TaskConnection&);
	TaskConnection& operator= (const TaskConnection&);

	
}; //lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

#endif	// _TaskConnection_H


