// ======================================================================
//
// Sock.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_Sock_H
#define	INCLUDED_Sock_H

// ======================================================================

#include "sharedNetwork/Address.h"

// ======================================================================

const unsigned int SOCK_ERROR = 0xFFFFFFFF;
typedef unsigned int SOCKET;

/**
	@brief a BSD socket abstraction

	Sock abstracts BSD sockets for platform independant operation. It
	also provides common socket operations to simplify socket management.

	@see BroadcastSock
	@see TcpSock
	@see UdpSock

	@author Justin Randall
*/
class Sock
{
public:
	/**
		@brief failure states for a socket
	*/
	enum ErrorCodes
	{
		SOCK_NO_ERROR,
		CONNECTION_FAILED,
		CONNECTION_CLOSED,
		CONNECTION_RESET
	};

	                            Sock();
	virtual                     ~Sock() = 0;
	bool                        bind(const Address & bindAddress);
	bool	                    bind();
	bool                        canSend() const;
	bool                        canRecv() const;
	const Address &             getBindAddress() const;
	const SOCKET                getHandle() const;
	const unsigned int          getInputBytesPending() const;
	const std::string           getLastError() const;
	const enum ErrorCodes       getLastErrorCode() const;
	const unsigned int          getMaxMessageSendSize() const;
	static void                 getPeerName(struct sockaddr_in & target, SOCKET s);

private:
	// disabled
	Sock(const Sock & source);
	Sock & operator= (const Sock & source);

protected:
	void                        setNonBlocking() const;
protected:
	int                         handle;

	/**
		@brief support for setting/getting last error from derived
		sock classes
	*/
	mutable enum ErrorCodes     lastError;
private:
	Address                     bindAddress;
};

//---------------------------------------------------------------------
/**
	@brief return the local address of the socket

	Until a socket is bound, the bind address may be reported as
	0.0.0.0:0

	@return a const Address reference describing the local address
	of the socket.

	@author Justin Randall
*/
inline const Address & Sock::getBindAddress() const
{
	return bindAddress;
}

//---------------------------------------------------------------------
/**
	@brief return the platform specific socket handle

	the handle returned is not portable and should only be used locally
	for Sock specific operations.

	@author Justin Randall
*/
inline const SOCKET Sock::getHandle() const
{
	return handle;
}

//---------------------------------------------------------------------
/**
	@brief get the last error code on the socket

	@return the last error code on the socket

	@see Sock::ErrorCodes

	@author Justin Randall
*/
inline const enum Sock::ErrorCodes Sock::getLastErrorCode() const
{
	return lastError;
}

//---------------------------------------------------------------------

#endif	// _Sock_H

