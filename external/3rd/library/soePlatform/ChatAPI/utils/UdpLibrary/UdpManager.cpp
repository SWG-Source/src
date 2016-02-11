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

namespace UdpLibrary
{


    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // UdpManagerHandler default implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////////
void UdpManagerHandler::OnConnectRequest(UdpConnection * /*con*/)
{
}

int UdpManagerHandler::OnUserSuppliedEncrypt(UdpConnection * /*con*/, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    memcpy(destData, sourceData, sourceLen);
    return(sourceLen);
}

int UdpManagerHandler::OnUserSuppliedEncrypt2(UdpConnection * /*con*/, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    memcpy(destData, sourceData, sourceLen);
    return(sourceLen);
}

int UdpManagerHandler::OnUserSuppliedDecrypt(UdpConnection * /*con*/, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    memcpy(destData, sourceData, sourceLen);
    return(sourceLen);
}

int UdpManagerHandler::OnUserSuppliedDecrypt2(UdpConnection * /*con*/, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    memcpy(destData, sourceData, sourceLen);
    return(sourceLen);
}


    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // SimulationParameters implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////////
UdpSimulationParameters::UdpSimulationParameters()
{
    simulateIncomingByteRate = 0;
    simulateIncomingLossPercent = 0.0f;
    simulateOutgoingByteRate = 0;
    simulateOutgoingLossPercent = 0.0f;
    simulateDestinationOverloadLevel = 0;
    simulateOutgoingOverloadLevel = 0;
    simulateOutgoingLatency = 0;
    simulateIncomingLatency = 0;
}


    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // UdpManager implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////////
UdpManager::UdpManager(const UdpParams *params) : mConnectionList(&UdpConnection::mConnectionLink)
                                                         , mDisconnectPendingList(&UdpConnection::mDisconnectPendingLink)
                                                         , mSimulateIncomingList(&SimulateQueueEntry::mLink)
                                                         , mSimulateOutgoingList(&SimulateQueueEntry::mLink)
                                                         , mPoolCreatedList(&PooledLogicalPacket::mCreatedLink)
                                                         , mPoolAvailableList(&PooledLogicalPacket::mAvailableLink)
                                                         , mAvailableEventList(&CallbackEvent::mLink)
                                                         , mEventList(&CallbackEvent::mLink)
{
    assert(params->clockSyncDelay >= 0);                            // negative clockSyncDelay is not allowed (makes no sense)
    assert(params->crcBytes >= 0 && params->crcBytes <= 4);            // crc bytes must be between 0 and 4
    assert(params->encryptMethod[0] >= 0 && params->encryptMethod[0] < cEncryptMethodCount);        // illegal encryption method specified
    assert(params->encryptMethod[1] >= 0 && params->encryptMethod[1] < cEncryptMethodCount);        // illegal encryption method specified
    assert(params->hashTableSize > 0);                                // a hash table size greater than zero is required
    assert(params->maxRawPacketSize >= 64);                            // raw packet size must be at least 64 bytes
    assert(params->incomingBufferSize >= params->maxRawPacketSize);    // incoming socket buffer size must be at least as big as one raw packet
    assert(params->keepAliveDelay >= 0);                            // keep alive delay can't be negative
    assert(params->portAliveDelay >= 0);                            // port alive delay can't be negative
    assert(params->maxConnections > 0);                                // must have at least 1 connection allowed
    assert(params->outgoingBufferSize >= params->maxRawPacketSize);    // outgoing socket buffer must larger than a raw packet size
    assert(params->packetHistoryMax > 0);                            // packet history must be at least 1
    assert(params->port >= 0);                                        // port cannot be negative
    assert(params->userSuppliedEncryptExpansionBytes + params->userSuppliedEncryptExpansionBytes2 < params->maxRawPacketSize);    // if encryption expansion is larger than raw packet size, we are screwed
    assert(params->reliable[0].maxOutstandingBytes >= params->maxRawPacketSize);
    assert(params->reliable[1].maxOutstandingBytes >= params->maxRawPacketSize);
    assert(params->reliable[2].maxOutstandingBytes >= params->maxRawPacketSize);
    assert(params->reliable[3].maxOutstandingBytes >= params->maxRawPacketSize);
    assert(params->port != 0 || params->portRange == 0);

    mParams = *params;
    mParams.maxRawPacketSize = udpMin(mParams.maxRawPacketSize, (int)cHardMaxRawPacketSize);
    if (mParams.maxDataHoldSize == -1)
    {
        mParams.maxDataHoldSize = mParams.maxRawPacketSize;
    }
    if (mParams.pooledPacketSize == -1)
    {
        mParams.pooledPacketSize = mParams.maxRawPacketSize;
    }

    mParams.maxDataHoldSize = udpMin(mParams.maxDataHoldSize, mParams.maxRawPacketSize);
    mParams.packetHistoryMax = udpMax(1, mParams.packetHistoryMax);
    mPacketHistoryPosition = 0;
    mPassThroughData = nullptr;
    mBackgroundThread = nullptr;

    if (mParams.udpDriver != nullptr)
    {
        mDriver = mParams.udpDriver;
    }
    else
    {
        mDriver = new UdpPlatformDriver();
    }

    typedef PacketHistoryEntry *PacketHistoryEntryPtr;
    mPacketHistory = new PacketHistoryEntryPtr[mParams.packetHistoryMax];

    int i;
    for (i = 0; i < mParams.packetHistoryMax; i++)
    {
        mPacketHistory[i] = new PacketHistoryEntry(mParams.maxRawPacketSize);
    }

    Clock();        // call Clock to initialize mCachedClock
    ResetStats();
    mRandomSeed = (int)CachedClock();
    mLastReceiveTime = 0;
    mLastSendTime = 0;
    mLastEmptySocketBufferStamp = 0;
    mProcessingInducedLag = 0;
    mMinimumScheduledStamp = 0;
    mInsideGiveTime = false;
    mEventListBytes = 0;

    for (i = 0; i < mParams.pooledPacketInitial && i < mParams.pooledPacketMax; i++)
    {
        PooledLogicalPacket *lp = new PooledLogicalPacket(this, mParams.pooledPacketSize);
        PoolReturn(lp);   // steals our reference
    }

    mSimulateNextOutgoingTime = 0;
    mSimulateNextIncomingTime = 0;
    mSimulateOutgoingQueueBytes = 0;

    if (mParams.avoidPriorityQueue)
        mPriorityQueue = nullptr;
    else
        mPriorityQueue = new PriorityQueue<UdpConnection, UdpClockStamp>(mParams.maxConnections);

    mAddressHashTable = new ObjectHashTable<UdpConnection, HashTableMember1<UdpConnection> >(mParams.hashTableSize);
    mConnectCodeHashTable = new ObjectHashTable<UdpConnection, HashTableMember2<UdpConnection> >(udpMax(mParams.hashTableSize / 5, 10));        // rarely used, so make it a fraction of the main tables size

    if (mParams.portRange == 0)
    {
        CreateAndBindSocket(mParams.port);
    }
    else
    {
        int r = Random() % mParams.portRange;
        for (int i = 0; i < mParams.portRange; i++)
        {
            CreateAndBindSocket(mParams.port + ((r + i) % mParams.portRange));
            if (mErrorCondition != cErrorConditionCouldNotBindSocket)
                break;
        }
    }
}

UdpManager::~UdpManager()
{
        // Since the background thread holds a reference to the UdpManager while it is running, this should
        // not be possible.  The only way it could happen is if somebody released the manager who should not have.
    assert(mBackgroundThread == nullptr);

        // next thing we must do is tell all the connections to disconnect (which severs their link to this dying manager)
        // this has to be done first since they will call back into us and have themselves removed from our connection-list/priority-queue/etc
        // additionally, they may try sending packets to other connections that are not yet disconnected, so we need to make sure that as a
        // managed we are in a fully operational state until everybody is disconnected from us.
    {
        UdpGuard cg(&mConnectionGuard);

        UdpConnection *cur = mConnectionList.First();
        while (cur != nullptr)
        {
            cur->AddRef();
            cur->InternalDisconnect(0, UdpConnection::cDisconnectReasonManagerDeleted);   // this will cause it to remove us from the mConnectionList
            cur->Release();

                // the above call ended up calling us back and removing them from our connection list, so now mConnectionList is pointing to the next entry
            cur = mConnectionList.First();
        }
    }


    {
            // release any objects that were pending disconnection
        UdpGuard guard(&mDisconnectPendingGuard);
        mDisconnectPendingList.ReleaseAll();
    }


    {
            // next we need to tell all the pooled packets we have created that they can no longer check themselves back into use
            // when they are released
        UdpGuard guard(&mPoolGuard);

        PooledLogicalPacket *walk = mPoolCreatedList.RemoveHead();
        while (walk != nullptr)
        {
            walk->mUdpManager = nullptr;
            walk = mPoolCreatedList.RemoveHead();
        }
            // next release the ones we have in our available pool
        mPoolAvailableList.ReleaseAll();
    }

    if (mParams.lingerDelay != 0)
    {
        mDriver->Sleep(mParams.lingerDelay);        // sleep momentarily before closing socket to give it a chance to empty the socket buffer if there is something in it
    }

    CloseSocket();

    if (mParams.udpDriver == nullptr)
    {
        delete mDriver;     // we were not given a driver to use, so we must own this driver we have, so destroy it
    }
    mDriver = nullptr;

    delete mAddressHashTable;
    delete mConnectCodeHashTable;
    delete mPriorityQueue;
    for (int i = 0; i < mParams.packetHistoryMax; i++)
    {
        delete mPacketHistory[i];
    }
    delete[] mPacketHistory;

        // delete simulation queues
    mSimulateOutgoingList.DeleteAll();
    mSimulateIncomingList.DeleteAll();

        // delete event queues
    mAvailableEventList.DeleteAll();
    mEventList.DeleteAll();
}

void UdpManager::GetSimulation(UdpSimulationParameters *simulationParameters) const
{
    UdpGuard guard(&mGiveTimeGuard);
    *simulationParameters = mSimulation;
}

void UdpManager::SetSimulation(const UdpSimulationParameters *simulationParameters)
{
    UdpGuard guard(&mGiveTimeGuard);
    mSimulation = *simulationParameters;
}

void UdpManager::CreateAndBindSocket(int usePort)
{
    CloseSocket();
    mErrorCondition = cErrorConditionNone;
    if (!mDriver->SocketOpen(usePort, mParams.incomingBufferSize, mParams.outgoingBufferSize, mParams.bindIpAddress))
    {
        mErrorCondition = cErrorConditionCouldNotBindSocket;
    }
}

void UdpManager::CloseSocket()
{
    mDriver->SocketClose();
}


UdpManager::ErrorCondition UdpManager::GetErrorCondition() const
{
    return(mErrorCondition);
}


void UdpManager::ProcessDisconnectPending()
{
    UdpGuard guard(&mDisconnectPendingGuard);

    UdpConnection *entry = mDisconnectPendingList.First();
    while (entry != nullptr)
    {
        UdpConnection *next = mDisconnectPendingList.Next(entry);
        if (entry->GetStatus() == UdpConnection::cStatusDisconnected)
        {
            mDisconnectPendingList.Remove(entry);
            entry->Release();
        }
        entry = next;
    }
}

void UdpManager::RemoveConnection(UdpConnection *con)
{
    assert(con != nullptr);        // attemped to remove a nullptr connection object

        // note: it's a bug to Remove a connection object that is already removed...should never be able to happen.
    UdpGuard cg(&mConnectionGuard);

    if (mPriorityQueue != nullptr)
    {
        mPriorityQueue->Remove(con);
    }
    mConnectionList.Remove(con);
    mAddressHashTable->Remove(con);
    mConnectCodeHashTable->Remove(con);
    con->Release();
}

void UdpManager::AddConnection(UdpConnection *con)
{
    assert(con != nullptr);        // attemped to add a nullptr connection object

    UdpGuard cg(&mConnectionGuard);
    con->AddRef();      // UdpManager keeps a soft reference to the connection (ie. if it sees it is the only one holding a reference, it releases it)
    mConnectionList.InsertHead(con);
    mAddressHashTable->Insert(con, AddressHashValue(con->mIp, con->mPort));
    mConnectCodeHashTable->Insert(con, con->mConnectCode);
}

void UdpManager::FlushAllMultiBuffer()
{
    UdpRef ref(this);   // hold reference to ourselves for duration of call (must be before guard object, since guard object references us).  In this way, if the application releases us during a callback, we don't disappear until the stack is unwound

    mConnectionGuard.Enter();
    UdpConnection *cur = mConnectionList.First();
    if (cur != nullptr)
        cur->AddRef();
    mConnectionGuard.Leave();

    while (cur != nullptr)
    {
        cur->FlushMultiBuffer();

        mConnectionGuard.Enter();
        UdpConnection *next = mConnectionList.Next(cur);
        if (next != nullptr)
            next->AddRef();
        mConnectionGuard.Leave();

        cur->Release();
        cur = next;
    }
}

void UdpManager::DisconnectAll()
{
    UdpRef ref(this);   // hold reference to ourselves for duration of call (must be before guard object, since guard object references us).  In this way, if the application releases us during a callback, we don't disappear until the stack is unwound

    mConnectionGuard.Enter();
    UdpConnection *cur = mConnectionList.First();
    if (cur != nullptr)
        cur->AddRef();
    mConnectionGuard.Leave();

    while (cur != nullptr)
    {
        mConnectionGuard.Enter();
        UdpConnection *next = mConnectionList.Next(cur);
        if (next != nullptr)
            next->AddRef();
        mConnectionGuard.Leave();

        cur->Disconnect();
        cur->Release();
        cur = next;
    }
}

void UdpManager::DeliverEvents(int maxProcessingTime)
{
    UdpRef ref(this);   // hold reference to ourselves for duration of call (must be before guard object, since guard object references us).  In this way, if the application releases us during a callback, we don't disappear until the stack is unwound

    UdpClockStamp start = Clock();
    for (;;)
    {
        CallbackEvent *ce = EventListPop();
        if (ce == nullptr)
            break;

        switch(ce->mEventType)
        {
            case CallbackEvent::cCallbackEventRoutePacket:
            {
                ce->mSource->OnRoutePacket((const udp_uchar *)ce->mPayload->GetDataPtr(), ce->mPayload->GetDataLen());
                break;
            }
            case CallbackEvent::cCallbackEventConnectComplete:
            {
                ce->mSource->OnConnectComplete();
                break;
            }
            case CallbackEvent::cCallbackEventTerminated:
            {
                ce->mSource->OnTerminated();
                break;
            }
            case CallbackEvent::cCallbackEventCrcReject:
            {
                ce->mSource->OnCrcReject((const udp_uchar *)ce->mPayload->GetDataPtr(), ce->mPayload->GetDataLen());
                break;
            }
            case CallbackEvent::cCallbackEventPacketCorrupt:
            {
                ce->mSource->OnPacketCorrupt((const udp_uchar *)ce->mPayload->GetDataPtr(), ce->mPayload->GetDataLen(), ce->mReason);
                break;
            }
            case CallbackEvent::cCallbackEventConnectRequest:
            {
                {   // guard block
                    UdpGuard hguard(&mHandlerGuard);
                    if (mParams.handler != nullptr)
                    {
                        mParams.handler->OnConnectRequest(ce->mSource);
                    }
                }

                if (ce->mSource->GetHandler() == nullptr)   // if application did not set a handler, then the connection is considered refused
                {
                    ce->mSource->InternalDisconnect(0, UdpConnection::cDisconnectReasonConnectionRefused);
                }
                break;
            }
            case CallbackEvent::cCallbackEventNone:
            default:
                break;
        }

        ce->ClearEventData();
        AvailableEventReturn(ce);

        if (ClockElapsed(start) >= maxProcessingTime)
        {
            UdpGuard sg(&mStatsGuard);
            mManagerStats.maxDeliveryTimeExceeded++;
            break;
        }
    }
}

void UdpManager::SetEventQueuing(bool on)
{
    UdpGuard guard(&mGiveTimeGuard);   // don't allow changing of queuing mode while inside GiveTime from another thread
    mParams.eventQueuing = on;
}

bool UdpManager::GetEventQueuing() const
{
    UdpGuard guard(&mGiveTimeGuard);
    return(mParams.eventQueuing);
}

bool UdpManager::GiveTime(int maxPollingTime, bool giveConnectionsTime)
{
    UdpRef ref(this);   // hold reference to ourselves for duration of call (must be before guard object, since guard object references us).  In this way, if the application releases us during a callback, we don't disappear until the stack is unwound
    UdpGuard guard(&mGiveTimeGuard);

    Clock();        // update cached clock

    if (!mParams.eventQueuing && mEventList.Count() > 0)
    {
            // if we have events queued and we are not in queuing mode, we must deliver those events
            // before we can actually do a give time (this should only happen when the queuing mode is changed on the fly)
        DeliverEvents(maxPollingTime);
        return(true);
    }

        //////////////////////////////////////////////
        // do the work (prevent re-entry)
        //////////////////////////////////////////////
    if (mInsideGiveTime)
    {
        assert(0);          // odds are this an application bug, so we want them to see this fairly quickly when it happens
        return(false);      // prevent re-entrancy of GiveTime function, since that will cause all sorts of problems
    }
    mInsideGiveTime = true;

        // process incoming raw packets from the port
    {
        UdpGuard sg(&mStatsGuard);
        mManagerStats.iterations++;
    }

        // if we are currently not queuing, yet we have stuff remaining in the queue, then
        // we must deliver everything in the queue before we can do more processing of the socket
        // note: this would only happen in theory if the queuing mode were being changed on the fly
    bool found = false;
    if (maxPollingTime != 0)
    {
        UdpClockStamp start = CachedClock();
        for (;;)
        {
#ifdef UDPLIBRARY_NO_SIMULATION
            PacketHistoryEntry *e = ActualReceive();
#else
            PacketHistoryEntry *e = SimulationReceive();
#endif

            if (e == nullptr)
            {
                mLastEmptySocketBufferStamp = CachedClock();
                break;
            }

                // if the application takes too long to process packets, or doesn't give the UdpManager frequent enough time via GiveTime
                // then it's possible that we will have a clock-sync packet that is sitting in the socket buffer waiting to be processed
                // we don't want the applications inability to give us frequent processing time to totally whack up the clock sync stuff
                // so we have the clock-sync code ignore clock-sync packets that get stalled in the socket buffer for too long because our
                // application is busy processing other packets that were queued before it, or because the application paused for a long
                // time before calling GiveTime.
                // note: this is intended to prevent cpu induced stalls from causing a sync packet to appear to take longer.  For example
                // if the player is on a modem, it's possible for the socket-buffer to fill up while the application is stalled and cause the
                // the sync-packet to actually get stalled at the terminal buffer on the other end up of the modem.  When the application starts
                // processing again, it will empty the socket-buffer, but then the get an empty-socket-buffer briefly until the terminal server
                // can send the rest of the buffered packets on over.  A large client side receive socket buffer may help in this regard.
            mProcessingInducedLag = CachedClockElapsed(mLastEmptySocketBufferStamp);
            found = true;
            ProcessRawPacket(e);
            
            if (ClockElapsed(start) >= maxPollingTime)
            {
                UdpGuard sg(&mStatsGuard);
                mManagerStats.maxPollingTimeExceeded++;
                break;
            }
        }
    }

    if (giveConnectionsTime)
    {
        if (mPriorityQueue != nullptr)
        {
                // give time to everybody in the priority-queue that needs it
            UdpClockStamp curPriority = CachedClock();

                // at the time we start processing the priority queue, we should effectively be taking a snap-shot
                // of everybody who needs time, before we give anybody time.  Otherwise, it is possible that in the
                // process of giving one connection time, another connection could get bumped up the queue to the point
                // where it needs time now as well (for example, one connection sending another connection data during the
                // give time phase).  Although very rare, in theory this could result in an infinite loop situation.
                // To solve this, we simply set the earliest time period that somebody can schedule for to 1 ms after
                // the current time stamp that we are processing, effectively making it impossible for any connection
                // to be given time twice in the same interation of the loop below

            mConnectionGuard.Enter();
            mMinimumScheduledStamp = curPriority + 1;
            mConnectionGuard.Leave();

            int processed = 0;
            for (;;)
            {
                UdpConnection *top;

                mConnectionGuard.Enter();
                top = mPriorityQueue->TopRemove(curPriority);
                if (top != nullptr)
                    top->AddRef();      // must always addref connections while inside the connection guard
                mConnectionGuard.Leave();
                if (top == nullptr)
                  break;

                top->GiveTime(true);
                top->Release();

                processed++;
            }

            {
                UdpGuard sg(&mStatsGuard);
                mManagerStats.priorityQueueProcessed += processed;
                mManagerStats.priorityQueuePossible += mConnectionList.Count();
            }
        }
        else
        {
                // give time to everybody
            mConnectionGuard.Enter();
            UdpConnection *cur = mConnectionList.First();
            if (cur != nullptr)
                cur->AddRef();
            mConnectionGuard.Leave();

            while (cur != nullptr)
            {
                cur->GiveTime(true);

                mConnectionGuard.Enter();
                UdpConnection *next = mConnectionList.Next(cur);
                if (next != nullptr)
                    next->AddRef();
                mConnectionGuard.Leave();

                cur->Release();
                cur = next;
            }
        }

        ProcessDisconnectPending();
    }


    {
        UdpGuard guard(&mSimulateGuard);
        UdpClockStamp curStamp = CachedClock();

        SimulateQueueEntry *entry = mSimulateOutgoingList.First();
        while (entry != nullptr && curStamp >= mSimulateNextOutgoingTime)
        {
            mSimulateOutgoingList.Remove(entry);
            SimulateQueueEntry *next = mSimulateOutgoingList.First();

                // simulate a delay before next packet is considered (ie. simple lag)
            if (next != nullptr)
            {
                int latencyDelay = (mSimulation.simulateOutgoingLatency - CachedClockElapsed(next->mQueueTime));
                mSimulateNextOutgoingTime = curStamp + latencyDelay;
            }

                // simulate a max data rate
            UdpClockStamp nextRateTime = curStamp;
            if (mSimulation.simulateOutgoingByteRate > 0)
            {
                nextRateTime += (entry->mDataLen * 1000 / mSimulation.simulateOutgoingByteRate);
                mSimulateNextOutgoingTime = udpMax(mSimulateNextOutgoingTime, nextRateTime);
            }

            ActualSendHelper(entry->mData, entry->mDataLen, entry->mIp, entry->mPort);

            UdpConnection *con = AddressGetConnection(entry->mIp, entry->mPort);
            if (con != nullptr)
            {
                con->mSimulateOutgoingQueueBytes -= entry->mDataLen;
                con->Release();
            }
            mSimulateOutgoingQueueBytes -= entry->mDataLen;
            delete entry;

            entry = next;
        }
    }

    mInsideGiveTime = false;
    return(found);
}

UdpConnection *UdpManager::EstablishConnection(const char *serverAddress, int serverPort, int timeout)
{
    UdpGuard guard(&mGiveTimeGuard);      // probably not needed, I don't see any reason we can't do this while GiveTime is happening in the background...the connection list is protected independently...still, better safe than sorry

    assert(serverAddress != nullptr);

    char useServerAddress[512];
    UdpLibrary::UdpMisc::Strncpy(useServerAddress, serverAddress, sizeof(useServerAddress));
    char *portPtr = strchr(useServerAddress, ':');
    if (portPtr != nullptr)
    {
        *portPtr++ = 0;
        serverPort = atoi(portPtr);
    }

    assert(useServerAddress[0] != 0);
    assert(serverPort != 0);        // can't connect to no port

    if (mConnectionList.Count() >= mParams.maxConnections)
        return(nullptr);

        // get server address
    UdpPlatformAddress destIp;
    if (!mDriver->GetHostByName(&destIp, useServerAddress))
    {
        return(nullptr);       // could not resolve name
    }

        // first, see if we already have a connection object managing this ip/port, if we do, then fail
    UdpConnection *con = AddressGetConnection(destIp, serverPort);
    if (con != nullptr)
    {
        con->Release();
        return(nullptr);       // already connected to this address/port
    }
    return(new UdpConnection(this, destIp, serverPort, timeout));
}

void UdpManager::KeepUntilDisconnected(UdpConnection *con)
{
    UdpGuard guard(&mDisconnectPendingGuard);
    con->AddRef();
    mDisconnectPendingList.InsertTail(con);
}

void UdpManager::GetStats(UdpManagerStatistics *stats)
{
    UdpGuard sg(&mStatsGuard);

    assert(stats != nullptr);
    *stats = mManagerStats;
    stats->poolAvailable = mPoolAvailableList.Count();
    stats->poolCreated = mPoolCreatedList.Count();

    {
        UdpGuard guard(&mDisconnectPendingGuard);
        stats->disconnectPendingCount = mDisconnectPendingList.Count();
    }

    stats->connectionCount = mConnectionList.Count();
    stats->eventListCount = mEventList.Count();
    stats->eventListBytes = mEventListBytes;
    stats->elapsedTime = CachedClockElapsed(mManagerStatsResetTime);
}

void UdpManager::ResetStats()
{
    UdpGuard guard(&mStatsGuard);

    mManagerStatsResetTime = CachedClock();
    memset(&mManagerStats, 0, sizeof(mManagerStats));
}

void UdpManager::DumpPacketHistory(const char *filename) const
{
    UdpGuard guard(&mGiveTimeGuard);

    assert(filename != nullptr);
    assert(filename[0] != 0);
    FILE *file = fopen(filename, "wt");
    if (file != nullptr)
    {
            // dump history of packets...
        for (int i = 0; i < mParams.packetHistoryMax; i++)
        {
            int pos = (mPacketHistoryPosition + i) % mParams.packetHistoryMax;

            if (mPacketHistory[pos]->mLen > 0)
            {
                char hold[256];
                udp_uchar *ptr = mPacketHistory[pos]->mBuffer;
                fprintf(file, "%16s,%5d %3d: ", mPacketHistory[pos]->mIp.GetAddress(hold, sizeof(hold)), mPacketHistory[pos]->mPort, mPacketHistory[pos]->mLen);
                int len = mPacketHistory[pos]->mLen;
                while (len-- > 0)
                {
                    fprintf(file, "%02x ", *ptr);
                    ptr++;
                }
                fprintf(file, "\n");
            }
        }
        fclose(file);
    }
}

UdpPlatformAddress UdpManager::GetLocalIp() const
{
    UdpPlatformAddress ip;
    mDriver->SocketGetLocalIp(&ip);
    return(ip);
}

int UdpManager::GetLocalPort() const
{
    return(mDriver->SocketGetLocalPort());
}

UdpManager::PacketHistoryEntry *UdpManager::SimulationReceive()
{
    if (mSimulateIncomingList.Count() == 0 && mSimulation.simulateIncomingLatency == 0)
    {
        return(ActualReceive());
    }

        // pull packets from wire and queue them
    UdpClockStamp curStamp = CachedClock();
    for (;;)
    {
        PacketHistoryEntry *entry = ActualReceive();
        if (entry == nullptr)
            break;

        SimulateQueueEntry *qe = new SimulateQueueEntry(entry->mBuffer, entry->mLen, entry->mIp, entry->mPort, curStamp);
        mSimulateIncomingList.InsertTail(qe);
    }

    SimulateQueueEntry *winner = mSimulateIncomingList.First();
    if (winner != nullptr && CachedClockElapsed(winner->mQueueTime) >= mSimulation.simulateIncomingLatency)
    {
        mSimulateIncomingList.Remove(winner);
        int pos = mPacketHistoryPosition;
        memcpy(mPacketHistory[pos]->mBuffer, winner->mData, winner->mDataLen);
        mPacketHistory[pos]->mLen = winner->mDataLen;
        mPacketHistory[pos]->mIp = winner->mIp;
        mPacketHistory[pos]->mPort = winner->mPort;
        mPacketHistoryPosition = (mPacketHistoryPosition + 1) % mParams.packetHistoryMax;
        delete winner;
        return(mPacketHistory[pos]);
    }
    return(nullptr);
}

UdpManager::PacketHistoryEntry *UdpManager::ActualReceive()
{
    UdpClockStamp curStamp = CachedClock();
    if (mSimulation.simulateIncomingByteRate > 0 && curStamp < mSimulateNextIncomingTime)
        return(nullptr);

    UdpPlatformAddress fromAddress;
    int fromPort = 0;

    for (;;)
    {
        int pos = mPacketHistoryPosition;
        int res = mDriver->SocketReceive((char *)mPacketHistory[pos]->mBuffer, mParams.maxRawPacketSize, &fromAddress, &fromPort);
        if (res < 0)
        {
            break;
        }

            // found data
            // note: this will simulate loss of ICMP errors packets too, since ICMP error packets are return from the driver as inline 0-byte packets
        if (mSimulation.simulateIncomingLossPercent > 0.0f && ((Random() % 10000) < (mSimulation.simulateIncomingLossPercent * 100)))
        {
            continue;    // packet, what packet?
        }

        if (mSimulation.simulateIncomingByteRate > 0)
        {
            mSimulateNextIncomingTime = curStamp + (res * 1000 / mSimulation.simulateIncomingByteRate);
        }

        mPacketHistory[pos]->mLen = res;
        mPacketHistory[pos]->mIp = fromAddress;
        mPacketHistory[pos]->mPort = fromPort;
        mPacketHistoryPosition = (mPacketHistoryPosition + 1) % mParams.packetHistoryMax;

        {
            UdpGuard sg(&mStatsGuard);
            mLastReceiveTime = curStamp;
            mManagerStats.bytesReceived += res;
            mManagerStats.packetsReceived++;
        }
        return(mPacketHistory[pos]);
    }
    return(nullptr);
}

void UdpManager::ActualSend(const udp_uchar *data, int dataLen, UdpPlatformAddress ip, int port)
{
    {
        UdpGuard sg(&mStatsGuard);
        mLastSendTime = CachedClock();
        mManagerStats.bytesSent += dataLen;
        mManagerStats.packetsSent++;
    }

    if (mSimulation.simulateOutgoingByteRate != 0 || mSimulation.simulateOutgoingLatency != 0)
    {
        UdpGuard guard(&mSimulateGuard);

            // simulating outgoing byte-rate, so queue it up for sending later
        if (mSimulation.simulateOutgoingOverloadLevel > 0 && mSimulateOutgoingQueueBytes + dataLen > mSimulation.simulateOutgoingOverloadLevel)
            return;        // no room, packet gets lost

        UdpConnection *con = AddressGetConnection(ip, port);
        if (con != nullptr)
        {
            if (mSimulation.simulateDestinationOverloadLevel > 0 && con->mSimulateOutgoingQueueBytes + dataLen > mSimulation.simulateDestinationOverloadLevel)
            {
                con->Release();
                return;        // no room, packet gets lost
            }
            con->mSimulateOutgoingQueueBytes += dataLen;
            con->Release();
        }
        mSimulateOutgoingQueueBytes += dataLen;
        SimulateQueueEntry *entry = new SimulateQueueEntry(data, dataLen, ip, port, LastSendTime());

        mSimulateOutgoingList.InsertTail(entry);

        if (mSimulateOutgoingList.Count() == 1)
        {
            mSimulateNextOutgoingTime = udpMax(LastSendTime() + mSimulation.simulateOutgoingLatency, mSimulateNextOutgoingTime);
        }
        return;
    }
    ActualSendHelper(data, dataLen, ip, port);
}

void UdpManager::ActualSendHelper(const udp_uchar *data, int dataLen, UdpPlatformAddress ip, int port)
{
    if (mSimulation.simulateOutgoingLossPercent > 0.0f && ((Random() % 10000) < (mSimulation.simulateOutgoingLossPercent * 100)))
        return;

    if (!mDriver->SocketSend((const char *)data, dataLen, &ip, port))
    {
            // assume a send error is a socket overflow, which we track only for statistical purposes
        UdpGuard sg(&mStatsGuard);
        mManagerStats.socketOverflowErrors++;
    }
}

void UdpManager::SendPortAlive(UdpPlatformAddress ip, int port)
{
    char buf[2];
    buf[0] = 0;
    buf[1] = UdpConnection::cUdpPacketPortAlive;
    mDriver->SocketSendPortAlive(buf, 2, &ip, port);
}

void UdpManager::ProcessRawPacket(const PacketHistoryEntry *e)
{
    if (e->mLen == 2 && e->mBuffer[0] == 0 && e->mBuffer[1] == UdpConnection::cUdpPacketPortAlive)
        return;        // port-alive packets are not supposed to reach the destination machine, but on the odd chance they do, pretend like they never existed
    
    UdpConnection *con = AddressGetConnection(e->mIp, e->mPort);

    if (con == nullptr)
    {
        if (e->mLen == 0)     // len = 0 = ICMP error
        {
                // we received an ICMP error packet for an unknown connection, so simply ignore it
                // if we attempt to process it, it will send a destination-unreachable packet to the ip/port in question
                // and create an infinite ICMP reply loop
            return;
        }

            // packet coming from an unknown ip/port
            // if it is a connection request packet, then establish a new connection object to reply to it
            // connection establish packet must always be at least 6 bytes long as we must have a version number, no matter how it changes
        if (e->mBuffer[0] == 0 && e->mBuffer[1] == UdpConnection::cUdpPacketConnect)
        {
            if (mConnectionList.Count() >= mParams.maxConnections)
                return;        // can't handle any more connections, so ignore this request entirely

            if (mParams.handler != nullptr)
            {
                UdpConnection *newcon = new UdpConnection(this, e);
                CallbackConnectRequest(newcon);
                newcon->Release();
            }
        }
        else
        {
            if (mParams.allowPortRemapping)
            {
                if (e->mBuffer[0] == 0 && e->mBuffer[1] == UdpConnection::cUdpPacketRequestRemap)
                {
                        // ok, we got a packet from somebody, that we don't know who they are, but, it appears they are asking
                        // for their address/port to be remapped.  If we allow port (and/or address) remapping, then go ahead
                        // an honor their request if possible
                    udp_uchar *ptr = e->mBuffer + 2;
                    int connectCode = UdpMisc::GetValue32(ptr);
                    ptr += 4;
                    int encryptCode = UdpMisc::GetValue32(ptr);

                    UdpConnection *con = ConnectCodeGetConnection(connectCode);
                    if (con != nullptr)
                    {
                        if (mParams.allowAddressRemapping || con->mIp == e->mIp)
                        {
                                // one final security check to ensure these are really the same connection, compare encryption codes
                            if (con->mConnectionConfig.encryptCode == encryptCode)
                            {
                                    // remapping is allowed, remap ourselves to the address of the incoming request
                                UdpGuard guard(&mConnectionGuard);

                                mAddressHashTable->Remove(con);
                                con->mIp = e->mIp;
                                con->mPort = e->mPort;
                                mAddressHashTable->Insert(con, AddressHashValue(con->mIp, con->mPort));

                                con->Release();
                                return;
                            }
                        }

                        con->Release();
                    }
                }
            }


                // got a packet from somebody and we don't know who they are and the packet we got was not a connection request
                // just in case they are a previous client who thinks they are still connected, we will send them an internal
                // packet telling them that we don't know who they are
            if (mParams.replyUnreachableConnection)
            {
                    // do not reply back with unreachable if the packet coming in is a terminate or unreachable packet itself
                if (e->mBuffer[0] != 0 || (e->mBuffer[0] == 0 && e->mBuffer[1] != UdpConnection::cUdpPacketUnreachableConnection && e->mBuffer[1] != UdpConnection::cUdpPacketTerminate))
                {
                        // since we do not have a connection-object associated with this incoming packet, there is no way we could
                        // encrypt it or add CRC bytes to it, since we have no idea what the other end of the connection is expecting
                        // in this regard.  As such, the UnreachableConnection packet (like the connect and confirm packets) is one
                        // of those internal packet types that is designated as not being encrypted or CRC'ed.
                    unsigned char buf[8];
                    buf[0] = 0;
                    buf[1] = UdpConnection::cUdpPacketUnreachableConnection;
                    ActualSend(buf, 2, e->mIp, e->mPort);
                }
            }
        }
        return;
    }

    con->ProcessRawPacket(e);
    con->Release();
}

UdpConnection *UdpManager::AddressGetConnection(UdpPlatformAddress ip, int port) const
{
    UdpGuard guard(&mConnectionGuard);

    UdpConnection *found = mAddressHashTable->FindFirst(AddressHashValue(ip, port));
    while (found != nullptr)
    {
        if (found->mIp == ip && found->mPort == port)
        {
            found->AddRef();
            return(found);
        }
        found = mAddressHashTable->FindNext(found);
    }
    return(nullptr);
}

UdpConnection *UdpManager::ConnectCodeGetConnection(int connectCode) const
{
    UdpGuard guard(&mConnectionGuard);

    UdpConnection *found = mConnectCodeHashTable->FindFirst(connectCode);
    while (found != nullptr)
    {
        if (found->mConnectCode == connectCode)
        {
            found->AddRef();
            return(found);
        }
        found = mConnectCodeHashTable->FindNext(found);
    }
    return(nullptr);
}

LogicalPacket *UdpManager::CreatePacket(const void *data, int dataLen, const void *data2, int dataLen2)
{
    if (mParams.pooledPacketMax > 0)
    {
        int totalLen = dataLen + dataLen2;
        if (totalLen <= mParams.pooledPacketSize)
        {
            UdpGuard guard(&mPoolGuard);
            PooledLogicalPacket *lp = mPoolAvailableList.RemoveHead();
            if (lp == nullptr)
            {
                    // create a new pooled packet to fulfil request
                lp = new PooledLogicalPacket(this, mParams.pooledPacketSize);
            }
            lp->SetData(data, dataLen, data2, dataLen2);
            return(lp);
        }
    }

    return(UdpMisc::CreateQuickLogicalPacket(data, dataLen, data2, dataLen2));
}

void UdpManager::PoolCreated(PooledLogicalPacket *packet)
{
    UdpGuard guard(&mPoolGuard);
    mPoolCreatedList.InsertHead(packet);
}

void UdpManager::PoolDestroyed(PooledLogicalPacket *packet)
{
    UdpGuard guard(&mPoolGuard);
    mPoolCreatedList.Remove(packet);
}

char *UdpManager::GetLocalString(char *buf, int bufLen) const
{
    if (bufLen < 22)
        return(nullptr);
    UdpPlatformAddress ip = GetLocalIp();
    int port = GetLocalPort();
    char hold[256];
    sprintf(buf, "%s:%d", ip.GetAddress(hold, sizeof(hold)), port);
    return(buf);
}

UdpManager::CallbackEvent *UdpManager::AvailableEventBorrow()
{
    UdpGuard guard(&mAvailableEventGuard);
    CallbackEvent *ce = mAvailableEventList.RemoveHead();
    if (ce == nullptr)
    {
        ce = new CallbackEvent();
    }
    return(ce);
}

void UdpManager::AvailableEventReturn(CallbackEvent *ce)
{
    if (mAvailableEventList.Count() < mParams.callbackEventPoolMax)
    {
        UdpGuard guard(&mAvailableEventGuard);
        mAvailableEventList.InsertHead(ce);
    }
    else
    {
        delete ce;
    }
}

void UdpManager::EventListAppend(CallbackEvent *ce)
{
    UdpGuard guard(&mEventListGuard);
    mEventList.InsertTail(ce);
    if (ce->mPayload != nullptr)
    {
        mEventListBytes += ce->mPayload->GetDataLen();
    }
}

UdpManager::CallbackEvent *UdpManager::EventListPop()
{
    UdpGuard guard(&mEventListGuard);
    CallbackEvent *event = mEventList.RemoveHead();
    if (event != nullptr && event->mPayload != nullptr)
    {
        mEventListBytes -= event->mPayload->GetDataLen();
    }
    return(event);
}


void UdpManager::ThreadStart()
{
    UdpGuard guard(&mThreadGuard);
    if (mBackgroundThread == nullptr)
    {
        mBackgroundThread = new UdpManagerThread(this, mParams.threadSleepTime);
        mBackgroundThread->Start();
    }
}

void UdpManager::ThreadStop()
{
    UdpGuard guard(&mThreadGuard);
    if (mBackgroundThread != nullptr)
    {
        assert(mRefCount > 1);    // caller must hold a reference, and thread must hold a reference, so this should be true.  If it asserts, it means the caller is using a UdpManager that it does not hold a reference to.
        mBackgroundThread->Stop(true);
        mBackgroundThread->Release();
        mBackgroundThread = nullptr;
    }
}

void UdpManager::CallbackRoutePacket(UdpConnection *con, const udp_uchar *data, int dataLen)
{
    if (mParams.eventQueuing)
    {
        CallbackEvent *ce = AvailableEventBorrow();
        LogicalPacket *packet = CreatePacket(data, dataLen);
        ce->SetEventData(CallbackEvent::cCallbackEventRoutePacket, con, packet);
        packet->Release();
        EventListAppend(ce);
    }
    else
    {
        con->OnRoutePacket(data, dataLen);
    }
}

void UdpManager::CallbackConnectComplete(UdpConnection *con)
{
    if (mParams.eventQueuing)
    {
        CallbackEvent *ce = AvailableEventBorrow();
        ce->SetEventData(CallbackEvent::cCallbackEventConnectComplete, con);
        EventListAppend(ce);
    }
    else
    {
        con->OnConnectComplete();
    }
}

void UdpManager::CallbackTerminated(UdpConnection *con)
{
    if (mParams.eventQueuing)
    {
        CallbackEvent *ce = AvailableEventBorrow();
        ce->SetEventData(CallbackEvent::cCallbackEventTerminated, con);
        EventListAppend(ce);
    }
    else
    {
        con->OnTerminated();
    }
}

void UdpManager::CallbackCrcReject(UdpConnection *con, const udp_uchar *data, int dataLen)
{
    if (mParams.eventQueuing)
    {
        CallbackEvent *ce = AvailableEventBorrow();
        LogicalPacket *packet = CreatePacket(data, dataLen);
        ce->SetEventData(CallbackEvent::cCallbackEventCrcReject, con, packet);
        packet->Release();
        EventListAppend(ce);
    }
    else
    {
        con->OnCrcReject(data, dataLen);
    }
}

void UdpManager::CallbackPacketCorrupt(UdpConnection *con, const udp_uchar *data, int dataLen, UdpCorruptionReason reason)
{
    if (mParams.eventQueuing)
    {
        CallbackEvent *ce = AvailableEventBorrow();
        LogicalPacket *packet = CreatePacket(data, dataLen);
        ce->SetEventData(CallbackEvent::cCallbackEventPacketCorrupt, con, packet);
        ce->mReason = reason;
        packet->Release();
        EventListAppend(ce);
    }
    else
    {
        con->OnPacketCorrupt(data, dataLen, reason);
    }
}

void UdpManager::CallbackConnectRequest(UdpConnection *con)
{
    if (mParams.eventQueuing)
    {
        CallbackEvent *ce = AvailableEventBorrow();
        ce->SetEventData(CallbackEvent::cCallbackEventConnectRequest, con);
        EventListAppend(ce);
    }
    else
    {
        {   // guard block
            UdpGuard hguard(&mHandlerGuard);
            if (mParams.handler != nullptr)
            {
                mParams.handler->OnConnectRequest(con);
            }
        }

        if (con->GetHandler() == nullptr)   // if application did not set a handler, then the connection is considered refused
        {
            con->InternalDisconnect(0, UdpConnection::cDisconnectReasonConnectionRefused);
        }
    }
}


    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // CallbackEvent implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////////
UdpManager::CallbackEvent::CallbackEvent()
{
    mEventType = cCallbackEventNone;
    mSource = nullptr;
    mPayload = nullptr;
    mReason = cUdpCorruptionReasonNone;
}

UdpManager::CallbackEvent::~CallbackEvent()
{
    ClearEventData();
}

void UdpManager::CallbackEvent::SetEventData(CallbackEventType eventType, UdpConnection *con, const LogicalPacket *payload)
{
    mEventType = eventType;
    mSource = con;
    mSource->AddRef();
    if (payload != nullptr)
    {
        mPayload = payload;
        mPayload->AddRef();
    }
}

void UdpManager::CallbackEvent::ClearEventData()
{
    if (mSource != nullptr)
    {
        mSource->Release();
        mSource = nullptr;
    }

    if (mPayload != nullptr)
    {
        mPayload->Release();
        mPayload = nullptr;
    }
}



    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // PacketHistory implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////////
UdpManager::PacketHistoryEntry::PacketHistoryEntry(int maxRawPacketSize)
{
    mBuffer = new udp_uchar[maxRawPacketSize];
    mPort = 0;
    mLen = 0;
}

UdpManager::PacketHistoryEntry::~PacketHistoryEntry()
{
    delete[] mBuffer;
}



    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // SimulateQueueEntry functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////
UdpManager::SimulateQueueEntry::SimulateQueueEntry(const udp_uchar *data, int dataLen, UdpPlatformAddress ip, int port, UdpClockStamp queueTime)
{
    mData = new udp_uchar[dataLen];
    mDataLen = dataLen;
    memcpy(mData, data, dataLen);
    mIp = ip;
    mPort = port;
    mQueueTime = queueTime;
}

UdpManager::SimulateQueueEntry::~SimulateQueueEntry()
{
    delete[] mData;
}


    ///////////////////////////////////////////////////////////
    // UdpManagerThread
    ///////////////////////////////////////////////////////////
UdpManagerThread::UdpManagerThread(UdpManager *manager, int sleepTime)
{
    mUdpManager = manager;
    mUdpManager->AddRef();
    mStop = false;
    mSleepTime = sleepTime;
}

UdpManagerThread::~UdpManagerThread()
{
    Stop(true);
    mUdpManager->Release();
}

void UdpManagerThread::Run()
{
    while (!mStop)
    {
        mUdpManager->GiveTime();
        mUdpManager->Sleep(mSleepTime);
    }
}

void UdpManagerThread::Stop(bool waitUntilStopped)
{
    mStop = true;
    if (waitUntilStopped)
    {
        while (IsRunning())
        {
            mUdpManager->Sleep(10);
        }
    }
}


}   // namespace

