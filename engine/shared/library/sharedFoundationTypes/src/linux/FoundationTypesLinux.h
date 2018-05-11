// PRIVATE.  Do not export this header file outside the package.

#ifndef INCLUDED_FoundationTypesLinux_H
#define INCLUDED_FoundationTypesLinux_H

// ======================================================================
// specify what platform we're running on.

#define PLATFORM_UNIX
#define PLATFORM_LINUX

#include <cstdio>
#include <sys/bitypes.h>

// ======================================================================
// basic types that we assume to be around

typedef unsigned int           uint;
typedef unsigned char          uint8;
typedef unsigned short         uint16;
typedef unsigned long          uint32;
typedef signed char            int8;
typedef signed short           int16;
typedef signed long            int32;
typedef float                  real;
typedef FILE*                  FILE_HANDLE;

typedef int64_t                 int64;
typedef u_int64_t               uint64;

#endif


