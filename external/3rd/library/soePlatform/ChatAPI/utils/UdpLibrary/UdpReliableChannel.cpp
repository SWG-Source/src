// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

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
    // ReliableChannel implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////////
UdpReliableChannel::UdpReliableChannel(int channelNumber, UdpConnection *con, UdpReliableConfig *config) : mLogicalPacketList(&LogicalPacket::mReliableLink)
{
    mUdpConnection = con;
    mChannelNumber = channelNumber;
    mConfig = *config;
    mConfig.maxOutstandingPackets = udpMin(mConfig.maxOutstandingPackets, (int)UdpManager::cHardMaxOutstandingPackets);

    mAveragePingTime = 800;        // start out fairly high so we don't do a lot of resends on a bad connection to start out with
    mTrickleLastSend = 0;

    int fragmentSize = mConfig.fragmentSize;
    if (fragmentSize == 0 || fragmentSize > mUdpConnection->mConnectionConfig.maxRawPacketSize)
        fragmentSize = mUdpConnection->mConnectionConfig.maxRawPacketSize;
    mMaxDataBytes = (fragmentSize - UdpConnection::cUdpPacketReliableSize - mUdpConnection->mConnectionConfig.crcBytes - mUdpConnection->mEncryptExpansionBytes);
    assert(mMaxDataBytes > 0);        // fragment size/max-raw-packet-size set too small to allow for reliable deliver
    if (mConfig.trickleSize != 0)
        mMaxDataBytes = udpMin(mMaxDataBytes, mConfig.trickleSize);

    mMaxCoalesceAttemptBytes = -1;
    if (mConfig.coalesce)
    {
        mMaxCoalesceAttemptBytes = mMaxDataBytes - 5;    // 2 bytes for group-header, 3 bytes for length of packet
    }

    mReliableIncomingId = 0;
    mReliableOutgoingId = 0;
    mReliableOutgoingPendingId = 0;
    mReliableOutgoingBytes = 0;
    mLogicalBytesQueued = 0;

    mCoalescePacket = nullptr;
    mCoalesceStartPtr = nullptr;
    mCoalesceEndPtr = nullptr;
    mCoalesceCount = 0;

    mBufferedAckPtr = nullptr;

    mStatDuplicatePacketsReceived = 0;
    mStatResentPacketsAccelerated = 0;
    mStatResentPacketsTimedOut = 0;

    mWindowResetTime = 0;

    mCongestionWindowMinimum = udpMax(mMaxDataBytes, mConfig.congestionWindowMinimum);
    mCongestionWindowStart = udpMin(4 * mMaxDataBytes, udpMax(2 * mMaxDataBytes, 4380));
    mCongestionWindowStart = udpMax(mCongestionWindowStart, mCongestionWindowMinimum);
    mCongestionSlowStartThreshhold = udpMin(mConfig.maxOutstandingPackets * mMaxDataBytes, mConfig.maxOutstandingBytes);
    mCongestionWindowSize = mCongestionWindowStart;

    mBigDataLen = 0;
    mBigDataTargetLen = 0;
    mBigDataPtr = nullptr;
    mFragmentNextPos = 0;
    mLastTimeStampAcknowledged = 0;
    mMaxxedOutCurrentWindow = false;
    mNextNeedTime = 0;

    mPhysicalPackets = new PhysicalPacket[mConfig.maxOutstandingPackets];
    mReliableIncoming = new IncomingQueueEntry[mConfig.maxInstandingPackets];
}

UdpReliableChannel::~UdpReliableChannel()
{
    if (mCoalescePacket != nullptr)
    {
        mCoalescePacket->Release();
        mCoalescePacket = nullptr;
    }

    const LogicalPacket *cur = mLogicalPacketList.RemoveHead();
    while (cur != nullptr)
    {
        cur->Release();
        cur = mLogicalPacketList.RemoveHead();
    }

    delete[] mPhysicalPackets;        // destructor will release any logical packets as appropriate
    delete[] mReliableIncoming;        // destructor will delete any data as appropriate

        // delete any big packet that might be under construction
    delete[] mBigDataPtr;
}

void UdpReliableChannel::Send(const udp_uchar *data, int dataLen, const udp_uchar *data2, int dataLen2)
{
    if (mLogicalPacketList.Count() == 0 && mCoalescePacket == nullptr)
    {
            // if we are adding something to a previously empty logical queue, then it is possible that
            // we may be able to send it, so mark ourselves to take time the next time it is offered
        mNextNeedTime = 0;
        mUdpConnection->ScheduleTimeNow();
    }

    if (dataLen + dataLen2 <= mMaxCoalesceAttemptBytes)
    {
        SendCoalesce(data, dataLen, data2, dataLen2);
    }
    else
    {
        FlushCoalesce();
        LogicalPacket *packet = mUdpConnection->mUdpManager->CreatePacket(data, dataLen, data2, dataLen2);
        QueueLogicalPacket(packet);
        packet->Release();
    }
}

