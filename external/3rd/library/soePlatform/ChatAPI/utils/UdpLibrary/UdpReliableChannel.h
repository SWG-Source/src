#ifndef UDPLIBRARY_UDPRELIABLECHANNEL_H
#define UDPLIBRARY_UDPRELIABLECHANNEL_H

// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

namespace UdpLibrary
{

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // The purpose of this class is to manage the reliable transmission of packets on top of the inherently
    // unreliable UDP layer.  This is an internal object and should not be manually created or talked to by the user.
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UdpReliableChannel
{
    protected:
        friend class UdpConnection;

        UdpReliableChannel(int channelNumber, UdpConnection *connection, UdpReliableConfig *config);
        ~UdpReliableChannel();
        void GetChannelStatus(UdpConnection::ChannelStatus *channelStatus) const;
        int GetAveragePing() const;
        int TotalPendingBytes() const;        // returns total bytes outstanding

        void ReliablePacket(const udp_uchar *data, int dataLen);
        void Send(const udp_uchar *data, int dataLen, const udp_uchar *data2, int dataLen2);
        void AckPacket(const udp_uchar *data, int dataLen);
        void AckAllPacket(const udp_uchar *data, int dataLen);
        void ClearBufferedAck();
        int GiveTime();

    protected:
        enum ReliablePacketMode { cReliablePacketModeReliable, cReliablePacketModeFragment, cReliablePacketModeDelivered };

        class PhysicalPacket
        {
            public:
                PhysicalPacket();
                ~PhysicalPacket();

            public:
                UdpClockStamp mFirstTimeStamp;
                UdpClockStamp mLastTimeStamp;
                const LogicalPacket *mParent;            // physical packets hold an addref on the logical packet.  Once all of the logical packet data has been divied out to physical packets, the logical queue releases it
                const udp_uchar *mDataPtr;                // within parent's data (it's possible it is not pointing to the beginning in the case of large packets)
                int mDataLen;
        };

        class IncomingQueueEntry
        {
            public:
                IncomingQueueEntry();
                ~IncomingQueueEntry();

            public:
                LogicalPacket *mPacket;
                ReliablePacketMode mMode;
        };
        friend class IncomingQueueEntry;

        udp_int64 GetReliableOutgoingId(int reliableStamp) const;
        udp_int64 GetReliableIncomingId(int reliableStamp) const;
        void Ack(udp_int64 reliableId);
        void ProcessPacket(ReliablePacketMode mode, const udp_uchar *data, int dataLen);
        bool PullDown(int windowSpaceLeft);
        void FlushCoalesce();
        void SendCoalesce(const udp_uchar *data, int dataLen, const udp_uchar *data2 = nullptr, int dataLen2 = 0);
        void QueueLogicalPacket(LogicalPacket *packet);

        UdpReliableConfig mConfig;
        UdpConnection *mUdpConnection;
        UdpClockStamp mLastTimeStampAcknowledged;
        UdpClockStamp mTrickleLastSend;
        UdpClockStamp mNextNeedTime;
        UdpClockStamp mWindowResetTime;
        int mChannelNumber;
        udp_int64 mReliableOutgoingId;
        udp_int64 mReliableOutgoingPendingId;
        int mReliableOutgoingBytes;
        int mLogicalBytesQueued;
        udp_uchar *mBigDataPtr;
        int mBigDataLen;
        int mBigDataTargetLen;
        int mAveragePingTime;
        int mMaxDataBytes;
        int mFragmentNextPos;
        PhysicalPacket *mPhysicalPackets;
        UdpLinkedList<LogicalPacket> mLogicalPacketList;

        int mCongestionWindowStart;
        int mCongestionWindowSize;
        int mCongestionSlowStartThreshhold;
        int mCongestionWindowMinimum;
        bool mMaxxedOutCurrentWindow;

        udp_int64 mReliableIncomingId;
        IncomingQueueEntry *mReliableIncoming;

        LogicalPacket *mCoalescePacket;
        udp_uchar *mCoalesceStartPtr;
        udp_uchar *mCoalesceEndPtr;
        int mCoalesceCount;
        int mMaxCoalesceAttemptBytes;

        udp_uchar *mBufferedAckPtr;

        int mStatDuplicatePacketsReceived;
        int mStatResentPacketsAccelerated;
        int mStatResentPacketsTimedOut;
};


        /////////////////////////////////////////////////////////////////////////
        // inline implementations
        /////////////////////////////////////////////////////////////////////////

        // UdpReliableChannel
inline void UdpReliableChannel::AckPacket(const udp_uchar *data, int dataLen)
{
    if (dataLen < 4)
    {
        mUdpConnection->CallbackCorruptPacket(data, dataLen, cUdpCorruptionReasonAckBad);
        return;
    }

    Ack(GetReliableOutgoingId((udp_ushort)UdpMisc::GetValue16(data + 2)));
}

inline int UdpReliableChannel::GetAveragePing() const
{
    return(mAveragePingTime);
}

inline int UdpReliableChannel::TotalPendingBytes() const
{
    return(mLogicalBytesQueued + mReliableOutgoingBytes);
}

inline void UdpReliableChannel::ClearBufferedAck()
{
    mBufferedAckPtr = nullptr;
}

inline udp_int64 UdpReliableChannel::GetReliableOutgoingId(int reliableStamp) const
{
        // since we can never have anywhere close to 65000 packets outstanding, we only need to
        // to send the low order word of the reliableId in the UdpPacketReliable and UdpPacketAck
        // packets, because we can reconstruct the full id from that, we just need to take
        // into account the wrap around issue.  We calculate it based of the high-word of the
        // next packet we are going to send.  If it ends up being larger then we know
        // we wrapped and can fix it up by simply subtracting 1 from the high-order word.
    udp_int64 reliableId = reliableStamp | (mReliableOutgoingId & (~(udp_int64)0xffff));
    if (reliableId > mReliableOutgoingId)
        reliableId -= 0x10000;
    return(reliableId);
}

inline udp_int64 UdpReliableChannel::GetReliableIncomingId(int reliableStamp) const
{
        // since we can never have anywhere close to 65000 packets outstanding, we only need to
        // to send the low order word of the reliableId in the UdpPacketReliable and UdpPacketAck
        // packets, because we can reconstruct the full id from that, we just need to take
        // into account the wrap around issue.  We basically prepend the last-known
        // high-order word.  If we end up significantly below the head of our chain, then we
        // know we need to pick the entry 0x10000 higher.  If we fall significantly above
        // our previous high-end, then we know we need to go the other way.
    udp_int64 reliableId = reliableStamp | (mReliableIncomingId & (~(udp_int64)0xffff));
    if (reliableId < mReliableIncomingId - UdpManager::cHardMaxOutstandingPackets)
        reliableId += 0x10000;
    if (reliableId > mReliableIncomingId + UdpManager::cHardMaxOutstandingPackets)
        reliableId -= 0x10000;
    return(reliableId);
}

}   // namespace

#endif
