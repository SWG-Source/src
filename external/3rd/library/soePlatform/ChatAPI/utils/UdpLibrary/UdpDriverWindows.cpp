// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#if defined(WIN32)
	#define _CRT_SECURE_NO_DEPRECATE		// gets rid of deprecation warnings in VS 2005 (don't want to change to secure-versions as it hampers portability)
#endif

#include "UdpDriver.h"
#include "UdpHelper.h"

#ifdef WIN32

#pragma warning(push, 3)
#pragma warning(disable:4706)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#ifndef UDPLIBRARY_SINGLE_THREAD
  #include <process.h>
#endif

#pragma warning(pop)

#include <cstdio>


namespace UdpLibrary
{

typedef int socklen_t;

    ////////////////////////////////////////////////////////////
    // internal data definition
    ////////////////////////////////////////////////////////////
class UdpPlatformData
{
    public:
        SOCKET socket;
        unsigned startTtl;
        UdpPlatformGuardObject clockGuard;
        unsigned globalLow;
        int globalHigh;
};

class UdpPlatformGuardData
{
    public:
        CRITICAL_SECTION mCriticalSection;
};

class UdpPlatformThreadData
{
    public:
        HANDLE handle;
        bool running;
};

    
    ////////////////////////////////////////////////////////////
    // platform specific implementation
    ////////////////////////////////////////////////////////////
UdpPlatformDriver::UdpPlatformDriver()
{
    mData = new UdpPlatformData;
    mData->socket = INVALID_SOCKET;
    mData->startTtl = 32;
    mData->globalLow = 0;
    mData->globalHigh = 0;

    WSADATA wsaData;
    #if defined(UDPLIBRARY_WINSOCK2)
        WSAStartup(MAKEWORD(2,0), &wsaData);
    #else
        WSAStartup(MAKEWORD(1,1), &wsaData);
    #endif
}

UdpPlatformDriver::~UdpPlatformDriver()
{
    WSACleanup();

    delete mData;
}

bool UdpPlatformDriver::SocketOpen(int port, int incomingBufferSize, int outgoingBufferSize, const char *bindIpAddress)
{
    mData->socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (mData->socket != INVALID_SOCKET)
    {
        unsigned long lb = 1;
        int err = ioctlsocket(mData->socket, FIONBIO, &lb);
        int nb = outgoingBufferSize;
        err = setsockopt(mData->socket, SOL_SOCKET, SO_SNDBUF, (char *)&nb, sizeof(nb));
        nb = incomingBufferSize;
        err = setsockopt(mData->socket, SOL_SOCKET, SO_RCVBUF, (char *)&nb, sizeof(nb));
        int optLen = sizeof(mData->startTtl);
        getsockopt(mData->socket, IPPROTO_IP, IP_TTL, (char *)&mData->startTtl, &optLen);
    
            // bind it to any address
        struct sockaddr_in addr_loc;
        addr_loc.sin_family = PF_INET;
        addr_loc.sin_port = htons((unsigned short)port);
        addr_loc.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bindIpAddress != nullptr && bindIpAddress[0] != 0)
        {
            unsigned long address = inet_addr(bindIpAddress);
            if (address != INADDR_NONE)
            {
                addr_loc.sin_addr.s_addr = address;        // this is already in network order from the call above
            }
        }

        if (bind(mData->socket, (struct sockaddr *)&addr_loc, sizeof(addr_loc)) != 0)
        {
            SocketClose();
            return(false);
        }
    }
    else
    {
        return(false);
    }
    return(true);
}

void UdpPlatformDriver::SocketClose()
{
    if (mData->socket != INVALID_SOCKET)
    {
        closesocket(mData->socket);
        mData->socket = INVALID_SOCKET;
    }
}

int UdpPlatformDriver::SocketReceive(char *buffer, int bufferSize, UdpPlatformAddress *ipAddress, int *port)
{
    struct sockaddr_in addr_from;
    socklen_t sf = sizeof(addr_from);
    int res = recvfrom(mData->socket, buffer, bufferSize, 0, (struct sockaddr *)&addr_from, &sf);

    if (res != SOCKET_ERROR)
    {
        memcpy(ipAddress->mData, &addr_from.sin_addr.s_addr, 4);
        *port = (int)ntohs(addr_from.sin_port);
        return(res);
    }

        // windows is kind enough to put ICMP error packets inline within the stream as errors, so we
        // can easily see the errors indicating that the destination address is unreachable for some reason
    if (WSAGetLastError() == WSAECONNRESET)
    {
        memcpy(ipAddress->mData, &addr_from.sin_addr.s_addr, 4);
        *port = (int)ntohs(addr_from.sin_port);
        return(0);
    }

    return(-1);     // no more packets found
}