void UdpReliableChannel::FlushCoalesce()
{
    if (mCoalescePacket != nullptr)
    {
        if (mCoalesceCount == 1)
        {
                // only one packet in coalesce, so remove the coalesce/group header and make it just a simple raw packet
            int firstLen;
            int skipLen = UdpMisc::GetVariableValue(mCoalesceStartPtr + 2, (udp_uint *)&firstLen);
            memmove(mCoalesceStartPtr, mCoalesceStartPtr + 2 + skipLen, firstLen);
            mCoalesceEndPtr = mCoalesceStartPtr + firstLen;
        }

        mCoalescePacket->SetDataLen((int)(mCoalesceEndPtr - mCoalesceStartPtr));
        QueueLogicalPacket(mCoalescePacket);
        mCoalescePacket->Release();
        mCoalescePacket = nullptr;
    }
}

void UdpReliableChannel::SendCoalesce(const udp_uchar *data, int dataLen, const udp_uchar *data2, int dataLen2)
{
    int totalLen = dataLen + dataLen2;
    if (mCoalescePacket == nullptr)
    {
        mCoalescePacket = mUdpConnection->mUdpManager->CreatePacket(nullptr, mMaxDataBytes);
        mCoalesceEndPtr = mCoalesceStartPtr = (udp_uchar *)mCoalescePacket->GetDataPtr();
        *mCoalesceEndPtr++ = 0;
        *mCoalesceEndPtr++ = UdpConnection::cUdpPacketGroup;
        mCoalesceCount = 0;
    }
    else
    {
        int spaceLeft = mMaxDataBytes - (int)(mCoalesceEndPtr - mCoalesceStartPtr);
        if (totalLen + 3 > spaceLeft)        // 3 bytes to ensure PutVariableValue has room for the length indicator (this limits us to 64k coalescing, which is ok, since fragments can't get that big)
        {
            FlushCoalesce();
            SendCoalesce(data, dataLen, data2, dataLen2);
            return;
        }
    }

        // append on end of coalesce
    mCoalesceCount++;
    mCoalesceEndPtr += UdpMisc::PutVariableValue(mCoalesceEndPtr, totalLen);
    if (data != nullptr)
        memcpy(mCoalesceEndPtr, data, dataLen);
    mCoalesceEndPtr += dataLen;
    if (data2 != nullptr)
        memcpy(mCoalesceEndPtr, data2, dataLen2);
    mCoalesceEndPtr += dataLen2;
}

void UdpReliableChannel::QueueLogicalPacket(LogicalPacket *packet)
{
    packet->AddRef();
    mLogicalBytesQueued += packet->GetDataLen();
    mLogicalPacketList.InsertTail(packet);
}

bool UdpReliableChannel::PullDown(int windowSpaceLeft)
{
        // the pull-down on-demand method will give us the opportunity to late-combine as many tiny logical packets as we can,
        // reducing the number of tracked-packets that are required.  This effectively reduces the number of acks that we are
        // going to get back, which can be substantial in situations where there are a LOT of tiny reliable packets being sent.
        // operating with fewer outstanding physical-packets to track is more CPU efficient as well.

        // (NOTE: as of this writing, the below implementation does not do the late-combine techique yet)
        // (NOTE: we currently combine on send instead, which generally accomplishes the same thing)

    bool pulledDown = false;
    int physicalCount = (int)(mReliableOutgoingId - mReliableOutgoingPendingId);
    while (windowSpaceLeft > 0 && physicalCount < mConfig.maxOutstandingPackets)
    {
        if (mLogicalPacketList.Count() == 0)
        {
            FlushCoalesce();        // this is guaranteed to stick
            if (mLogicalPacketList.Count() == 0)
                break;        // nothing flushed, so we are done
        }

        int nextSpot = (int)(mReliableOutgoingId % mConfig.maxOutstandingPackets);

            // ok, we can move something down, even if it is only a fragment of the logical packet
        PhysicalPacket *entry = &mPhysicalPackets[nextSpot];
        entry->mParent = mLogicalPacketList.First();
        entry->mParent->AddRef();        // add ref from physical packet
        entry->mFirstTimeStamp = 0;
        entry->mLastTimeStamp = 0;

            // calculate how much we can send based on our starting position (mFragmentNextPos) in the logical packet.
            // if we can't send it the rest of data to end of packet, then send the fragment portion and addref, otherwise send the whole thing and pop the logical packet
        int dataLen = entry->mParent->GetDataLen();
        const udp_uchar *data = (const udp_uchar *)entry->mParent->GetDataPtr();
        int bytesLeft = dataLen - mFragmentNextPos;
        int bytesToSend = udpMin(bytesLeft, mMaxDataBytes);

        entry->mDataPtr = data + mFragmentNextPos;

                // if not sending entire packet
        if (bytesToSend != dataLen)
        {
                // mark it as a fragment
            if (mFragmentNextPos == 0)
                bytesToSend -= 4;    // fragment start has a 4 byte header specifying size of following large data, so make room for it so we don't exceed max raw packet size
        }
        entry->mDataLen = bytesToSend;
        mReliableOutgoingBytes += bytesToSend;

        if (bytesToSend == bytesLeft)
        {
            mFragmentNextPos = 0;
            const LogicalPacket *lp = mLogicalPacketList.RemoveHead();
            lp->Release();                    // release from logical queue
        }
        else
        {
            mFragmentNextPos += bytesToSend;
        }

        mLogicalBytesQueued -= bytesToSend;                // as fragments are sent, decrease the number of logical bytes queued
        mReliableOutgoingId++;
        physicalCount++;
        windowSpaceLeft -= bytesToSend;
        pulledDown = true;
    }
    return(pulledDown);
}

