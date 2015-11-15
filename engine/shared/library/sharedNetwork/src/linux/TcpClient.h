// TcpClient.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TcpClient_H
#define	_INCLUDED_TcpClient_H

//-----------------------------------------------------------------------

#include "Archive/ByteStream.h"
#include "sharedNetwork/Address.h"
#include <string>

//-----------------------------------------------------------------------

class Connection;
class TcpServer;

//-----------------------------------------------------------------------

class TcpClient
{
public:
	TcpClient(int sock, TcpServer *);
	TcpClient(const std::string & address, const unsigned short port);

	void         addRef();
	static void  install();
	void         release();
	static void  remove();
	void         send(const unsigned char * const buffer, const int length);

	unsigned short getBindPort() const;
	std::string const &getRemoteAddress() const;
	unsigned short getRemotePort() const;
	void setPendingSendAllocatedSizeLimit(unsigned int limit);

	// only used by clients
	void    update();
	static void  flushPendingWrites();

protected:
	friend class TcpServer;
	friend class Connection;

	void    setSockOptions();
	void    commit();
	int     getSocket() const;
	void    onConnectionClosed();
	void    onConnectionOpened();
	void    onReceive(const unsigned char * const recvBuf, const int bytes);
	void    queryConnect();
	void    queueReceive();
	void    setConnection(Connection *);
	void    checkKeepalive();
	void    clearTcpServer();
	void	setRawTCP( bool bNewValue );		

private:
	TcpClient & operator = (const TcpClient & rhs);
	TcpClient(const TcpClient & source);
	~TcpClient();
	void  flush  ();
	void  setupSocket();
	int                  m_socket;
	TcpServer *          m_tcpServer;
	Archive::ByteStream  m_pendingSend;
	Connection *         m_connection;
	unsigned char *      m_recvBuffer;
	int                  m_recvBufferLength;
	Address              m_remoteAddress;
	int                  m_refCount;
	bool                 m_connected;
	unsigned long        m_lastSendTime;
	unsigned short       m_bindPort;
	bool		     m_rawTCP;
};

//-----------------------------------------------------------------------

inline void TcpClient::setPendingSendAllocatedSizeLimit(const unsigned int limit)
{
	m_pendingSend.setAllocatedSizeLimit(limit);
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TcpClient_H

