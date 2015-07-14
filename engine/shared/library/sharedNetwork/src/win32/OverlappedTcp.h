// OverlappedTcp.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_OverlappedTcp_H
#define	_INCLUDED_OverlappedTcp_H

//-----------------------------------------------------------------------

#include <winsock2.h>

//-----------------------------------------------------------------------

class TcpClient;
class TcpServer;

//-----------------------------------------------------------------------

struct OverlappedTcp
{
	~OverlappedTcp();
	OVERLAPPED         m_overlapped;

	enum OPERATIONS
	{
		INVALID,
		ACCEPT,
		SEND,
		RECV
	};

	unsigned char *    m_acceptData; // getting peer name during accept
	DWORD              m_bytes;
	enum OPERATIONS    m_operation;
	const TcpServer *  m_tcpServer;
	TcpClient *        m_tcpClient; // accepted sock when operation is accept
	WSABUF             m_recvBuf;
};

//---------------------------------------------------------------------

OverlappedTcp *  getFreeOverlapped();
void             releaseOverlapped(OverlappedTcp *);

//-----------------------------------------------------------------------

#endif	// _INCLUDED_OverlappedTcp_H
