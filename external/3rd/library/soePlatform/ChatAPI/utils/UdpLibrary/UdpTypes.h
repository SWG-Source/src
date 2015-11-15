#ifndef UDPLIBRARY_UDPTYPES_H
#define UDPLIBRARY_UDPTYPES_H

// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#include <assert.h>

namespace UdpLibrary
{

class UdpManager;
class UdpConnection;
class LogicalPacket;
class UdpPlatformAddress;

#if defined(WIN32)
    typedef __int64 udp_int64;
    #pragma warning(disable:4121)   // an error in microsofts compiler makes it think that pointers-to-members require 12 byte alignment, and since we are only doing 8 byte alignment by default, we get this warning.  Turns out that alignment is actually a non-issue for pointers-to-members since they are accessed only 4 bytes at a time. (found this info on the internet)
#else
    typedef long long udp_int64;
#endif

typedef unsigned char udp_uchar;
typedef unsigned short udp_ushort;
typedef unsigned int udp_uint;
typedef unsigned long udp_ulong;
typedef udp_int64 UdpClockStamp;
typedef UdpPlatformAddress UdpIpAddress;        // deprecated, use UdpPlatformAddress directly instead

enum UdpCorruptionReason { cUdpCorruptionReasonNone
                         , cUdpCorruptionReasonMultiPacket
                         , cUdpCorruptionReasonReliablePacketTooShort
                         , cUdpCorruptionReasonInternalPacketTooShort
                         , cUdpCorruptionReasonDecryptFailed 
                         , cUdpCorruptionReasonZeroLengthPacket
                         , cUdpCorruptionReasonPacketShorterThanCrcBytes
                         , cUdpCorruptionReasonMisformattedGroup
                         , cUdpCorruptionReasonFragmentOversized
                         , cUdpCorruptionReasonFragmentBad
                         , cUdpCorruptionReasonFragmentExpected
                         , cUdpCorruptionReasonAckBad
                         };


enum UdpChannel { cUdpChannelUnreliable                    // unreliable/unordered/buffered
                , cUdpChannelUnreliableUnbuffered        // unreliable/unordered/unbuffered
                , cUdpChannelOrdered                    // unreliable/ordered/buffered
                , cUdpChannelOrderedUnbuffered            // unreliable/ordered/unbuffered
                , cUdpChannelReliable1                    // reliable (as per channel config)
                , cUdpChannelReliable2                    // reliable (as per channel config)
                , cUdpChannelReliable3                    // reliable (as per channel config)
                , cUdpChannelReliable4                    // reliable (as per channel config)
                , cUdpChannelCount            // count of number of channels
                };

class UdpRefCount
{
    public:
        UdpRefCount();
        virtual void AddRef() const;
        virtual void Release() const;
        virtual void NoRef() const;
        virtual int GetRefCount() const;

    protected:
        virtual ~UdpRefCount();

        mutable int mRefCount;
        mutable bool mNoRef;
};

    /////////////////////////////////////////////////////////////////////////////////
    // UdpRefCount implementation (entirely inline)
    /////////////////////////////////////////////////////////////////////////////////

inline UdpRefCount::UdpRefCount() 
{ 
    mRefCount = 1;
    mNoRef = false; 
}  

inline UdpRefCount::~UdpRefCount() 
{ 
    assert(mRefCount == 0 || mNoRef);
}

inline void UdpRefCount::AddRef() const 
{ 
    assert(!mNoRef); 
    assert(mRefCount > 0); 
    ++mRefCount; 
}

inline void UdpRefCount::Release() const 
{ 
    assert(!mNoRef);
    assert(mRefCount > 0); 
    if (--mRefCount == 0) 
    {
        delete this;
    }
}

inline void UdpRefCount::NoRef() const
{ 
    assert(mRefCount == 1); 
    mNoRef = true; 
}

inline int UdpRefCount::GetRefCount() const
{
    return(mRefCount);
}


}   // namespace

#endif
