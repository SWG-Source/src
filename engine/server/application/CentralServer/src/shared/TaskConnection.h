// TaskConnection.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_TaskConnection_H
#define	_TaskConnection_H

//-----------------------------------------------------------------------

class TaskCommandChannel;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------------

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
	bool  isConnected() const;

private:
	TaskConnection(const TaskConnection&);
	TaskConnection& operator= (const TaskConnection&);

	void onConnectionServerConnectionClosed(const struct OnConnectionServerConnectionClosed &);

private:
	MessageDispatch::Callback * callback;
	bool m_connected;

}; //lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

inline bool TaskConnection::isConnected() const
{
	return m_connected;
}

//-----------------------------------------------------------------------

#endif	// _TaskConnection_H


