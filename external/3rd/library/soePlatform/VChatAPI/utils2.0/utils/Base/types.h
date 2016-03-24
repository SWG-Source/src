#ifndef SOE__TYPES_H
#define SOE__TYPES_H


#define XSTRINGIFY(S) STRINGIFY(S)
#define STRINGIFY(S) #S

#ifdef linux
#include <sys/bitypes.h>
#endif

#define DECLSPEC
#define DECLIMPORT extern

#ifdef WIN32
#	define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#	define FILENAME __FILE__
#endif

// printf format specifier for 64 bit number since Windows and Linux differ
#ifdef WIN32
#define FMT_INT64 "%I64d"
#define FMT_UINT64 "%I64u"
#define atoi64(x) _atoi64(x)
#elif linux
#define FMT_INT64 "%lld"
#define FMT_UINT64 "%llu"
#define atoi64(x) atoll(x)
#endif


namespace soe
{
	typedef char                    int8;
	typedef unsigned char           uint8;
	typedef short                   int16;
	typedef unsigned short          uint16;

#ifdef WIN32
	typedef int                     int32;
	typedef unsigned                uint32;
	typedef __int64                 int64;
	typedef unsigned __int64        uint64;

#elif linux

	typedef int32_t                 int32;
	typedef u_int32_t               uint32;
	typedef int64_t                 int64;
	typedef u_int64_t               uint64;
//! the previous seem erroneous
//	typedef signed int              int32;
//	typedef unsigned int            uint32;
//	typedef signed long long        int64;
//	typedef unsigned long long      uint64;
#endif

}


#endif 


