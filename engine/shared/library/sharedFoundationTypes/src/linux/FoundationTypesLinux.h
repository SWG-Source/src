// PRIVATE.  Do not export this header file outside the package.

#ifndef INCLUDED_FoundationTypesLinux_H
#define INCLUDED_FoundationTypesLinux_H

// ======================================================================
// specify what platform we're running on.

#define PLATFORM_UNIX
#define PLATFORM_LINUX

#include <cstdio>
#include <stdint.h>
// ======================================================================
// basic types that we assume to be around

typedef uint32_t               uint;
typedef unsigned char          uint8;
typedef unsigned short         uint16;
typedef uint32_t               uint32;
typedef signed char            int8;
typedef signed short           int16;
typedef int32_t                int32;
typedef int64_t                int64;
typedef uint64_t               uint64;
typedef float                  real;
typedef FILE*                  FILE_HANDLE;

#endif


