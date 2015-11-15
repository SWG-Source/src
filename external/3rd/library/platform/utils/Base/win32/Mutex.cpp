////////////////////////////////////////
//  Mutex.cpp
//
//  Purpose:
// 	    1. Implementation of the CMutex class.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//
#if !defined(_MT)
#   pragma message( "Excluding Base::CMutex - requires multi-threaded compile. (_MT)" ) 
#else

#include "Mutex.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

CMutex::CMutex()
    {
    InitializeCriticalSection(&mCriticalSection);
    }

CMutex::~CMutex()
    {
    DeleteCriticalSection(&mCriticalSection);
    }
};
#ifdef EXTERNAL_DISTRO
};
#endif


#endif  //  #if defined(_MT)

