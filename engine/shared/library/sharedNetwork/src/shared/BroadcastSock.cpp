// BroadcastSock.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//---------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "sharedNetwork/BroadcastSock.h"

#include "sharedNetwork/Address.h"
#include "sharedNetwork/UdpSock.h"

//---------------------------------------------------------------------

//---------------------------------------------------------------------
/**
	@brief construct a default broadcast sock

	This constructor invokes the UdpSock default constructor, then 
	sets the SO_BROADCAST option on the resulting UdpSock object.

	@author Justin Randall
*/
BroadcastSock::BroadcastSock() :
udp(new UdpSock)
{
	udp->enableBroadcast();
}

//-----------------------------------------------------------------------

const bool BroadcastSock::bind(const unsigned short port)
{
	return udp->bind(Address("", port));
}

//-----------------------------------------------------------------------

const bool BroadcastSock::canRecv() const
{
	return udp->canRecv();
}

//-----------------------------------------------------------------------

const unsigned int BroadcastSock::recvFrom(Address & a, void * targetBuffer, const unsigned int targetBufferSize) const
{
	unsigned int result = 0;
	if(udp)
	{
		result = udp->recvFrom(a, targetBuffer, targetBufferSize);
	}
	return result;
}

//-----------------------------------------------------------------------

const unsigned int BroadcastSock::sendTo(const std::string & a, const unsigned short p, const void * b, const unsigned int l) const
{
	return udp->sendTo(Address(a, p), b, l);
}

//---------------------------------------------------------------------
/**
	@brief destroy the BroadcastSock. doesn't do anything special.

	@author Justin Randall
*/
BroadcastSock::~BroadcastSock()
{
	delete udp;
}

//---------------------------------------------------------------------

