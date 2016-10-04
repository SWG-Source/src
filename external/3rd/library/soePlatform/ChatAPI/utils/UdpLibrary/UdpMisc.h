#ifndef UDPLIBRARY_UDPMISC_H
#define UDPLIBRARY_UDPMISC_H

// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#include <stdlib.h>
#include <string.h>
#include "UdpDriver.h"

namespace UdpLibrary
{

class LogicalPacket;

class UdpMisc
{
        ////////////////////////////////////////////////////////////////////////////////////////////
        // This is a group of miscellaneous function used as part of the implementation
        // The application is free to use these helper functions as well if finds them useful.
        // they are stuck into this class so as to avoid conflicts with the application
        ////////////////////////////////////////////////////////////////////////////////////////////
    public:
        static int ClockDiff(UdpClockStamp start, UdpClockStamp stop);    // returns a time difference in milliseconds (if difference is over 23 days, it returns 23 days)
		static unsigned long int Crc32(const void *buffer, int bufferLen, int encryptValue = 0);                // calculate a 32-bit crc for a buffer (encrypt value simple scrambles the crc at the beginning so the same packet doesn't produce the same crc on different connections)
        static int Random(int *seed);                                // random number generator
        static void Sleep(int milliseconds);

                // returns the time difference between two sync-stamps that are based on the same clock
                // for example, two LocalSyncStampShort stamps can be compared.  Only differences up to
                // 32 seconds can be timed.  There is also a UdpConnect::ServerSyncStampShort function that returns
                // you a sync-stamp that you can compare with ServerSyncStampShort values generated on other 
                // machines that are synchronized against the same server.  This ServerSyncStampShort function and
                // this delta time function serve as the basis for calculating one-way travel times for packets.
        static int SyncStampShortDeltaTime(udp_ushort stamp1, udp_ushort stamp2);
        static int SyncStampLongDeltaTime(udp_uint stamp1, udp_uint stamp2);    // same as Short version only no limit

            // used to alloc/resize/free an allocation previously created with this function
            // rounding causes that it pre-allocates additional space up to the rounded buffer size
            // this allows the function to be called over and over again as tiny members are added, yet
            // only do an actual realloc periodically.  A 4-byte header is invisibly prepended on the
            // allocated block such that it can keep track of the actual size of the block.  The built-in
            // memory manager does a little bit of this, but if you know you have an allocation that is likely
            // to grow quite a bit, you can set the round'ing size up to a fairly large number and avoid
            // unnecessary reallocs at the cost of a little potentially wasted space
            // initial allocations are done by passing in ptr==nullptr, freeing is done by passing in bytes==0
        static void *SmartResize(void *ptr, int bytes, int round = 1);

            // the following two functions store values in the buffer as a variable length (B1, 0xffB2B1, 0xffffffB4B3B2B1)
            // such that values under 254 take one byte, values under 65535 take three bytes, and larger values take seven bytes
        static udp_uint PutVariableValue(void *buffer, udp_uint value);            // returns the number of bytes it took to store the value in the buffer
        static udp_uint GetVariableValue(const void *buffer, udp_uint *value);    // returns the number of bytes it took to get the value from the buffer

            // these functions are used to aid in portability and serve to ensure that the packet-headers are interpretted in the same
            // manner on all platforms
        static int PutValue64(void *buffer, udp_int64 value);       // puts a 64-bit value into the buffer in big-endian format, returns number of bytes used(8)
        static int PutValue32(void *buffer, udp_uint value);        // puts a 32-bit value into the buffer in big-endian format, returns number of bytes used(4)
        static int PutValue24(void *buffer, udp_uint value);        // puts a 24-bit value into the buffer in big-endian format, returns number of bytes used(4)
        static int PutValue16(void *buffer, udp_ushort value);      // puts a 16-bit value into the buffer in big-endian format, returns number of bytes used(2)
        static int PutValueLE32(void *buffer, udp_uint value);      // puts a 32-bit value in little-endian format

        static udp_int64 GetValue64(const void *buffer);            // gets a 64-bit value from the buffer in big-endian format
        static udp_uint GetValue32(const void *buffer);             // gets a 32-bit value from the buffer in big-endian format
        static udp_uint GetValue24(const void *buffer);             // gets a 24-bit value from the buffer in big-endian format
        static udp_ushort GetValue16(const void *buffer);           // gets a 16-bit value from the buffer in big-endian format

        static LogicalPacket *CreateQuickLogicalPacket(const void *data, int dataLen, const void *data2 = nullptr, int dataLen2 = 0);

            // looks up the specified name and translates it to an IP address
            // this is a blocking call that can at times take a significant amount of time, but will generally be fast (less than 300ms)
        static UdpPlatformAddress GetHostByName(const char *hostName);
        static UdpPlatformAddress GetSelfAddress(bool preferNoRoute);

        static char *Strncpy(char *dest, const char *source, int len);
        static char *Strncat(char *dest, const char *source, int len);
};