int UdpReliableChannel::GiveTime()
{
    udp_uchar buf[256];
    udp_uchar *bufPtr;

    UdpClockStamp hotClock = mUdpConnection->GetUdpManager()->CachedClock();

    if (hotClock < mNextNeedTime)
        return(UdpMisc::ClockDiff(hotClock, mNextNeedTime));

        // if we are a trickle channel, then don't try sending more until trickleRate has expired.  We are only allowed
        // to send up to trickleBytes at a time every trickleRate milliseconds; however, if we don't send the full trickleBytes
        // in one GiveTime call, then it won't get to try sending more bytes until this timer has expired, even if we had not used
        // up the entire trickleBytes allotment the last time we were in here...this should not cause any significant problems
    if (mConfig.trickleRate > 0)
    {
        int nextAllowedSendTime = mConfig.trickleRate - UdpMisc::ClockDiff(mTrickleLastSend, hotClock);
        if (nextAllowedSendTime > 0)
            return(nextAllowedSendTime);
    }


        // lot a tweaking goes into calculating the optimal resend time.  Set it too large and you can stall the pipe
        // at the beginning of the connection fairly easily
    int optimalResendDelay = (mAveragePingTime * mConfig.resendDelayPercent / 100) + mConfig.resendDelayAdjust;        // percentage of average ping plus a fixed amount
    optimalResendDelay = udpMin(mConfig.resendDelayCap, optimalResendDelay);                                        // never let the resend delay get over max

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // see if any of the physical packets can actually be sent (either resends, or initial sends, whatever
        // if not, calculate when exactly somebody is expected to need sending
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    mMaxxedOutCurrentWindow = false;
    int outstandingNextSendTime = 10 * 60000;
            // if we have something to do
    if (mReliableOutgoingPendingId < mReliableOutgoingId || mLogicalPacketList.Count() != 0 || mCoalescePacket != nullptr)
    {
            // first, let's calculate how many bytes we figure is outstanding based on who is still waiting for an ack-packet
        UdpClockStamp oldestResendTime = udpMax(hotClock - optimalResendDelay, mLastTimeStampAcknowledged);    // anything older than this, we need to resend

        bool readyQueueOverflow;
        do
        {
            readyQueueOverflow = false;
            int useMaxOutstandingBytes = udpMin(mConfig.maxOutstandingBytes, mCongestionWindowSize);
            int outstandingBytes = 0;
            enum { cReadyQueueSize = 1000 };
            PhysicalPacket *readyQueue[cReadyQueueSize];
            PhysicalPacket **readyEnd = readyQueue + (sizeof(readyQueue) / sizeof(PhysicalPacket *));
            PhysicalPacket **readyPtr = readyQueue;

            int windowSpaceLeft = useMaxOutstandingBytes;
            for (udp_int64 i = mReliableOutgoingPendingId; i <= mReliableOutgoingId; i++)
            {
                if (i == mReliableOutgoingId)
                {
                        // this packet is not really here yet, we need to pull it down if possible
                        // if not possible, we need to break out of the loop as we are done
                    if (!PullDown(windowSpaceLeft))
                        break;
                }

                    // if this packet has not been acked and it is NOT ready to be sent (was recently sent) then we consider it outstanding
                    // note: packets needing re-sending probably got lost and are therefore not outstanding
                PhysicalPacket *entry = &mPhysicalPackets[i % mConfig.maxOutstandingPackets];
                if (entry->mDataPtr != nullptr)        // acked packets set the dataPtr to nullptr
                {
                        // if this packet is ready to be sent (ie: needs time now, or some later packet has already been ack'ed)
                    windowSpaceLeft -= entry->mDataLen;        // window-space is effectively taken whether we have sent it yet or not
                    if (entry->mLastTimeStamp < oldestResendTime)
                    {
                        if (readyPtr < readyEnd)        // if we have queue space
                        {
                            *readyPtr++ = entry;
                        }
                        else
                        {
                                // we ran out of space in the ready-queue, so we are going to have to do this whole process
                                // over again.  Normally we would just want to make sure the readyQueue was really big to save
                                // us doing all this stuff twice; however, that uses up a ton of stack space we may not have
                                // and really only happens in situations where there is a ton of stuff waiting to be sent
                                // (ie, super high bandwidth situations really).  So I don't mind taking a performance hit
                                // for those situations to keep the stack size small for the vast majority of normal situations.
                                // note: this was initially added in support of the tiny stack the PS3 had.
                            readyQueueOverflow = true;
                        }
                    }
                    else
                    {
                        outstandingBytes += entry->mDataLen;
                        outstandingNextSendTime = udpMin(outstandingNextSendTime, optimalResendDelay - UdpMisc::ClockDiff(entry->mLastTimeStamp, hotClock));
                    }

                        // if we have reached a point in the queue where there are no sent packets
                        // and our outstanding bytes plus how much we intend to send is greater than the window we have
                        // then we can quit step-2, since there is nothing else to be gained by continuing.
                    if (entry->mFirstTimeStamp == 0 && (windowSpaceLeft <= 0))
                        break;
                }
            }

                // second, send ready entries until the max outstanding is reached
            int toleranceLossCount = 0;
            bool allowWindowReset = (UdpMisc::ClockDiff(mWindowResetTime, hotClock) > mAveragePingTime);        // only allow it to reset the window again if it has been longer than the average ping time
            int trickleSent = 0;
            PhysicalPacket **readyWalk = readyQueue;
            PhysicalPacket *pendingReliableBasePtr = &mPhysicalPackets[mReliableOutgoingPendingId % mConfig.maxOutstandingPackets];
            while (readyWalk < readyPtr && outstandingBytes < useMaxOutstandingBytes)
            {
                    // prepare packet and send it
                PhysicalPacket *entry = *readyWalk++;

                    // prepare reliable header and send it with data
                const udp_uchar *parentBase = (const udp_uchar *)entry->mParent->GetDataPtr();

                bool fragment = false;
                if (entry->mDataPtr != parentBase || entry->mDataLen != entry->mParent->GetDataLen())
                    fragment = true;

                    // we can calculate what our reliableId should be based on our position in the array
                    // need to handle the case where we wrap around the end of the array
                udp_int64 reliableId;
                if (entry >= pendingReliableBasePtr)
                {
                    reliableId = mReliableOutgoingPendingId + (entry - pendingReliableBasePtr);
                }
                else
                {
                    reliableId = mReliableOutgoingPendingId + (&mPhysicalPackets[mConfig.maxOutstandingPackets] - pendingReliableBasePtr) + (entry - &mPhysicalPackets[0]);
                }

                    // prep the actual packet and send it
                bufPtr = buf;
                *bufPtr++ = 0;
                *bufPtr++ = (udp_uchar)(((fragment) ? UdpConnection::cUdpPacketFragment1 : UdpConnection::cUdpPacketReliable1) + mChannelNumber);    // mark us as a fragment if we are one
                bufPtr += UdpMisc::PutValue16(bufPtr, (udp_ushort)(reliableId & 0xffff));
                if (fragment && entry->mDataPtr == parentBase)
                    bufPtr += UdpMisc::PutValue32(bufPtr, entry->mParent->GetDataLen());        // first fragment has a total-length byte after the reliable header
                mUdpConnection->BufferedSend(buf, (int)(bufPtr - buf), entry->mDataPtr, entry->mDataLen, false);

                    // update state information
                if (entry->mFirstTimeStamp == 0)
                {
                    entry->mFirstTimeStamp = hotClock;
                }
                else
                {
                        // trying to send the packet again, let's see how long we have been trying to send this packet.  If we
                        // have an unacknowledged timeout set and it is older than that, then terminate the connection.
                        // note: we only check for the oldest unacknowledged age against the timeout at the point in time
                        // that we are considering sending the packet again.  This can technically cause it to wait slightly
                        // longer than the specified timeout setting before disconnecting the connection, but should be
                        // close enough for all practical purposes and allows for more efficient processing of this setting internally.
                    if (mUdpConnection->mUdpManager->mParams.oldestUnacknowledgedTimeout > 0)
                    {
                        int age = UdpMisc::ClockDiff(entry->mFirstTimeStamp, hotClock);
                        if (age > mUdpConnection->mUdpManager->mParams.oldestUnacknowledgedTimeout)
                        {
                            mUdpConnection->InternalDisconnect(0, UdpConnection::cDisconnectReasonUnacknowledgedTimeout);
                            return(0);
                        }
                    }

                        // were we resent because of a later ack came in? or because we timed out?
                    if (entry->mLastTimeStamp < mLastTimeStampAcknowledged)
                    {
                            // we are resending this packet due to an accelleration (receiving a later packet ack)
                            // so recalc slow start threshhold and congestion window size as per Reno fast-recovery algorithm
                        if (allowWindowReset && toleranceLossCount > mConfig.toleranceLossCount)
                        {
                            allowWindowReset = false;
                            mWindowResetTime = hotClock;
                            mCongestionWindowSize = mCongestionWindowSize * 3 / 4;
                            mCongestionWindowSize = udpMax(mCongestionWindowMinimum, mCongestionWindowSize);    // never let congestion window get smaller than a single packet
                            mCongestionSlowStartThreshhold = mCongestionWindowSize;
                            useMaxOutstandingBytes = udpMin(mConfig.maxOutstandingBytes, mCongestionWindowSize);
                        }

                            // when resends are caused by a selective ack, that means later data is making it, so we may not be overloading
                            // the connection after all.  Reno fast recovery calls for less shrinking of the window in this circumstance
                            // already.  This tolerance code allows us to do even better, by allowing us to experience some small amount
                            // of packetloss without immediately considering such loss to be indicative that the connection is being overloaded.
                            // It does this by only allowing the window to get reset in cases where teh selective-ack forces a certain number
                            // of packets to be accellerated.  If the application sets the tolerance at something like 10, then isolated packet
                            // losses will have no effect on the flow control window, yet a burst of loss of 10 would more strongly indicate
                            // a overloaded condition.  Typically an application will only set this tolerance if they need extremely high
                            // bandwidth (ie, can't afford the window getting reset), and are known to be on a connection that may experience
                            // random packetloss.  This is typical of a LFN type network.
                        toleranceLossCount++;

                        mStatResentPacketsAccelerated++;
                        mUdpConnection->mConnectionStats.resentPacketsAccelerated++;
                        mUdpConnection->mUdpManager->IncrementResentPacketsAccelerated();
                    }
                    else
                    {
                            // we are resending this packet due to a timeout, so we are seriously overloading things probably
                            // so recalc slow start threshhold and congestion window size as per Reno algorithm
                            // no tolerance on timed-out window resets, since by definition you have not received later acks
                            // or this would be an accellerated timeout, which means that either your volume is really low
                            // anyways (so resetting the window is fine), or you seriously overloaded the other side as nothing
                            // later made it either.
                        if (allowWindowReset)
                        {
                            allowWindowReset = false;
                            mWindowResetTime = hotClock;

                            mCongestionSlowStartThreshhold = udpMax(mMaxDataBytes * 2, mCongestionWindowSize / 2);
                            mCongestionWindowSize = mCongestionWindowStart;
                            useMaxOutstandingBytes = udpMin(mConfig.maxOutstandingBytes, mCongestionWindowSize);

                                // because a resend has occurred due to a timeout, slow down the resend times slightly
                                // when things start flowing again, it will fix itself up quickly anyways
                            mAveragePingTime += 100;

                                // When a connection goes temporarily dead, everything that is in the current
                                // window will end up getting timedout.  If the window were large, this could result in the
                                // mAveragePingTime growing quite large and creating very long stalls in the pipe once it does
                                // start moving again.  To prevent this, we cap mAveragePingTime when these events occur to prevent
                                // long stalls when the pipe finally reopens.
                            mAveragePingTime = udpMin(mConfig.resendDelayCap, mAveragePingTime);
                        }

                        mStatResentPacketsTimedOut++;
                        mUdpConnection->mConnectionStats.resentPacketsTimedOut++;
                        mUdpConnection->mUdpManager->IncrementResentPacketsTimedOut();
                    }
                }

                entry->mLastTimeStamp = hotClock;

                outstandingNextSendTime = udpMin(outstandingNextSendTime, optimalResendDelay);        // this packet is now outstanding, so factor it into the outstandingNextSendTime calculation
                outstandingBytes += entry->mDataLen;
                mTrickleLastSend = hotClock;
                trickleSent += entry->mDataLen;

                if (mConfig.trickleSize != 0 && trickleSent >= mConfig.trickleSize)
                    break;
            }

            if (outstandingBytes >= useMaxOutstandingBytes)
            {
                mMaxxedOutCurrentWindow = true;
            }

            // if we filled up the ready queue (ie, if we may still have data to send)
            // and we still have space left in the flow window, then repeat the process.
            // this should only happen when the flow-control window is enormous.
        } while (readyQueueOverflow && !mMaxxedOutCurrentWindow);
    }
    else
    {
            // we have nothing in the pipe at all, reset the congestion window (this means everything has been acked, so the pipe is totally empty
            // we need to reset the window back to prevent a sudden flood next time a large chunk of data is sent.
            // we also need to avoid having the slowly sent reliable packets constantly increase the window size (since none will ever get lost)
            // such that when it does come time to send a big chunk of data, it thinks the window-size is enormous.
            // resetting the window back to small will only have an effect if a large chunk of data is then sent, at which time it will quickly
            // ramp up with the slow-start method.
        mCongestionWindowSize = mCongestionWindowStart;
    }

    // printf("%d,%d\n", mCongestionWindowSize, mCongestionSlowStartThreshhold);    // useful debug output for graphing congestion window

    int nextAllowedSendTime = mConfig.trickleRate - UdpMisc::ClockDiff(mTrickleLastSend, hotClock);
    nextAllowedSendTime = udpMax(0, udpMax(nextAllowedSendTime, outstandingNextSendTime));
    mNextNeedTime = hotClock + nextAllowedSendTime;
    return(nextAllowedSendTime);
}

