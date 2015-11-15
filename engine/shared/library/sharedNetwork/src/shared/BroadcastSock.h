// BroadcastSock.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//---------------------------------------------------------------------

#ifndef	INCLUDED_BroadcastSock_H
#define	INCLUDED_BroadcastSock_H

//-----------------------------------------------------------------------

#include <string>

class Address;
class UdpSock;

//---------------------------------------------------------------------
/**
	@brief Extends UdpSock to receive broadcast messages. 

	A BroadcastSock will receive messages sent to
	the broadcast address, which is usually 255.255.255.255. 

	BroadcastSock objects cannot listen for traffic on
	the loopback interface. 

	Example:
	\code
	BroadcastSock b;
	UdpSock c;
	b.bind("", 45555);
	c.bind();

	std::string s("Hi!");
	c.sendTo(Address("255.255.255.255", 45555), s.c_str(), s.length());
	unsigned char r[10] = {"\\0"};
	b.recv(r, 10);	// r should contain Hi!
	\endcode

	@see UdpSock
	
	@author Justin Randall
*/
class BroadcastSock 
{
public:
	BroadcastSock();
	virtual ~BroadcastSock();
	const bool  bind(const unsigned short port);
	const unsigned int sendTo(const std::string & address, const unsigned short port, const void * sourceBuffer, const unsigned int sourceBufferLength) const;
	const bool canRecv() const;
	const unsigned int recvFrom(Address & outAddress, void * targetBuffer, const unsigned int targetBufferSize) const;

private:
	BroadcastSock(const BroadcastSock & source);
	BroadcastSock & operator = (const BroadcastSock & rhs);

private:
	UdpSock *  udp;
};

/**
	\include TestBroadcastSock.h
	\example TestBroadcastSock.cpp
*/
//---------------------------------------------------------------------

#endif	// _BroadcastSock_H