    ////////////////////////////////////////////////////////////////////////////
    // inline implementations
    ////////////////////////////////////////////////////////////////////////////

       // UdpMisc
inline int UdpMisc::ClockDiff(UdpClockStamp start, UdpClockStamp stop)
{
    UdpClockStamp t = (stop - start);
    if (t > 0x7fffffff)        // only time differences up to about 23 days can be measured with this function
        return(0x7fffffff);
    return((int)t);
}

inline int UdpMisc::PutValue64(void *buffer, udp_int64 value)
{
    udp_uchar *bufptr = (udp_uchar *)buffer;
    *bufptr++ = (udp_uchar)(value >> 56);
    *bufptr++ = (udp_uchar)((value >> 48) & 0xff);
    *bufptr++ = (udp_uchar)((value >> 40) & 0xff);
    *bufptr++ = (udp_uchar)((value >> 32) & 0xff);
    *bufptr++ = (udp_uchar)((value >> 24) & 0xff);
    *bufptr++ = (udp_uchar)((value >> 16) & 0xff);
    *bufptr++ = (udp_uchar)((value >> 8) & 0xff);
    *bufptr = (udp_uchar)(value & 0xff);
    return(8);
}

inline udp_int64 UdpMisc::GetValue64(const void *buffer)
{
    const udp_uchar *bufptr = (const udp_uchar *)buffer;
    return(((udp_int64)*bufptr << 56) | ((udp_int64)*(bufptr + 1) << 48) | ((udp_int64)*(bufptr + 2) << 40) | ((udp_int64)*(bufptr + 3) << 32) | ((udp_int64)*(bufptr + 4) << 24) | ((udp_int64)*(bufptr + 5) << 16) | ((udp_int64)*(bufptr + 6) << 8) | (udp_int64)*(bufptr + 7));
}

inline int UdpMisc::PutValue32(void *buffer, udp_uint value)
{
    udp_uchar *bufptr = (udp_uchar *)buffer;
    *bufptr++ = (udp_uchar)(value >> 24);
    *bufptr++ = (udp_uchar)((value >> 16) & 0xff);
    *bufptr++ = (udp_uchar)((value >> 8) & 0xff);
    *bufptr = (udp_uchar)(value & 0xff);
    return(4);
}

inline int UdpMisc::PutValueLE32(void *buffer, udp_uint value)
{
    udp_uchar *bufptr = (udp_uchar *)buffer;
    *bufptr++ = (udp_uchar)(value & 0xff);
    *bufptr++ = (udp_uchar)((value >> 8) & 0xff);
    *bufptr++ = (udp_uchar)((value >> 16) & 0xff);
    *bufptr = (udp_uchar)(value >> 24);
    return(4);
}

inline udp_uint UdpMisc::GetValue32(const void *buffer)
{
    const udp_uchar *bufptr = (const udp_uchar *)buffer;
    return((*bufptr << 24) | (*(bufptr + 1) << 16) | (*(bufptr + 2) << 8) | *(bufptr + 3));
}

inline int UdpMisc::PutValue24(void *buffer, udp_uint value)
{
    udp_uchar *bufptr = (udp_uchar *)buffer;
    *bufptr++ = (udp_uchar)((value >> 16) & 0xff);
    *bufptr++ = (udp_uchar)((value >> 8) & 0xff);
    *bufptr = (udp_uchar)(value & 0xff);
    return(3);
}

inline udp_uint UdpMisc::GetValue24(const void *buffer)
{
    const udp_uchar *bufptr = (const udp_uchar *)buffer;
    return((*bufptr << 16) | (*(bufptr + 1) << 8) | *(bufptr + 2));
}

inline int UdpMisc::PutValue16(void *buffer, udp_ushort value)
{
    udp_uchar *bufptr = (udp_uchar *)buffer;
    *bufptr++ = (udp_uchar)((value >> 8) & 0xff);
    *bufptr = (udp_uchar)(value & 0xff);
    return(2);
}

inline udp_ushort UdpMisc::GetValue16(const void *buffer)
{
    const udp_uchar *bufptr = (const udp_uchar *)buffer;
    return((udp_ushort)((*bufptr << 8) | *(bufptr + 1)));
}

inline char *UdpMisc::Strncpy(char *dest, const char *source, int len)
{
    if (len > 0)
    {
        char *walk = dest;
        char *end = dest + len - 1;
        while (*source != 0 && walk < end)
        {
            *walk++ = *source++;
        }
        *walk = 0;
    }
    return(dest);
}

inline char *UdpMisc::Strncat(char *dest, const char *source, int len)
{
    int clen = (int)strlen(dest);
    len -= clen;
    if (len > 1)
    {
        Strncpy(dest + clen, source, len);
    }
    return(dest);
}

template <typename ValueType> const ValueType &udpMax(const ValueType &a, const ValueType &b)
{
    if (a < b)
        return b;
    return a;
}

template<typename ValueType> const ValueType &udpMin(const ValueType &a, const ValueType &b)
{
    if (b < a)
        return b;
    return a;
}

}   // namespace

#endif