void UdpReliableChannel::GetChannelStatus(UdpConnection::ChannelStatus *channelStatus) const
{
    int coalesceBytes = 0;
    if (mCoalescePacket != nullptr)
        coalesceBytes = (int)(mCoalesceEndPtr - mCoalesceStartPtr);

    channelStatus->totalPendingBytes = mLogicalBytesQueued + mReliableOutgoingBytes + coalesceBytes;
    channelStatus->queuedPackets = mLogicalPacketList.Count();
    channelStatus->queuedBytes = mLogicalBytesQueued;
    channelStatus->incomingLargeTotal = mBigDataTargetLen;
    channelStatus->incomingLargeSoFar = mBigDataLen;
    channelStatus->duplicatePacketsReceived = mStatDuplicatePacketsReceived;
    channelStatus->resentPacketsAccelerated = mStatResentPacketsAccelerated;
    channelStatus->resentPacketsTimedOut = mStatResentPacketsTimedOut;
    channelStatus->congestionSlowStartThreshhold = mCongestionSlowStartThreshhold;
    channelStatus->congestionWindowSize = mCongestionWindowSize;
    channelStatus->ackAveragePing = mAveragePingTime;
    channelStatus->oldestUnacknowledgedAge = 0;

    if (mReliableOutgoingPendingId < mReliableOutgoingId)
    {
            // oldest pending packet will be definition be the oldestUnacknowledged, since it is impossible
            // for any packet after it to have possibly been sent before it was sent for its first time
            // since queue is effectively in first-send order.  It is also impossible for this packet to have
            // been acknowledged, since the pending id advances the moment the oldest is acknowledged, meaning
            // that the pendingId is always pointing to something that has either been sent (or not sent at all)
        PhysicalPacket *entry = &mPhysicalPackets[mReliableOutgoingPendingId % mConfig.maxOutstandingPackets];
        if (entry->mFirstTimeStamp != 0)        // if has been sent (we know it hasn't been acknowledged or we couldn't possibly be pointing at it as pending)
        {
            if (mUdpConnection->GetUdpManager() != nullptr)
            {
                channelStatus->oldestUnacknowledgedAge = mUdpConnection->GetUdpManager()->CachedClockElapsed(entry->mFirstTimeStamp);
            }
        }
    }
}

