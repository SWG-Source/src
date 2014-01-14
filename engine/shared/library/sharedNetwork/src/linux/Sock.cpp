//---------------------------------------------------------------------
#pragma warning(disable : 4127)

#include "FirstSharedNetwork.h"
#include <cassert>
#include "Sock.h"
#include <netdb.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

const int INVALID_SOCKET = 0xFFFFFFFF;

//---------------------------------------------------------------------

Sock::Sock() :
handle(INVALID_SOCKET),
lastError(Sock::SOCK_NO_ERROR),
bindAddress()
{
}

//---------------------------------------------------------------------

Sock::~Sock()
{
	// ensure we don't block, and that pending
	// data is sent with a graceful shutdown
	int err;
	err = close(handle);
	assert(err == 0);
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
	setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

	bindAddress = newBindAddress;
	assert(handle != INVALID_SOCKET);
	socklen_t namelen = sizeof(struct sockaddr_in);
	int err = ::bind(handle, reinterpret_cast<const struct sockaddr *>(&(bindAddress.getSockAddr4())), namelen);
	if(err == 0)
	{
		result = true;
		struct sockaddr_in a;
		int r;
		r = getsockname(handle, reinterpret_cast<struct sockaddr *>(&a), &namelen);
		assert(r == 0);
		bindAddress = a;
	}
	else
	{
		result = false;
	}

	assert(err == 0);
	return result;
}

//---------------------------------------------------------------------

bool Sock::bind()
{
	bool result = false;
	struct sockaddr_in a;
	socklen_t namelen = sizeof(struct sockaddr_in);
	memset(&a, 0, sizeof(struct sockaddr_in));
	a.sin_family = AF_INET;
	a.sin_port = 0;
	a.sin_addr.s_addr = INADDR_ANY;
	assert(handle != INVALID_SOCKET);
	int err = ::bind(handle, reinterpret_cast<struct sockaddr *>(&a), namelen);
	if(err == 0)
	{
		result = true;
		int r;
		r = getsockname(handle, reinterpret_cast<struct sockaddr *>(&a), &namelen);
		assert(r == 0);
		bindAddress = a;
	}
	assert(err == 0);
	return result;
}

//---------------------------------------------------------------------

bool Sock::canRecv() const
{
	pollfd r;
	r.fd = handle;
	r.events = POLLIN | POLLPRI | POLLERR;

	int err = poll(&r, 1, 0);

	if(err < 0)
	{
	    switch(errno)
		{
		    case EINTR: // the system has interrupted the call, it's too busy to send right now
			    break;
            default:
				break;
		}
	}

	return (err > 0);
}

//---------------------------------------------------------------------

bool Sock::canSend() const
{
	pollfd w;
	w.fd = handle;
	w.events = POLLOUT | POLLERR;
	int err = poll(&w, 1, 0);

	if(err < 0)
	{
	    switch(errno)
		{
		    case EINTR: // the system has interrupted the call, it's too busy to send right now
			    break;
            default:
				break;
		}
	}

	return (err > 0);
}

//---------------------------------------------------------------------

const unsigned int Sock::getInputBytesPending() const
{
	unsigned long int bytes = 0;
	int err;
	err = ioctl(handle, FIONREAD, &bytes); //lint !e1924 (I don't know WHAT Microsoft is doing here!)
	assert(err == 0);
	return bytes;
}

//---------------------------------------------------------------------

const std::string Sock::getLastError() const
{
		std::string errString;

		switch(errno)
		{
		case EBADF:
			errString = "The argument s is not a valid descriptor.";
			break;
		case ENOTSOCK:
			errString = "The argument s is a file, not a socket.";
			break;
		case ENOPROTOOPT:
			errString = "The option is unknown at the level indicated.";
			break;
		case EFAULT:
			errString = "The address pointed to by optval is not in a valid part of the process address space.  For getsockopt, this error may also be returned if optlen is not in  a  valid part of the process address space.";
			break;
		case ENOTTY:
			errString = "The scoket handle is not associated with a character special device.";
			break;
		case EINVAL:
			errString = "argument is not valid.";
			break;
		case EAGAIN:
			errString = "EAGAIN";
			break;
		case EDEADLK:
			errString = "EDEADLK";
			break;
                case ECONNRESET:
		        errString = "The connection was reset by the peer.";
			lastError = CONNECTION_RESET;
			break;
		default:
			errString = strerror(errno);
			break;
		}

		return errString;
}

//---------------------------------------------------------------------

void Sock::getPeerName(struct sockaddr_in & target, int s)
{
	socklen_t namelen = sizeof(struct sockaddr_in);
	int err;
	err = getpeername(s, reinterpret_cast<sockaddr *>(&(target)), &namelen);
	assert(err != -1);
}

//-----------------------------------------------------------------------
/** @brief determine the maximum message size that may be sent on this socket
*/
const unsigned int Sock::getMaxMessageSendSize() const
{
	int maxMsgSize = 1500;
	/** @todo get msg size via IP interface on Linux
	int optlen = sizeof(int);
	int result = getsockopt(handle, SOL_SOCKET, SO_MAX_MSG_SIZE, reinterpret_cast<char *>(&maxMsgSize), &optlen);
	if(result != 0)
	{
		perror("Sock::getMaxMessageSendSize() : getsockopt()");
	}
	*/
	return static_cast<unsigned int>(maxMsgSize);
}

//---------------------------------------------------------------------

void Sock::setNonBlocking() const
{
	assert(handle != INVALID_SOCKET);
	unsigned long int nb = 1;
	int err;
	err = ioctl(handle, FIONBIO, &nb); //lint !e569 // loss of precision in the FIONBIO macro, beyond my control
	assert(err == 0);
}

//---------------------------------------------------------------------

