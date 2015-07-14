// TcpClient.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TcpClient_H
#define	_INCLUDED_TcpClient_H

//-----------------------------------------------------------------------

#include <winsock2.h>
#include "Archive/ByteStream.h"
#include <vector>

//-----------------------------------------------------------------------

class Connection;

//-----------------------------------------------------------------------

class TcpClient
{
public:
	explicit TcpClient(HANDLE parentIOCP);
	TcpClient(const std::string & address, const unsigned short port);
	~TcpClient();

	static void  install();
	static void  remove();
	void    send(const unsigned char * const buffer, const int length);
	unsigned short getBindPort() const;
	std::string const &getRemoteAddress() const;
	unsigned short getRemotePort() const;
	void setPendingSendAllocatedSizeLimit(unsigned int limit);

	// only used by clients
	void    update();
	static void  flushPendingWrites();

protected:
	friend TcpServer;
	friend Connection;

	void    addRef();
	void    commit(const unsigned char * const buffer, const int bufferLen);
	SOCKET  getSocket() const;
	void    onConnectionClosed();
	void    onConnectionOpened();
	void    onReceive(const unsigned char * const recvBuf, const int bytes);
	void    queryConnect();
	void    queueReceive();
	void    release();
	void    setConnection(Connection *);

	void    checkKeepalive();
    void    setRawTCP( bool bNewValue );


private:
	TcpClient & operator = (const TcpClient & rhs);
	TcpClient(const TcpClient & source);
	void  flush  ();

	WSAEVENT     m_connectEvent;
	SOCKET       m_socket;
	TcpServer *  m_tcpServer;
	HANDLE       m_localIOCP;
	Archive::ByteStream  m_pendingSend;
	Connection * m_connection;
	int          m_refCount;
	bool         m_connected;
	bool         m_ownHandle;
	unsigned long m_lastSendTime;
	unsigned short  m_bindPort;

	bool		m_rawTCP;
};

//-----------------------------------------------------------------------

inline void TcpClient::setPendingSendAllocatedSizeLimit(const unsigned int limit)
{
	m_pendingSend.setAllocatedSizeLimit(limit);
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TcpClient_H
