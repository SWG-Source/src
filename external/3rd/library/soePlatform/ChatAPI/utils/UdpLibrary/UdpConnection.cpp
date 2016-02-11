// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#if defined(WIN32)
	#define _CRT_SECURE_NO_DEPRECATE		// gets rid of deprecation warnings in VS 2005 (don't want to change to secure-versions as it hampers portability)
#endif

#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "UdpLibrary.h"
#include "UdpReliableChannel.h"

namespace UdpLibrary
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // UdpConnectionHandler default implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////////
void UdpConnectionHandler::OnConnectComplete(UdpConnection * /*con*/)
{
}

void UdpConnectionHandler::OnTerminated(UdpConnection * /*con*/)
{
}

void UdpConnectionHandler::OnCrcReject(UdpConnection * /*con*/, const udp_uchar * /*data*/, int /*dataLen*/)
{
}

void UdpConnectionHandler::OnPacketCorrupt(UdpConnection * /*con*/, const udp_uchar * /*data*/, int /*dataLen*/, UdpCorruptionReason /*reason*/)
{
}



    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // UdpConnection implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////////

UdpConnection::UdpConnection(UdpManager *udpManager, UdpPlatformAddress destIp, int destPort, int timeout)
{
    UdpRef ref(this);
    UdpGuard myGuard(&mGuard);

        // client side initializations
    Init(udpManager, destIp, destPort);

    mConnectAttemptTimeout = timeout;
    mStatus = cStatusNegotiating;
    mConnectCode = mUdpManager->Random();
    mUdpManager->AddConnection(this);

    GiveTime(false);
}

UdpConnection::UdpConnection(UdpManager *udpManager, const UdpManager::PacketHistoryEntry *e)
{
    UdpRef ref(this);
    UdpGuard myGuard(&mGuard);

        // server side initialization
    Init(udpManager, e->mIp, e->mPort);

    mStatus = cStatusConnected;
    for (int j = 0; j < cEncryptPasses; j++)
        mConnectionConfig.encryptMethod[j] = mUdpManager->mParams.encryptMethod[j];
    mConnectionConfig.crcBytes = mUdpManager->mParams.crcBytes;
    mConnectionConfig.maxRawPacketSize = mUdpManager->mParams.maxRawPacketSize;
    mConnectionConfig.encryptCode = mUdpManager->Random();
    SetupEncryptModel();

        // steal the connect code out of the packet early such that ProcessRawPacket will think it's a valid connect packet instead of ignoring it
        // plus, the AddConnection function needs to know our connect code
    mConnectCode = UdpMisc::GetValue32(e->mBuffer + 6);
    mUdpManager->AddConnection(this);

    ProcessRawPacket(e);
    GiveTime(false);
}

void UdpConnection::Init(UdpManager *udpManager, UdpPlatformAddress destIp, int destPort)
{
    mUdpManager = udpManager;
    mIp = destIp;
    mPort = destPort;

    mFlaggedPortUnreachable = false;

    mLastPortAliveTime = mLastSendTime = 0;            // makes it send out the first connect packet immediately (if we are in negotiating mode)
    mLastReceiveTime = mUdpManager->CachedClock();
    mLastClockSyncTime = 0;
    mDataHoldTime = 0;
    mGettingTime = false;
    mHandler = nullptr;
    mOtherSideProtocolVersion = 0;

    mNoDataTimeout = mUdpManager->mParams.noDataTimeout;
    mKeepAliveDelay = mUdpManager->mParams.keepAliveDelay;

    mMultiBufferData = new udp_uchar[mUdpManager->mParams.maxRawPacketSize];
    mMultiBufferPtr = mMultiBufferData;

    mIcmpErrorRetryStartStamp = 0;        // when the timer started for ICMP error retry delay (gets reset on a successful packet receive)
    mPortRemapRequestStartStamp = 0;

    mEncryptXorBuffer = nullptr;
    mEncryptExpansionBytes = 0;
    mOrderedCountOutgoing = 0;
    mOrderedCountOutgoing2 = 0;
    mOrderedStampLast = 0;
    mOrderedStampLast2 = 0;
    mDisconnectReason = cDisconnectReasonNone;
    mOtherSideDisconnectReason = cDisconnectReasonNone;

    mConnectAttemptTimeout = 0;
    mConnectionCreateTime = mUdpManager->CachedClock();
    mSimulateOutgoingQueueBytes = 0;
    mPassThroughData = nullptr;
    mSilentDisconnect = false;

    mLastSendBin = 0;
    mLastReceiveBin = 0;
    mOutgoingBytesLastSecond = 0;
    mIncomingBytesLastSecond = 0;
    memset(mSendBin, 0, sizeof(mSendBin));
    memset(mReceiveBin, 0, sizeof(mReceiveBin));

    PingStatReset();
    mSyncTimeDelta = 0;
    memset(mChannel, 0, sizeof(mChannel));
    memset(&mConnectionStats, 0, sizeof(mConnectionStats));
}

UdpConnection::~UdpConnection()
{
    UdpGuard myGuard(&mGuard);

    assert(mUdpManager == nullptr);    // this should not be possible, since the UdpManager holds a reference to us until we are disconnected and disassociated from the manager.  If you are hitting this, then odds are the application is releasing the UdpConnection object more times than it should

    for (int i = 0; i < cReliableChannelCount; i++)
        delete mChannel[i];
    delete[] mMultiBufferData;
    delete[] mEncryptXorBuffer;
}

void UdpConnection::PortUnreachable()
{
    if (!mUdpManager->mParams.processIcmpErrors)
        return;

    if (!mUdpManager->mParams.processIcmpErrorsDuringNegotiating)
    {
        if (mStatus == cStatusNegotiating)        // during negotiating phase, ignore port unreachable errors, since it may be a case of the client starting up first
            return;
    }

    if (mUdpManager->mParams.icmpErrorRetryPeriod != 0)
    {
        if (mIcmpErrorRetryStartStamp == 0)
        {
            mIcmpErrorRetryStartStamp = mUdpManager->CachedClock();        // start timer on how long we will ignore ICMP errors
            return;
        }

        if (mUdpManager->CachedClockElapsed(mIcmpErrorRetryStartStamp) < mUdpManager->mParams.icmpErrorRetryPeriod)
        {
            return;        // ignoring ICMP errors for a period of time
        }
    }

    InternalDisconnect(0, cDisconnectReasonIcmpError);
}

void UdpConnection::InternalDisconnect(int flushTimeout, DisconnectReason reason)
{
    UdpGuard guard(&mGuard);    // must guard this because the UdpManager can potentially call this in a worker thread while we are inside of a Send call from the main thread, causing us to be disconnected mid-processing on the Send call

    if (mDisconnectReason == cDisconnectReasonNone)
    {
        mDisconnectReason = reason;
    }

        // if we are in a negotiating state, then you can't have a flushTimeout, any disconnect will occur immediately
    if (mStatus == cStatusNegotiating)
        flushTimeout = 0;

    if (mUdpManager != nullptr)
    {
        if (flushTimeout > 0)
        {
            FlushMultiBuffer();
            mDisconnectFlushStamp = mUdpManager->CachedClock();
            mDisconnectFlushTimeout = flushTimeout;
            ScheduleTimeNow();

            if (mStatus != cStatusDisconnectPending)
            {
                mStatus = cStatusDisconnectPending;
                mUdpManager->KeepUntilDisconnected(this);
            }
            return;
        }

            // send a termination packet to the other side
            // do not send a termination packet if we are still negotiating (we are not allowed to send any packets while negotiating)
            // if you attempt to send a packet while negotiating, then it will potentially attempt to encrypt it before an encryption
            // method is determined, resulting in a function call through an invalid pointer
        if (!mSilentDisconnect)
        {
            if (mStatus == cStatusConnected || mStatus == cStatusDisconnectPending)
            {
                SendTerminatePacket(mConnectCode, mDisconnectReason);
            }
        }

        UdpManager *holdUdpManager = mUdpManager;
        mUdpManager = nullptr;
        mStatus = cStatusDisconnected;

            // only hold a reference to the UdpManager if it is not currently being destructed.
            // if it is currently being destructed, we obviously can't addref it.  We know it
            // is being destructed as our disconnect reason will be ManagerDeleted.  Since we
            // can only get this reason if we are called from the UdpManager destructor, it is
            // safe to call the UdpManager without reffing it.  The UdpManager is setup such
            // that the RemoveConnection call is safe, even after the destuctor has started
            // running.
        if (reason != cDisconnectReasonManagerDeleted)
        {
            holdUdpManager->AddRef();
        }

        holdUdpManager->RemoveConnection(this);

        if (reason != cDisconnectReasonManagerDeleted)
        {
            holdUdpManager->CallbackTerminated(this);
            holdUdpManager->Release();
        }
    }
}

void UdpConnection::SendTerminatePacket(int connectCode, DisconnectReason reason)
{
    udp_uchar buf[256];
    buf[0] = 0;
    buf[1] = cUdpPacketTerminate;
    UdpMisc::PutValue32(buf + 2, connectCode);
    UdpMisc::PutValue16(buf + 6, (udp_ushort)reason);
    PhysicalSend(buf, 8, true);
}


