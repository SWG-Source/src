// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#include <assert.h>

#include "UdpLibrary.h"

namespace UdpLibrary
{

        /////////////////////////////////////////////////////
        // LogicalPacket implementation
        /////////////////////////////////////////////////////
LogicalPacket::LogicalPacket()
{
}

LogicalPacket::~LogicalPacket()
{
}

bool LogicalPacket::IsInternalPacket() const
{
    return(false);
}

        /////////////////////////////////////////////////////
        // SimpleLogicalPacket implementation
        /////////////////////////////////////////////////////
SimpleLogicalPacket::SimpleLogicalPacket(const void *data, int dataLen)
{
    mDataLen = dataLen;
    mData = new udp_uchar[mDataLen];
    if (data != nullptr)
        memcpy(mData, data, mDataLen);
}

SimpleLogicalPacket::~SimpleLogicalPacket()
{
    delete[] mData;
}

void *SimpleLogicalPacket::GetDataPtr()
{
    return(mData);
}

const void *SimpleLogicalPacket::GetDataPtr() const
{
    return(mData);
}

int SimpleLogicalPacket::GetDataLen() const
{
    return(mDataLen);
}

void SimpleLogicalPacket::SetDataLen(int len)
{
    assert(len <= mDataLen);
    mDataLen = len;
}
        /////////////////////////////////////////////////////
        // GroupLogicalPacket implementation
        /////////////////////////////////////////////////////
GroupLogicalPacket::GroupLogicalPacket() : LogicalPacket()
{
    mDataLen = 0;
    mData = nullptr;
}

GroupLogicalPacket::~GroupLogicalPacket()
{
    UdpMisc::SmartResize(mData, 0);        // free it
}

void GroupLogicalPacket::AddPacket(const LogicalPacket *packet)
{
    assert(packet != nullptr);
    AddPacketInternal(packet->GetDataPtr(), packet->GetDataLen(), packet->IsInternalPacket());
}

void GroupLogicalPacket::AddPacket(const void *data, int dataLen)
{
    assert(data != nullptr);
    assert(dataLen >= 0);
    AddPacketInternal(data, dataLen, false);
}

void GroupLogicalPacket::AddPacketInternal(const void *data, int dataLen, bool isInternalPacket)
{
    if (dataLen == 0)
        return;
    mData = (udp_uchar *)UdpMisc::SmartResize(mData, mDataLen + dataLen + 10, 512);        // 7 is the most bytes that could be needed to specify the length of the data to follow, 2 is for internal header-bytes, 1 is for zero-escape if needed (if they need to be added on)
    if (mDataLen == 0)
    {
        mData[0] = 0;
        mData[1] = UdpConnection::cUdpPacketGroup;
        mDataLen = 2;
    }

    udp_uchar *ptr = mData + mDataLen;
    if (!isInternalPacket && *(const udp_uchar *)data == 0)
    {
        ptr += UdpMisc::PutVariableValue(ptr, dataLen + 1);
        *ptr++ = 0;    // packet is not internal and starts with 0, so we need to zero-escape it so it knows it's an application packet
    }
    else
        ptr += UdpMisc::PutVariableValue(ptr, dataLen);

    memcpy(ptr, data, dataLen);
    ptr += dataLen;
    mDataLen = (int)(ptr - mData);
}

void *GroupLogicalPacket::GetDataPtr()
{
    return(mData);
}

const void *GroupLogicalPacket::GetDataPtr() const
{
    return(mData);
}

int GroupLogicalPacket::GetDataLen() const
{
    return(mDataLen);
}

void GroupLogicalPacket::SetDataLen(int /*len*/)
{
    assert(0);    // not allowed to set the len of a group logical packet
}

bool GroupLogicalPacket::IsInternalPacket() const
{
    return(true);
}

    ///////////////////////////////////////////////////////////////////////////////////////////
    // PooledLogicalPacket implementation
    ///////////////////////////////////////////////////////////////////////////////////////////
PooledLogicalPacket::PooledLogicalPacket(UdpManager *manager, int len)
{
    mMaxDataLen = len;
    mData = new udp_uchar[mMaxDataLen];
    mDataLen = 0;
    mUdpManager = manager;
    mUdpManager->PoolCreated(this);
}

PooledLogicalPacket::~PooledLogicalPacket()
{
    if (mUdpManager != nullptr)
    {
        mUdpManager->PoolDestroyed(this);
        mUdpManager = nullptr;
    }

    delete[] mData;
}

void PooledLogicalPacket::AddRef() const
{
    LogicalPacket::AddRef();
}

void PooledLogicalPacket::Release() const
{
    if (GetRefCount() == 1 && mUdpManager != nullptr)
    {
            // the PoolReturn function steals our reference (ie, we don't release, they don't addref), this is for thread safety reasons
        mUdpManager->PoolReturn(const_cast<PooledLogicalPacket *>(this));
        return;   // it would be non-thread-safe for us to touch this object in any way after is is added back to the pool, since some other thread might steal it before we get back to here (or if the pool didn't want us, we are potentially deleted at this point)
    }
    LogicalPacket::Release();
}

void PooledLogicalPacket::TrueRelease() const
{
      // this function is used by the UdpManager to delete this object in situations where it has decided that it does
      // not want the object returned to the pool (pool is full)
    LogicalPacket::Release();
}

void *PooledLogicalPacket::GetDataPtr()
{
    return(mData);
}

const void *PooledLogicalPacket::GetDataPtr() const
{
    return(mData);
}

int PooledLogicalPacket::GetDataLen() const
{
    return(mDataLen);
}

void PooledLogicalPacket::SetDataLen(int len)
{
    assert(len <= mMaxDataLen);
    mDataLen = len;
}

void PooledLogicalPacket::SetData(const void *data, int dataLen, const void *data2, int dataLen2)
{
    mDataLen = dataLen + dataLen2;
    if (data != nullptr)
        memcpy(mData, data, dataLen);
    if (data2 != nullptr)
        memcpy(mData + dataLen, data2, dataLen2);
}


}   // namespace