void UdpReliableChannel::ReliablePacket(const udp_uchar *data, int dataLen)
{
    udp_uchar buf[256];
    udp_uchar *bufPtr;

    if (dataLen <= UdpConnection::cUdpPacketReliableSize)
    {
        mUdpConnection->CallbackCorruptPacket(data, dataLen, cUdpCorruptionReasonReliablePacketTooShort);
        return;
    }

    int packetType = data[1];
    udp_ushort reliableStamp = UdpMisc::GetValue16(data + 2);
    udp_int64 reliableId = GetReliableIncomingId(reliableStamp);

    if (reliableId >= mReliableIncomingId + mConfig.maxInstandingPackets)
        return;        // if we do not have buffer space to hold onto this packet, then we simply must pretend like it was lost

    if (reliableId >= mReliableIncomingId)
    {
        ReliablePacketMode mode = (ReliablePacketMode)((packetType - UdpConnection::cUdpPacketReliable1) / cReliableChannelCount);

            // is this the packet we are waiting for
        if (mReliableIncomingId == reliableId)
        {
                // if so, process it immediately
            ProcessPacket(mode, data + UdpConnection::cUdpPacketReliableSize, dataLen - UdpConnection::cUdpPacketReliableSize);
            mReliableIncomingId++;

                // process other packets that have arrived
            while (mReliableIncoming[mReliableIncomingId % mConfig.maxInstandingPackets].mPacket != nullptr)
            {
                int spot = (int)(mReliableIncomingId % mConfig.maxInstandingPackets);
                if (mReliableIncoming[spot].mMode != cReliablePacketModeDelivered)
                {
                    ProcessPacket(mReliableIncoming[spot].mMode, (udp_uchar *)mReliableIncoming[spot].mPacket->GetDataPtr(), mReliableIncoming[spot].mPacket->GetDataLen());
                }
                
                mReliableIncoming[spot].mPacket->Release();
                mReliableIncoming[spot].mPacket = nullptr;
                mReliableIncomingId++;
            }
        }
        else
        {
                // not the one we need next, but it is later than the one we need , so store it in our buffer until it's turn comes up
            int spot = (int)(reliableId % mConfig.maxInstandingPackets);
            if (mReliableIncoming[spot].mPacket == nullptr)        // only make the copy of it if we don't already have it in our buffer (in cases where it was sent twice, there would be no harm in the copy again since it must be the same packet, it's just inefficient)
            {
                mReliableIncoming[spot].mMode = mode;
                mReliableIncoming[spot].mPacket = mUdpConnection->mUdpManager->CreatePacket(data + UdpConnection::cUdpPacketReliableSize, dataLen - UdpConnection::cUdpPacketReliableSize);

                    // on out of order deliver, we need to keep a copy of it as if we were doing ordered-delivery in order to prevent duplicates
                    // we will mark the packet in the queue as already delivered to prevent it from getting delivered a second time when the stalled packet
                    // arrives and unwinds the queue
                if (mode == cReliablePacketModeReliable && mConfig.outOfOrder)
                {
                    ProcessPacket(cReliablePacketModeReliable, (udp_uchar *)mReliableIncoming[spot].mPacket->GetDataPtr(), mReliableIncoming[spot].mPacket->GetDataLen());
                    mReliableIncoming[spot].mMode = cReliablePacketModeDelivered;
                }
            }
            else
            {
                mStatDuplicatePacketsReceived++;
                mUdpConnection->mConnectionStats.duplicatePacketsReceived++;
                mUdpConnection->mUdpManager->IncrementDuplicatePacketsReceived();
            }
        }
    }
    else
    {
        mStatDuplicatePacketsReceived++;
        mUdpConnection->mConnectionStats.duplicatePacketsReceived++;
        mUdpConnection->mUdpManager->IncrementDuplicatePacketsReceived();
    }

    bool ackAll = false;
    bufPtr = buf;
    *bufPtr++ = 0;
    if (mReliableIncomingId > reliableId)
    {
            // ack everything up to the current head of our chain (minus one since the stamp represents the next one we want to get)
        *bufPtr++ = (udp_uchar)(UdpConnection::cUdpPacketAckAll1 + mChannelNumber);
        bufPtr += UdpMisc::PutValue16(bufPtr, (udp_ushort)((mReliableIncomingId - 1) & 0xffff));
        ackAll = true;
    }
    else
    {
            // a simple ack for us only
        *bufPtr++ = (udp_uchar)(UdpConnection::cUdpPacketAck1 + mChannelNumber);
        bufPtr += UdpMisc::PutValue16(bufPtr, (udp_ushort)(reliableId & 0xffff));
    }

    if (mBufferedAckPtr != nullptr && mConfig.ackDeduping && ackAll)
    {
        memcpy(mBufferedAckPtr, buf, bufPtr - buf);
    }
    else
    {
        udp_uchar *ptr = mUdpConnection->BufferedSend(buf, (int)(bufPtr - buf), nullptr, 0, true);    // safe to append on our data, it is stack data
        if (mBufferedAckPtr == nullptr)
        {
                // the buffered-ack ptr should always point to the earliest ack in the buffer, such that
                // a replacement ack-all will be processed by the receiver before any selective acks that may
                // have been buffered up.  Thus, only repoint the buffered ack ptr if it was previously unset.
            mBufferedAckPtr = ptr;
        }
    }
}