void UdpConnection::SetSilentDisconnect(bool silent)
{
    UdpGuard guard(&mGuard);
        // this function tells the connection to disconnect silently, meaning that it should
        // not send a packet to the other side telling it that the connection is being terminated
    mSilentDisconnect = silent;
}

bool UdpConnection::Send(UdpChannel channel, const void *data, int dataLen)
{
    UdpGuard myGuard(&mGuard);

    assert(dataLen >= 0);

    if (mStatus != cStatusConnected)    // if we are no longer connected (not allowed to send more when we are pending disconnect either)
        return(false);
    if (dataLen == 0)        // zero length packets are ignored
        return(false);

    assert(data != nullptr);        // can't send a nullptr packet

        // zero-escape application packets that start with 0
    if ((*(const udp_uchar *)data) == 0)
    {
        udp_uchar hold = 0;
        return(InternalSend(channel, &hold, 1, (const udp_uchar *)data, dataLen));
    }

    return(InternalSend(channel, (const udp_uchar *)data, dataLen));
}

bool UdpConnection::Send(UdpChannel channel, const LogicalPacket *packet)
{
    UdpGuard myGuard(&mGuard);

    assert(packet != nullptr);        // can't send a nullptr packet

    if (mStatus != cStatusConnected)    // if we are no longer connected
        return(false);
    int dataLen = packet->GetDataLen();
    if (dataLen == 0)
        return(false);

        // zero-escape application packets that start with 0
    const udp_uchar *data = (const udp_uchar *)packet->GetDataPtr();
    if (!packet->IsInternalPacket() && data[0] == 0)
    {
        udp_uchar hold = 0;
        return(InternalSend(channel, &hold, 1, data, dataLen));
    }

    return(InternalSend(channel, data, dataLen));
}

bool UdpConnection::InternalSend(UdpChannel channel, const udp_uchar *data, int dataLen, const udp_uchar *data2, int dataLen2)
{
    assert(channel >= 0 && channel < cUdpChannelCount);
    assert(mStatus != cStatusNegotiating);    // you are not allowed to start sending data on a connection that is still in the process of negotiating (only applicable client-side obviously since servers never have connections in this state)

    mUdpManager->IncrementApplicationPacketsSent();
    mConnectionStats.applicationPacketsSent++;

        // promote unreliable packets that are larger than maxRawPacketSize to be reliable
    int totalDataLen = dataLen + dataLen2;

    int rawDataBytesMax = (mConnectionConfig.maxRawPacketSize - mConnectionConfig.crcBytes - mEncryptExpansionBytes);
    if ((channel == cUdpChannelUnreliable || channel == cUdpChannelUnreliableUnbuffered) && totalDataLen > rawDataBytesMax)
        channel = cUdpChannelReliable1;
    else if ((channel == cUdpChannelOrdered || channel == cUdpChannelOrderedUnbuffered) && totalDataLen > rawDataBytesMax - cUdpPacketOrderedSize)
        channel = cUdpChannelReliable1;

    udp_uchar tempBuffer[UdpManager::cHardMaxRawPacketSize];
    switch(channel)
    {
        case cUdpChannelUnreliable:
            BufferedSend(data, dataLen, data2, dataLen2, false);
            return(true);
            break;
        case cUdpChannelUnreliableUnbuffered:
        {
            udp_uchar *bufPtr = tempBuffer;
            memcpy(bufPtr, data, dataLen);
            if (data2 != nullptr)
                memcpy(bufPtr + dataLen, data2, dataLen2);
            PhysicalSend(bufPtr, totalDataLen, true);
            return(true);
            break;
        }
        case cUdpChannelOrdered:
        {
            udp_uchar *bufPtr = tempBuffer;
            bufPtr[0] = 0;
            bufPtr[1] = cUdpPacketOrdered;
            UdpMisc::PutValue16(bufPtr + 2, (udp_ushort)(++mOrderedCountOutgoing & 0xffff));
            memcpy(bufPtr + 4, data, dataLen);
            if (data2 != nullptr)
                memcpy(bufPtr + 4 + dataLen, data2, dataLen2);
            BufferedSend(bufPtr, totalDataLen + 4, nullptr, 0, true);
            return(true);
            break;
        }
        case cUdpChannelOrderedUnbuffered:
        {
            udp_uchar *bufPtr = tempBuffer;
            bufPtr[0] = 0;
            bufPtr[1] = cUdpPacketOrdered2;
            UdpMisc::PutValue16(bufPtr + 2, (udp_ushort)(++mOrderedCountOutgoing2 & 0xffff));
            memcpy(bufPtr + 4, data, dataLen);
            if (data2 != nullptr)
                memcpy(bufPtr + 4 + dataLen, data2, dataLen2);
            PhysicalSend(bufPtr, totalDataLen + 4, true);
            return(true);
            break;
        }
        case cUdpChannelReliable1:
        case cUdpChannelReliable2:
        case cUdpChannelReliable3:
        case cUdpChannelReliable4:
        {
            int num = channel - cUdpChannelReliable1;
            if (mChannel[num] == nullptr)
                mChannel[num] = new UdpReliableChannel(num, this, &mUdpManager->mParams.reliable[num]);
            mChannel[num]->Send(data, dataLen, data2, dataLen2);
            return(true);
            break;
        }
        default:
            break;
    }
    return(false);
}

void UdpConnection::PingStatReset()
{
    UdpGuard myGuard(&mGuard);

    mLastClockSyncTime = 0; // tells it to resync the clock pronto
    mSyncStatMasterFixupTime = 0;
    mSyncStatMasterRoundTime = 0;
    mSyncStatLow = 0;
    mSyncStatHigh = 0;
    mSyncStatLast = 0;
    mSyncStatTotal = 0;
    mSyncStatCount = 0;
    mConnectionStats.averagePingTime = 0;
    mConnectionStats.highPingTime = 0;
    mConnectionStats.lowPingTime = 0;
    mConnectionStats.lastPingTime = 0;
    mConnectionStats.masterPingTime = 0;
}

void UdpConnection::GetStats(UdpConnectionStatistics *cs)
{
    UdpGuard myGuard(&mGuard);

    assert(cs != nullptr);

    if (mUdpManager == nullptr)
        return;
    *cs = mConnectionStats;

    if (mUdpManager->mParams.clockSyncDelay == 0)
        cs->masterPingAge = -1;
    else
        cs->masterPingAge = mUdpManager->CachedClockElapsed(mSyncStatMasterFixupTime);

    cs->percentSentSuccess = 1.0;
    cs->percentReceivedSuccess = 1.0;
    if (cs->syncOurSent > 0)
        cs->percentSentSuccess = (float)cs->syncTheirReceived / (float)cs->syncOurSent;
    if (cs->syncTheirSent > 0)
        cs->percentReceivedSuccess = (float)cs->syncOurReceived / (float)cs->syncTheirSent;
    cs->reliableAveragePing = 0;
    if (mChannel[0] != nullptr)
        cs->reliableAveragePing = mChannel[0]->GetAveragePing();
}