bool UdpPlatformDriver::SocketSend(const char *data, int dataLen, const UdpPlatformAddress *ipAddress, int port)
{
    struct sockaddr_in addr_dest;
    addr_dest.sin_family = PF_INET;
    memcpy(&addr_dest.sin_addr.s_addr, ipAddress->mData, 4);
    addr_dest.sin_port = htons((unsigned short)port);
    if (SOCKET_ERROR == sendto(mData->socket, data, dataLen, 0, (struct sockaddr *)&addr_dest, sizeof(addr_dest)))
    {
        return(false);
    }
    return(true);
}

void UdpPlatformDriver::SocketSendPortAlive(const char *data, int dataLen, const UdpPlatformAddress *ipAddress, int port)
{
    int val = 5;
    setsockopt(mData->socket, IPPROTO_IP, IP_TTL, (char *)&val, sizeof(val));
    SocketSend(data, dataLen, ipAddress, port);
    setsockopt(mData->socket, IPPROTO_IP, IP_TTL, (char *)&mData->startTtl, sizeof(mData->startTtl));
}

bool UdpPlatformDriver::SocketGetLocalIp(UdpPlatformAddress *ipAddress)
{
    struct sockaddr_in addr_self;
    memset(&addr_self, 0, sizeof(addr_self));
    socklen_t len = sizeof(addr_self);
    getsockname(mData->socket, (struct sockaddr *)&addr_self, &len);
    memcpy(ipAddress->mData, &addr_self.sin_addr.s_addr, 4);
    return(true);
}

int UdpPlatformDriver::SocketGetLocalPort()
{
    struct sockaddr_in addr_self;
    memset(&addr_self, 0, sizeof(addr_self));
    socklen_t len = sizeof(addr_self);
    getsockname(mData->socket, (struct sockaddr *)&addr_self, &len);
    return(ntohs(addr_self.sin_port));
}

bool UdpPlatformDriver::GetHostByName(UdpPlatformAddress *ipAddress, const char *hostName)
{
    unsigned long address = inet_addr(hostName);
    if (address == INADDR_NONE)
    {
        struct hostent *lphp;
        lphp = gethostbyname(hostName);
        if (lphp == nullptr)
        {
            address = 0;
        }
        else
        {
            address = ((struct in_addr *)(lphp->h_addr))->s_addr;
        }
    }

    memcpy(ipAddress->mData, &address, 4);
    return(address != 0);
}

bool UdpPlatformDriver::GetSelfAddress(UdpPlatformAddress *ipAddress, bool preferNoRoute)
{
    unsigned long address = 0;

    char hostname[1024];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        struct hostent *entry = gethostbyname(hostname);
        if (entry != nullptr)
        {
            for (int i = 0; entry->h_addr_list[i] != 0; i++)
            {
                in_addr *sin = (in_addr *)entry->h_addr_list[i];

                if (address == 0)
                {
                    address = sin->s_addr;
                }
                else if (sin->s_net != 127)        // never return 127.0.0.1 as self address if there is another option
                {
                    bool routeable = true;
                    if (sin->s_net == 10 || (sin->s_net == 172 && sin->s_host == 16) || (sin->s_net == 192 && sin->s_host == 168))
                    {
                        routeable = false;
                    }

                    if (preferNoRoute && !routeable)
                    {
                        address = sin->s_addr;
                    }
                    else if (!preferNoRoute && routeable)
                    {
                        address = sin->s_addr;
                    }
                }
            }
        }
    }

    memcpy(ipAddress->mData, &address, 4);
    return(address != 0);
}

void UdpPlatformDriver::Sleep(int milliseconds)
{
    ::Sleep((DWORD)milliseconds);
}

