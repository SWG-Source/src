// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "UdpLibrary.h"

namespace UdpLibrary
{

    ////////////////////////////////////////////////////////
    // UdpGuardedRefCount
    ////////////////////////////////////////////////////////
void UdpGuardedRefCount::AddRef() const
{
    UdpGuard guard(&mGuard);
    UdpRefCount::AddRef();
}

void UdpGuardedRefCount::Release() const 
{ 
    mGuard.Enter();
    assert(!mNoRef);
    assert(mRefCount > 0); 
    bool done = (--mRefCount == 0);
    mGuard.Leave();

    if (done)
    {
        delete this;
    }
}





}   // namespace