void UdpConnection::ProcessRawPacket(const UdpManager::PacketHistoryEntry *e)
{
    UdpRef ref(this);
    UdpGuard guard(&mGuard);

    if (mUdpManager == nullptr)
        return;

    if (e->mLen == 0)
    {
            // length 0 packet indicates an ICMP dest-unreachable error packet on this connection (the driver puts these errors inline as 0 byte packets)
        PortUnreachable();
        return;
    }

    if (e->mBuffer[0] != 0 || e->mBuffer[1] != cUdpPacketUnreachableConnection)
    {
            // if we get any type of packet other than an unreachable-connection packet, then we can assume that our remapping
            // request succeeded, and clear the timer for how long we should attempt to do the remapping.  The reason we need
            // to send requests for a certain amount of time, is the server may already have dozens of unreachable-connection packets
            // on the wire on the way to us, before we manage to request that the remapping occur.
        mPortRemapRequestStartStamp = 0;
    }

    mIcmpErrorRetryStartStamp = 0;        // we received a packet successfully, so assume we have recovered from any ICMP error state we may have been in, so we can reset the timer
    mLastReceiveTime = mUdpManager->CachedClock();
    mConnectionStats.totalPacketsReceived++;
    mConnectionStats.totalBytesReceived += e->mLen;

        // track incoming data rate
    ExpireReceiveBin();
    mReceiveBin[mLastReceiveBin % cBinCount] += e->mLen;
    mIncomingBytesLastSecond += e->mLen;

    if (e->mBuffer[0] == 0 && e->mBuffer[1] == cUdpPacketKeepAlive)
    {
            // encryption can't mess up the first two bytes of an internal packet, so this is safe to check
            // if it is a keep alive packet, then we don't need to do any more processing beyond setting
            // the mLastReceiveTime.  We do this check here instead of letting it pass on through harmlessly
            // like we used to do in order to avoid getting rescheduled in the priority queue.  There is absolutely
            // no reason to reschedule us due to an incoming keep alive packet since the keep-alive packet has the
            // longest rescheduling of anything that needs time, so the worst thing that might happen is we might
            // end up getting sheduled time sooner than we might otherwise need to.  And obviously scheduling
            // ourselves for immediate-time is even sooner than that, so there is no point.
            // This turns out to be important for applications that have lots of connections (tens of thousands)
            // that rarely talk but send keep alives...no reason to make the server do a lot work over these things.
        return;
    }

        // whenever we receive a packet, it could potentially change when we want time scheduled again
        // so effectively we should reprioritize ourself to the top.  By doing it this way instead of
        // simply giving time and recalculating, we can effectively avoid giving ourself time and reprioritizing
        // ourself over and over again as more and more packets arrive in rapid succession
        // note: this cannot happen while we are in our UdpConnection::GiveTime function, so there is no need to squeltch check 
        // it like we do the others.
        // note: this was moved to the top of the function from the bottom.  This doesn't effect anything as it doesn't matter
        // when we schedule ourself for future processing.  Moving it to the top allowed us to get scheduled even if the packet
        // we processed got rejected for some reason (crc mismatch or bad size).
    ScheduleTimeNow();

    if (e->mLen < 1)
    {
        CallbackCorruptPacket(e->mBuffer, e->mLen, cUdpCorruptionReasonZeroLengthPacket);
        return;        // invalid packet len
    }

        // first see if we are a special connect/confirm/unreachable packet, if so, process us immediately
    if (IsNonEncryptPacket(e->mBuffer))
    {
        ProcessCookedPacket(e->mBuffer, e->mLen);
    }
    else
    {
            // if we are still awaiting confirmation packet, then we must ignore any other incoming data packets
            // this can happen if the confirm packet is lost and the server has dumped a load of data on the newly created connection
        if (mStatus == cStatusNegotiating)
            return;

        udp_uchar *finalStart = e->mBuffer;
        int finalLen = e->mLen;

        if (mConnectionConfig.crcBytes > 0)
        {
            if (finalLen < mConnectionConfig.crcBytes)
            {
                CallbackCorruptPacket(e->mBuffer, e->mLen, cUdpCorruptionReasonPacketShorterThanCrcBytes);
                return;        // invalid packet len
            }

            udp_uchar *crcPtr = finalStart + (finalLen - mConnectionConfig.crcBytes);
            int actualCrc = UdpMisc::Crc32(finalStart, finalLen - mConnectionConfig.crcBytes, mConnectionConfig.encryptCode);
            int wantCrc = 0;
            switch(mConnectionConfig.crcBytes)
            {
                case 1:
                    wantCrc = *crcPtr;
                    actualCrc &= 0xff;
                    break;
                case 2:
                    wantCrc = UdpMisc::GetValue16(crcPtr);
                    actualCrc &= 0xffff;
                    break;
                case 3:
                    wantCrc = UdpMisc::GetValue24(crcPtr);
                    actualCrc &= 0xffffff;
                    break;
                case 4:
                    wantCrc = UdpMisc::GetValue32(crcPtr);
                    break;
            }
            if (wantCrc != actualCrc && mUdpManager != nullptr)
            {
                mConnectionStats.crcRejectedPackets++;
                mUdpManager->IncrementCrcRejectedPackets();
                mUdpManager->CallbackCrcReject(this, e->mBuffer, e->mLen);
                return;
            }
            finalLen -= mConnectionConfig.crcBytes;
        }

        udp_uchar tempDecryptBuffer[2][UdpManager::cHardMaxRawPacketSize];

        for (int j = cEncryptPasses - 1; j >= 0; j--)
        {
            if (mConnectionConfig.encryptMethod[j] != cEncryptMethodNone)
            {
                    // connect/confirm/unreachable packets are not encrypted, other packets are encrypted from the second or third byte on as appropriate
                udp_uchar *decryptPtr = tempDecryptBuffer[j % 2];
                *decryptPtr++ = finalStart[0];

                if (finalStart[0] == 0)
                {
                    if (finalLen < 2)
                    {
                        CallbackCorruptPacket(e->mBuffer, e->mLen, cUdpCorruptionReasonInternalPacketTooShort);
                        return;        // invalid packet len
                    }

                    *decryptPtr++ = finalStart[1];
                    int len = (this->*(mDecryptFunction[j]))(decryptPtr, finalStart + 2, finalLen - 2);
                    if (mUdpManager == nullptr)
                        return;

                    if (len == -1)
                    {
                        CallbackCorruptPacket(e->mBuffer, e->mLen, cUdpCorruptionReasonDecryptFailed);
                        return;        // decrypt failed, throw away packet
                    }
                    decryptPtr += len;
                }
                else
                {
                    int len = (this->*(mDecryptFunction[j]))(decryptPtr, finalStart + 1, finalLen - 1);
                    if (len == -1)
                    {
                        CallbackCorruptPacket(e->mBuffer, e->mLen, cUdpCorruptionReasonDecryptFailed);
                        return;        // decrypt failed, throw away packet
                    }
                    decryptPtr += len;
                }

                finalStart = tempDecryptBuffer[j % 2];
                finalLen = (int)(decryptPtr - finalStart);
            }
        }

        ProcessCookedPacket(finalStart, finalLen);
    }
}

void UdpConnection::CallbackRoutePacket(const udp_uchar *data, int dataLen)
{
    if (mStatus == cStatusConnected)
    {
        mUdpManager->IncrementApplicationPacketsReceived();
        mConnectionStats.applicationPacketsReceived++;
        mUdpManager->CallbackRoutePacket(this, data, dataLen);    // callback through UdpManager in case it wants to queue the event
    }
}

void UdpConnection::CallbackCorruptPacket(const udp_uchar *data, int dataLen, UdpCorruptionReason reason)
{
    if (mStatus == cStatusConnected)
    {
        mConnectionStats.corruptPacketErrors++;
        mUdpManager->IncrementCorruptPacketErrors();
        mUdpManager->CallbackPacketCorrupt(this, data, dataLen, reason);
        InternalDisconnect(0, cDisconnectReasonCorruptPacket);
    }
}

