////////////////////////////////////////
//  Types.h
//
//  Purpose:
// 	    1. Define integer types that are unambiguous with respect to size
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#ifndef BASE_LINUX_TYPES_H
#define BASE_LINUX_TYPES_H

#include <sys/bitypes.h>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif

namespace Base
{
//#define INT32_MAX  0x7FFFFFFF
//#define INT32_MIN  0x80000000
//#define UINT32_MAX 0xFFFFFFFF

typedef signed   char           int8;
typedef unsigned char           uint8;
typedef signed   short          int16;
typedef unsigned short          uint16;

typedef int32_t                 int32;
typedef u_int32_t               uint32;
typedef int64_t                 int64;
typedef u_int64_t               uint64;
}
#ifdef EXTERNAL_DISTRO
};
#endif
#endif  //  BASE_LINUX_TYPES_H

