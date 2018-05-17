// PRIVATE.  Do not export this header file outside the package.

// ======================================================================
//
// FoundationTypesWin32.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FoundationTypesWin32_H
#define INCLUDED_FoundationTypesWin32_H

// ======================================================================
// specify what platform we're running on.

#define PLATFORM_WIN32

// ======================================================================
// basic types that we assume to be around

typedef unsigned char          uint8;
typedef unsigned short         uint16;
typedef unsigned long          uint32;
typedef unsigned __int64       uint64;
typedef signed char            int8;
typedef signed short           int16;
typedef signed long            int32;
typedef signed __int64         int64;
typedef int                    FILE_HANDLE;

// ======================================================================

#endif
