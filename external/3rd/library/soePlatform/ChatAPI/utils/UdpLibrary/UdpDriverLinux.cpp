// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#include "UdpDriver.h"
#include "UdpHelper.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>        // needed by gcc 3.1 for linux
#include <pthread.h>


namespace UdpLibrary
{

typedef int SOCKET;
const int INVALID_SOCKET = 0xFFFFFFFF;
const int SOCKET_ERROR   = 0xFFFFFFFF;

int IcmpReceive(SOCKET socket, unsigned *ipAddress, int *port);


    ////////////////////////////////////////////////////////////
    // internal data definition
    ////////////////////////////////////////////////////////////
class UdpPlatformData
{
    public:
        SOCKET socket;
        unsigned startTtl;
        UdpPlatformGuardObject clockGuard;
        UdpClockStamp lastStamp;
        UdpClockStamp currentCorrection;
};

class UdpPlatformGuardData
{
    public:
        pthread_mutex_t mutex;
};

class UdpPlatformThreadData
{
    public:
        pthread_t handle;
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
    mData->currentCorrection = 0;
    mData->lastStamp = 0;
}

UdpPlatformDriver::~UdpPlatformDriver()
{
    delete mData;
}

bool UdpPlatformDriver::SocketOpen(int port, int incomingBufferSize, int outgoingBufferSize, const char *bindIpAddress)
{
    mData->socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (mData->socket != INVALID_SOCKET)
    {
            // open socket stuff
        unsigned long nb = 1;
        int err = ioctl(mData->socket, FIONBIO, &nb);
        assert(err != -1);
        nb = outgoingBufferSize;
        err = setsockopt(mData->socket, SOL_SOCKET, SO_SNDBUF, &nb, sizeof(nb));
        assert(err == 0);
        nb = incomingBufferSize;
        err = setsockopt(mData->socket, SOL_SOCKET, SO_RCVBUF, &nb, sizeof(nb));
        assert(err == 0);
        nb = 0;
        err = setsockopt(mData->socket, SOL_SOCKET, SO_BSDCOMPAT, &nb, sizeof(nb));
        assert(err == 0);
        nb = 1;
        err = setsockopt(mData->socket, SOL_IP, IP_RECVERR, &nb, sizeof(nb));
        assert(err == 0);
        int optLen = sizeof(mData->startTtl);
        getsockopt(mData->socket, IPPROTO_IP, IP_TTL, &mData->startTtl, (socklen_t *)&optLen);

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
        close(mData->socket);
        mData->socket = INVALID_SOCKET;
    }
}

int UdpPlatformDriver::SocketReceive(char *buffer, int bufferSize, UdpPlatformAddress *ipAddress, int *port)
{
        // check for standard packets
    struct sockaddr_in addr_from;
    socklen_t sf = sizeof(addr_from);
    int res = recvfrom(mData->socket, buffer, bufferSize, 0, (struct sockaddr *)&addr_from, &sf);
    if (res != SOCKET_ERROR)
    {
        memcpy(ipAddress->mData, &addr_from.sin_addr.s_addr, 4);
        *port = (int)ntohs(addr_from.sin_port);
        return(res);
    }

        // out of standard packets, check for ICMP error packets
    unsigned address;
    int ret = IcmpReceive(mData->socket, &address, port);
    memcpy(ipAddress->mData, &address, 4);
    return(ret);
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
        // port alive packet send
    unsigned long val = 5;
    setsockopt(mData->socket, IPPROTO_IP, IP_TTL, &val, sizeof(val));
    SocketSend(data, dataLen, ipAddress, port);
    val = mData->startTtl;
    setsockopt(mData->socket, IPPROTO_IP, IP_TTL, &val, sizeof(val));
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
                int s_net = (sin->s_addr >> 24);
                int s_host = ((sin->s_addr >> 16) & 0xff);

                if (address == 0)
                {
                    address = sin->s_addr;
                }
                else if (s_net != 127)        // never return 127.0.0.1 as self address if there is another option
                {
                    bool routeable = true;
                    if (s_net == 10 || (s_net == 172 && s_host == 16) || (s_net == 192 && s_host == 168))
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
    struct timeval tv;
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = (milliseconds % 1000) * 1000;
    select(0, 0, 0, 0, &tv);
}

UdpClockStamp UdpPlatformDriver::Clock()
{
    UdpGuard guard(&mData->clockGuard);

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    UdpClockStamp cs = static_cast<UdpClockStamp>(tv.tv_sec) * 1000 + static_cast<UdpClockStamp>(tv.tv_usec / 1000);
    cs += mData->currentCorrection;
    if (cs < mData->lastStamp)
    {
            // clock moved backwards (somebody changed it), don't ever let this happen
            // if clock moves forward, there is no way we can recognize it, code will just
            // have to deal with it.  In the case of the UdpLibrary, it will likely result
            // in a ton of pending packets thinking they have gotten lost and being sent, fairly harmless.
        mData->currentCorrection += (mData->lastStamp - cs);
        cs = mData->lastStamp;
    }
    mData->lastStamp = cs;
    return(cs);
}

int IcmpReceive(SOCKET socket, unsigned *address, int *port)
{
        // we can use some ICMP errors to our advantage to more quickly realize that the connection on the other end has disappeared
    struct sock_extended_err 
    {
        unsigned ee_errno;
        unsigned char ee_origin;
        unsigned char ee_type;
        unsigned char ee_code;
        unsigned char ee_pad;
        unsigned ee_info;
        unsigned ee_data;
    };

    unsigned char msg_control[1024];
    struct msghdr msgh = {0};
    struct sockaddr_in msg_name;
    socklen_t sf = sizeof(msg_name);
    msgh.msg_name = &msg_name;
    msgh.msg_namelen = sf;
    msgh.msg_iov = 0;
    msgh.msg_iovlen = 0;
    msgh.msg_control = msg_control;
    msgh.msg_controllen = sizeof(msg_control);
    
    int err = recvmsg(socket, &msgh, MSG_ERRQUEUE);
    if (err == -1)
        return(-1);

    struct cmsghdr *cmsg;
    for(cmsg = CMSG_FIRSTHDR(&msgh); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msgh, cmsg))
    {
        if(cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVERR)
        {
            sock_extended_err *ee = (sock_extended_err *)CMSG_DATA(cmsg);
            if (ee->ee_origin == 2 && ee->ee_type == ICMP_DEST_UNREACH)        // ICMP origin, destination-unreachable error         // note: process all code types: && ee->ee_code == ICMP_PORT_UNREACH
            {
                memcpy(address, &msg_name.sin_addr.s_addr, 4);
                *port = (int)ntohs(msg_name.sin_port);
                return(0);      // ICMP error return
            }
        }
    }

    return(-1);
}


