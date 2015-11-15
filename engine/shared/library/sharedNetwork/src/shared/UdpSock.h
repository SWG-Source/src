// UdpSock.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//---------------------------------------------------------------------

#ifndef	INCLUDED_UdpSock_H
#define	INCLUDED_UdpSock_H

//---------------------------------------------------------------------

#include "sharedNetwork/Sock.h"

//---------------------------------------------------------------------
/**
	@brief a datagram socket

	The UdpSock class describes an abstraction for datagram sockets. 
	UdpSock sends and receives unreliably with no flow control.

	@author Justin Randall
*/
class UdpSock : public Sock
{
public:
	UdpSock();
	virtual ~UdpSock();

	virtual const unsigned int recvFrom         (Address & outTargetAddress, void * targetBuffer, const unsigned int bufferSize) const;
	virtual const unsigned int sendTo           (const Address & targetAddress, const void * sourceBuffer, const unsigned int length) const;
	void                       enableBroadcast  ();

private:
	UdpSock(const UdpSock & source);
	UdpSock & operator = (const UdpSock & source);
};

/**
	\include TestUdpSock.h
	\example TestUdpSock.cpp
*/
//---------------------------------------------------------------------

#endif	// _UdpSock_H