UdpClockStamp UdpPlatformDriver::Clock()
{
    UdpGuard guard(&mData->clockGuard);
    unsigned low = GetTickCount();
    if (low < mData->globalLow)
    {
        mData->globalHigh++;
    }
    mData->globalLow = low;
    return(((UdpClockStamp)mData->globalHigh << 32) | low);
}

    ////////////////////////////////////////////////////////////
    // UdpPlatformGuardObject object implementation
    ////////////////////////////////////////////////////////////
UdpPlatformGuardObject::UdpPlatformGuardObject()
{
#ifndef UDPLIBRARY_SINGLE_THREAD
    mData = new UdpPlatformGuardData;
    InitializeCriticalSection(&mData->mCriticalSection);
#endif
}

UdpPlatformGuardObject::~UdpPlatformGuardObject()
{
#ifndef UDPLIBRARY_SINGLE_THREAD
    DeleteCriticalSection(&mData->mCriticalSection);
    delete mData;
#endif
}

void UdpPlatformGuardObject::Enter()
{
#ifndef UDPLIBRARY_SINGLE_THREAD
    EnterCriticalSection(&mData->mCriticalSection);
#endif
}

void UdpPlatformGuardObject::Leave()
{
#ifndef UDPLIBRARY_SINGLE_THREAD
    LeaveCriticalSection(&mData->mCriticalSection);
#endif
}


    ///////////////////////////////////////////////////////////////
    // UdpPlatformThreadObject implementation
    ///////////////////////////////////////////////////////////////
unsigned __stdcall GoThread(void *param)
{
    UdpPlatformThreadObject *thread = (UdpPlatformThreadObject *)param;
    thread->mThreadData->running = true;
    thread->Run();
    thread->mThreadData->running = false;
    thread->Release();
    return(0);
}

UdpPlatformThreadObject::~UdpPlatformThreadObject()
{
#ifndef UDPLIBRARY_SINGLE_THREAD
    if (mThreadData->handle != nullptr)
    {
        CloseHandle(mThreadData->handle);
        mThreadData->handle = nullptr;
    }
#endif
    delete mThreadData;
}

void UdpPlatformThreadObject::Start()
{
    AddRef();
#ifndef UDPLIBRARY_SINGLE_THREAD
    unsigned threadId;
    mThreadData->handle = (HANDLE)_beginthreadex(nullptr, 0, &GoThread, this, 0, &threadId);
#else
    GoThread(this);   // run it inline in main thread (blocks til it's finished, so odds are it won't work, but they shouldn't be using it anyhow in this mode)
#endif
}

UdpPlatformThreadObject::UdpPlatformThreadObject()
{
    mThreadData = new UdpPlatformThreadData;
    mThreadData->handle = nullptr;
    mThreadData->running = false;
}

bool UdpPlatformThreadObject::IsRunning()
{
    return(mThreadData->running);
}


    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // UdpPlatformAddress implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////////
UdpPlatformAddress::UdpPlatformAddress()
{
    memset(mData, 0, sizeof(mData));
}

UdpPlatformAddress::UdpPlatformAddress(const UdpPlatformAddress &source)
{
    memcpy(mData, source.mData, sizeof(mData));
}

UdpPlatformAddress &UdpPlatformAddress::operator=(const UdpPlatformAddress &e)
{ 
    memcpy(mData, e.mData, sizeof(mData));
    return(*this);
}

char *UdpPlatformAddress::GetAddress(char *buffer, int bufferLen) const
{
	if (buffer != nullptr) {
		if (bufferLen < 16)
		{
			*buffer = 0;
			return(buffer);
		}
		assert(buffer != nullptr);
		sprintf(buffer, "%d.%d.%d.%d", mData[0], mData[1], mData[2], mData[3]);
		return(buffer);
	}

	return nullptr;
}

void UdpPlatformAddress::SetAddress(const char *address)
{
    for (int i = 0; i < 4; i++)
    {
        mData[i] = (unsigned char)atoi(address);
        while (*address >= '0' && *address <= '9') 
            address++; 
        if (*address != 0)
            address++;
    }
}

bool UdpPlatformAddress::operator==(const UdpPlatformAddress &e) const
{ 
    return(memcmp(mData, e.mData, sizeof(mData)) == 0);
}


int UdpPlatformAddress::GetHash()
{
    return(*(int *)mData);
}

}   // namespace

#endif // WIN32