void UdpConnection::ProcessCookedPacket(const udp_uchar *data, int dataLen)
{
    udp_uchar buf[256];
    udp_uchar *bufPtr;
    if (mUdpManager == nullptr)
        return;

    if (data[0] == 0 && dataLen > 1)
    {
            // internal packet, so process it internally
        switch(data[1])
        {
            case cUdpPacketConnect:
            {
                int otherSideProtocolVersion = UdpMisc::GetValue32(data + 2);
                int connectCode = UdpMisc::GetValue32(data + 6);

                if (mStatus == cStatusNegotiating)
                {
                        // why are we receiving a connect-request coming from the guy we ourselves are currently
                        // in the process of trying to connect to?  Odds are very high that what is actually
                        // happening is we are trying to connect to ourself.  In either case, we should reply
                        // back telling them they are terminated.
                    if (connectCode == mConnectCode)
                        SendTerminatePacket(connectCode, cDisconnectReasonConnectingToSelf);
                    else
                        SendTerminatePacket(connectCode, cDisconnectReasonMutualConnectError);
                    return;
                }

                if (connectCode == mConnectCode)
                {
                    mOtherSideProtocolVersion = otherSideProtocolVersion;
                    mConnectionConfig.maxRawPacketSize = udpMin((int)UdpMisc::GetValue32(data + 10), mConnectionConfig.maxRawPacketSize);

                        // send confirm packet (if our connect code matches up)
                        // prepare UdpPacketConnect packet
                    bufPtr = buf;
                    *bufPtr++ = 0;
                    *bufPtr++ = cUdpPacketConfirm;
                    bufPtr += UdpMisc::PutValue32(bufPtr, mConnectCode);
                    bufPtr += UdpMisc::PutValue32(bufPtr, mConnectionConfig.encryptCode);
                    *bufPtr++ = (udp_uchar)mConnectionConfig.crcBytes;
                    for (int j = 0; j < cEncryptPasses; j++)
                        *bufPtr++ = (udp_uchar)mConnectionConfig.encryptMethod[j];
                    bufPtr += UdpMisc::PutValue32(bufPtr, mConnectionConfig.maxRawPacketSize);
                    bufPtr += UdpMisc::PutValue32(bufPtr, UdpManager::cProtocolVersion);
                    RawSend(buf, (int)(bufPtr - buf));
                }
                else
                {
                        // ok, we got a connect-request packet from the ip/port of something we thought we already had a connection to.
                        // Additionally, the connect-request packet has a different code, meaning it is not just a stragling connect-request
                        // packet that got sent after we accepted the connection.
                        // This means that the other side has probably terminated the connection and is attempting to connect again.
                        // if we just ignore the new connect-request, it will actually result in the new connection-attempt effectively
                        // keeping this connection object alive.  So, instead, when we get this situation, we will terminate this connection
                        // and ignore the connect-request packet.  The connect-request packet will be sent again 1 second later by the client
                        // at which time we won't exist and out UdpManager will establish a new connection object for it.
                    InternalDisconnect(0, cDisconnectReasonNewConnectionAttempt);
                    return;
                }
                break;
            }
            case cUdpPacketConfirm:
            {
                    // unpack UdpPacketConfirm packet into stack variables
                Configuration config;
                int otherSideProtocolVersion = 0;
                
                int connectCode        = UdpMisc::GetValue32(data + 2);
                config.encryptCode    = UdpMisc::GetValue32(data + 6);
                config.crcBytes        = *(data + 10);
                for (int j = 0; j < cEncryptPasses; j++)
                    config.encryptMethod[j] = (EncryptMethod)*(data + 11 + j);
                config.maxRawPacketSize = UdpMisc::GetValue32(data + 11 + cEncryptPasses);
                if (dataLen > 15 + cEncryptPasses)
                {
                    otherSideProtocolVersion = UdpMisc::GetValue32(data + 15 + cEncryptPasses);
                }

                    // only actually process the confirm if we are negotiating (expecting it) and the connect-code matches up
                if (mStatus == cStatusNegotiating && mConnectCode == connectCode)
                {
                    mConnectionConfig = config;
                    mOtherSideProtocolVersion = otherSideProtocolVersion;
                    SetupEncryptModel();
                    mStatus = cStatusConnected;
                    mUdpManager->CallbackConnectComplete(this);
                }
                break;
            }
            case cUdpPacketRequestRemap:
            {
                    // if a request remap packet managed to get routed to our connection, it is because
                    // the mapping is already correct, so we can just ignore this packet at this point
                    // this will happen when the client sends multiple remap-requests, the first one will
                    // cause the actual remapping to occur, and the subsequent ones will manage to make
                    // it into here
                break;
            }
            case cUdpPacketZeroEscape:
            {
                CallbackRoutePacket(data + 1, dataLen - 1);
                break;
            }
            case cUdpPacketOrdered:
            {
                udp_ushort orderedStamp = UdpMisc::GetValue16(data + 2);
                int diff = (int)orderedStamp - (int)mOrderedStampLast;
                if (diff <= 0)        // equal here makes it strip dupes too
                    diff += 0x10000;
                if (diff < 30000)
                {
                    mOrderedStampLast = orderedStamp;
                    CallbackRoutePacket(data + cUdpPacketOrderedSize, dataLen - cUdpPacketOrderedSize);
                }
                else
                {
                    mConnectionStats.orderRejectedPackets++;
                    mUdpManager->IncrementOrderRejectedPackets();
                }
                break;
            }
            case cUdpPacketOrdered2:
            {
                udp_ushort orderedStamp = UdpMisc::GetValue16(data + 2);
                int diff = (int)orderedStamp - (int)mOrderedStampLast2;
                if (diff <= 0)        // equal here makes it strip dupes too
                    diff += 0x10000;
                if (diff < 30000)
                {
                    mOrderedStampLast2 = orderedStamp;
                    CallbackRoutePacket(data + cUdpPacketOrderedSize, dataLen - cUdpPacketOrderedSize);
                }
                else
                {
                    mConnectionStats.orderRejectedPackets++;
                    mUdpManager->IncrementOrderRejectedPackets();
                }
                break;
            }
            case cUdpPacketTerminate:
            {
                int connectCode = UdpMisc::GetValue32(data + 2);
                if (dataLen >= 8)        // to remain protocol compatible with previous version, the other side disconnect reason is an optional field on this packet
                {
                    mOtherSideDisconnectReason = (DisconnectReason)UdpMisc::GetValue16(data + 6);
                }

                if (mConnectCode == connectCode)
                {
                        // since other side explicitly told us they had terminated, there is no reason for us to send a terminate
                        // packet back to them as well (as it will almost always result in some for of unreachable-destination reply)
                        // so, put ourselves in silent-disconnect mode when this happens
                    SetSilentDisconnect(true);        
                    InternalDisconnect(0, cDisconnectReasonOtherSideTerminated);
                    return;
                }
                break;
            }
            case cUdpPacketUnreachableConnection:
            {
                if (mUdpManager->mParams.allowPortRemapping)
                {
                    if (mPortRemapRequestStartStamp == 0)
                    {
                        mPortRemapRequestStartStamp = mUdpManager->CachedClock();
                    }

                    enum { cMaximumTimeAllowedForPortRemapping = 5000 };
                    if (mUdpManager->CachedClockElapsed(mPortRemapRequestStartStamp) < cMaximumTimeAllowedForPortRemapping)
                    {
                        bufPtr = buf;
                        *bufPtr++ = 0;
                        *bufPtr++ = cUdpPacketRequestRemap;
                        bufPtr += UdpMisc::PutValue32(bufPtr, mConnectCode);
                        bufPtr += UdpMisc::PutValue32(bufPtr, mConnectionConfig.encryptCode);
                        RawSend(buf, (int)(bufPtr - buf));        // since the destination doesn't have an associated connection for us to decrypt us, we must be sent unencrypted
                        break;
                    }
                }

                InternalDisconnect(0, cDisconnectReasonUnreachableConnection);
                return;
                break;
            }
            case cUdpPacketMulti:
            {
                const udp_uchar *ptr = data + 2;
                const udp_uchar *endPtr = data + dataLen;
                while (ptr < endPtr)
                {
                    int len = *(const udp_uchar *)ptr++;
                    const udp_uchar *nextPtr = ptr + len;
                    if (nextPtr > endPtr)
                    {
                            // multi-packet lengths didn't properly add up to total packet length
                            // meaning we likely got a corrupt packet.  If you have CRC bytes enabled, it seems
                            // quite unlikely this could ever occur.  Odds are it has happened because the application
                            // (while processing this packet) ended up touching the packet-data and corrupting the next
                            // packet in the multi-sequence.
                        CallbackCorruptPacket(data, dataLen, cUdpCorruptionReasonMultiPacket);
                        if (mUdpManager == nullptr)
                            return;
                    }
                    else
                    {
                        ProcessCookedPacket(ptr, len);
                    }
                    ptr = nextPtr;
                }
                break;
            }
            case cUdpPacketClockSync:
            {
                    // unpacket UdpPacketClockSync packet
                UdpPacketClockSync pp;
                pp.zeroByte            = *data;
                pp.packetType       = *(data + 1);
                pp.timeStamp        = UdpMisc::GetValue16(data + 2);
                pp.masterPingTime    = UdpMisc::GetValue32(data + 4);
                pp.averagePingTime    = UdpMisc::GetValue32(data + 8);
                pp.lowPingTime        = UdpMisc::GetValue32(data + 12);
                pp.highPingTime        = UdpMisc::GetValue32(data + 16);
                pp.lastPingTime        = UdpMisc::GetValue32(data + 20);
                pp.ourSent            = UdpMisc::GetValue64(data + 24);
                pp.ourReceived        = UdpMisc::GetValue64(data + 32);

                mConnectionStats.averagePingTime = pp.averagePingTime;
                mConnectionStats.highPingTime = pp.highPingTime;
                mConnectionStats.lowPingTime = pp.lowPingTime;
                mConnectionStats.lastPingTime = pp.lastPingTime;
                mConnectionStats.masterPingTime = pp.masterPingTime;
                mConnectionStats.syncOurReceived = mConnectionStats.totalPacketsReceived;
                mConnectionStats.syncOurSent = mConnectionStats.totalPacketsSent;
                mConnectionStats.syncTheirReceived = pp.ourReceived;
                mConnectionStats.syncTheirSent = pp.ourSent;

                if (mUdpManager->mProcessingInducedLag > 1000)   // if it has been over a second since our manager got processing time, then we should not reflect clock-sync packets as we will have introduced too much lag ourselves.
                    break;

                        // prepare UdpPacketClockReflect packet
                bufPtr = buf;
                *bufPtr++ = 0;
                *bufPtr++ = cUdpPacketClockReflect;
                bufPtr += UdpMisc::PutValue16(bufPtr, pp.timeStamp);                    // timeStamp
                bufPtr += UdpMisc::PutValue32(bufPtr, mUdpManager->LocalSyncStampLong());    // serverSyncStampLong
                bufPtr += UdpMisc::PutValue64(bufPtr, pp.ourSent);                        // yourSent
                bufPtr += UdpMisc::PutValue64(bufPtr, pp.ourReceived);                    // yourReceived
                bufPtr += UdpMisc::PutValue64(bufPtr, mConnectionStats.totalPacketsSent);            // ourSent
                bufPtr += UdpMisc::PutValue64(bufPtr, mConnectionStats.totalPacketsReceived);        // ourReceived
                PhysicalSend(buf, (int)(bufPtr - buf), true);
                break;
            }
            case cUdpPacketClockReflect:
            {
                UdpPacketClockReflect pp;
                pp.zeroByte                = *data;
                pp.packetType            = *(data + 1);
                pp.timeStamp            = UdpMisc::GetValue16(data + 2);
                pp.serverSyncStampLong  = UdpMisc::GetValue32(data + 4);
                pp.yourSent                = UdpMisc::GetValue64(data + 8);
                pp.yourReceived            = UdpMisc::GetValue64(data + 16);
                pp.ourSent                = UdpMisc::GetValue64(data + 24);
                pp.ourReceived            = UdpMisc::GetValue64(data + 32);

                mConnectionStats.syncOurReceived = pp.yourReceived;
                mConnectionStats.syncOurSent = pp.yourSent;
                mConnectionStats.syncTheirReceived = pp.ourReceived;
                mConnectionStats.syncTheirSent = pp.ourSent;

                if (mUdpManager->mProcessingInducedLag > 1000)   // if it has been over a second since our manager got processing time, then we should ignore the timing aspects of the clock-sync packets as we will have introduced too much lag ourselves.
                    break;

                udp_ushort curStamp = mUdpManager->LocalSyncStampShort();
                int roundTime = UdpMisc::SyncStampShortDeltaTime(pp.timeStamp, curStamp);

                mSyncStatCount++;
                mSyncStatTotal += roundTime;
                if (mSyncStatLow == 0 || roundTime < mSyncStatLow)
                    mSyncStatLow = roundTime;
                if (roundTime > mSyncStatHigh)
                    mSyncStatHigh = roundTime;
                mSyncStatLast = roundTime;

                    // see if we should use this sync to reset the master sync time
                    // if have better (or close to better) round time or it has been a while
                int elapsed = mUdpManager->CachedClockElapsed(mSyncStatMasterFixupTime);
                if (roundTime <= mSyncStatMasterRoundTime + 20 || elapsed > 120000)
                {
                        // resync on this packet unless this packet is a real loser (unless it just been a very long time, then sync up anyhow)
                    if (roundTime < mSyncStatMasterRoundTime * 2 || elapsed > 240000)
                    {
                        mSyncTimeDelta = (pp.serverSyncStampLong - mUdpManager->LocalSyncStampLong()) + (udp_uint)(roundTime / 2);
                        mSyncStatMasterFixupTime = mUdpManager->CachedClock();
                        mSyncStatMasterRoundTime = roundTime;
                    }
                }

                    // update connection statistics
                mConnectionStats.averagePingTime = (mSyncStatCount > 0) ? (mSyncStatTotal / mSyncStatCount) : 0;
                mConnectionStats.highPingTime = mSyncStatHigh;
                mConnectionStats.lowPingTime = mSyncStatLow;
                mConnectionStats.lastPingTime = roundTime;
                mConnectionStats.masterPingTime = mSyncStatMasterRoundTime;
                break;
            }
            case cUdpPacketKeepAlive:
                break;
            case cUdpPacketReliable1:
            case cUdpPacketReliable2:
            case cUdpPacketReliable3:
            case cUdpPacketReliable4:
            case cUdpPacketFragment1:
            case cUdpPacketFragment2:
            case cUdpPacketFragment3:
            case cUdpPacketFragment4:
            {
                int num = (data[1] - cUdpPacketReliable1) % cReliableChannelCount;
                if (mChannel[num] == nullptr)
                    mChannel[num] = new UdpReliableChannel(num, this, &mUdpManager->mParams.reliable[num]);
                mChannel[num]->ReliablePacket(data, dataLen);
                break;
            }
            case cUdpPacketAck1:
            case cUdpPacketAck2:
            case cUdpPacketAck3:
            case cUdpPacketAck4:
            {
                int num = data[1] - cUdpPacketAck1;
                if (mChannel[num] != nullptr)
                    mChannel[num]->AckPacket(data, dataLen);
                break;
            }
            case cUdpPacketAckAll1:
            case cUdpPacketAckAll2:
            case cUdpPacketAckAll3:
            case cUdpPacketAckAll4:
            {
                int num = data[1] - cUdpPacketAckAll1;
                if (mChannel[num] != nullptr)
                    mChannel[num]->AckAllPacket(data, dataLen);
                break;
            }
            case cUdpPacketGroup:
            {
                const udp_uchar *ptr = data + 2;
                const udp_uchar *endPtr = data + dataLen;
                while (ptr < endPtr)
                {
                    udp_uint len;
                    ptr += UdpMisc::GetVariableValue(ptr, &len);

                    if (ptr > endPtr || len > (udp_uint)(endPtr - ptr))
                    {
                            // specified more data in this piece than is left in the entire packet
                            // this is either corruption, or more likely a hacker
                        CallbackCorruptPacket(data, dataLen, cUdpCorruptionReasonMisformattedGroup);
                        break;
                    }

                    ProcessCookedPacket(ptr, len);
                    ptr += len;
                }
                break;
            }
        }
    }
    else
    {
        CallbackRoutePacket(data, dataLen);
    }
}

