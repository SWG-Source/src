#ifndef	BASE_LINUX_ARCHIVE_H
#define	BASE_LINUX_ARCHIVE_H

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif

namespace Base
{


#ifdef PACK_BIG_ENDIAN

    inline double byteSwap(double value)    { byteReverse(&value); return value; }
    inline float byteSwap(float value)      { byteReverse(&value); return value; }
    inline uint64 byteSwap(uint64 value)    { byteReverse(&value); return value; }
    inline int64 byteSwap(int64 value)      { byteReverse(&value); return value; }
    inline uint32 byteSwap(uint32 value)    { byteReverse(&value); return value; }
    inline int32 byteSwap(int32 value)      { byteReverse(&value); return value; }
    inline uint16 byteSwap(uint16 value)    { byteReverse(&value); return value; }
    inline int16 byteSwap(int16 value)      { byteReverse(&value); return value; }

#else

    inline double byteSwap(double value)    { return value; }
    inline float byteSwap(float value)      { return value; }
    inline uint64 byteSwap(uint64 value)    { return value; }
    inline int64 byteSwap(int64 value)      { return value; }
    inline uint32 byteSwap(uint32 value)    { return value; }
    inline int32 byteSwap(int32 value)      { return value; }
    inline uint16 byteSwap(uint16 value)    { return value; }
    inline int16 byteSwap(int16 value)      { return value; }

#endif


}
#ifdef EXTERNAL_DISTRO
};
#endif

#endif	
