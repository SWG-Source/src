////////////////////////////////////////
//  Types.h
//
//  Purpose:
// 	    1. Define integer types that are unambiguous with respect to size
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#ifndef BASE_WIN32_TYPES_H
#define BASE_WIN32_TYPES_H

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

#define INT32_MAX  0x7FFFFFFF
#define INT32_MIN  0x80000000
#define UINT32_MAX 0xFFFFFFFF

typedef signed char             int8;
typedef unsigned char           uint8;
typedef short                   int16;
typedef unsigned short          uint16;

typedef int                     int32;
typedef unsigned                uint32;
typedef __int64                 int64;
typedef unsigned __int64        uint64;

};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif  //  BASE_WIN32_TYPES_H