void UdpConnection::FlushChannels()
{
    UdpRef ref(this);   // in case application tries to delete us during this give time (could only occur due to a ConnectComplete callback timeout)
    UdpGuard myGuard(&mGuard);

    GiveTime(false);        // gives our reliable channels time to send any data recently added to their queues.  Reschedules us as well, which is ok.
    FlushMultiBuffer();
}

void UdpConnection::FlagPortUnreachable()
{
    mFlaggedPortUnreachable = true;
}

void UdpConnection::GiveTime(bool fromManager)
{
    UdpGuard myGuard(&mGuard);
    if (mUdpManager == nullptr)
        return;

    if (fromManager && GetRefCount() == 2)
    {
            // if the manager's connection list and it's stack-variable are our only two references, then the application
            // has forsaken us, so disconnect
        InternalDisconnect(0, cDisconnectReasonApplicationReleased);
        return;
    }

    UdpRef manRef(mUdpManager);   // hold a reference to the UdpManager so it doesn't disappear while we are inside our GiveTime
    mGettingTime = true;        // lets the internal code know we are in the process of getting time.  We do this so when actual packets are sent while we are getting time, we don't reprioritize ourselves to 0
    InternalGiveTime();
    mGettingTime = false;
}

void UdpConnection::InternalGiveTime()
{
    udp_uchar buf[256];
    udp_uchar *bufPtr;

    int nextSchedule = 10 * 60 * 1000;        // give us time in 10 minutes (unless somebody wants it sooner)
    mConnectionStats.iterations++;

    if (mFlaggedPortUnreachable)
    {
        mFlaggedPortUnreachable = false;
        PortUnreachable();
    }

    switch(mStatus)
    {
        case cStatusNegotiating:
        {
            if (mConnectAttemptTimeout > 0 && ConnectionAge() > mConnectAttemptTimeout)
            {
                InternalDisconnect(0, cDisconnectReasonConnectFail);
                return;
                break;
            }

            int elapsed = mUdpManager->CachedClockElapsed(mLastSendTime);
            if (elapsed >= mUdpManager->mParams.connectAttemptDelay)
            {
                    // prepare UdpPacketConnect packet
                bufPtr = buf;
                *bufPtr++ = 0;
                *bufPtr++ = cUdpPacketConnect;
                bufPtr += UdpMisc::PutValue32(bufPtr, UdpManager::cProtocolVersion);
                bufPtr += UdpMisc::PutValue32(bufPtr, mConnectCode);
                bufPtr += UdpMisc::PutValue32(bufPtr, mUdpManager->mParams.maxRawPacketSize);
                RawSend(buf, (int)(bufPtr - buf));
                elapsed = 0;
            }

            nextSchedule = udpMin(nextSchedule, mUdpManager->mParams.connectAttemptDelay - elapsed);
            break;
        }
        case cStatusConnected:
        case cStatusDisconnectPending:
        {
                // sync clock if required
            if (mUdpManager->mParams.clockSyncDelay > 0)
            {
                    // sync periodically.  If our current master round time is very bad, then sync more frequently (this is important to quickly get a sync up and running)
                int elapsed = mUdpManager->CachedClockElapsed(mLastClockSyncTime);
                if (elapsed > mUdpManager->mParams.clockSyncDelay 
                            || (mSyncStatMasterRoundTime > 3000 && elapsed > 2000) 
                            || (mSyncStatMasterRoundTime > 1000 && elapsed > 5000)
                            || (mSyncStatCount < 2 && elapsed > 10000))
                {
                        // send a clock-sync packet
                    int averagePing = (mSyncStatCount > 0) ? (mSyncStatTotal / mSyncStatCount) : 0;

                    bufPtr = buf;
                    *bufPtr++ = 0;
                    *bufPtr++ = cUdpPacketClockSync;
                    bufPtr += UdpMisc::PutValue16(bufPtr, mUdpManager->LocalSyncStampShort());    // timeStamp
                    bufPtr += UdpMisc::PutValue32(bufPtr, mSyncStatMasterRoundTime);        // masterPingTime
                    bufPtr += UdpMisc::PutValue32(bufPtr, averagePing);                        // averagePingTime
                    bufPtr += UdpMisc::PutValue32(bufPtr, mSyncStatLow);                    // lowPingTime
                    bufPtr += UdpMisc::PutValue32(bufPtr, mSyncStatHigh);                    // highPingTime
                    bufPtr += UdpMisc::PutValue32(bufPtr, mSyncStatLast);                    // lastPingTime
                    bufPtr += UdpMisc::PutValue64(bufPtr, mConnectionStats.totalPacketsSent + 1);    // ourSent (add 1 to include this packet we are about to send since other side will count it as received before getting it)
                    bufPtr += UdpMisc::PutValue64(bufPtr, mConnectionStats.totalPacketsReceived);    // ourReceived
                    PhysicalSend(buf, (int)(bufPtr - buf), true);        // don't buffer this, we need it to be as timely as possible, it still needs to be encrypted though, so don't raw send it.

                    mLastClockSyncTime = mUdpManager->CachedClock();
                    elapsed = 0;
                }

                    // schedule us next time for a clock-sync packet
                nextSchedule = udpMin(nextSchedule, mUdpManager->mParams.clockSyncDelay - elapsed);
            }

                // give reliable channels processing time and see when they want more time
            int totalPendingBytes = 0;
            for (int i = 0; i < cReliableChannelCount; i++)
            {
                if (mChannel[i] != nullptr)
                {
                    totalPendingBytes += mChannel[i]->TotalPendingBytes();
                    int myNext = mChannel[i]->GiveTime();
                    if (mUdpManager == nullptr)
                        return;        // giving the reliable channel time caused it to callback the application which may disconnect us
                    nextSchedule = udpMin(nextSchedule, myNext);
                }
            }

            if (mUdpManager->mParams.reliableOverflowBytes != 0 && totalPendingBytes >= mUdpManager->mParams.reliableOverflowBytes)
            {
                InternalDisconnect(0, cDisconnectReasonReliableOverflow);
                return;
            }
                
                // if we have multi-buffer data
            if (mMultiBufferPtr - mMultiBufferData > 2)
            {
                int elapsed = mUdpManager->CachedClockElapsed(mDataHoldTime);
                if (elapsed >= mUdpManager->mParams.maxDataHoldTime)
                    FlushMultiBuffer();            // having just sent it, there is no data in the buffer so no reason to adjust the schedule for when it may be needed again
                else
                    nextSchedule = udpMin(nextSchedule, mUdpManager->mParams.maxDataHoldTime - elapsed);    // schedule us processing time for when it does need to be sent
            }

                // see if we need to keep connection alive
            int elapsed = mUdpManager->CachedClockElapsed(mLastSendTime);
            if (mKeepAliveDelay > 0)
            {
                if (elapsed >= mKeepAliveDelay)
                {
                        // send keep-alive packet
                    bufPtr = buf;
                    *bufPtr++ = 0;
                    *bufPtr++ = cUdpPacketKeepAlive;
                    PhysicalSend(buf, (int)(bufPtr - buf), true);
                    elapsed = 0;
                }

                    // schedule us next time for a keep-alive packet
                nextSchedule = udpMin(nextSchedule, mKeepAliveDelay - elapsed);
            }

                // see if we need to keep the port alive
            if (mUdpManager->mParams.portAliveDelay > 0)
            {
                int portElapsed = mUdpManager->CachedClockElapsed(mLastPortAliveTime);
                if (portElapsed >= mUdpManager->mParams.portAliveDelay)
                {
                    mLastPortAliveTime = mUdpManager->CachedClock();
                    mUdpManager->SendPortAlive(mIp, mPort);
                    portElapsed = 0;
                }

                    // schedule us next time for a keep-alive packet
                nextSchedule = udpMin(nextSchedule, mUdpManager->mParams.portAliveDelay - portElapsed);
            }

            if (mStatus == cStatusDisconnectPending)
            {
                int timeLeft = mDisconnectFlushTimeout - mUdpManager->CachedClockElapsed(mDisconnectFlushStamp);
                if (timeLeft < 0 || TotalPendingBytes() == 0)
                {
                    InternalDisconnect(0, mDisconnectReason);
                    return;
                }
                else
                {
                    nextSchedule = udpMin(nextSchedule, timeLeft);
                }
            }

            if (mNoDataTimeout > 0)
            {
                int lrt = LastReceive();
                if (lrt >= mNoDataTimeout)
                {
                    InternalDisconnect(0, cDisconnectReasonTimeout);
                    return;
                }
                else
                {
                    nextSchedule = udpMin(nextSchedule, mNoDataTimeout - lrt);
                }
            }

            break;
        }
        default:
            break;
    }

    if (mUdpManager != nullptr)
    {
            // safety to prevent us for scheduling ourselves for a time period that has already passed,
            // as doing so could result in infinite looping in the priority queue processing.
            // in theory this cannot happen, I should likely assert here just to make sure...
        if (nextSchedule < 0)
            nextSchedule = 0;

        mUdpManager->SetPriority(this, mUdpManager->CachedClock() + nextSchedule + 5);        // add 5ms to ensure that we are indeed slightly past the scheduled time
    }
}