void UdpReliableChannel::ProcessPacket(ReliablePacketMode mode, const udp_uchar *data, int dataLen)
{
    assert(dataLen > 0);

        // if we have a big packet under construction already, or we are a fragment and thus need to be constructing one, then append this on the end (will create new if it is the first fragment)
    if (mode == cReliablePacketModeReliable)
    {
            // we are not a fragment, nor was there a fragment in progress, so we are a simple reliable packet, just send it to the app
        if (mBigDataPtr != nullptr)
        {
            mUdpConnection->CallbackCorruptPacket(data, dataLen, cUdpCorruptionReasonFragmentExpected);
            return;
        }

        mUdpConnection->ProcessCookedPacket(data, dataLen);
    }
    else if (mode == cReliablePacketModeFragment)
    {
            // append onto end of big packet (or create new big packet if not existing already)
        if (mBigDataPtr == nullptr)
        {
            if (dataLen < 4)
            {
                mUdpConnection->CallbackCorruptPacket(data, dataLen, cUdpCorruptionReasonFragmentBad);
                return;
            }

            mBigDataTargetLen = UdpMisc::GetValue32(data);        // first fragment has a total-length int header on it.
            
            if (mBigDataTargetLen <= 0)     // negative or zero sized chunk to follow means packet corruption or tampering for sure
            {
                mUdpConnection->CallbackCorruptPacket(data, dataLen, cUdpCorruptionReasonFragmentBad);
                return;
            }

            if (mBigDataTargetLen > mUdpConnection->mUdpManager->mParams.incomingLogicalPacketMax)
            {
                mUdpConnection->CallbackCorruptPacket(data, dataLen, cUdpCorruptionReasonFragmentOversized);
                return;
            }

            mBigDataPtr = new udp_uchar[mBigDataTargetLen];
            mBigDataLen = 0;
            data += 4;
            dataLen -= 4;
        }

        int safetyMax = udpMin(mBigDataTargetLen - mBigDataLen, dataLen);    // can't happen in theory since they should add up exact, but protect against it if it does
        assert(safetyMax == dataLen);
        memcpy(mBigDataPtr + mBigDataLen, data, safetyMax);
        mBigDataLen += safetyMax;

        if (mBigDataTargetLen == mBigDataLen)
        {
                // send big-packet off to application
            mUdpConnection->ProcessCookedPacket(mBigDataPtr, mBigDataLen);

                // delete big packet, and reset
            delete[] mBigDataPtr;
            mBigDataLen = 0;
            mBigDataTargetLen = 0;
            mBigDataPtr = nullptr;
        }
    }
}

