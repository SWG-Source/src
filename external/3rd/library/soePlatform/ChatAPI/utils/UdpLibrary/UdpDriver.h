#ifndef UDPLIBRARY_UDPDRIVER_H
#define UDPLIBRARY_UDPDRIVER_H

#include "UdpTypes.h"

namespace UdpLibrary
{

// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

class UdpDriver
{
    public:
        virtual ~UdpDriver() {}

            // socket related functions
        virtual bool SocketOpen(int port, int incomingBufferSize, int outgoingBufferSize, const char *bindIpAddress) = 0;      // returns true if successful, false if it could not allocate/bind the socket to specified port
        virtual void SocketClose() = 0;
        virtual int SocketReceive(char *buffer, int bufferSize, UdpPlatformAddress *ipAddress, int *port) = 0;      // returns bytes read, -1 if no data, 0 for ICMP errors (from ip/port is filled in with who got the error)
        virtual bool SocketSend(const char *data, int dataLen, const UdpPlatformAddress *ipAddress, int port) = 0;
        virtual void SocketSendPortAlive(const char *data, int dataLen, const UdpPlatformAddress *ipAddress, int port) = 0;
        virtual bool SocketGetLocalIp(UdpPlatformAddress *ipAddress) = 0;
        virtual int SocketGetLocalPort() = 0;

            // non-socket related functions
        virtual bool GetHostByName(UdpPlatformAddress *ipAddress, const char *hostName) = 0;
        virtual bool GetSelfAddress(UdpPlatformAddress *ipAddress, bool preferNoRoute) = 0;
        virtual void Sleep(int milliseconds) = 0;
        virtual UdpClockStamp Clock() = 0;
};

class UdpPlatformData;
class UdpPlatformDriver : public UdpDriver
{
    public:
        UdpPlatformDriver();
        virtual ~UdpPlatformDriver();

            // socket related functions
        virtual bool SocketOpen(int port, int incomingBufferSize, int outgoingBufferSize, const char *bindIpAddress);
        virtual void SocketClose();
        virtual int SocketReceive(char *buffer, int bufferSize, UdpPlatformAddress *ipAddress, int *port);
        virtual bool SocketSend(const char *data, int dataLen, const UdpPlatformAddress *ipAddress, int port);
        virtual void SocketSendPortAlive(const char *data, int dataLen, const UdpPlatformAddress *ipAddress, int port);
        virtual bool SocketGetLocalIp(UdpPlatformAddress *ipAddress);
        virtual int SocketGetLocalPort();

            // non-socket related functions that only the platform driver has
        virtual bool GetHostByName(UdpPlatformAddress *ipAddress, const char *hostName);
        virtual bool GetSelfAddress(UdpPlatformAddress *ipAddress, bool preferNoRoute);
        virtual void Sleep(int milliseconds);
        virtual UdpClockStamp Clock();

    private:
        UdpPlatformData *mData;
};

class UdpPlatformGuardData;
class UdpPlatformGuardObject
{
    public:
        UdpPlatformGuardObject();
        ~UdpPlatformGuardObject();
        void Enter();
        void Leave();

    private:
        UdpPlatformGuardData *mData;
};

class UdpGuardedRefCount : public UdpRefCount
{
        // portable: implemented in UdpLibrary.cpp
    public:
        virtual void AddRef() const;
        virtual void Release() const;   

    protected:
        mutable UdpPlatformGuardObject mGuard;
};

class UdpPlatformThreadData;
class UdpPlatformThreadObject : public UdpGuardedRefCount
{
    public:
        UdpPlatformThreadObject();

        virtual void Start();        // starts thread running (can't do this in constructor because we need to give derived classes time to finish constructing)
        virtual bool IsRunning();
        virtual void Run() = 0;

    protected:
        virtual ~UdpPlatformThreadObject();

    public:
        UdpPlatformThreadData *mThreadData;
};

class UdpPlatformAddress
{
    public:
        UdpPlatformAddress();
        UdpPlatformAddress(const UdpPlatformAddress &source);
        bool operator==(const UdpPlatformAddress &e) const;
        UdpPlatformAddress &operator=(const UdpPlatformAddress &e);
        char *GetAddress(char *buffer, int bufferLen) const;
        void SetAddress(const char *address);
        int GetHash();

    protected:
        friend class UdpPlatformDriver;
            // note: platforms are required to be able to store their representation of the address in these 4 bytes
            // if we come across a platform that needs more space than this (or when we start doing IPv6), then we will
            // increase this space a bit.  This is sufficient for every platform so far and it avoids us having to
            // do an allocation, particularly since these things are passed around by-value a lot.
        unsigned char mData[4];
};



}   // namespace

#endif