int UdpConnection::TotalPendingBytes() const
{
    UdpGuard myGuard(&mGuard);

    int total = 0;
    for (int i = 0; i < cReliableChannelCount; i++)
    {
        if (mChannel[i] != nullptr)
            total += mChannel[i]->TotalPendingBytes();
    }
    return(total);
}

void UdpConnection::RawSend(const udp_uchar *data, int dataLen)
{
        // raw send resets last send time, so we need to potentially recalculate when we need time again
        // sends the actual physical packet (usually just after it has be prepped by PacketSend, but for connect/confirm/unreachable packets are bypass that step)
    mUdpManager->ActualSend(data, dataLen, mIp, mPort);
    mConnectionStats.totalPacketsSent++;
    mConnectionStats.totalBytesSent += dataLen;
    mLastPortAliveTime = mLastSendTime = mUdpManager->CachedClock();

        // track data rate
    ExpireSendBin();
    mSendBin[mLastSendBin % cBinCount] += dataLen;
    mOutgoingBytesLastSecond += dataLen;
    ScheduleTimeNow();
}

void UdpConnection::ExpireSendBin()
{
    udp_int64 curBin = mUdpManager->CachedClock() / cBinResolution;
    udp_int64 binDiff = curBin - mLastSendBin;
    if (binDiff > cBinCount)
    {
        memset(mSendBin, 0, sizeof(mSendBin));
        mOutgoingBytesLastSecond = 0;
    }
    else
    {
        for (udp_int64 i = 0; i < binDiff; i++)
        {
            int clearBin = (int)((curBin + i) % cBinCount);
            mOutgoingBytesLastSecond -= mSendBin[clearBin];
            mSendBin[clearBin] = 0;
        }
    }
    mLastSendBin = curBin;
}

void UdpConnection::ExpireReceiveBin()
{
    udp_int64 curBin = mUdpManager->CachedClock() / cBinResolution;
    udp_int64 binDiff = curBin - mLastReceiveBin;
    if (binDiff > cBinCount)
    {
        memset(mReceiveBin, 0, sizeof(mReceiveBin));
        mIncomingBytesLastSecond = 0;
    }
    else
    {
        for (udp_int64 i = 0; i < binDiff; i++)
        {
            int clearBin = (int)((curBin + i) % cBinCount);
            mIncomingBytesLastSecond -= mReceiveBin[clearBin];
            mReceiveBin[clearBin] = 0;
        }
    }
    mLastReceiveBin = curBin;
}

void UdpConnection::PhysicalSend(const udp_uchar *data, int dataLen, bool appendAllowed)
{
    if (mUdpManager == nullptr)
        return;

        // if we attempt to do a physical send (ie. encrypt/compress/crc a packet) while we are not connected
        // (especially if we are cStatusNegotiating), then it will potentially crash, because in the case of
        // cStatusNegotiating, we don't have the encryption method function pointer initialized yet, as the method
        // is part of the negotiations
    if (mStatus != cStatusConnected && mStatus != cStatusDisconnectPending)
        return;
    
        // this is physical packet send routine that compressed, encrypts the packet, and adds crc bytes to it as appropriate
        // no need to make sure we don't encrypt a connect/confirm/unreachable packet because those go directly to RawSend.
    udp_uchar tempEncryptBuffer[2][UdpManager::cHardMaxRawPacketSize + sizeof(int)];
    const udp_uchar *finalStart = data;
    int finalLen = dataLen;
    for (int j = 0; j < cEncryptPasses; j++)
    {
        if (mConnectionConfig.encryptMethod[j] != cEncryptMethodNone)
        {
            udp_uchar *destStart = tempEncryptBuffer[j % 2];
            *(int *)(destStart + finalLen + mEncryptExpansionBytes) = (int)0xcececece;        // overwrite debug signature

            udp_uchar *destPtr = destStart;
            *destPtr++ = finalStart[0];
            if (finalStart[0] == 0)
            {
                    // we know this internal packet will not be a connect or confirm packet since they are sent directly to RawSend to avoid getting encrypted
                *destPtr++ = finalStart[1];
                int len = (this->*(mEncryptFunction[j]))(destPtr, finalStart + 2, finalLen - 2);
                if (mUdpManager == nullptr)
                    return;

                    // if this assert triggers, it means the encryption pass expanded the size of the encrypted
                    // data more than was specified by the userSuppliedEncryptExpansionBytes setting, or at least
                    // tampered with the destination buffer past that length.  This is considered a buffer overwrite
                    // and will potentially cause bugs.
                assert(*(int *)(destStart + finalLen + mEncryptExpansionBytes) == (int)0xcececece);

                if (len == -1)
                    return;        // would be really odd for encryption to return an error, but if it does, throw it away
                destPtr += len;
            }
            else
            {
                int len = (this->*(mEncryptFunction[j]))(destPtr, finalStart + 1, finalLen - 1);

                    // if this assert triggers, it means the encryption pass expanded the size of the encrypted
                    // data more than was specified by the userSuppliedEncryptExpansionBytes setting, or at least
                    // tampered with the destination buffer past that length.  This is considered a buffer overwrite
                    // and will potentially cause bugs.
                assert(*(int *)(destStart + finalLen + mEncryptExpansionBytes) == (int)0xcececece);

                if (len == -1)
                    return;        // would be really odd for encryption to return an error, but if it does, throw it away
                destPtr += len;
            }

            finalStart = destStart;
            finalLen = (int)(destPtr - finalStart);
            appendAllowed = true;
        }
    }

    if (mConnectionConfig.crcBytes > 0)
    {
        if (!appendAllowed)
        {
                // if the buffer we are going to append onto was our original (ie. no encryption took place)
                // then we have to copy it all over to a temp buffer since we can't modify the original
            memcpy(tempEncryptBuffer[0], finalStart, finalLen);
            finalStart = tempEncryptBuffer[0];
        }

        int crc = UdpMisc::Crc32(finalStart, finalLen, mConnectionConfig.encryptCode);
        udp_uchar *crcPtr = const_cast<udp_uchar *>(finalStart) + finalLen;        // safe cast, since we make a copy of the data above if we would have ended up appending to the original
        switch(mConnectionConfig.crcBytes)
        {
            case 1:
                *crcPtr = (udp_uchar)(crc & 0xff);
                break;
            case 2:
                UdpMisc::PutValue16(crcPtr, (udp_ushort)(crc & 0xffff));
                break;
            case 3:
                UdpMisc::PutValue24(crcPtr, crc & 0xffffff);
                break;
            case 4:
                UdpMisc::PutValue32(crcPtr, crc);
                break;
        }
        finalLen += mConnectionConfig.crcBytes;
    }

    RawSend(finalStart, finalLen);
}

    // returns where it placed the data in the buffer (if it ended up in the buffer), such that the reliable channel
    // can note where the ack was placed and replace it
