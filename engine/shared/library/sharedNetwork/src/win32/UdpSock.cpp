// UdpSock.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//---------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "sharedNetwork/UdpSock.h"

#include <winsock.h>

//---------------------------------------------------------------------
/**
	@brief construct a UdpSock object

	Allocates the socket descriptor, sets it to non-blocking mode.

	@author Justin Randall
*/
UdpSock::UdpSock() :
Sock()
{
	handle = socket(AF_INET, SOCK_DGRAM, 0);
	setNonBlocking();
}

//---------------------------------------------------------------------
/**
	@brief destroy the UdpSock object

	Doesn't do anything special.

	@author Justin Randall
*/
UdpSock::~UdpSock()
{
}

//---------------------------------------------------------------------
/**
	@brief receive a datagram 
	
	Receives a datagram and populates the outAddr parameter with the
	source address of the message.

	Calling Sock::getInputBytesPending can provide a hint before 
	allocating the user supplied target buffer.

	@param outAddr           target Address reference that receives 
	                         the message source IP address
	@param targetBuffer      a user supplied buffer to receive the data. 
	@param targetBufferSize  the size of the user supplied buffer

	@author Justin Randall
*/
const unsigned int UdpSock::recvFrom(Address & outAddr, void * targetBuffer, const unsigned int bufferSize) const
{
	int fromLen = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	unsigned int result = ::recvfrom(handle, static_cast<char *>(targetBuffer), static_cast<int>(bufferSize), 0, reinterpret_cast<struct sockaddr *>(&addr), &fromLen); //lint !e732 // MS wants an int, should be unsigned IMO
	outAddr = addr;
	if(result == SOCK_ERROR)
	{
		OutputDebugString(getLastError().c_str());
	}
	return result;
}

//---------------------------------------------------------------------
/**
	@brief send a datagram to a remote system

	sendTo sends a datagram to a remote system.

	@param targetAddress   System to receive the datagram
	@param sourceBuffer    A user supplied buffer containint the data
	                       to be sent
	@param length          The amount of data in the source buffer to
	                       send

	@return The number of bytes sent on success

	@author Justin Randall
*/
const unsigned int UdpSock::sendTo(const Address & targetAddress, const void * sourceBuffer, const unsigned int length) const
{
	unsigned int bytesSent = 0;
	if(canSend())
	{
		int toLen = sizeof(struct sockaddr_in);
		bytesSent = ::sendto(handle, static_cast<const char *>(sourceBuffer), static_cast<int>(length), 0, reinterpret_cast<const struct sockaddr *>(&(targetAddress.getSockAddr4())), toLen); //lint !e732 // MS wants an int, should be unsigned IMO
		if(bytesSent != length)
		{
			OutputDebugString(getLastError().c_str());
		}
	}
	return bytesSent;
}

//-----------------------------------------------------------------------

void UdpSock::enableBroadcast()
{
	char optval = 1;
	int err;
	err = setsockopt(getHandle(), SOL_SOCKET, SO_BROADCAST, &optval, sizeof(char));
}

//---------------------------------------------------------------------
