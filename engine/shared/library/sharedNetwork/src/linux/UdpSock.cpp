
//---------------------------------------------------------------------

#include "FirstSharedNetwork.h"
#include <cassert>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "sharedNetwork/UdpSock.h"

const int INVALID_SOCKET = 0xFFFFFFFF;

//---------------------------------------------------------------------

UdpSock::UdpSock() :
Sock()
{
	handle = socket(AF_INET, SOCK_DGRAM, 17);
	assert(handle != INVALID_SOCKET);
	setNonBlocking();
}

//---------------------------------------------------------------------

UdpSock::~UdpSock()
{
}

//---------------------------------------------------------------------

const unsigned int UdpSock::recvFrom(Address & outAddr, void * targetBuffer, const unsigned int bufferSize) const
{
	socklen_t fromLen = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	unsigned int result = ::recvfrom(handle, static_cast<char *>(targetBuffer), static_cast<int>(bufferSize), 0, reinterpret_cast<struct sockaddr *>(&addr), &fromLen); //lint !e732 // MS wants an int, should be unsigned IMO
	outAddr = addr;
	return result;
}

//---------------------------------------------------------------------

const unsigned int UdpSock::sendTo(const Address & targetAddress, const void * sourceBuffer, const unsigned int length) const
{
	unsigned int bytesSent = 0;
	if(canSend())
	{
		socklen_t toLen = sizeof(struct sockaddr_in);
		bytesSent = ::sendto(handle, sourceBuffer, static_cast<const int>(length), 0, reinterpret_cast<const struct sockaddr *>(&(targetAddress.getSockAddr4())), toLen); //lint !e732 // MS wants an int, should be unsigned IMO
	}
	return bytesSent;
}

//-----------------------------------------------------------------------

void UdpSock::enableBroadcast()
{
	int optval = 1;
	socklen_t optlen = sizeof(int);
	setsockopt(handle, SOL_SOCKET, SO_BROADCAST, &optval, optlen);
}

//---------------------------------------------------------------------