udp_uchar *UdpConnection::BufferedSend(const udp_uchar *data, int dataLen, const udp_uchar *data2, int dataLen2, bool appendAllowed)
{
    if (mUdpManager == nullptr)
        return(nullptr);
    int used = (int)(mMultiBufferPtr - mMultiBufferData);

    int actualMaxDataHoldSize = udpMin(mUdpManager->mParams.maxDataHoldSize, mConnectionConfig.maxRawPacketSize);
        
    int totalDataLen = dataLen + dataLen2;
    if (totalDataLen > 255 || (totalDataLen + 3) > actualMaxDataHoldSize)
    {
            // too long of data to even attempt a multi-buffer of this packet, so let's just send it unbuffered
            // but first, to ensure the packet-order integrity is somewhat maintained, flush the multi-buffer
            // if it currently has something in it
        if (used > 2)
            FlushMultiBuffer();

            // now send it (the multi-buffer is empty if you need to use it temporarily to concatenate two data chunks -- it is large enough to hold the largest raw packet)
        if (data2 != nullptr)
        {
            memcpy(mMultiBufferData, data, dataLen);
            memcpy(mMultiBufferData + dataLen, data2, dataLen2);
            PhysicalSend(mMultiBufferData, totalDataLen, true);
        }
        else
            PhysicalSend(data, dataLen, appendAllowed);
        return(nullptr);
    }

        // if this data will not fit into buffer
        // note: we allow the multi-packet to grow as large as maxRawPacketSize, but down below we will flush it
        // as soon as it gets larger than maxDataHoldSize.
    if (used + totalDataLen + 1 > (mConnectionConfig.maxRawPacketSize - mConnectionConfig.crcBytes - mEncryptExpansionBytes))
    {
        FlushMultiBuffer();
        used = 0;
    }

        // add data to buffer
    if (used == 0)
    {
            // no buffered data yet, create multi-packet header
        *mMultiBufferPtr++ = 0;
        *mMultiBufferPtr++ = cUdpPacketMulti;

            // new multi-buffer started, so we need to potentially recalculate when we need time again
        mDataHoldTime = mUdpManager->CachedClock();            // set data hold time to when the first piece of data is stuck in the multi-buffer
        ScheduleTimeNow();
    }

    *(udp_uchar *)mMultiBufferPtr++ = (udp_uchar)totalDataLen;
    udp_uchar *placementPtr = mMultiBufferPtr;
    memcpy(mMultiBufferPtr, data, dataLen);
    mMultiBufferPtr += dataLen;
    if (data2 != nullptr)
    {
        memcpy(mMultiBufferPtr, data2, dataLen2);
        mMultiBufferPtr += dataLen2;
    }

    if ((mMultiBufferPtr - mMultiBufferData) >= actualMaxDataHoldSize)
    {
        FlushMultiBuffer();
        placementPtr = nullptr;    // it got flushed
    }
    return(placementPtr);
}

void UdpConnection::FlushMultiBuffer()
{
    UdpGuard myGuard(&mGuard);

    int len = (int)(mMultiBufferPtr - mMultiBufferData);
    if (len > 2)
    {
        if ((int)((udp_uchar)mMultiBufferData[2]) + 3 == len)
            PhysicalSend(mMultiBufferData + 3, len - 3, true);        // only one packet so don't send it as a multi-packet
        else
            PhysicalSend(mMultiBufferData, len, true);

            // notify all the reliable channels to clear their buffered acks
        for (int i = 0; i < cReliableChannelCount; i++)
        {
            if (mChannel[i] != nullptr)
            {
                mChannel[i]->ClearBufferedAck();
            }
        }

    }
    mMultiBufferPtr = mMultiBufferData;
}

int UdpConnection::EncryptNone(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    memcpy(destData, sourceData, sourceLen);
    return(sourceLen);
}

int UdpConnection::DecryptNone(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    memcpy(destData, sourceData, sourceLen);
    return(sourceLen);
}

int UdpConnection::EncryptUserSupplied(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    return(mUdpManager->EncryptUserSupplied(this, destData, sourceData, sourceLen));
}

int UdpConnection::DecryptUserSupplied(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    return(mUdpManager->DecryptUserSupplied(this, destData, sourceData, sourceLen));
}

int UdpConnection::EncryptUserSupplied2(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    return(mUdpManager->EncryptUserSupplied2(this, destData, sourceData, sourceLen));
}

int UdpConnection::DecryptUserSupplied2(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    return(mUdpManager->DecryptUserSupplied2(this, destData, sourceData, sourceLen));
}

int UdpConnection::EncryptXorBuffer(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    udp_uchar *destPtr = destData;
    const udp_uchar *walkPtr = sourceData;
    const udp_uchar *endPtr = sourceData + sourceLen;
    udp_uchar *encryptPtr = mEncryptXorBuffer;
    int prev = mConnectionConfig.encryptCode;
    while ((walkPtr + sizeof(int)) <= endPtr)
    {
        *(int *)destPtr = *(const int *)walkPtr ^ *(int *)encryptPtr ^ prev;
        prev = *(int *)destPtr;
        walkPtr += sizeof(int);
        destPtr += sizeof(int);
        encryptPtr += sizeof(int);
    }

    while (walkPtr != endPtr)
    {
        *destPtr = (udp_uchar)(*walkPtr ^ *encryptPtr);
        destPtr++;
        walkPtr++;
        encryptPtr++;
    }
    return(sourceLen);
}

int UdpConnection::DecryptXorBuffer(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    const udp_uchar *walkPtr = sourceData;
    const udp_uchar *endPtr = sourceData + sourceLen;
    udp_uchar *encryptPtr = mEncryptXorBuffer;
    udp_uchar *destPtr = destData;
    int hold;
    int prev = mConnectionConfig.encryptCode;
    while ((walkPtr + sizeof(int)) <= endPtr)
    {
        hold = *(const int *)walkPtr;
        *(int *)destPtr = *(const int *)walkPtr ^ prev ^ *(int *)encryptPtr;
        prev = hold;
        walkPtr += sizeof(int);
        destPtr += sizeof(int);
        encryptPtr += sizeof(int);
    }

    while (walkPtr != endPtr)
    {
        *destPtr = (udp_uchar)(*walkPtr ^ *encryptPtr);
        walkPtr++;
        destPtr++;
        encryptPtr++;
    }
    return(sourceLen);
}

int UdpConnection::EncryptXor(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    udp_uchar *destPtr = destData;
    const udp_uchar *walkPtr = sourceData;
    const udp_uchar *endPtr = sourceData + sourceLen;
    int prev = mConnectionConfig.encryptCode;

        // this does nothing on intel boxes.  This is bit of a hack to make the encryption on big-endian hardware match little-endian hardware
        // this needs be done for performance reasons so we can xor 4 bytes at a time without the associated GetValue penalty, and to remain
        // protocol compatible with previous versions of hte UdpLibrary that made a bad assumption
    char buf[4];
    UdpMisc::PutValueLE32(buf, prev);
    prev = *(int *)buf;

    while ((walkPtr + sizeof(int)) <= endPtr)
    {
        *(int *)destPtr = *(const int *)walkPtr ^ prev;
        prev = *(int *)destPtr;
        walkPtr += sizeof(int);
        destPtr += sizeof(int);
    }

        // need to do this fixup again when switching to single-byte comparisons
    UdpMisc::PutValueLE32(buf, prev);
    prev = *(int *)buf;

    while (walkPtr != endPtr)
    {
        *destPtr = (udp_uchar)(*walkPtr ^ prev);
        destPtr++;
        walkPtr++;
    }
    return(sourceLen);
}

