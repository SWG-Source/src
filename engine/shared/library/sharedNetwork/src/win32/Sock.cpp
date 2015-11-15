// Sock.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//---------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "sharedNetwork/Sock.h"

#include <winsock.h>

struct WinsockStartupObject
{
	WinsockStartupObject();
	~WinsockStartupObject();
};

WinsockStartupObject::WinsockStartupObject()
{
	WSADATA	wsaData;
	WORD	wVersionRequested;

	wVersionRequested = MAKEWORD(2,0); 
	int err;
	err = WSAStartup(wVersionRequested, &wsaData);
}

WinsockStartupObject::~WinsockStartupObject()
{
	WSACleanup();
}

WinsockStartupObject wso;

//---------------------------------------------------------------------
/**
	@brief construct a Sock

	This constructor sets handle to INVALID_SOCKET, lastError to
	Sock::SOCK_NO_ERROR and the bindAddress to the default Address.

	@see Address
	
	@author Justin Randall
*/
Sock::Sock() :
handle(INVALID_SOCKET),
lastError(Sock::SOCK_NO_ERROR),
bindAddress()
{
}

//---------------------------------------------------------------------
/**
	@brief destroy the Sock object

	checks for a valid socket close. It is an error to close a socket
	that will fail a close operation. 

	Also resets handle to INVALID_SOCKET

	@author Justin Randall
*/
Sock::~Sock()
{
	// ensure we don't block, and that pending
	// data is sent with a graceful shutdown
	int err;
	err = closesocket(handle); 
	if(err == SOCKET_ERROR)
	{
		OutputDebugString(getLastError().c_str());
	}
	handle = INVALID_SOCKET;
}

//---------------------------------------------------------------------
/**
	@brief Bind the socket to the specified local address
*/
bool Sock::bind(const Address & newBindAddress)
{
	bool result = false;

	int enable = 1;
	setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&enable), sizeof(enable));

	bindAddress = newBindAddress;
	int namelen = sizeof(struct sockaddr_in);
	int err = ::bind(handle, reinterpret_cast<const struct sockaddr *>(&(bindAddress.getSockAddr4())), namelen);
	if(err == 0)
	{
		result = true;
		struct sockaddr_in a;
		int r;
		r = getsockname(handle, reinterpret_cast<struct sockaddr *>(&a), &namelen);
		bindAddress = a;
	}
	else
	{
		result = false;
		OutputDebugString(getLastError().c_str());
		OutputDebugString("\n");
	}
	return result;
}

//---------------------------------------------------------------------
/**
	@brief bind the socket to the first available local address
	as provided by the operating system.

	This bind call is useful for client sockets, or server sockets that
	can report their new address to a locator service.

	@author Justin Randall
*/
bool Sock::bind() 
{
	bool result = false;
	struct sockaddr_in a;
	int namelen = sizeof(struct sockaddr_in);
	memset(&a, 0, sizeof(struct sockaddr_in));
	a.sin_family = AF_INET;
	a.sin_port = 0;
	a.sin_addr.s_addr = INADDR_ANY;
	int err = ::bind(handle, reinterpret_cast<struct sockaddr *>(&a), namelen);
	if(err == 0)
	{
		result = true;
		int r;
		r = getsockname(handle, reinterpret_cast<struct sockaddr *>(&a), &namelen);
		bindAddress = a;
	}
	return result;
}

//---------------------------------------------------------------------
/**
	@brief determine if a socket is ready to receive

	On Win32, select is invoked. On Linux, the poll() system call is
	used to deterine readability of a socket.

	@return true if the socket can read data without blocking.

	@author Justin Randall

	@todo Win32 should be using WSAEventSelect and WSAEnumNetworkEvents
	if they outperform select.
*/
bool Sock::canRecv() const
{
	struct timeval	tv;
	fd_set			r;

	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&r);
#pragma warning (disable : 4127)
	FD_SET(handle, &r); //lint !e717 // I have no idea why MS makes this a do { .. } while(0); macro?! Pointless.
	return (select(1, &r, 0, 0, &tv) > 0);
}

//---------------------------------------------------------------------
/**
	@brief determine writeability of the socket

	Win32 systems use select, Linux use poll.

	@return true if the socket can send data without blocking.
	@author Justin Randall
	
	@todo Win32 should be using WSAEventSelect and WSAEnumNetworkEvents
	if they outperform select.
*/
bool Sock::canSend() const
{
	struct timeval	tv;
	fd_set			w;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&w);
	FD_SET(handle, &w); //lint !e717 // I have no idea why MS makes this a do { .. } while(0); macro?! Pointless.

	return (select(1, 0, &w, 0, &tv) > 0);
}

//---------------------------------------------------------------------
/**
	@brief determine the number of bytes pending on the socket

	Uses ioctl (ioctlsocket on win32) to determine the number 
	of bytes pending.

	@return the number of unread bytes pending on the socket

	@author Justin Randall
*/
const unsigned int Sock::getInputBytesPending() const
{
	unsigned long int bytes = 0;
	int err;
	err = ioctlsocket(handle, FIONREAD, &bytes); //lint !e1924 (I don't know WHAT Microsoft is doing here!)
	return bytes;
}

