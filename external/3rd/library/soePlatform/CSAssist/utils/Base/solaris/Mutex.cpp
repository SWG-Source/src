////////////////////////////////////////
//  Mutex.cpp
//
//  Purpose:
// 	    1. Implementation of the CMutex class.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#if defined(_REENTRANT)


#include "Mutex.h"

namespace Base
{

CMutex::CMutex()
    {
	mInitialized = (pthread_mutex_init(&mMutex, 0) == 0);
    }

CMutex::~CMutex()
    {
	if (mInitialized)
        pthread_mutex_destroy(&mMutex);
    }

}

#endif  //  #if defined(_REENTRANT)