int UdpConnection::DecryptXor(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    const udp_uchar *walkPtr = sourceData;
    const udp_uchar *endPtr = sourceData + sourceLen;
    udp_uchar *destPtr = destData;
    int hold;
    int prev = mConnectionConfig.encryptCode;

        // this does nothing on intel boxes.  This is bit of a hack to make the encryption on big-endian hardware match little-endian hardware
        // this needs be done for performance reasons so we can xor 4 bytes at a time without the associated GetValue penalty, and to remain
        // protocol compatible with previous versions of hte UdpLibrary that made a bad assumption
    char buf[4];
    UdpMisc::PutValueLE32(buf, prev);
    prev = *(int *)buf;

    while ((walkPtr + sizeof(int)) <= endPtr)
    {
        hold = *(const int *)walkPtr;
        *(int *)destPtr = *(const int *)walkPtr ^ prev;
        prev = hold;
        walkPtr += sizeof(int);
        destPtr += sizeof(int);
    }

        // need to do this fixup again when switching to single-byte comparisons
    UdpMisc::PutValueLE32(buf, prev);
    prev = *(int *)buf;

    while (walkPtr != endPtr)
    {
        *destPtr = (udp_uchar)(*walkPtr ^ prev);
        walkPtr++;
        destPtr++;
    }
    return(sourceLen);
}

void UdpConnection::SetupEncryptModel()
{
    mEncryptExpansionBytes = 0;
    for (int j = 0; j < cEncryptPasses; j++)
    {
        switch(mConnectionConfig.encryptMethod[j])
        {
            default:
                assert(0);    // unknown encryption method specified during in UdpManager construction
                break;
            case cEncryptMethodNone:
            {
                    // point to method functions
                mDecryptFunction[j] = &UdpConnection::DecryptNone;
                mEncryptFunction[j] = &UdpConnection::EncryptNone;
                mEncryptExpansionBytes += 0;
                break;
            }
            case cEncryptMethodUserSupplied:
            {
                    // point to method functions
                mDecryptFunction[j] = &UdpConnection::DecryptUserSupplied;
                mEncryptFunction[j] = &UdpConnection::EncryptUserSupplied;
                mEncryptExpansionBytes += mUdpManager->mParams.userSuppliedEncryptExpansionBytes;
                break;
            }
            case cEncryptMethodUserSupplied2:
            {
                    // point to method functions
                mDecryptFunction[j] = &UdpConnection::DecryptUserSupplied2;
                mEncryptFunction[j] = &UdpConnection::EncryptUserSupplied2;
                mEncryptExpansionBytes += mUdpManager->mParams.userSuppliedEncryptExpansionBytes2;
                break;
            }
            case cEncryptMethodXorBuffer:
            {
                    // point to method functions
                mDecryptFunction[j] = &UdpConnection::DecryptXorBuffer;
                mEncryptFunction[j] = &UdpConnection::EncryptXorBuffer;
                mEncryptExpansionBytes += 0;

                    // set up encrypt buffer (random numbers generated based on seed)
                if (mEncryptXorBuffer == nullptr)
                {
                    int len = ((mUdpManager->mParams.maxRawPacketSize + 1) / 4) * 4;
                    mEncryptXorBuffer = new udp_uchar[len];
                    int seed = mConnectionConfig.encryptCode;
                    udp_uchar *sptr = mEncryptXorBuffer;
                    for (int i = 0; i < len; i++)
                        *sptr++ = (udp_uchar)(UdpMisc::Random(&seed) & 0xff);
                }
                break;
            }
            case cEncryptMethodXor:
            {
                    // point to method functions
                mDecryptFunction[j] = &UdpConnection::DecryptXor;
                mEncryptFunction[j] = &UdpConnection::EncryptXor;
                mEncryptExpansionBytes += 0;
                break;
            }
        }
    }
}

void UdpConnection::GetChannelStatus(UdpChannel channel, ChannelStatus *channelStatus) const
{
    UdpGuard myGuard(&mGuard);

    memset(channelStatus, 0, sizeof(*channelStatus));
    switch (channel)
    {
        case cUdpChannelReliable1:
        case cUdpChannelReliable2:
        case cUdpChannelReliable3:
        case cUdpChannelReliable4:
            if (mChannel[channel - cUdpChannelReliable1] != nullptr)
            {
                mChannel[channel - cUdpChannelReliable1]->GetChannelStatus(channelStatus);
            }
            break;
        default:
            break;
    }
}

const char *UdpConnection::DisconnectReasonText(DisconnectReason reason)
{
    static bool sInitialized = false;
    static char *sDisconnectReason[cDisconnectReasonCount];
    
    if (!sInitialized)
    {
        sInitialized = true;
        memset(sDisconnectReason, 0, sizeof(sDisconnectReason));
        sDisconnectReason[cDisconnectReasonNone] = "DisconnectReasonNone";
        sDisconnectReason[cDisconnectReasonIcmpError] = "DisconnectReasonIcmpError";
        sDisconnectReason[cDisconnectReasonTimeout] = "DisconnectReasonTimeout";
        sDisconnectReason[cDisconnectReasonOtherSideTerminated] = "DisconnectReasonOtherSideTerminated";
        sDisconnectReason[cDisconnectReasonManagerDeleted] = "DisconnectReasonManagerDeleted";
        sDisconnectReason[cDisconnectReasonConnectFail] = "DisconnectReasonConnectFail";
        sDisconnectReason[cDisconnectReasonApplication] = "DisconnectReasonApplication";
        sDisconnectReason[cDisconnectReasonUnreachableConnection] = "DisconnectReasonUnreachableConnection";
        sDisconnectReason[cDisconnectReasonUnacknowledgedTimeout] = "DisconnectReasonUnacknowledgedTimeout";
        sDisconnectReason[cDisconnectReasonNewConnectionAttempt] = "DisconnectReasonNewConnectionAttempt";
        sDisconnectReason[cDisconnectReasonConnectionRefused] = "DisconnectReasonConnectionRefused";
        sDisconnectReason[cDisconnectReasonMutualConnectError] = "DisconnectReasonConnectError";
        sDisconnectReason[cDisconnectReasonConnectingToSelf] = "DisconnectReasonConnectingToSelf";
        sDisconnectReason[cDisconnectReasonReliableOverflow] = "DisconnectReasonReliableOverflow";
        sDisconnectReason[cDisconnectReasonApplicationReleased] = "DisconnectReasonApplicationReleased";
        sDisconnectReason[cDisconnectReasonCorruptPacket] = "DisconnectReasonCorruptPacket";
    }

    return(sDisconnectReason[reason]);
}

char *UdpConnection::GetDestinationString(char *buf, int bufLen) const
{
    UdpGuard myGuard(&mGuard);

    if (bufLen < 22)
        return(nullptr);
    UdpPlatformAddress ip = GetDestinationIp();
    int port = GetDestinationPort();
    char hold[256];
    sprintf(buf, "%s:%d", ip.GetAddress(hold, sizeof(hold)), port);
    return(buf);
}

char *UdpConnection::GetDisconnectReasonText(char *buf, int bufLen) const
{
    bufLen--;   // leave room for terminating zero
    DisconnectReason reason = GetDisconnectReason();
    const char *reasonText = DisconnectReasonText(reason);
    if (reason != cDisconnectReasonOtherSideTerminated)
    {
        UdpMisc::Strncpy(buf, reasonText, bufLen);
        return(buf);
    }

    DisconnectReason otherReason = GetOtherSideDisconnectReason();
    const char *otherText = DisconnectReasonText(otherReason);
    UdpMisc::Strncpy(buf, reasonText, bufLen);
    UdpMisc::Strncat(buf, ",", bufLen);
    UdpMisc::Strncat(buf, otherText, bufLen);
    return(buf);
}

void UdpConnection::OnRoutePacket(const udp_uchar *data, int dataLen)
{
    UdpGuard myGuard(&mHandlerGuard);

    if (mHandler != nullptr)
    {
        mHandler->OnRoutePacket(this, data, dataLen);
    }
    else
    {
            // no handler at time of actually delivery to application of data, means the application must
            // have released us, so disconnect for that reason.
            // this is a bit screwy.  The problem is, if the application releases the connection while the
            // event queue has pending data, the connection will not know that it is not longer being
            // referenced by the application, so we will disconnect here.
            // this (along with the check just after OnConnectRequest) effectively requires that the application
            // always have a handler installed or else the connection will potentially get disconnected
        InternalDisconnect(0, UdpConnection::cDisconnectReasonApplicationReleased);
    }
}

void UdpConnection::OnConnectComplete()
{
    UdpGuard myGuard(&mHandlerGuard);
    if (mHandler != nullptr)
    {
        mHandler->OnConnectComplete(this);
    }
}

void UdpConnection::OnTerminated()
{
    UdpGuard myGuard(&mHandlerGuard);
    if (mHandler != nullptr)
    {
        mHandler->OnTerminated(this);
    }
}

void UdpConnection::OnCrcReject(const udp_uchar *data, int dataLen)
{
    UdpGuard myGuard(&mHandlerGuard);
    if (mHandler != nullptr)
    {
        mHandler->OnCrcReject(this, data, dataLen);
    }
}

void UdpConnection::OnPacketCorrupt(const udp_uchar *data, int dataLen, UdpCorruptionReason reason)
{
    UdpGuard myGuard(&mHandlerGuard);
    if (mHandler != nullptr)
    {
        mHandler->OnPacketCorrupt(this, data, dataLen, reason);
    }
}



}   // namespace