//---------------------------------------------------------------------
/**
	@brief determine the error state of the socket

	This routine also sets the lastError member of the Sock object, which
	is used to determine common errors (connection failure, connection
	closed, connection reset, etc..)

	@return an STL string that describes the error state of the socket,

	@author Justin Randall
*/
const std::string Sock::getLastError() const
{
		std::string errString;
		int iErr = WSAGetLastError();

		switch(iErr)
		{
		case WSAENOPROTOOPT:
			errString = "Bad protocol option. An unknown, invalid or unsupported option or level was specified in a getsockopt or setsockopt call.";
			break;
		case WSAENETDOWN:
			errString = "The network subsystem has failed.";
			break;
		case WSAEFAULT:
			errString = "The buf parameter is not completely contained in a valid part of the user address space.";
			break;
		case WSAENOTCONN:
			errString = "The socket is not connected.";
			lastError = Sock::CONNECTION_FAILED;
			break;
		case WSAEINTR:
			errString = "The (blocking) call was canceled through WSACancelBlockingCall.";
			break;
		case WSAEINPROGRESS:
			errString = "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
			break;
		case WSAENETRESET:
			errString = "The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.";
			break;
		case WSAENOTSOCK:
			errString = "The descriptor is not a socket.";
			break;
		case WSAEOPNOTSUPP:
			errString = "MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, out-of-band data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.";
			break;
		case WSAESHUTDOWN:
			errString = "The socket has been shut down; it is not possible to recv on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.";
			break;
		case WSAEWOULDBLOCK:
			errString = "The socket is marked as nonblocking and the receive operation would block.";
			break;
		case WSAEMSGSIZE:
			errString = "The message was too large to fit into the specified buffer and was truncated.";
			break;
		case WSAEINVAL:
			errString = "The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative.";
			break;
		case WSAECONNABORTED:
			errString = "The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.";
			lastError = Sock::CONNECTION_RESET;
			break;
		case WSAETIMEDOUT:
			errString = "The connection has been dropped because of a network failure or because the peer system failed to respond.";
			break;
		case WSAECONNRESET:
			errString = "The virtual circuit was reset by the remote side executing a \"hard\" or \"abortive\" close. The application should close the socket as it is no longer usable. On a UDP datagram socket this error would indicate that a previous send operation resulted in an ICMP \"Port Unreachable\" message.";
			lastError = Sock::CONNECTION_RESET;
			break;
		case WSAECONNREFUSED:
			errString = "No connection could be made because the target machine actively refused it. This usually results from trying to connect to a service that is inactive on the foreign host - i.e. one with no server application running. ";
			lastError = Sock::CONNECTION_FAILED;
			break;
		default:
			errString = "An unknown socket error has occurred.";
			break;
		}

		return errString;
}

//-----------------------------------------------------------------------
/** @brief determine the maximum message size that may be sent on this socket
*/
const unsigned int Sock::getMaxMessageSendSize() const
{
	/*
	int maxMsgSize = 400;
	int optlen = sizeof(int);
	int result = getsockopt(handle, SOL_SOCKET, SO_MAX_MSG_SIZE, reinterpret_cast<char *>(&maxMsgSize), &optlen);
	if(result != 0)
	{
		int errCode = WSAGetLastError();
		switch(errCode)
		{
		case WSANOTINITIALISED:
			OutputDebugString("A successful WSAStartup call must occur before using this function. ");
			break;
		case WSAENETDOWN:
			OutputDebugString("The network subsystem has failed. ");
			break;
		case WSAEFAULT:
			OutputDebugString("One of the optval or the optlen parameters is not a valid part of the user address space, or the optlen parameter is too small. ");
		case WSAEINPROGRESS:
			OutputDebugString("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. ");
			break;
		case WSAEINVAL:
			OutputDebugString("The level parameter is unknown or invalid. ");
			break;
		case WSAENOPROTOOPT:
			OutputDebugString("The option is unknown or unsupported by the indicated protocol family.  ");
			break;
		case WSAENOTSOCK: 
			OutputDebugString("The descriptor is not a socket. ");
			break;
		default:
			OutputDebugString("An unknown error occurred while processing getsockopt");
			break;
		}
	}
	*/
	return 400;
}

//---------------------------------------------------------------------
/**
	@brief  get a BSD sockaddr struct describing the remote address
	of a socket

	@param target   a BSD sockaddr struct that will receive the peer
	                address
	@param s        the socket to query for the peername

	@author Justin Randall
*/
void Sock::getPeerName(struct sockaddr_in & target, SOCKET s)
{
	int namelen = sizeof(struct sockaddr_in);
	int err;
	err = getpeername(s, reinterpret_cast<sockaddr *>(&(target)), &namelen);
}

//---------------------------------------------------------------------
/** @brief a support routine to place the socket in non-blocking mode

	@author Justin Randall
*/
void Sock::setNonBlocking() const
{
	unsigned long int nb = 1;
	int err;
	err = ioctlsocket(handle, FIONBIO, &nb); //lint !e569 // loss of precision in the FIONBIO macro, beyond my control
	if(err == SOCKET_ERROR)
		OutputDebugString(getLastError().c_str());
}

//---------------------------------------------------------------------