void UdpReliableChannel::AckAllPacket(const udp_uchar *data, int dataLen)
{
    if (dataLen < 4)
    {
        mUdpConnection->CallbackCorruptPacket(data, dataLen, cUdpCorruptionReasonAckBad);
        return;
    }

    udp_int64 reliableId = GetReliableOutgoingId((udp_ushort)UdpMisc::GetValue16(data + 2));

    if (mReliableOutgoingPendingId > reliableId)
    {
            // if we ackall'ed a packet and everything before the ackall address had already been acked, then we know
            // for certainty that we sent a packet over again that did not need to be sent over again (ie. wasn't lost, just slow)
            // so adjust the mAveragePingTime upward to slow down future resends
        mAveragePingTime += 400;
        mAveragePingTime = udpMin(mConfig.resendDelayCap, mAveragePingTime);
    }

    for (udp_int64 i = mReliableOutgoingPendingId; i <= reliableId; i++)
    {
        Ack(i);
    }
}

void UdpReliableChannel::Ack(udp_int64 reliableId)
{
        // if packet being acknowledged is possibly in our resend queue, then check for it
    if (reliableId >= mReliableOutgoingPendingId && reliableId < mReliableOutgoingId)
    {
        int pos = (int)(reliableId % mConfig.maxOutstandingPackets);
        PhysicalPacket *entry = &mPhysicalPackets[pos];

        if (entry->mDataPtr != nullptr)        // if this packet has not been acknowledged yet (sometimes we get back two acks for the same packet)
        {
            mNextNeedTime = 0;        // something got acked, so we actually need to take the time next time it is offered

                    // if the last time we gave this reliable channel processing time, it filled up the entire sliding window
                    // then go ahead and increase the window-size when incoming acks come in.  However, if the window wasn't full
                    // then don't increase the window size.  The problem is, a game application is likely to send reliable data
                    // at a relatively slow rate (2k/second for example), never filling the window.  The net result would be that
                    // every acknowledged packet would increase the window size, giving the reliable channel the impression that
                    // it's window can be very very large, when in fact, it is only not losing packets because the application is pacing
                    // itself.  The window could grow enormous, even 200k for a modem.  Then, if the application were to dump a load
                    // of data onto us all at once, it would flush it all out at once thinking it had a big window.  By only increasing 
                    // the window size when we have high enough volume to fill the window, we ensure this does not happen.  TCP does a similar
                    // thing, but what they do is reset the window if there has been a long stall.  We do that too, but because we are a game
                    // application that is likely to pace the data at the application level, we have a unique circumstances that need addressing.
            if (mMaxxedOutCurrentWindow)
            {
                if (mCongestionWindowSize < mCongestionSlowStartThreshhold)
                {
                    mCongestionWindowSize += mMaxDataBytes;    // slow-start mode
                }
                else
                {
                    int increase = (mMaxDataBytes * mMaxDataBytes / mCongestionWindowSize);
                    mCongestionWindowSize += udpMax(1, increase);    // congestion mode

                }
            }

            if (entry->mLastTimeStamp == entry->mFirstTimeStamp)
            {


                    // if the packet that is being acknowledged was only sent once, then we can safely use
                    // the round-trip time as an accurate measure of ping time.  By knowing ping time, we can
                    // better (more agressively) schedule resends of lost packets.  We will use a moving average
                    // that weights the current packet as 1/4 the average.
                int thisPingTime = mUdpConnection->GetUdpManager()->CachedClockElapsed(entry->mFirstTimeStamp);
                mAveragePingTime = (mAveragePingTime * 3 + thisPingTime) / 4;
            }

                // what this is doing is if we receive an ACK for a packet that was sent at TIME_X
                // we can assume that all packets sent before TIME_X that are not yet acknowledge were lost
                // and we can resend them immediately
                // since we do not know whether this ack is for last packet we sent, we have to assume it is for the first time this packet was sent (if sent multiple times)
                // otherwise, we could resend it, then receive the ack from the first packet, and think our last-ack time is the time of the second outgoing packet
                // which would cause about every packet in the queue to resend, even if they had just been sent
                // in situations where the first packet truely was lost and this is an ACK of the second packet, then the only
                // harm done is that the we may not resend some of the earlier sent packets quite as quickly.  This will only
                // happen in situations where a packet that was truely lost gets acked on it's second attempt...we just
                // won't be using that ack for the purposes of accelerating other resends...since odds are a non-lost packet
                // will accelerate those other resends shortly anyhow, there really is no loss
                // (note: we used to only set this value forward for packets that were never lost (one time sends); however, if this stamp
                // ever got set way high for some reason (in theory it can't happen), then we would get into a situation where it would
                // rapidly resend and possibly never get reset, causing infinite rapid resends, so we now set it every time to the first-stamp)
                // which will be safe, even if the packet were resent.)
            mLastTimeStampAcknowledged = entry->mFirstTimeStamp;


                // this packet we have queued has been acknowledged, so delete it from queue
            mReliableOutgoingBytes -= entry->mDataLen;
            entry->mDataLen = 0;
            entry->mDataPtr = nullptr;
            entry->mParent->Release();
            entry->mParent = nullptr;

                // advance the pending ptr until it reaches outgoingId or an entry that has yet to acknowledged
            while (mReliableOutgoingPendingId < mReliableOutgoingId)
            {
                if (mPhysicalPackets[mReliableOutgoingPendingId % mConfig.maxOutstandingPackets].mDataPtr != nullptr)
                    break;
                mReliableOutgoingPendingId++;
            }
        }
        else
        {
                // we got an ack for a packet that has already been acked.  This could be due to an ack-all packet that covered us so statistically
                // we can't do much with this information.
        }
    }

        // we don't need to try rescheduling ourself here, since our connection object reschedules to go immediately whenever any type
        // of packet arrives (including ack packets)
}


UdpReliableChannel::IncomingQueueEntry::IncomingQueueEntry()
{
    mPacket = nullptr;
    mMode = UdpReliableChannel::cReliablePacketModeReliable;
}

UdpReliableChannel::IncomingQueueEntry::~IncomingQueueEntry()
{
    if (mPacket != nullptr)
        mPacket->Release();
}


UdpReliableChannel::PhysicalPacket::PhysicalPacket()
{
    mParent = nullptr;
}

UdpReliableChannel::PhysicalPacket::~PhysicalPacket()
{
    if (mParent != nullptr)
        mParent->Release();
}


}   // namespace