    ////////////////////////////////////////////////////////////
    // UdpPlatformGuardObject object implementation
    ////////////////////////////////////////////////////////////
UdpPlatformGuardObject::UdpPlatformGuardObject()
{
    mData = new UdpPlatformGuardData;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mData->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

UdpPlatformGuardObject::~UdpPlatformGuardObject()
{
    pthread_mutex_destroy(&mData->mutex);
    delete mData;
}

void UdpPlatformGuardObject::Enter()
{
    pthread_mutex_lock(&mData->mutex);
}

void UdpPlatformGuardObject::Leave()
{
    pthread_mutex_unlock(&mData->mutex);
}


    ///////////////////////////////////////////////////////////////
    // UdpPlatformThreadObject implementation
    ///////////////////////////////////////////////////////////////
void *GoThread(void *param)
{
    UdpPlatformThreadObject *thread = (UdpPlatformThreadObject *)param;
    thread->mThreadData->running = true;
    thread->Run();
    thread->mThreadData->running = false;
    thread->mThreadData->handle = 0;
    thread->Release();
    return(nullptr);
}

UdpPlatformThreadObject::~UdpPlatformThreadObject()
{
    delete mThreadData;
}

void UdpPlatformThreadObject::Start()
{
    AddRef();
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&mThreadData->handle, &attr, &GoThread, (void *)this);
    pthread_attr_destroy(&attr);
}

UdpPlatformThreadObject::UdpPlatformThreadObject()
{
    mThreadData = new UdpPlatformThreadData;
    mThreadData->handle = 0;
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
    if (bufferLen < 16)
    {
        *buffer = 0;
        return(buffer);
    }
    assert(buffer != nullptr);
    sprintf(buffer, "%d.%d.%d.%d", mData[0], mData[1], mData[2], mData[3]);
    return(buffer);
}

void UdpPlatformAddress::SetAddress(const char *address)
{
    for (int i = 0; i < 4; i++)
    {
        mData[i] = (unsigned char)strtol(address, nullptr, 10);
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
