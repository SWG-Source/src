#ifndef	BASE_PLATFORM_H
#define	BASE_PLATFORM_H

#include <assert.h>


#ifdef WIN32
    #include "win32/Platform.h"
#elif linux
   #include "linux/Platform.h"
#elif sparc
    #include "solaris/Platform.h"
#else
    #error /Base/Platform.h: Undefine platform type
#endif


#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{


    template <class T> inline T rotlFixed(T x, unsigned int y)
    {
	    assert(y < sizeof(T)*8);
	    return (T)((x<<y) | (x>>(sizeof(T)*8-y)));
    }

    template <class T> inline T rotrFixed(T x, unsigned int y)
    {
	    assert(y < sizeof(T)*8);
	    return (x>>y) | (x<<(sizeof(T)*8-y));
    }

    template <class T> inline T rotlMod(T x, unsigned int y)
    {
	    y %= sizeof(T)*8;
	    return (x<<y) | (x>>(sizeof(T)*8-y));
    }

    template <class T> inline T rotrMod(T x, unsigned int y)
    {
	    y %= sizeof(T)*8;
	    return (x>>y) | (x<<(sizeof(T)*8-y));
    }

    inline uint16 byteReverse16(void * data)
    {
        uint16 value = *static_cast<uint16 *>(data);
	    return *static_cast<uint16 *>(data) = rotlFixed(value, 8U);
	    //  return rotlFixed(value, 8U);
    }

    inline uint32 byteReverse32(void * data)
    {
        uint32 value = *static_cast<uint32 *>(data);
	    return *static_cast<uint32 *>(data) = (rotrFixed(value, 8U) & 0xff00ff00) | (rotlFixed(value, 8U) & 0x00ff00ff);
	    //  return (rotrFixed(value, 8U) & 0xff00ff00) | (rotlFixed(value, 8U) & 0x00ff00ff);
    }
    inline uint64 byteReverse64(void * data)
    {
        uint64 value = *static_cast<uint64 *>(data);
	    return *static_cast<uint64 *>(data) = (
            uint64((rotrFixed(uint32(value), 8U) & 0xff00ff00) | (rotlFixed(uint32(value), 8U) & 0x00ff00ff)) << 32) | 
            (rotrFixed(uint32(value>>32), 8U) & 0xff00ff00) | (rotlFixed(uint32(value>>32), 8U) & 0x00ff00ff);
	    //  return (uint64(byteReverse(uint32(value))) << 32) | byteReverse(uint32(value>>32));
    }

    inline uint32 strlen(const unsigned short * string)
    {
		if (string == 0)
			return 0;

        uint32 length=0;
        while (*(string+length++) != 0);

        return length-1;
    }

    inline double getTimerLatency(Base::uint64 startTime, Base::uint64 finishTime=0)
    {
        Base::uint64 requestAge;
        Base::uint64 finish = (finishTime ? finishTime : Base::getTimer());
        if (finish < startTime)
            requestAge = (0 - 1) - startTime - finish;
        else
            requestAge = finish - startTime;

        return (double)((unsigned)(requestAge)) / (double)((unsigned)Base::getTimerFrequency());
    }

};

#ifdef EXTERNAL_DISTRO
};
#endif


#endif	//  BASE_PLATFORM_H

