#ifndef UDPLIBRARY_UDPCONNECTION_H
#define UDPLIBRARY_UDPCONNECTION_H

// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

namespace UdpLibrary
{

class UdpReliableChannel;

struct UdpConnectionStatistics
{
        /////////////////////////////////////////////////////////////////////////
        // these statistics are valid even if clock-sync is not used
        // these statistics are never reset and should not be as the negotiated
        // packetloss stats would get messed up if they were
        // as such, use UdpConnection::ConnectionAge to determine how long they have been accumulating
        /////////////////////////////////////////////////////////////////////////
    udp_int64 totalBytesSent;
    udp_int64 totalBytesReceived;
    udp_int64 totalPacketsSent;           // total packets we have sent
    udp_int64 totalPacketsReceived;       // total packets we have received
    udp_int64 crcRejectedPackets;         // total packets on our connection that have been rejected due to a crc error
    udp_int64 orderRejectedPackets;       // total packets on our connection that have been rejected due to an order error (only applicable for ordered channel)
    udp_int64 duplicatePacketsReceived;   // total reliable packets that we received where we had already received it before and threw it away
    udp_int64 resentPacketsAccelerated;   // number of times we have resent a packet due to receiving a later packet in the series
    udp_int64 resentPacketsTimedOut;      // number of times we have resent a packet due to the ack-timeout expiring
    udp_int64 applicationPacketsSent;
    udp_int64 applicationPacketsReceived;
    udp_int64 iterations;                 // number of times this connection has been given processing time
    udp_int64 corruptPacketErrors;        // number of misformed/corrupt packets

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // these statistics are only valid if clock-sync'ing is enabled (highly recommended) (will be valid on both client and server side)
        // these statistics are reset by PingStatReset and are negotiated periodically by the clock-sync stuff (Params::clockSyncDelay)
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int masterPingAge;                // only valid (and applicable) on client side
    int masterPingTime;
    int averagePingTime;
    int lowPingTime;
    int highPingTime;
    int lastPingTime;
    int reliableAveragePing;          // the average time (over last 3 acks) for a reliable packet to get acked (when packet is not lost)
    udp_int64 syncOurSent;            // total packets we have sent at time they reported their numbers
    udp_int64 syncOurReceived;        // total packets we have received at time they reported their numbers
    udp_int64 syncTheirSent;          // total packets they have sent
    udp_int64 syncTheirReceived;      // total packets they have received
    float percentSentSuccess;
    float percentReceivedSuccess;
};

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // The purpose of the UdpConnection is to manage a single logical connection
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UdpConnection : public UdpGuardedRefCount, public PriorityQueueMember, public HashTableMember1<UdpConnection>, public HashTableMember2<UdpConnection>
{
    public:
        enum Status { cStatusNegotiating, cStatusConnected, cStatusDisconnected, cStatusDisconnectPending, cStatusCount };

        enum DisconnectReason { cDisconnectReasonNone, cDisconnectReasonIcmpError, cDisconnectReasonTimeout
                              , cDisconnectReasonOtherSideTerminated, cDisconnectReasonManagerDeleted
                              , cDisconnectReasonConnectFail, cDisconnectReasonApplication
                              , cDisconnectReasonUnreachableConnection, cDisconnectReasonUnacknowledgedTimeout
                              , cDisconnectReasonNewConnectionAttempt, cDisconnectReasonConnectionRefused
                              , cDisconnectReasonMutualConnectError, cDisconnectReasonConnectingToSelf
                              , cDisconnectReasonReliableOverflow, cDisconnectReasonApplicationReleased
                              , cDisconnectReasonCorruptPacket
                              , cDisconnectReasonCount };
        
            // returns the current status of this connection
        Status GetStatus() const;

            // returns the reason that a connection was disconnected.  See the enum above for a list of all the reasons
        DisconnectReason GetDisconnectReason() const;
        char *GetDisconnectReasonText(char *buf, int bufLen) const;
        DisconnectReason GetOtherSideDisconnectReason() const;
        static const char *DisconnectReasonText(DisconnectReason reason);    // text-description of disconnect reason to aid in logging

            // sets the handler object for this connection.  If a handler object is specified, then the callback functions specified
            // in UdpManager::Params are ignored for this connection and the handler is used for the callback instead.
            // by default there is no handler.
        void SetHandler(UdpConnectionHandler *handler);
        UdpConnectionHandler *GetHandler() const;

            // set and get the pass-through data value.  Typically the application will set the pass through data
            // in the callback function for establishing a connection, then it will use the pass through data
            // in the callback function for routing packets.
        void SetPassThroughData(void *passThroughData);
        void *GetPassThroughData() const;

            // when called this connection is marked as terminated.  It is the responsibility of the application
            // to explicitly destroy connections that are no longer connected.  When this object is disconnected
            // it calls the UdpManager and has itself removed from the list of active connections, at which point
            // the only person having a pointer to this object is the application itself (which owns it)
            // (if the UdpManager is deleted before all UdpConnections are destroyed, the UdpManager loops through
            // all of the connections it has calling Disconnect on them such that they know that they no longer
            // have a udp manager that they can send data through)
            //
            // setting a flushTimeout tells the connection to stay alive for that amount of time trying to send any pending
            // reliable data before shutting down.  Once the application calls Disconnect even with a flushTimeout, the application
            // should not attempt to use the connection in any significant way (see docs and release notes for details)
            // note: the notifyApplication parameter was removed from this function and the functionality of the library
            // was changed such that ANYTIME the connection objects state changes to cStatusDisconnected, the OnTerminated callback
            // function gets called.
        void Disconnect(int flushTimeout = 0);

            // sends a logical packet on the specified channel, returns FALSE if packet could not be queued for sending (should never happen)
            // Internally, a packet that starts with a 0 byte is considered an internal control packet.  If a logical packet starts with a 0
            // byte, then there will an extra control byte of overhead in order to facilitate it.  It is recommended that if packet size
            // is critical, that you don't start the packet with a 0 byte. Typically an application will have a packet-type byte on the front 
            // of application packets; the application packet types should simply start at 1.
        bool Send(UdpChannel channel, const void *data, int dataLen);

            // same as the regular Send only it takes a LogicalPacket instead.  There are two huge advantages to having it take a
            // LogicalPacket.  First, we can send the same LogicalPacket to multiple locations and each connection will not necessarily
            // have to make its own copy of the data at the time it is put into the send queue (instead each connection just increments
            // the buffer ref-count).  Second, it allows the application to pre-generate very large packets (like file update packets potentially)
            // and hold onto them for the entire length of the application, then, whenever any player needs that chunk of data, it can send them
            // the already formatted LogicalPacket.
        bool Send(UdpChannel channel, const LogicalPacket *packet);

            // manually forces all channels to send-off any data they have queued up waiting for processing time to send
            // this mainly applies to reliable channels.  When you send a reliable packet, it actually only adds it to the reliable
            // queue until the connection is given processing time by the manager object.  This call forces it to attempt to
            // send that queued data immediately (subject to normal flow control restrictions).  This also flushes the multi-buffer
            // for the channel.  If you send reliable data and want to ensure that it goes out immediately after the send, this is the
            // best call to make.
        void FlushChannels();

            // manually forces buffered data to be sent immediately
        void FlushMultiBuffer();

            // returns the number of bytes sent/received in the last second to this connection (accurate to within cBinResolution(25) milliseconds)
            // these functions are not const as they expire the older bin data internally in order to calculate the number
        int OutgoingBytesLastSecond();
        int IncomingBytesLastSecond();

            // returns the total number of bytes outstanding in all reliable channels.  When this is zero, you know for sure
            // that all sent reliable data has arrived at destination and is confirmed.
        int TotalPendingBytes() const;

            // returns how long has elapsed since this connection received data (in milliseconds)
        int LastReceive() const;

            // returns how long has elapsed since this connection received data (in milliseconds), using useStamp as the current time (optimization)
        int LastReceive(UdpClockStamp useStamp) const;

            // returns how long has elapsed since this connection sent data (in milliseconds)
        int LastSend() const;

            // returns how long this connection has been in existence (in milliseconds)
        int ConnectionAge() const;

            // returns the UdpManager object that is managing this connection
            // will return nullptr if the connection has been disconnected for some reason (because disconnecting severes the link to UdpManager)
        UdpManager *GetUdpManager() const;

            // returns the 32-bit encryption-code that was negotiated as part of the connection-establishment process.
            // this is a randomly generated number that both the client and the server have in common.  It is exposed
            // via this interface primarily to allow user-supplied encrypt routines access to it.
            // this code is generated by the server side in response to a connect request.
        int GetEncryptCode() const;

            // this returns the connection-code.  This is very similar to the encrypt-code in that it is randomly
            // generated and both ends of the connection will report the same value.  The difference is that this
            // code's purpose is part of the internal protocol to ensure that old connections don't try to process
            // new connection request packets.  Unlike the encrypt-code, this value is generated by the client
            // and is part of the connect-request packet.  Nevertheless, since this number will be the same random
            // number on both ends of the connection, it too can be used as a potential encryption key for the user
            // supplied encrypt routines.  It's not quite as secure as the encrypt code since this value in theory
            // could be hacked to be something predictable on the client side.
        int GetConnectCode() const;

            // returns a sync-stamp that can be compared to other ServerSyncStamp's generated on other machines
            // in order to calculate the one-way travel time for a packet.  It can only accurate calculate
            // packet travel times under 32 seconds, would should be completely safe.  You must use the
            // UdpManager::SyncStampDeltaTime function in order to calculate the elapsed time between
            // the two stamps.
        udp_ushort ServerSyncStampShort() const;
        udp_uint ServerSyncStampLong() const;
        int ServerSyncStampShortElapsed(udp_ushort syncStamp) const;
        int ServerSyncStampLongElapsed(udp_uint syncStamp) const;

            // returns the IP address/port this connection is linked to
        UdpPlatformAddress GetDestinationIp() const;
        int GetDestinationPort() const;
        char *GetDestinationString(char *buf, int bufLen) const;

            // statistical functions
        void GetStats(UdpConnectionStatistics *cs);
        void PingStatReset();        // resets the ping-stat information, causing it to resync the clock etc (if in clock-sync mode).  Generally this is not done, it was added for backward compatibility


            // functions for manipulating the automatic no-data-disconnect stuff on a per-connection basis
        void SetNoDataTimeout(int noDataTimeout);        // 0=never timeout, otherwise in milliseconds (overrides UdpManager::Params::noDataTimeout setting, which is the default)
        int GetNoDataTimeout() const;

            // functions for manipulating the keep-alive packet sending on a per-connection basis
        void SetKeepAliveDelay(int keepAliveDelay);
        int GetKeepAliveDelay() const;

            // configures whether this connection is in silent-disconnect mode or not.  By default, the connection is not in silent
            // disconnect mode, which means that when this connection is terminated, it will send a final terminate-packet to the
            // other side telling them that we are disconnected, allowing them to quickly realize that the connection is now dead.
            // In some circumstances, it may be desireable to not do this, and this can be accomplished by calling this function
            // passing in 'true' to put it in silent mode.  This may be desireable in cases where you are disconnecting a cheater
            // and don't want them to have immediate notification that they did something bad.  Or, if you are attempting to test
            // timeout functionality on the other end and want to simulate a truly dead connection.  Normally, you will not want
            // to mess with this.  It was added to the API to support some internal functionality, see its use in the source-code
            // or release-notes for details.
        void SetSilentDisconnect(bool silent);


            // returns the current queue-status of the reliable channel specified.  Unreliable channels will always report zero.
        struct ChannelStatus
        {
            int totalPendingBytes;            // total bytes of data in channel that have yet to be acknowledged (includes queuedBytes plus physical-packet bytes that have yet to be acknowledged)
            int queuedPackets;                // number of logical packets in the queue
            int queuedBytes;                // number of bytes in the logical queue (the logical queue does NOT include pending physical packets)
            int incomingLargeTotal;            // total number of bytes in the currently incoming logical packet (only meaningful obviously if a fragmented file is in tranist)
            int incomingLargeSoFar;            // number of bytes received so far in the currently incoming logical packet
            int oldestUnacknowledgedAge;    // age of the oldest unacknowledged (but sent) packet (in milliseconds)
            int duplicatePacketsReceived;    // number of times we received a packet that we had already received
            int resentPacketsAccelerated;    // number of times we have resent a packet due to receiving a later packet in the series
            int resentPacketsTimedOut;        // number of times we have resent a packet due to the ack-timeout expiring
            int congestionSlowStartThreshhold;    // current threshhold for slow-start algorithm
            int congestionWindowSize;            // current sliding window size
            int ackAveragePing;                    // average time for a packet to be acknowledged (used in calculating optimal resend timeouts)
        };
        void GetChannelStatus(UdpChannel channel, ChannelStatus *channelStatus) const;

    protected:
        friend class UdpManager;
        friend class UdpReliableChannel;

            // note: if connectPacket is nullptr, that means this connection object is being created to establish
            // a new connection to the specified ip/port (ie. the connection starts out in cStatusNegotiating mode)
            // if connectPacket is non-nullptr, that menas this connection object is being created to handle an
            // incoming connect request and it will start out in cStatusConnected mode.
        UdpConnection(UdpManager *udpManager, UdpPlatformAddress destIp, int destPort, int timeout);    // starts connection-establishment protocol
        UdpConnection(UdpManager *udpManager, const UdpManager::PacketHistoryEntry *e);                // starts already connected, replying to connection request

            // gives this connection processing time (only given processing time by the manager object and then
            // only when the connection has scheduled itself to receive processing time)
        void GiveTime(bool fromManager);
        void ProcessRawPacket(const UdpManager::PacketHistoryEntry *e);
        void PortUnreachable();
        void FlagPortUnreachable();

            // these functions are called by the manager to forward these events to this connection
            // all events get sent to the UdpManager for potential event-queuing, then forwarded back
            // to the connection for actual deliver, since the connection object needs to hold a
            // guard such that the handler doesn't get deleted during event delivery
        void OnRoutePacket(const udp_uchar *data, int dataLen);
        void OnConnectComplete();
        void OnTerminated();
        void OnCrcReject(const udp_uchar *data, int dataLen);
        void OnPacketCorrupt(const udp_uchar *data, int dataLen, UdpCorruptionReason reason);

    protected:
        typedef int (UdpConnection::* IEncryptFunction)(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        typedef int (UdpConnection::* IDecryptFunction)(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        IDecryptFunction mDecryptFunction[cEncryptPasses];
        IEncryptFunction mEncryptFunction[cEncryptPasses];

        UdpPlatformAddress mIp;
        int mPort;
        int mSimulateOutgoingQueueBytes;            // used by UdpManager to track how many bytes are in it's simulation queue headed to each destination

    private:
        ~UdpConnection();
        void Init(UdpManager *udpManager, UdpPlatformAddress destIp, int destPort);

            // note: BufferedSend is capable of optionally taking two chunks of data at once, which are then concatenated together as if they were one chunk of data
            // into the multi-buffer.  Providing this facility prevents the UdpReliableChannel object from having to make a copy of all the data it sends
            // in order to stick a realiable header on it.
            // we don't bother extending this down to the PhysicalSend (in case the BufferedSend does a pass through due to size) because the encryption code
            // is incapable of sourcing from two different chunks and outputting to one chunk.  It's not possible to change that either, since the encyption
            // takes place 32 bits at a time and you could end up straddling boundaries between chunks.
        void RawSend(const udp_uchar *data, int dataLen);                // nothing happens to the data here, it is given to the udpmanager and sent out the port
        void PhysicalSend(const udp_uchar *data, int dataLen, bool appendAllowed);        // sends a physical packet (encrypts and adds crc bytes)
        udp_uchar *BufferedSend(const udp_uchar *data, int dataLen, const udp_uchar *data2, int dataLen2, bool appendAllowed);        // buffers logical packets waiting til we have more data (makes multi-packets)
        bool InternalSend(UdpChannel channel, const udp_uchar *data, int dataLen, const udp_uchar *data2 = nullptr, int dataLen2 = 0);

        void InternalGiveTime();
        void InternalDisconnect(int flushTimeout, DisconnectReason reason);
        void ProcessCookedPacket(const udp_uchar *data, int dataLen);
        void DecryptIt(const udp_uchar *data, int dataLen);
        void ScheduleTimeNow();
        void ExpireSendBin();
        void ExpireReceiveBin();
        void SendTerminatePacket(int connectCode, DisconnectReason reason);
        void CallbackRoutePacket(const udp_uchar *data, int dataLen);
        void CallbackCorruptPacket(const udp_uchar *data, int dataLen, UdpCorruptionReason reason);
        bool IsNonEncryptPacket(const udp_uchar *data) const;

            // these encrypt-method functions return the length of the encrypted/decrypted data
            // new methods of encryption/compression can be easily added by simply creating the
            // functions for them and changing the SetupEncryptModel function as appropriate
            // since raw packets are encrypted in the first place and have a limited size
            // the decrypted data will never be larger than a maxRawPacketSize.  Both of encrypt
            // and decrypt are guaranteed to have enough room in dest buffers to hold the results.
            // Encryption function is allowed to expand the data at most the number of bytes
            // it reserves for this purpose in the SetupEncryptModel function.
        int EncryptNone(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int DecryptNone(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int EncryptXor(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int DecryptXor(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int EncryptXorBuffer(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int DecryptXorBuffer(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int EncryptUserSupplied(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int DecryptUserSupplied(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int EncryptUserSupplied2(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int DecryptUserSupplied2(udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        void SetupEncryptModel();


        UdpLinkedListMember<UdpConnection> mConnectionLink;
        UdpLinkedListMember<UdpConnection> mDisconnectPendingLink;

        Status mStatus;
        void *mPassThroughData;
        UdpManager *mUdpManager;
        int mConnectCode;
        UdpConnectionStatistics mConnectionStats;
        UdpClockStamp mConnectionCreateTime;
        int mConnectAttemptTimeout;
        int mNoDataTimeout;
        DisconnectReason mDisconnectReason;
        DisconnectReason mOtherSideDisconnectReason;
        bool mFlaggedPortUnreachable;
        bool mSilentDisconnect;

        UdpReliableChannel *mChannel[cReliableChannelCount];

        struct Configuration
        {
            int encryptCode;
            int crcBytes;
            EncryptMethod encryptMethod[cEncryptPasses];
            int maxRawPacketSize;        // negotiated maxRawPacketSize (ie. smaller of what two sides are set to)
        };

        Configuration mConnectionConfig;
        int mOtherSideProtocolVersion;

        UdpClockStamp mLastClockSyncTime;
        UdpClockStamp mDataHoldTime;
        UdpClockStamp mLastSendTime;
        UdpClockStamp mLastReceiveTime;
        UdpClockStamp mLastPortAliveTime;
        udp_uchar *mMultiBufferData;
        udp_uchar *mMultiBufferPtr;

        int mOrderedCountOutgoing;
        int mOrderedCountOutgoing2;
        udp_ushort mOrderedStampLast;
        udp_ushort mOrderedStampLast2;

        udp_uchar *mEncryptXorBuffer;
        int mEncryptExpansionBytes;

        udp_uint mSyncTimeDelta;
        int mSyncStatTotal;
        int mSyncStatCount;
        int mSyncStatLow;
        int mSyncStatHigh;
        int mSyncStatLast;
        int mSyncStatMasterRoundTime;
        UdpClockStamp mSyncStatMasterFixupTime;

        bool mGettingTime;
        UdpConnectionHandler *mHandler;
        
        int mKeepAliveDelay;

        UdpClockStamp mIcmpErrorRetryStartStamp;
        UdpClockStamp mPortRemapRequestStartStamp;

        UdpClockStamp mDisconnectFlushStamp;
        int mDisconnectFlushTimeout;
        mutable UdpPlatformGuardObject mGuard;
        mutable UdpPlatformGuardObject mHandlerGuard;


            // data rate management functions
        enum { cBinResolution = 25, cBinCount = 1000 / cBinResolution };
        udp_int64 mLastSendBin;
        udp_int64 mLastReceiveBin;
        int mOutgoingBytesLastSecond;
        int mIncomingBytesLastSecond;
        int mSendBin[cBinCount];
        int mReceiveBin[cBinCount];


            //////////////////////////////////////////////////////////////////////////////////////////////////////
            // The following structs represent what the internal packets look like.  In practice, most of these
            // structs are never used and exist only for documentation clarity.  Internally packets are
            // manually assembled such that struct packing and byte-ordering issues won't be an issue.
            //////////////////////////////////////////////////////////////////////////////////////////////////////
        struct UdpPacketConnect
        {
            udp_uchar zeroByte;
            udp_uchar packetType;
            int protocolVersion;
            int connectCode;
            int maxRawPacketSize;
        };

        struct UdpPacketConfirm
        {
            udp_uchar zeroByte;
            udp_uchar packetType;
            int connectCode;
            Configuration config;
            int maxRawPacketSize;
        };

        struct UdpPacketTerminate
        {
            udp_uchar zeroByte;
            udp_uchar packetType;
            int connectCode;
        };

        struct UdpPacketKeepAlive
        {
            udp_uchar zeroByte;
            udp_uchar packetType;
        };

        struct UdpPacketGroup
        {
                // this format is prepped by the GroupLogicalPacket object, which reports itself to the UdpConnection object as an internal packet
                // type such that it doesn't get treated as an application-packet even though the application is the one sending it
            udp_uchar zeroByte;
            udp_uchar packetType;
                // variableValue/data, repeated...
        };

        struct UdpPacketClockSync
        {
            udp_uchar zeroByte;
            udp_uchar packetType;
            udp_ushort timeStamp;
            int masterPingTime;
            int averagePingTime;
            int lowPingTime;
            int highPingTime;
            int lastPingTime;
            udp_int64 ourSent;    
            udp_int64 ourReceived;
        };

        struct UdpPacketClockReflect
        {
            udp_uchar zeroByte;
            udp_uchar packetType;
            udp_ushort timeStamp;
            udp_uint serverSyncStampLong;
            udp_int64 yourSent;
            udp_int64 yourReceived;
            udp_int64 ourSent;
            udp_int64 ourReceived;
        };

        struct UdpPacketReliable
        {
            udp_uchar zeroByte;
            udp_uchar packetType;
            udp_ushort reliableStamp;
        };

        struct UdpPacketReliableFragmentStart
        {
            UdpPacketReliable reliable;
            int length;
        };

        struct UdpPacketAck
        {
            udp_uchar zeroByte;
            udp_uchar packetType;
            udp_ushort reliableStamp;
        };

        struct UdpPacketOrdered
        {
            udp_uchar zeroByte;
            udp_uchar packetType;
            udp_ushort orderStamp;
        };


        enum { cUdpPacketReliableSize = 4 };
        enum { cUdpPacketOrderedSize = 4 };

    protected:
        friend class GroupLogicalPacket;    // so it can see cUdpPacketGroup enum

            // note: cUdpPacketReliable, cUdpPacketFragment both indicate a reliable-packet header.  They are marked
            // differently such that we can support large packets without any additional header overhead, a fragment marked packet means
            // that the packet is part of a larger packet being assembled.  The first fragment has an additional 4 bytes on the header specifying
            // the length to follow.  The order of those entries is important
        enum UdpPacketType { cUdpPacketZeroEscape, cUdpPacketConnect, cUdpPacketConfirm, cUdpPacketMulti, cUdpPacketBig
                        , cUdpPacketTerminate, cUdpPacketKeepAlive
                        , cUdpPacketClockSync, cUdpPacketClockReflect
                        , cUdpPacketReliable1, cUdpPacketReliable2, cUdpPacketReliable3, cUdpPacketReliable4
                        , cUdpPacketFragment1, cUdpPacketFragment2, cUdpPacketFragment3, cUdpPacketFragment4
                        , cUdpPacketAck1, cUdpPacketAck2, cUdpPacketAck3, cUdpPacketAck4
                        , cUdpPacketAckAll1, cUdpPacketAckAll2, cUdpPacketAckAll3, cUdpPacketAckAll4
                        , cUdpPacketGroup, cUdpPacketOrdered, cUdpPacketOrdered2, cUdpPacketPortAlive
                        , cUdpPacketUnreachableConnection, cUdpPacketRequestRemap };

};


        /////////////////////////////////////////////////////////////////////////
        // inline implementations
        /////////////////////////////////////////////////////////////////////////
        // UdpConnection
inline void UdpConnection::ScheduleTimeNow()
{
        // if we are current in our GiveTime function getting time, then there is no need to reprioritize to 0 when we send a raw packet, since
        // the last thing we do in out GiveTime is do a scheduling calculation based on the last time a packet was sent.  This little check
        // prevents us from reprioritizing to 0, only to shortly thereafter be reprioritized to where we actually belong.
    if (!mGettingTime)
    {
        if (mUdpManager != nullptr)
            mUdpManager->SetPriority(this, 0);
    }
}

inline void UdpConnection::SetHandler(UdpConnectionHandler *handler)
{
    UdpGuard guard(&mHandlerGuard);
    mHandler = handler;
}

inline UdpConnectionHandler *UdpConnection::GetHandler() const
{
    UdpGuard guard(&mHandlerGuard);
    return(mHandler);
}

inline bool UdpConnection::IsNonEncryptPacket(const udp_uchar *data) const
{
    if (data[0] == 0)
    {
        if (data[1] == cUdpPacketConnect || data[1] == cUdpPacketConfirm || data[1] == cUdpPacketUnreachableConnection || data[1] == cUdpPacketRequestRemap)
            return(true);
    }
    return(false);
}

inline int UdpConnection::GetEncryptCode() const
{
    UdpGuard guard(&mGuard);
    return(mConnectionConfig.encryptCode);
}

inline int UdpConnection::GetConnectCode() const
{
    UdpGuard guard(&mGuard);
    return(mConnectCode);
}

inline int UdpConnection::LastReceive(UdpClockStamp useStamp) const
{
    UdpGuard guard(&mGuard);
    return(UdpMisc::ClockDiff(mLastReceiveTime, useStamp));
}

inline int UdpConnection::LastReceive() const
{
    UdpGuard guard(&mGuard);
    if (mUdpManager == nullptr)
        return(0);
    return(mUdpManager->CachedClockElapsed(mLastReceiveTime));
}

inline int UdpConnection::ConnectionAge() const
{
    UdpGuard guard(&mGuard);
    if (mUdpManager == nullptr)
        return(0);
    return(mUdpManager->CachedClockElapsed(mConnectionCreateTime));
}

inline int UdpConnection::LastSend() const
{
    UdpGuard guard(&mGuard);
    if (mUdpManager == nullptr)
        return(0);
    return(mUdpManager->CachedClockElapsed(mLastSendTime));
}

inline udp_ushort UdpConnection::ServerSyncStampShort() const
{
    UdpGuard guard(&mGuard);
    if (mUdpManager == nullptr)
        return(0);
    return((udp_ushort)(mUdpManager->LocalSyncStampShort() + (mSyncTimeDelta & 0xffff)));
}

inline udp_uint UdpConnection::ServerSyncStampLong() const
{
    UdpGuard guard(&mGuard);
    if (mUdpManager == nullptr)
        return(0);
    return(mUdpManager->LocalSyncStampLong() + mSyncTimeDelta);
}

inline int UdpConnection::ServerSyncStampShortElapsed(udp_ushort syncStamp) const
{
    return(UdpMisc::SyncStampShortDeltaTime(syncStamp, ServerSyncStampShort()));
}

inline int UdpConnection::ServerSyncStampLongElapsed(udp_uint syncStamp) const
{
    return(UdpMisc::SyncStampLongDeltaTime(syncStamp, ServerSyncStampLong()));
}

inline UdpManager *UdpConnection::GetUdpManager() const
{
    UdpGuard guard(&mGuard);
    return(mUdpManager);
}

inline UdpConnection::Status UdpConnection::GetStatus() const
{
    UdpGuard guard(&mGuard);
    return(mStatus);
}

inline UdpConnection::DisconnectReason UdpConnection::GetDisconnectReason() const
{
    UdpGuard guard(&mGuard);
    return(mDisconnectReason);
}

inline UdpConnection::DisconnectReason UdpConnection::GetOtherSideDisconnectReason() const
{
    UdpGuard guard(&mGuard);
    return(mOtherSideDisconnectReason);
}

inline int UdpConnection::OutgoingBytesLastSecond()
{
    UdpGuard guard(&mGuard);
    if (mUdpManager == nullptr)
        return(0);

    ExpireSendBin();
    return(mOutgoingBytesLastSecond);
}

inline int UdpConnection::IncomingBytesLastSecond()
{
    UdpGuard guard(&mGuard);
    if (mUdpManager == nullptr)
        return(0);

    ExpireReceiveBin();
    return(mIncomingBytesLastSecond);
}

inline void UdpConnection::SetPassThroughData(void *passThroughData)
{
    UdpGuard guard(&mGuard);
    mPassThroughData = passThroughData;
}

inline void *UdpConnection::GetPassThroughData() const
{
    UdpGuard guard(&mGuard);
    return(mPassThroughData);
}

inline UdpPlatformAddress UdpConnection::GetDestinationIp() const
{
    UdpGuard guard(&mGuard);
    return(mIp);
}

inline int UdpConnection::GetDestinationPort() const
{
    UdpGuard guard(&mGuard);
    return(mPort);
}

inline void UdpConnection::SetNoDataTimeout(int noDataTimeout)
{
    UdpGuard guard(&mGuard);
    mNoDataTimeout = noDataTimeout;
}

inline int UdpConnection::GetNoDataTimeout() const
{
    UdpGuard guard(&mGuard);
    return(mNoDataTimeout);
}

inline void UdpConnection::Disconnect(int flushTimeout)
{
    UdpRef ref(this);     // in case application releases us during the disconnect, we need to hold this reference so our guard object can be destroyed first
    UdpGuard guard(&mGuard);
    InternalDisconnect(flushTimeout, cDisconnectReasonApplication);
}

inline void UdpConnection::SetKeepAliveDelay(int keepAliveDelay)
{
    UdpGuard guard(&mGuard);
    mKeepAliveDelay = keepAliveDelay;
}

inline int UdpConnection::GetKeepAliveDelay() const
{
    UdpGuard guard(&mGuard);
    return(mKeepAliveDelay);
}

}   // namespace

#endif
