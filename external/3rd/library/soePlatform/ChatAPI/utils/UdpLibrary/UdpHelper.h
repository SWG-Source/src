#ifndef UDPLIBRARY_UDPHELPER_H
#define UDPLIBRARY_UDPHELPER_H

// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

namespace UdpLibrary
{

class UdpGuard
{
    public:
        UdpGuard(UdpPlatformGuardObject *obj) { mObj = obj; mObj->Enter(); }
        ~UdpGuard() { mObj->Leave(); }
    protected:
        UdpPlatformGuardObject *mObj;
};

class UdpRef
{
    public:
        UdpRef(UdpRefCount *obj) { mObj = obj; mObj->AddRef(); }
        ~UdpRef() { mObj->Release(); }

    private:
        UdpRefCount *mObj;
};

} // namespace

#endif
