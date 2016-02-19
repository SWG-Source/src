#include "ScopeLock.h"

#ifdef INCLUDE_SCOPELOCK

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

    CScopeLock::CScopeLock(CMutex& mutex) :
        mMutex(&mutex)
    {
        mMutex->Lock();
    }

    CScopeLock::CScopeLock(const CScopeLock& lock) :
        mMutex(lock.mMutex)
    {
        mMutex->Lock();
    }

    CScopeLock::~CScopeLock()
    {
        mMutex->Unlock();
    }
};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif  //  INCLUDE_SCOPELOCK
