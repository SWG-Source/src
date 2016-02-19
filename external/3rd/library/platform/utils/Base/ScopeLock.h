// ScopeLock.h: interface for the CScopeLock class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SCOPELOCK_H
#define SCOPELOCK_H

#if defined _MT || defined _REENTRANT
#   define INCLUDE_SCOPELOCK
#endif

#ifdef INCLUDE_SCOPELOCK

#include "Mutex.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{
    class CScopeLock  
    {
        public:
	        CScopeLock(CMutex& mutex);
	        CScopeLock(const CScopeLock& lock);
	        virtual ~CScopeLock();
        private:
            CMutex *mMutex;
    };
};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif  //  INCLUDE_SCOPELOCK

#endif  //  SCOPELOCK_H
